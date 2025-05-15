/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * GX2 Monitoring
 *
 * Currently, `GX2SwapScanBuffers()` is what drives the whole plugin. We count frames, we
 * start/stop GPU performance, we ask the overlay to "render", all from this hook. The
 * mutual dependency, between this module and the overlay module, is ugly, but
 * unavoidable.
 *
 * We also hook into `GX2Init()` and `GX2Shutdown()`, to ensure we don't call GX2Perf
 * functions while GX2 is in an invalid state.
 */


#include <cstdio>
#include <cstdlib>              // malloc(), free()
#include <optional>
#include <ranges>
// #include <source_location>
#include <variant>
#include <vector>

#include <coreinit/debug.h> // DEBUG
#include <coreinit/memexpheap.h>
#include <coreinit/memunitheap.h>
#include <gx2/event.h>          // GX2DrawDone()
#include <gx2/surface.h>
#include <gx2/swap.h>
#include <wups.h>

#include <memory/mappedmemory.h>

#include <wupsxx/logger.hpp>

// WUT lacks <gx2/perf.h>
#include "gx2_perf.h"

#include "gx2_mon.hpp"

#include "cfg.hpp"
#include "overlay.hpp"
#include "utils.hpp"


using std::uint32_t;

namespace logger = wups::logger;


#define TRACE                                           \
    do {                                                \
        auto here = std::source_location::current();    \
        logger::printf("%s:%u: %s\n",                   \
                       here.file_name(), here.line(),   \
                       here.function_name());           \
    }                                                   \
    while (false)


/*
 * Profiling notes:
 *
 * `GX2PerfInit()` needs an allocator parameter that's used to allocate/deallocate memory
 * during the profiling. To minimize changing memory allocations within the game, we
 * allocate memory from libmappedmemory.
 */


// Define this to use a Unit Heap instead of an Expanded Heap.
// Unit Heap is faster.
#define USE_UNIT_HEAP

// Define this to add some error reporting during allocation.
// #define DEBUG_ALLOC_FUNCS

namespace heap {

    void* raw_memory = nullptr;
    const uint32_t raw_size = 4096;
    const uint32_t alignment = 32;

#ifdef USE_UNIT_HEAP
    const uint32_t block_size = 32; // Large enough for all GX2Perf allocations.
#endif

    MEMHeapHandle handle = nullptr;

#ifdef DEBUG_ALLOC_FUNCS
    MEMAllocatorAllocFn real_alloc_func = nullptr;

    void*
    my_alloc_func(MEMAllocator* a, uint32_t size)
    {
#ifdef USE_UNIT_HEAP
        if (size > block_size) {
            logger::printf("ERROR: trying to allocate %u, but can only allocate up to %u.\n",
                           size,
                           block_size);
            return nullptr;
        }
#endif

        void* result = real_alloc_func(a, size);
        logger::printf("allocating %u bytes: %p\n", size, result);
        return result;
    }


    MEMAllocatorFreeFn real_free_func = nullptr;

    void
    my_free_func(MEMAllocator* a, void* ptr)
    {
        logger::printf("freeing %p\n", ptr);
        return real_free_func(a, ptr);
    }


    MEMAllocatorFunctions my_funcs {
        .alloc = my_alloc_func,
        .free = my_free_func
    };
#endif

    MEMAllocator
    make_allocator()
    {
        if (!raw_memory)
            OSFatal("ERROR!!!!!! Papaya HUD could not allocate memory\n");

#ifdef DEBUG_ALLOC_FUNCS
#ifdef USE_UNIT_HEAP
        uint32_t total_free = block_size * MEMCountFreeBlockForUnitHeap(handle);
#else
        uint32_t total_free = MEMGetTotalFreeSizeForExpHeap(handle);
#endif
        logger::printf("Heap total free size: %u\n", total_free);
#endif

        MEMAllocator result;
#ifdef USE_UNIT_HEAP
        MEMInitAllocatorForUnitHeap(&result, handle);
#else
        MEMInitAllocatorForExpHeap(&result, handle, alignment);
#endif

#ifdef DEBUG_ALLOC_FUNCS
        real_alloc_func = result.funcs->alloc;
        real_free_func = result.funcs->free;
        result.funcs = &my_funcs;
#endif

        return result;
    }


