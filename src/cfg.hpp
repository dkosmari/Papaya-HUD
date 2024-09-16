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

#include "wupsxx/button_combo.hpp"
#include "wupsxx/color.hpp"


namespace cfg {

    extern bool                      button_rate;
    extern wups::utils::color        color_bg;
    extern wups::utils::color        color_fg;
    extern bool                      cpu_busy;
    extern bool                      cpu_busy_percent;
    extern bool                      enabled;
    extern bool                      fs_read;
    extern bool                      gpu_busy;
    extern bool                      gpu_busy_percent;
    extern bool                      gpu_fps;
    extern std::chrono::milliseconds interval;
    extern bool                      net_bw;
    extern bool                      time;
    extern bool                      time_24h;
    extern wups::utils::button_combo toggle_shortcut;

    void init();

    void load();
    void save();

} // namespace cfg

#endif
