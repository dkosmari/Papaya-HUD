/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
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

#include <wupsxx/bool_item.hpp>
#include <wupsxx/cafe_glyphs.h>
#include <wupsxx/category.hpp>
#include <wupsxx/color_item.hpp>
#include <wupsxx/duration_items.hpp>
#include <wupsxx/init.hpp>
#include <wupsxx/logger.hpp>
#include <wupsxx/shortcut_item.hpp>
#include <wupsxx/storage.hpp>

#include "cfg.hpp"

#include "overlay.hpp"


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define WARNING_GLYPH "\uE010"


namespace cfg {

    namespace logger = wups::logger;

    using std::chrono::milliseconds;
    using wups::shortcut::combo;
    using wups::color;

    using namespace std::literals;


    // Note: keep the same order as the UI.

    WUPSXX_OPTION("Enabled",
                  bool, enabled, true);

    WUPSXX_OPTION(" └ Toggle shortcut",
                  combo, toggle_shortcut,
                  combo::from_vpad(VPAD_BUTTON_TV | VPAD_BUTTON_L));

    WUPSXX_OPTION("Time",
                  bool, time, true);

    WUPSXX_OPTION(" └ Format",
                  bool, time_24h, true);

    WUPSXX_OPTION("System uptime",
                  bool, uptime, false);

    WUPSXX_OPTION("Play time",
                  bool, play_time, false);

    WUPSXX_OPTION("Frames per second",
                  bool, gpu_fps, true);

    WUPSXX_OPTION("Screen resolution",
                  bool, gpu_resolution, true);

    WUPSXX_OPTION("GPU utilization " WARNING_GLYPH,
                  bool, gpu_busy, false);

    WUPSXX_OPTION(" └ Show percentage",
                  bool, gpu_busy_percent, false);

    WUPSXX_OPTION("CPU utilization",
                  bool, cpu_busy, true);

    WUPSXX_OPTION(" └ Show percentage",
                  bool, cpu_busy_percent, false);

    WUPSXX_OPTION(" └ Also show ARM CPU " WARNING_GLYPH,
                  bool, cpu_busy_arm, false);

    WUPSXX_OPTION("Network configuration",
                  bool, net_cfg, false);

    WUPSXX_OPTION("Network bandwidth",
                  bool, net_bw, true);

    WUPSXX_OPTION("  └ Combine upload and download values",
                  bool, net_bw_combined, true);

    WUPSXX_OPTION("Filesystem performance",
                  bool, fs_perf, true);

    WUPSXX_OPTION("  └ Combine read and write values",
                  bool, fs_perf_combined, true);

    WUPSXX_OPTION("Audio details",
                  bool, audio, true);

    WUPSXX_OPTION("  └ Audio utilization " WARNING_GLYPH,
                  bool, audio_busy, false);

    WUPSXX_OPTION("Button presses per second",
                  bool, button_rate, true);

    WUPSXX_OPTION("Battery levels",
                  bool, battery, true);

    WUPSXX_OPTION("  └ Show percentage",
                  bool, battery_percent, false);

    WUPSXX_OPTION("Foreground color",
                  color, color_fg, color(0x60, 0xff, 0x60));

    WUPSXX_OPTION("Background color",
                  color, color_bg, color(0x00, 0x00, 0x00, 0xc0));

    WUPSXX_OPTION("Update interval",
                  milliseconds, interval, 1s, 100ms, 5s);


    const std::vector<wups::option_base*> all_options{
        &enabled,
        &toggle_shortcut,
        &time,
        &time_24h,
        &uptime,
        &play_time,
        &gpu_fps,
        &gpu_resolution,
        &gpu_busy,
        &gpu_busy_percent,
        &cpu_busy,
        &cpu_busy_percent,
        &cpu_busy_arm,
        &net_cfg,
        &net_bw,
        &net_bw_combined,
        &fs_perf,
        &fs_perf_combined,
        &audio,
        &audio_busy,
        &button_rate,
        &battery,
        &battery_percent,
        &color_fg,
        &color_bg,
        &interval,
    };


    wups::shortcut::handle toggle_shortcut_handle;

    void
    menu_open(wups::category& root)
    {
        using wups::make_item;

        root.add(make_item(enabled,
                           {
                               .true_label = "yes",
                               .false_label = "no"
                           }));
        root.add(make_item(toggle_shortcut, toggle_shortcut_handle));
        root.add(make_item(time));
        root.add(make_item(time_24h,
                           {
                               .true_label = "24h",
                               .false_label = "12h"
                           }));
        root.add(make_item(uptime));
        root.add(make_item(play_time));
        root.add(make_item(gpu_fps));
        root.add(make_item(gpu_resolution));
        root.add(make_item(gpu_busy));
        root.add(make_item(gpu_busy_percent));
        root.add(make_item(cpu_busy));
        root.add(make_item(cpu_busy_percent));
        root.add(make_item(cpu_busy_arm));
        root.add(make_item(net_cfg));
        root.add(make_item(net_bw));
        root.add(make_item(net_bw_combined));
        root.add(make_item(fs_perf));
        root.add(make_item(fs_perf_combined));
        root.add(make_item(audio));
        root.add(make_item(audio_busy));
        root.add(make_item(button_rate));
        root.add(make_item(battery));
        root.add(make_item(battery_percent));
        root.add(make_item(color_fg, false));
        root.add(make_item(color_bg, true));
        root.add(make_item(interval,
                           {
                               .fast_increment = 1000ms,
                               .slow_increment = 100ms
                           }));
    }


    void
    menu_close()
    {
        cfg::save();

        // Note: FS monitoring might run in other threads.
        OSMemoryBarrier();

        if (enabled.value)
            overlay::create_or_reset();
        else
            overlay::destroy();
    }


    void
    initialize()
    {
        try {
            wups::init(PACKAGE_NAME, menu_open, menu_close);
        }
        catch (std::exception& e) {
            logger::printf("Error initializing config API: %s\n", e.what());
        }

        logger::printf("calling cfg::load()\n");
        load();

        try {
            logger::printf("creating button combo\n");
            auto [h, conflict] = wups::shortcut::create("Toggle HUD",
                                                        toggle_shortcut.value,
                                                        [](wups::shortcut::ctr_set,
                                                           wups::shortcut::handle)
                                                        {
                                                            overlay::toggle();
                                                        });
            toggle_shortcut_handle = h;
            if (conflict)
                logger::printf("Shortcut has conflict\n");
        }
        catch (std::exception& e) {
            logger::printf("Error setting up button combo: %s\n", e.what());
        }

    }


    void
    finalize()
    {
        wups::shortcut::destroy(toggle_shortcut_handle);
    }


    void
    load()
    {
        for (auto& opt : all_options)
            opt->load();
    }


    void
    save()
    {
        try {
            for (const auto& opt : all_options)
                opt->store();
            wups::save();
        }
        catch (std::exception& e) {
            logger::printf("Error saving config: %s\n", e.what());
        }
    }

} // namespace cfg
