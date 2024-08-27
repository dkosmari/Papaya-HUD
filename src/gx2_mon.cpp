/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
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
#include <wups.h>

#include <memory/mappedmemory.h>

// WUT lacks <gx2/perf.h>
#include "gx2_perf.h"
// WUT also lacks <coreinit/allocator.h>
#include "coreinit_allocator.h"

#include "gx2_mon.hpp"

#include "cfg.hpp"
#include "logger.hpp"
#include "overlay.hpp"
#include "utils.hpp"


#define TRACE                                           \
    do {                                                \
        auto here = std::source_location::current();    \
        logger::printf("%s:%u: %s\n",                   \
                       here.file_name(), here.line(),   \
                       here.function_name());           \
    }                                                   \
    while (false)


#define USE_UNIT_HEAP

namespace {

    void* lmm_ptr = nullptr;
    const std::uint32_t lmm_size = 4096;
    const std::uint32_t lmm_align = 32;

#ifdef USE_UNIT_HEAP
    const std::uint32_t lmm_unit_size = 32;
#endif

    MEMHeapHandle lmm_handle = nullptr;


    MEMAllocatorAlloc real_allocator_alloc = nullptr;

    void*
    my_allocator_alloc(MEMAllocator* a, std::uint32_t size)
    {
#ifdef USE_UNIT_HEAP
        if (size > lmm_unit_size) {
            logger::printf("ERROR: trying to allocate %u, but can only allocate up to %u.\n",
                           size,
                           lmm_unit_size);
            return nullptr;
        }
#endif

        void* result = real_allocator_alloc(a, size);
        // logger::printf("allocating %u bytes: %p\n", size, result);
        return result;
    }


    MEMAllocatorFree real_allocator_free = nullptr;

    void
    my_allocator_free(MEMAllocator* a, void* ptr)
    {
        // logger::printf("freeing %p\n", ptr);
        return real_allocator_free(a, ptr);
    }


    MEMAllocatorFunctions my_funcs {
        .alloc = my_allocator_alloc,
        .free = my_allocator_free
    };


    MEMAllocator
    libmappedmemory_allocator()
    {
        if (!lmm_ptr)
            OSFatal("ERROR!!!!!! could not allocate memory for lmm_ptr\n");

#ifdef USE_UNIT_HEAP
        std::uint32_t total_free = lmm_unit_size * MEMCountFreeBlockForUnitHeap(lmm_handle);
#else
        std::uint32_t total_free = MEMGetTotalFreeSizeForExpHeap(lmm_handle);
#endif
        logger::printf("Heap total free size: %u\n", total_free);

        MEMAllocator result;
#ifdef USE_UNIT_HEAP
        MEMInitAllocatorForUnitHeap(&result, lmm_handle);
#else
        MEMInitAllocatorForExpHeap(&result, lmm_handle, lmm_align);
#endif
        real_allocator_alloc = result.funcs->alloc;
        real_allocator_free = result.funcs->free;
        result.funcs = &my_funcs;

        return result;
    }


    // __attribute__((__constructor__))
    void
    initialize_lmm_heap()
    {
        if (lmm_ptr)
            return;

        lmm_ptr = MEMAllocFromMappedMemoryForGX2Ex(lmm_size, lmm_align);
        if (lmm_ptr) {
#ifdef USE_UNIT_HEAP
            lmm_handle = MEMCreateUnitHeapEx(lmm_ptr,
                                             lmm_size,
                                             lmm_unit_size,
                                             lmm_align,
                                             0);
#else
            lmm_handle = MEMCreateExpHeapEx(lmm_ptr, lmm_size, 0);
#endif
            if (!lmm_handle) {
                MEMFreeToMappedMemory(lmm_ptr);
                lmm_ptr = nullptr;
            }
        }
    }


