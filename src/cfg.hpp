// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CFG_HPP
#define CFG_HPP

namespace cfg {

    namespace keys {
        extern const char* bandwidth;
        extern const char* cpu;
        extern const char* enabled;
        extern const char* fps;
        extern const char* fs;
    }

    namespace labels {
        extern const char* bandwidth;
        extern const char* cpu;
        extern const char* enabled;
        extern const char* fps;
        extern const char* fs;
    }

    namespace defaults {
        extern const bool bandwidth;
        extern const bool cpu;
        extern const bool enabled;
        extern const bool fps;
        extern const bool fs;
    }


    extern bool bandwidth;
    extern bool cpu;
    extern bool enabled;
    extern bool fps;
    extern bool fs;


    void init();

    void load();
    void save();


} // namespace cfg

#endif
