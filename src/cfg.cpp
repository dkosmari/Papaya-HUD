/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * Configuration
 *
 * This is where all configuration options are handled.
 */

#include <coreinit/cache.h>     // OSMemoryBarrier()

#include <wups.h>

#include "cfg.hpp"

#include "logger.hpp"
#include "overlay.hpp"
#include "wupsxx/bool_item.hpp"
#include "wupsxx/button_combo_item.hpp"
#include "wupsxx/category.hpp"
#include "wupsxx/color_item.hpp"
#include "wupsxx/storage.hpp"
#include "wupsxx/duration_items.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


namespace cfg {

    using std::chrono::milliseconds;
    using wups::config::button_combo;
    using wups::config::color;

    using namespace std::literals;


    namespace labels {
        const char* button_rate      = "Button press rate";
        const char* color_bg         = "Background color";
        const char* color_fg         = "Foreground color";
        const char* cpu_busy         = "CPU utilization";
        const char* cpu_busy_percent = " └ show percentage";
        const char* enabled          = "Enabled";
        const char* fs_read          = "Filesystem";
        const char* gpu_busy         = "GPU utilization";
        const char* gpu_busy_percent = " └ show percentage";
        const char* gpu_fps          = "Frames per second";
        const char* interval         = "Update interval";
        const char* net_bw           = "Network bandwidth";
        const char* time             = "Time";
        const char* time_24h         = " └ format";
        const char* toggle_shortcut  = "Toggle shortcut";
    }


    namespace defaults {
        const bool         button_rate      = true;
        const color        color_bg         = {0x00, 0x00, 0x00, 0xc0};
        const color        color_fg         = {0x60, 0xff, 0x60};
        const bool         cpu_busy         = true;
        const bool         cpu_busy_percent = false;
        const bool         enabled          = true;
        const bool         fs_read          = true;
        const bool         gpu_busy         = true;
        const bool         gpu_busy_percent = false;
        const bool         gpu_fps          = true;
        const milliseconds interval         = 1000ms;
        const bool         net_bw           = true;
        const bool         time             = true;
        const bool         time_24h         = true;
        const button_combo toggle_shortcut  = wups::config::vpad_combo{
            VPAD_BUTTON_TV | VPAD_BUTTON_LEFT
        };
    }


    bool         button_rate      = defaults::button_rate;
    color        color_bg         = defaults::color_bg;
    color        color_fg         = defaults::color_fg;
    bool         cpu_busy         = defaults::cpu_busy;
    bool         cpu_busy_percent = defaults::cpu_busy_percent;
    bool         enabled          = defaults::enabled;
    bool         fs_read          = defaults::fs_read;
    bool         gpu_busy         = defaults::gpu_busy;
    bool         gpu_busy_percent = defaults::gpu_busy_percent;
    bool         gpu_fps          = defaults::gpu_fps;
    milliseconds interval         = defaults::interval;
    bool         net_bw           = defaults::net_bw;
    bool         time             = defaults::time;
    bool         time_24h         = defaults::time_24h;
    button_combo  toggle_shortcut = defaults::toggle_shortcut;


    WUPSConfigAPICallbackStatus
    menu_open(WUPSConfigCategoryHandle root_handle)
    {
        wups::config::category root{root_handle};

        root.add(wups::config::bool_item::create(labels::enabled,
                                                 enabled,
                                                 defaults::enabled,
                                                 "yes", "no"));

        root.add(wups::config::bool_item::create(labels::time,
                                                 time,
                                                 defaults::time,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(labels::time_24h,
                                                 time_24h,
                                                 defaults::time_24h,
                                                 "24h", "12h"));

        root.add(wups::config::bool_item::create(labels::gpu_fps,
                                                 gpu_fps,
                                                 defaults::gpu_fps,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(labels::gpu_busy,
                                                 gpu_busy,
                                                 defaults::gpu_busy,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(labels::gpu_busy_percent,
                                                 gpu_busy_percent,
                                                 defaults::gpu_busy_percent,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(labels::cpu_busy,
                                                 cpu_busy,
                                                 defaults::cpu_busy,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(labels::cpu_busy_percent,
                                                 cpu_busy_percent,
                                                 defaults::cpu_busy_percent,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(labels::net_bw,
                                                 net_bw,
                                                 defaults::net_bw,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(labels::fs_read,
                                                 fs_read,
                                                 defaults::fs_read,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(labels::button_rate,
                                                 button_rate,
                                                 defaults::button_rate,
                                                 "on", "off"));

        root.add(wups::config::color_item::create(labels::color_fg,
                                                  color_fg,
                                                  defaults::color_fg,
                                                  false));

        root.add(wups::config::color_item::create(labels::color_bg,
                                                  color_bg,
                                                  defaults::color_bg,
                                                  true));

        root.add(wups::config::milliseconds_item::create(labels::interval,
                                                         interval,
                                                         defaults::interval,
                                                         100ms, 5000ms,
                                                         100ms));

        root.add(wups::config::button_combo_item::create(labels::toggle_shortcut,
                                                         toggle_shortcut,
                                                         defaults::toggle_shortcut));

        return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
    }


    void
    menu_close()
    {
        cfg::save();

        // Note: FS monitoring might run in other threads.
        OSMemoryBarrier();

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
            logger::printf("Error initializing WUPS config API: %s\n",
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

#define LOAD(x) load_or_init(#x, x, defaults::x)
            LOAD(button_rate);
            LOAD(color_bg);
            LOAD(color_fg);
            LOAD(cpu_busy);
            LOAD(cpu_busy_percent);
            LOAD(enabled);
            LOAD(fs_read);
            LOAD(gpu_busy);
            LOAD(gpu_busy_percent);
            LOAD(gpu_fps);
            LOAD(interval);
            LOAD(net_bw);
            LOAD(time);
            LOAD(time_24h);
            LOAD(toggle_shortcut);
#undef LOAD
        }
        catch (std::exception& e) {
            logger::printf("Error loading config: %s\n", e.what());
        }
    }


    void
    save()
    {
        try {
#define STORE(x) wups::storage::store(#x, x)
            STORE(button_rate);
            STORE(color_bg);
            STORE(color_fg);
            STORE(cpu_busy);
            STORE(cpu_busy_percent);
            STORE(enabled);
            STORE(fs_read);
            STORE(gpu_busy);
            STORE(gpu_busy_percent);
            STORE(gpu_fps);
            STORE(interval);
            STORE(net_bw);
            STORE(time);
            STORE(time_24h);
            STORE(toggle_shortcut);
#undef STORE
            wups::storage::save();
        }
        catch (std::exception& e) {
            logger::printf("Error saving config: %s\n", e.what());
        }
    }
}