    void
    initialize()
    {
        if (raw_memory)
            return;

        raw_memory = MEMAllocFromMappedMemoryForGX2Ex(raw_size, alignment);
        if (raw_memory) {
#ifdef USE_UNIT_HEAP
            handle = MEMCreateUnitHeapEx(raw_memory, raw_size, block_size, alignment, 0);
#else
            handle = MEMCreateExpHeapEx(lmm_ptr, lmm_size, 0);
#endif
            if (!handle) {
                MEMFreeToMappedMemory(raw_memory);
                raw_memory = nullptr;
            }
        }
    }


    void
    finalize()
    {
        if (handle) {
#ifdef USE_UNIT_HEAP
            MEMDestroyUnitHeap(handle);
#else
            MEMDestroyExpHeap(handle);
#endif
            handle = nullptr;
        }
        if (raw_memory) {
            MEMFreeToMappedMemory(raw_memory);
            raw_memory = nullptr;
        }
    }

} // namespace heap


template<std::ranges::forward_range R>
auto
average(R&& seq)
{
    using T = std::ranges::range_value_t<R>;
    T sum = T{0};
    unsigned num = 0;
    for (const auto& x : seq) {
        sum += x;
        ++num;
    }
    return sum / num;
}


// TODO: this namespace belongs to a separate module

namespace gx2 {


    using metric_or_stat = std::variant<GX2PerfMetric, GX2StatId>;


    using metric_result = std::variant<std::uint64_t, float>;


    metric_result
    convert(const GX2MetricResult& res, GX2PerfMetric metric)
    {
        auto type = GX2GetPerfMetricType(metric);
        if (type == GX2_PERF_METRIC_TYPE_U64)
            return res.u64Result;
        else
            return res.f32Result;
    }


    // RAII wrapper for GX2PerfData
    struct perf_data {

        GX2PerfData data;


        perf_data(unsigned max_tags, MEMAllocator& allocator)
        {
            // logger::printf("checking out allocator\n");
            // logger::printf(" .funcs=%p\n", allocator.funcs);
            // logger::printf(" .funcs->alloc=%p\n", allocator.funcs->alloc);
            // logger::printf(" .funcs->free=%p\n", allocator.funcs->free);
            // logger::printf(" .heap=%p\n", allocator.heap);
            // logger::printf(" .arg1=0x%08x\n", allocator.arg1);
            // logger::printf(" .arg2=0x%08x\n", allocator.arg2);

            GX2PerfInit(&data, max_tags, &allocator);

            // logger::printf("GX2PerfInit() returned\n");
        }


        ~perf_data()
        {
            GX2PerfFree(&data);
        }


        // Delete both copy and move constructors
        perf_data(const perf_data&) = delete;


        // collection method

        void
        set_collection_method(GX2PerfCollectionMethod method)
        {
            GX2PerfSetCollectionMethod(&data, method);
        }


        GX2PerfCollectionMethod
        get_collection_method()
            const
        {
            return GX2PerfGetCollectionMethod(&data);
        }


        // metrics

        bool
        enable_metric(GX2PerfMetric metric)
        {
            return GX2PerfMetricEnable(&data, GX2_PERF_TYPE_GPU_METRIC, metric);
        }


        bool
        enable_metric(GX2StatId stat)
        {
            return GX2PerfMetricEnable(&data, GX2_PERF_TYPE_GPU_STAT, stat);
        }


        std::optional<metric_or_stat>
        get_metric(uint32_t index)
        {
            GX2PerfType type;
            static_assert(sizeof type == 4);
            uint32_t id;
            if (!GX2PerfMetricGetEnabled(&data, index, &type, &id))
                return {};
            switch (type) {
            case GX2_PERF_TYPE_GPU_METRIC:
                return static_cast<GX2PerfMetric>(id);
            case GX2_PERF_TYPE_GPU_STAT:
                return static_cast<GX2StatId>(id);
            case GX2_PERF_TYPE_MEM_STAT: // TODO: figure out how to handle this
            default:
                return {};
            }
        }


