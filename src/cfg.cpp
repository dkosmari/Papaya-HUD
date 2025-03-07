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

#include "cfg.hpp"

#include "logger.hpp"
#include "overlay.hpp"

#include "wupsxx/init.hpp"
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
    using wups::button_combo::combo;
    using wups::color;

    using namespace std::literals;


    // Note: keep the same order as the UI.

    WUPSXX_OPTION(bool, enabled, true,
                  "Enabled");

    WUPSXX_OPTION(combo, toggle_shortcut,
                  combo::from_vpad(VPAD_BUTTON_TV | VPAD_BUTTON_L),
                  " └ Toggle shortcut");

    WUPSXX_OPTION(bool, time, true,
                  "Time");

    WUPSXX_OPTION(bool, time_24h, true,
                  " └ Format");

    WUPSXX_OPTION(bool, gpu_fps, true,
                  "Frames per second");

    WUPSXX_OPTION(bool, gpu_busy, false,
                  "GPU utilization");

    WUPSXX_OPTION(bool, gpu_busy_percent, false,
                  " └ Show percentage");

    WUPSXX_OPTION(bool, cpu_busy, true,
                  "CPU utilization");

    WUPSXX_OPTION(bool, cpu_busy_percent, false,
                  " └ Show percentage");

    WUPSXX_OPTION(bool, net_cfg, false,
                  "Network configuration");

    WUPSXX_OPTION(bool, net_bw, true,
                  "Network bandwidth");

    WUPSXX_OPTION(bool, fs_read, true,
                  "Filesystem read rate");

    WUPSXX_OPTION(bool, button_rate, true,
                  "Button press rate");

    WUPSXX_OPTION(color, color_fg, color(0x60, 0xff, 0x60),
                  "Foreground color");

    WUPSXX_OPTION(color, color_bg, color(0x00, 0x00, 0x00, 0xc0),
                  "Background color");

    WUPSXX_OPTION(milliseconds, interval, 1000ms,
                  "Update interval");


    const std::vector<wups::option_base*> all_options{
        &enabled,
        &toggle_shortcut,
        &time,
        &time_24h,
        &gpu_fps,
        &gpu_busy,
        &gpu_busy_percent,
        &cpu_busy,
        &cpu_busy_percent,
        &net_cfg,
        &net_bw,
        &fs_read,
        &button_rate,
        &color_fg,
        &color_bg,
        &interval,
    };


    wups::button_combo::handle toggle_shortcut_handle;

    void
    menu_open(wups::category& root)
    {
        using wups::make_item;

        root.add(make_item(enabled, "yes", "no"));
        root.add(make_item(toggle_shortcut, toggle_shortcut_handle));
        root.add(make_item(time, "on", "off"));
        root.add(make_item(time_24h, "24h", "12h"));
        root.add(make_item(gpu_fps, "on", "off"));
        root.add(make_item(gpu_busy, "on", "off"));
        root.add(make_item(gpu_busy_percent, "on", "off"));
        root.add(make_item(cpu_busy, "on", "off"));
        root.add(make_item(cpu_busy_percent, "on", "off"));
        root.add(make_item(net_cfg, "on", "off"));
        root.add(make_item(net_bw, "on", "off"));
        root.add(make_item(fs_read, "on", "off"));
        root.add(make_item(button_rate, "on", "off"));
        root.add(make_item(color_fg, false));
        root.add(make_item(color_bg, true));
        root.add(make_item(interval, 100ms, 5000ms, 100ms));
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

        load();

        try {
            auto [h, conflict] = wups::button_combo::create("[" PACKAGE_NAME "] Toggle HUD",
                                                            toggle_shortcut.value,
                                                            [](wups::button_combo::ctr_set,
                                                               wups::button_combo::handle)
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
        wups::button_combo::destroy(toggle_shortcut_handle);
    }


    void
    load()
    {
        try {
            for (auto& opt : all_options)
                opt->load();
        }
        catch (std::exception& e) {
            logger::printf("Error loading config: %s\n", e.what());
        }
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
