// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CFG_HPP
#define CFG_HPP

#include "wupsxx/color.hpp"


namespace cfg {

    extern wups::config::color color_bg;
    extern wups::config::color color_fg;
    extern bool                cpu_busy;
    extern bool                enabled;
    extern bool                fs_read;
    extern bool                gpu_fps;
    extern bool                gpu_perf;
    extern bool                net_bw;
    extern bool                time;


    void init();

    void load();
    void save();

} // namespace cfg

#endif
