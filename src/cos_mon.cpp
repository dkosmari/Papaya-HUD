/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * CafeOS Shell Monitoring
 *
 * In this file we take advantage of the leftover "CafeOS Shell" functions left behind
 * inside retail coreinit.
 */

#include <cstdio>

#include "cos_mon.hpp"

#include "cfg.hpp"
#include "utils.hpp"


namespace cos_mon {

    using get_core_utilization_ptr = float (*)(unsigned);
    const get_core_utilization_ptr get_core_utilization =
        reinterpret_cast<get_core_utilization_ptr>(0x020298d4 - 0xfe3c00);


    void
    initialize()
    {}


    void
    finalize()
    {}


    void
    reset()
    {}


    const char*
    get_report(float)
    {
        static char buf[128];

        auto c0 = get_core_utilization(0);
        auto c1 = get_core_utilization(1);
        auto c2 = get_core_utilization(2);

        if (cfg::cpu_busy_percent)
            std::snprintf(buf, sizeof buf,
                          "CPU0: %2.0f%%  CPU1: %2.0f%%  CPU2: %2.0f%%",
                          c0, c1, c2);
        else
            std::snprintf(buf, sizeof buf,
                          "CPU: %s %s %s",
                          utils::percent_to_bar(c0),
                          utils::percent_to_bar(c1),
                          utils::percent_to_bar(c2));


        return buf;
    }

}
