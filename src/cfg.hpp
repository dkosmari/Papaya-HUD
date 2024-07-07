// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CFG_HPP
#define CFG_HPP

namespace cfg {

    namespace keys {
        extern const char* bandwidth;
        extern const char* enabled;
        extern const char* fps;
    }

    namespace labels {
        extern const char* bandwidth;
        extern const char* enabled;
        extern const char* fps;
    }

    namespace defaults {
        extern const bool bandwidth;
        extern const bool enabled;
        extern const bool fps;
    }


    extern bool bandwidth;
    extern bool enabled;
    extern bool fps;


    void init();

    void load();
    void save();


} // namespace cfg

#endif