    // __attribute__((__destructor__))
    void
    finalize_lmm_heap()
    {
        if (lmm_handle) {
#ifdef USE_UNIT_HEAP
            MEMDestroyUnitHeap(lmm_handle);
#else
            MEMDestroyExpHeap(lmm_handle);
#endif
            lmm_handle = nullptr;
        }
        if (lmm_ptr) {
            MEMFreeToMappedMemory(lmm_ptr);
            lmm_ptr = nullptr;
        }
    }

} // namespace






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
            logger::printf("checking out allocator\n");
            logger::printf(" .funcs=%p\n", allocator.funcs);
            logger::printf(" .funcs->alloc=%p\n", allocator.funcs->alloc);
            logger::printf(" .funcs->free=%p\n", allocator.funcs->free);
            logger::printf(" .heap=%p\n", allocator.heap);
            logger::printf(" .arg1=0x%08x\n", allocator.arg1);
            logger::printf(" .arg2=0x%08x\n", allocator.arg2);

            GX2PerfInit(&data, max_tags, &allocator);

            logger::printf("GX2PerfInit() returned\n");
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
        get_metric(std::uint32_t index)
        {
            GX2PerfType type;
            static_assert(sizeof type == 4);
            std::uint32_t id;
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
                allocator{libmappedmemory_allocator()},
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
                GX2DrawDone();

                // if on last frame
                if (++pass >= num_passes) {
                    data.frame_finish();
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


        const char*
        get_report(float /*dt*/)
        {
            if (!prof)
                return "";

            float avg_gpu_busy = average(prof->gpu_busy_vec);
            unsigned n_samples = prof->gpu_busy_vec.size();
            prof->gpu_busy_vec.clear();

            if (n_samples == 0)
                return "GPU: ?";

            static char buf[16];
            if (cfg::gpu_busy_percent)
                std::snprintf(buf, sizeof buf,
                              "GPU: %2.0f%%",
                              avg_gpu_busy);
            else
                std::snprintf(buf, sizeof buf,
                              "GPU: %s",
                              utils::percent_to_bar(avg_gpu_busy));

            return buf;
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
        on_frame_finish()
        {
            ++counter;
        }


        const char*
        get_report(float dt)
        {
            static char buf[32];

            float fps = counter / dt;
            counter = 0;

            std::snprintf(buf, sizeof buf, "%02.0f fps", fps);
            return buf;
        }

    } // namespace fps


    void
    initialize()
    {
        // TRACE;

        // FIFA 13 will call GX2Init() after closing the Home Menu, AFTER it comes into
        // the foreground. So we avoid doing any initialization until our GX2Init() hook
        // is called.
        if (!overlay::gx2_init)
            return;

        if (cfg::gpu_busy)
            perf::initialize();
        if (cfg::gpu_fps)
            fps::initialize();
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
        if (cfg::gpu_fps)
            fps::initialize();

        perf::finalize();
        if (cfg::gpu_busy)
            perf::initialize();

    }


    void
    on_application_start()
    {
        initialize_lmm_heap();
    }


    void
    on_application_ends()
    {
        finalize_lmm_heap();
    }


    DECL_FUNCTION(void, GX2SwapScanBuffers, void)
    {
        // skip all work if the plugin is disabled
        if (!cfg::enabled)
            return real_GX2SwapScanBuffers();

        if (cfg::gpu_fps)
            fps::on_frame_finish();

        if (cfg::gpu_busy)
            perf::on_frame_finish();

        overlay::render();

        real_GX2SwapScanBuffers();

        if (cfg::gpu_busy)
            perf::on_frame_start();

    }

    WUPS_MUST_REPLACE(GX2SwapScanBuffers, WUPS_LOADER_LIBRARY_GX2, GX2SwapScanBuffers);


    DECL_FUNCTION(void, GX2Init, std::uint32_t* attr)
    {
        // logger::printf("GX2Init() was called on core %u\n", OSGetCoreId());
        real_GX2Init(attr);
        overlay::gx2_init = true;

        if (cfg::enabled)
            overlay::create_or_reset();
    }

    WUPS_MUST_REPLACE(GX2Init, WUPS_LOADER_LIBRARY_GX2, GX2Init);


    DECL_FUNCTION(void, GX2Shutdown, void)
    {
        // logger::printf("GX2Shutdown() was called\n");
        overlay::destroy();
        overlay::gx2_init = false;
        real_GX2Shutdown();
    }

    WUPS_MUST_REPLACE(GX2Shutdown, WUPS_LOADER_LIBRARY_GX2, GX2Shutdown);


    DECL_FUNCTION(void, GX2ResetGPU, std::uint32_t arg)
    {
        // logger::printf("GX2ResetGPU() was called\n");
        overlay::destroy();
        real_GX2ResetGPU(arg);
        if (cfg::enabled)
            overlay::create_or_reset();
    }

    WUPS_MUST_REPLACE(GX2ResetGPU, WUPS_LOADER_LIBRARY_GX2, GX2ResetGPU);

} // namespace gx2_mon