        void
        clear_metrics()
        {
            GX2PerfMetricsClear(&data);
        }


        // tags

        void
        set_tag(GX2PerfTag tag, bool enable)
        {
            GX2PerfTagEnable(&data, tag, enable);
        }


        void
        enable_all_tags()
        {
            GX2PerfTagEnableAll(&data);
        }


        bool
        is_tag_enabled(GX2PerfTag tag)
            const
        {
            return GX2PerfTagIsEnabled(&data, tag);
        }


        // start/end frame

        void
        frame_start()
        {
            GX2PerfFrameStart(&data);
        }


        void
        frame_finish()
        {
            GX2PerfFrameEnd(&data);
        }


        // start/end pass

        unsigned
        get_num_passes()
            const
        {
            return GX2PerfGetNumPasses(&data);
        }


        void
        pass_start()
        {
            GX2PerfPassStart(&data);
        }


        void
        pass_finish()
        {
            GX2PerfPassEnd(&data);
        }


        // start/end tag

        void
        tag_start(GX2PerfTag tag)
        {
            GX2PerfTagStart(&data, tag);
        }

        void
        tag_finish(GX2PerfTag tag)
        {
            GX2PerfTagEnd(&data, tag);
        }


        // results

        std::optional<metric_result>
        get_frame_result(GX2PerfMetric metric)
            const
        {
            GX2MetricResult result;
            if (!GX2PerfGetResultByFrame(&data,
                                         GX2_PERF_TYPE_GPU_METRIC, metric,
                                         &result))
                return {};
            return convert(result, metric);
        }

        // Missing: overload get_frame_result() for GX2StatId


        std::optional<metric_result>
        get_tag_result(GX2PerfMetric metric,
                       unsigned tag,
                       unsigned number)
            const
        {
            GX2MetricResult result;

            if (!GX2PerfGetResultByTagId(&data,
                                         GX2_PERF_TYPE_GPU_METRIC, metric,
                                         tag, number,
                                         &result))
                return {};
            return convert(result, metric);
        }

        // Missing: overload get_tag_result() for GX2StatId


        // Missing: wrapper for GX2PerfGetResultByTagSequence()


        // printing

        void
        print_frame_results()
            const
        {
            GX2PerfPrintFrameResults(&data);
        }


        // Missing: wrapper for GX2PerfPrintTagResults()


        // pass coherence

        void
        set_pass_coherence(bool enable)
        {
            GX2PerfSetPassCoherEnable(&data, enable);
        }


        bool
        get_pass_coherence()
            const
        {
            return GX2PerfGetPassCoherEnable(&data);
        }

    };

} // namespace gx2


namespace gx2_mon {

    namespace perf {

        struct profiler {

            unsigned pass;
            unsigned num_passes;
            bool frame_open;
            bool pass_open;
            bool started;
            MEMAllocator allocator;
            gx2::perf_data data;

            bool gpu_busy_enabled;
            std::vector<float> gpu_busy_vec;


            profiler() :
                pass{0},
                num_passes{0},
                frame_open{false},
                pass_open{false},
                started{false},
                allocator{heap::make_allocator()},
                data{1, allocator},
                gpu_busy_enabled{false}
            {
                // TRACE;

                data.set_collection_method(GX2_PERF_COLLECT_TAGS_ACCUMULATE);
                data.set_tag(0, true);
            }


            ~profiler()
            {
                // TRACE;
                // logger::printf("    frame_open = %s\n", frame_open ? "true" : "false");
                // logger::printf("    started = %s\n", started ? "true" : "false");
                // logger::printf("    pass = %u\n", pass);
                // logger::printf("    num_passes = %u\n", num_passes);
            }


            void
            start_frame()
            {
                started = true;

                if (pass == 0) {
                    // if on frame start, set up all metrics
                    data.clear_metrics();
                    gpu_busy_enabled = data.enable_metric(GX2_PERF_F32_GPU_BUSY);
                    if (!gpu_busy_enabled)
                        logger::printf("no slot available for GPU_BUSY\n");
                    num_passes = data.get_num_passes();
                    data.frame_start();
                    frame_open = true;
                }

                data.pass_start();
                data.tag_start(0);
                pass_open = true;
            }


