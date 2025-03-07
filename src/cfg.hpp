/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CFG_HPP
#define CFG_HPP

#include <chrono>
#include <string>
#include <vector>

#include "wupsxx/button_combo.hpp"
#include "wupsxx/color.hpp"
#include "wupsxx/option.hpp"


namespace cfg {

    extern wups::option<bool> button_rate;
    extern wups::option<bool> cpu_busy;
    extern wups::option<bool> cpu_busy_percent;
    extern wups::option<bool> enabled;
    extern wups::option<bool> fs_read;
    extern wups::option<bool> gpu_fps;
    extern wups::option<bool> gpu_busy;
    extern wups::option<bool> gpu_busy_percent;
    extern wups::option<bool> net_bw;
    extern wups::option<bool> net_cfg;
    extern wups::option<bool> time;
    extern wups::option<bool> time_24h;
    extern wups::option<wups::color> color_fg;
    extern wups::option<wups::color> color_bg;
    extern wups::option<std::chrono::milliseconds> interval;

    void initialize();
    void finalize();

    void load();
    void save();

} // namespace cfg

#endif
