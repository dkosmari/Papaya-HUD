// SPDX-License-Identifier: GPL-3.0-or-later

#include <wups.h>

#include "cfg.hpp"

#include "logging.hpp"
#include "overlay.hpp"
#include "wupsxx/bool_item.hpp"
#include "wupsxx/category.hpp"
#include "wupsxx/color_item.hpp"
#include "wupsxx/storage.hpp"
#include "wupsxx/duration_items.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PACKAGE);


namespace cfg {

    using std::chrono::milliseconds;
    using wups::config::color;

    using namespace std::literals;


    namespace keys {
        const char* color_bg = "color_bg";
        const char* color_fg = "color_fg";
        const char* cpu_busy = "cpu_busy";
        const char* enabled  = "enabled";
        const char* fs_read  = "fs_read";
        const char* gpu_fps  = "gpu_fps";
        const char* gpu_perf = "gpu_perf";
        const char* interval = "interval";
        const char* net_bw   = "net_bw";
        const char* time     = "time";
    }


    namespace labels {
        const char* color_bg = "Background color";
        const char* color_fg = "Foreground color";
        const char* cpu_busy = "CPU utilization";
        const char* enabled  = "Enabled";
        const char* fs_read  = "Filesystem";
        const char* gpu_fps  = "Frames per second";
        const char* gpu_perf = "GPU utilization";
        const char* interval = "Update interval";
        const char* net_bw   = "Network bandwidth";
        const char* time     = "Time";
    }


    namespace defaults {
        const color        color_bg = {0x00, 0x00, 0x00, 0xc0};
        const color        color_fg = {0x60, 0xff, 0x60};
        const bool         cpu_busy = true;
        const bool         enabled  = true;
        const bool         fs_read  = true;
        const bool         gpu_fps  = true;
        const bool         gpu_perf = true;
        const milliseconds interval = 1000ms;
        const bool         net_bw   = true;
        const bool         time     = true;
    }


    color        color_bg = defaults::color_bg;
    color        color_fg = defaults::color_fg;
    bool         cpu_busy = defaults::cpu_busy;
    bool         enabled  = defaults::enabled;
    bool         fs_read  = defaults::fs_read;
    bool         gpu_fps  = defaults::gpu_fps;
    bool         gpu_perf = defaults::gpu_perf;
    milliseconds interval = defaults::interval;
    bool         net_bw   = defaults::net_bw;
    bool         time     = defaults::time;


    WUPSConfigAPICallbackStatus
    menu_open(WUPSConfigCategoryHandle root_handle)
    {
        wups::config::category root{root_handle};

        root.add(wups::config::bool_item::create(keys::enabled,
                                                 labels::enabled,
                                                 enabled,
                                                 defaults::enabled,
                                                 "yes", "no"));

        root.add(wups::config::bool_item::create(keys::time,
                                                 labels::time,
                                                 time,
                                                 defaults::time,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(keys::gpu_fps,
                                                 labels::gpu_fps,
                                                 gpu_fps,
                                                 defaults::gpu_fps,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(keys::gpu_perf,
                                                 labels::gpu_perf,
                                                 gpu_perf,
                                                 defaults::gpu_perf,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(keys::cpu_busy,
                                                 labels::cpu_busy,
                                                 cpu_busy,
                                                 defaults::cpu_busy,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(keys::net_bw,
                                                 labels::net_bw,
                                                 net_bw,
                                                 defaults::net_bw,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(keys::fs_read,
                                                 labels::fs_read,
                                                 fs_read,
                                                 defaults::fs_read,
                                                 "on", "off"));

        root.add(wups::config::color_item::create(keys::color_fg,
                                                  labels::color_fg,
                                                  color_fg,
                                                  defaults::color_fg,
                                                  false));

        root.add(wups::config::color_item::create(keys::color_bg,
                                                  labels::color_bg,
                                                  color_bg,
                                                  defaults::color_bg,
                                                  true));

        root.add(wups::config::milliseconds_item::create(keys::interval,
                                                         labels::interval,
                                                         interval,
                                                         defaults::interval,
                                                         100ms, 5000ms,
                                                         100ms));

        return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
    }


    void
    menu_close()
    {
        cfg::save();

        if (enabled)
            overlay::create_or_reset();
        else
            overlay::destroy();
    }


    void
    init()
    {
        WUPSConfigAPIOptionsV1 options{ .name = PACKAGE_NAME };
        auto status = WUPSConfigAPI_Init(options, menu_open, menu_close);
        if (status != WUPSCONFIG_API_RESULT_SUCCESS) {
            logging::printf("Error initializing WUPS config API: %s\n",
                            WUPSConfigAPI_GetStatusStr(status));
        }

        load();
    }


    void
    load()
    {
        using wups::storage::load_or_init;
        using wups::storage::load_or_init_str;

        try {

#define LOI(x) load_or_init(keys::x, x, defaults::x)
            LOI(color_bg);
            LOI(color_fg);
            LOI(cpu_busy);
            LOI(enabled);
            LOI(fs_read);
            LOI(gpu_fps);
            LOI(gpu_perf);
            LOI(interval);
            LOI(net_bw);
            LOI(time);
#undef LOI
        }
        catch (std::exception& e) {
            logging::printf("Error loading config: %s\n", e.what());
        }
    }


    void
    save()
    {
        try {
            wups::storage::save();
        }
        catch (std::exception& e) {
            logging::printf("Error saving config: %s\n", e.what());
        }
    }
}