            void
            finish_frame()
            {
                if (!started)
                    return;

                if (!frame_open) {
                    logger::printf("ERROR: frame not open\n");
                    return;
                }

                if (!pass_open) {
                    logger::printf("ERROR: pass not open\n");
                    return;
                }

                data.tag_finish(0);
                data.pass_finish();
                pass_open = false;
                // GX2DrawDone();

                // if on last frame
                if (++pass >= num_passes) {
                    data.frame_finish();
                    GX2DrawDone();
                    if (gpu_busy_enabled) {
                        auto gpu_busy_res = data.get_frame_result(GX2_PERF_F32_GPU_BUSY);
                        if (gpu_busy_res) {
                            float sample = std::get<float>(*gpu_busy_res);
                            if (gpu_busy_vec.size() < 1000)
                                gpu_busy_vec.push_back(sample);
                            else
                                logger::printf("gpu_busy_vec is growing too much! %u\n",
                                               static_cast<unsigned>(gpu_busy_vec.size()));
                        } else {
                            static unsigned error_counter = 0;
                            ++error_counter;
                            if (error_counter < 100 || error_counter % 1024 == 0)
                                logger::printf("failed to get GPU_BUSY result (%u)\n",
                                               error_counter);
                        }
                    }
                    // data.print_frame_results();
                    pass = 0;
                    frame_open = false;
                }
            }

        };


        std::optional<profiler> prof;


        void
        initialize()
        {
            if (prof)
                return;

            // initialize_lmm_heap();

            // TRACE;
            prof.emplace();
        }


        void
        finalize()
        {
            if (!prof)
                return;
            // TRACE;
            prof.reset();

            // finalize_lmm_heap();
        }


        void
        on_frame_start()
        {
            if (prof)
                prof->start_frame();
        }


        void
        on_frame_finish()
        {
            if (prof)
                prof->finish_frame();
        }


        void
        get_report(out_span& out,
                   float /*dt*/)
        {
            if (!prof)
                return;

            float avg_gpu_busy = average(prof->gpu_busy_vec);
            unsigned n_samples = prof->gpu_busy_vec.size();
            prof->gpu_busy_vec.clear();

            if (n_samples == 0) {
                out.append("GPU: ?");
                return;
            }

            if (cfg::gpu_busy_percent.value)
                out.printf("GPU: %2.1f%%", avg_gpu_busy);
            else
                out.printf("GPU: %s", utils::percent_to_bar(avg_gpu_busy));
        }

    } // namespace perf


    namespace fps {

        unsigned counter = 0;


        void
        initialize()
        {
            counter = 0;
        }


        void
        finalize()
        {}


        void
        on_frame_start()
        {}


        void
        on_frame_finish()
        {
            ++counter;
        }


        void
        get_report(out_span& out,
                   float dt)
        {
            float fps = counter / dt;
            counter = 0;
            out.printf("%02.1f fps", fps);
        }

    } // namespace fps


    namespace resolution {

        struct uvec2 {
            unsigned x, y;

            constexpr
            bool operator ==(const uvec2& other)
                const noexcept = default;
        };

        std::optional<uvec2> tv;
        std::optional<uvec2> drc;


        void
        initialize()
        {
            tv.reset();
            drc.reset();
        }


        void
        finalize()
        {}


        void
        on_frame_start()
        {
            tv.reset();
            drc.reset();
        }


        void
        on_frame_finish()
        {}


        void
        get_report(out_span& out,
                   float)
        {
            if (tv && drc) {
                if (*tv == *drc) // matching resolutions
                    out.printf("%ux%u",
                               tv->x, tv->y);
                else // different resolutions
                    out.printf("%ux%u / %ux%u",
                               tv->x, tv->y,
                               drc->x, drc->y);
            } else {
                // only one set
                if (tv)
                    out.printf("%ux%u", tv->x, tv->y);
                if (drc)
                    out.printf("%ux%u", drc->x, drc->y);
            }
        }

    } // namespace resolution


