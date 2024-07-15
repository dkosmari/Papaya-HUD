// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CFG_HPP
#define CFG_HPP

#include "wupsxx/color.hpp"


namespace cfg {

    extern bool                bandwidth;
    extern wups::config::color bg_color;
    extern bool                cpu;
    extern bool                enabled;
    extern wups::config::color fg_color;
    extern bool                fps;
    extern bool                fs;
    extern bool                time;



    void init();

    void load();
    void save();

} // namespace cfg

#endif
