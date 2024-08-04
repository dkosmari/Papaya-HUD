// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>            // clamp()
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "cos_mon.hpp"


namespace cos_mon {

    using get_core_utilization_ptr = float (*)(unsigned);
    const get_core_utilization_ptr get_core_utilization =
        reinterpret_cast<get_core_utilization_ptr>(0x020298d4 - 0xfe3c00);


    const std::array<const char*, 9> bars{
        "　", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"
    };


    const char*
    percent_to_bar(float p)
    {
        long idx = std::lround(8 * p / 100.0);
        idx = std::clamp(idx, 0l, 8l);
        return bars[idx];
    }


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

#if 0
        std::snprintf(buf, sizeof buf,
                      "CPU0: %2.0f%%  CPU1: %2.0f%%  CPU2: %2.0f%%",
                      c0, c1, c2);
#else
        std::snprintf(buf, sizeof buf,
                      "CPU: %s %s %s",
                      percent_to_bar(c0),
                      percent_to_bar(c1),
                      percent_to_bar(c2));
#endif

        return buf;
    }

}