    void
    initialize()
    {
        // TRACE;

        // FIFA 13 will call GX2Init() after closing the Home Menu, AFTER it comes into
        // the foreground. So we avoid doing any initialization until our GX2Init() hook
        // is called.
        if (!overlay::gx2_init)
            return;

        if (cfg::gpu_busy.value)
            perf::initialize();

        if (cfg::gpu_fps.value)
            fps::initialize();

        if (cfg::gpu_resolution.value)
            resolution::initialize();
    }


    void
    finalize()
    {
        // TRACE;

        perf::finalize();
        fps::finalize();
    }


    void
    reset()
    {
        // TRACE;

        fps::finalize();
        if (cfg::gpu_fps.value)
            fps::initialize();

        perf::finalize();
        if (cfg::gpu_busy.value)
            perf::initialize();

    }


    void
    on_application_start()
    {
        heap::initialize();
    }


    void
    on_application_ends()
    {
        heap::finalize();
    }


    void
    on_frame_start()
    {
        if (cfg::gpu_busy.value)
            perf::on_frame_start();

        if (cfg::gpu_fps.value)
            fps::on_frame_start();

        if (cfg::gpu_resolution.value)
            resolution::on_frame_start();
    }


    void
    on_frame_finish()
    {
        if (cfg::gpu_busy.value)
            perf::on_frame_finish();

        if (cfg::gpu_fps.value)
            fps::on_frame_finish();

        if (cfg::gpu_resolution.value)
            resolution::on_frame_finish();
    }


    DECL_FUNCTION(void, GX2SwapScanBuffers,
                  void)
    {
        overlay::process_toggle_request_from_gx2();

        // skip all work if the plugin is disabled
        if (!cfg::enabled.value)
            return real_GX2SwapScanBuffers();

        on_frame_finish();

        overlay::render();

        real_GX2SwapScanBuffers();

        on_frame_start();
    }

    WUPS_MUST_REPLACE(GX2SwapScanBuffers,
                      WUPS_LOADER_LIBRARY_GX2,
                      GX2SwapScanBuffers);


    DECL_FUNCTION(void, GX2Init,
                  uint32_t* attr)
    {
        // logger::printf("GX2Init() was called on core %u\n", OSGetCoreId());
        real_GX2Init(attr);
        overlay::gx2_init = true;

        if (cfg::enabled.value)
            overlay::create_or_reset();
    }

    WUPS_MUST_REPLACE(GX2Init,
                      WUPS_LOADER_LIBRARY_GX2,
                      GX2Init);


    DECL_FUNCTION(void, GX2Shutdown,
                  void)
    {
        // logger::printf("GX2Shutdown() was called\n");
        overlay::destroy();
        overlay::gx2_init = false;
        real_GX2Shutdown();
    }

    WUPS_MUST_REPLACE(GX2Shutdown,
                      WUPS_LOADER_LIBRARY_GX2,
                      GX2Shutdown);


    DECL_FUNCTION(void, GX2ResetGPU,
                  uint32_t arg)
    {
        // logger::printf("GX2ResetGPU() was called\n");
        overlay::destroy();
        real_GX2ResetGPU(arg);
        if (cfg::enabled.value)
            overlay::create_or_reset();
    }

    WUPS_MUST_REPLACE(GX2ResetGPU,
                      WUPS_LOADER_LIBRARY_GX2,
                      GX2ResetGPU);


    DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer,
                  const GX2ColorBuffer* buffer,
                  GX2ScanTarget scanTarget)
    {
        // peek inside the color buffer
        if (cfg::enabled.value && cfg::gpu_resolution.value) {
            using resolution::uvec2;
            switch (scanTarget) {
                case GX2_SCAN_TARGET_TV:
                    resolution::tv = uvec2{buffer->surface.width, buffer->surface.height};
                    break;
                case GX2_SCAN_TARGET_DRC:
                    resolution::drc = uvec2{buffer->surface.width, buffer->surface.height};
                    break;
                default:
                    ;
            }
        }

        real_GX2CopyColorBufferToScanBuffer(buffer, scanTarget);
    }

    WUPS_MUST_REPLACE(GX2CopyColorBufferToScanBuffer,
                      WUPS_LOADER_LIBRARY_GX2,
                      GX2CopyColorBufferToScanBuffer);

} // namespace gx2_mon
