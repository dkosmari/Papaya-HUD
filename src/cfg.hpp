/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CFG_HPP
#define CFG_HPP

#include <chrono>

#include "wupsxx/color.hpp"


namespace cfg {

    extern wups::config::color       color_bg;
    extern wups::config::color       color_fg;
    extern bool                      cpu_busy;
    extern bool                      enabled;
    extern bool                      fs_read;
    extern bool                      gpu_fps;
    extern bool                      gpu_perf;
    extern std::chrono::milliseconds interval;
    extern bool                      net_bw;
    extern bool                      time;
    extern bool                      time_24h;

    void init();

    void load();
    void save();

} // namespace cfg

#endif
