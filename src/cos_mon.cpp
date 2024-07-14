// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>
#include <cstring>

#include "cos_mon.hpp"


namespace cos_mon {

    using get_core_utilization_ptr = double (*)(unsigned);
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

        double c0 = get_core_utilization(0);
        double c1 = get_core_utilization(1);
        double c2 = get_core_utilization(2);

        std::snprintf(buf, sizeof buf,
                      "core 0: %2.0f%% | core 1: %2.0f%% | core 2: %2.0f%%",
                      c0, c1, c2);
        return buf;
    }

}
