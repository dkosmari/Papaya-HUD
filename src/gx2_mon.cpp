// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <optional>
#include <ranges>
#include <source_location>
#include <variant>
#include <vector>

#include <coreinit/core.h>      // OSGetCoreId()
#include <coreinit/memory.h>    // OSGetForegroundBucket()
#include <gx2/event.h>          // GX2DrawDone()
#include <wups.h>

#include "gx2_mon.hpp"

#include "cfg.hpp"
#include "overlay.hpp"
#include "logging.hpp"


// WUT lacks <gx2/perf.h>
#include "gx2_perf.h"
// WUT also doesn't have the allocator functions.
#include "coreinit_allocator.h"


#define TRACE                                           \
    do {                                                \
        auto here = std::source_location::current();    \
        logging::printf("%s:%u: %s\n",                  \
                        here.file_name(), here.line(),  \
                        here.function_name());          \
    }                                                   \
    while (false)


namespace coreinit {

    MEMAllocator
    default_heap_allocator()
    {
        MEMAllocator result;
        MEMInitAllocatorForDefaultHeap(&result);
        return result;
    }

}


/*
 * Known incompatibilities:
 *   - Youtube
 *
 * TODO: investigate if GX2Init() is being called on an unusual core.
 */


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
            GX2PerfInit(&data, max_tags, &allocator);
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
                allocator{coreinit::default_heap_allocator()},
                data{1, allocator},
                gpu_busy_enabled{false}
            {
                TRACE;

                data.set_collection_method(GX2_PERF_COLLECT_TAGS_ACCUMULATE);
                data.set_tag(0, true);
            }


            ~profiler()
            {
                TRACE;
                // logging::printf("    frame_open = %s\n", frame_open ? "true" : "false");
                // logging::printf("    started = %s\n", started ? "true" : "false");
                // logging::printf("    pass = %u\n", pass);
                // logging::printf("    num_passes = %u\n", num_passes);
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
                        logging::printf("no slot available for GPU_BUSY\n");
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
                    logging::printf("ERROR: frame not open\n");
                    return;
                }

                if (!pass_open) {
                    logging::printf("ERROR: pass not open\n");
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
                                logging::printf("gpu_busy_vec is growing too much! %u\n",
                                                static_cast<unsigned>(gpu_busy_vec.size()));
                        } else
                            logging::printf("failed to get GPU_BUSY result");
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
            TRACE;
            prof.emplace();
        }


        void
        finalize()
        {
            if (!prof)
                return;
            TRACE;
            prof.reset();
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

            static char buf[32];
#if 0
            std::snprintf(buf, sizeof buf,
                          "GPU: %2.0f%%",
                          avg_gpu_busy);
#else
            const std::array<const char*, 9> bars{
                "　",
                "▁",
                "▂",
                "▃",
                "▄",
                "▅",
                "▆",
                "▇",
                "█"
            };

            long idx = std::lround(8 * avg_gpu_busy / 100.0);
            idx = std::clamp(idx, 0l, 8l);
            std::snprintf(buf, sizeof buf,
                          "GPU: %s",
                          bars[idx]);
#endif

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

            std::snprintf(buf, sizeof buf, "%04.1f fps", fps);
            return buf;
        }

    } // namespace fps


    void
    initialize()
    {
        TRACE;

        // FIFA 13 will call GX2Init() after closing the Home Menu, AFTER it comes into
        // the foreground. So we avoid doing any initialization until our GX2Init() hook
        // is called.
        if (!overlay::gx2_init)
            return;

        if (cfg::gpu_perf)
            perf::initialize();
        if (cfg::gpu_fps)
            fps::initialize();
    }


    void
    finalize()
    {
        TRACE;

        perf::finalize();
        fps::finalize();
    }


    void
    reset()
    {
        TRACE;

        fps::finalize();
        if (cfg::gpu_fps)
            fps::initialize();

        perf::finalize();
        if (cfg::gpu_perf)
            perf::initialize();

    }



    DECL_FUNCTION(void, GX2SwapScanBuffers, void)
    {
        // skip all work if the plugin is disabled
        if (!cfg::enabled)
            return real_GX2SwapScanBuffers();

        if (cfg::gpu_fps)
            fps::on_frame_finish();

        if (cfg::gpu_perf)
            perf::on_frame_finish();

        overlay::render();

        real_GX2SwapScanBuffers();

        if (cfg::gpu_perf)
            perf::on_frame_start();

    }

    WUPS_MUST_REPLACE(GX2SwapScanBuffers, WUPS_LOADER_LIBRARY_GX2, GX2SwapScanBuffers);


    DECL_FUNCTION(void, GX2Init, std::uint32_t* attr)
    {
        // logging::printf("GX2Init() was called on core %u\n", OSGetCoreId());
        real_GX2Init(attr);
        overlay::gx2_init = true;

        if (cfg::enabled)
            overlay::create_or_reset();
    }

    WUPS_MUST_REPLACE(GX2Init, WUPS_LOADER_LIBRARY_GX2, GX2Init);


    DECL_FUNCTION(void, GX2Shutdown, void)
    {
        // logging::printf("GX2Shutdown() was called\n");
        overlay::destroy();
        overlay::gx2_init = false;
        real_GX2Shutdown();
    }

    WUPS_MUST_REPLACE(GX2Shutdown, WUPS_LOADER_LIBRARY_GX2, GX2Shutdown);


    DECL_FUNCTION(void, GX2ResetGPU, std::uint32_t arg)
    {
        // logging::printf("GX2ResetGPU() was called\n");
        overlay::destroy();
        real_GX2ResetGPU(arg);
        if (cfg::enabled)
            overlay::create_or_reset();
    }

    WUPS_MUST_REPLACE(GX2ResetGPU, WUPS_LOADER_LIBRARY_GX2, GX2ResetGPU);

} // namespace gx2_mon
