// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CFG_HPP
#define CFG_HPP

namespace cfg {

    extern bool bandwidth;
    extern bool cpu;
    extern bool enabled;
    extern bool fps;
    extern bool fs;
    extern bool time;


    void init();

    void load();
    void save();

} // namespace cfg

#endif
