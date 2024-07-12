// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>
#include <cstring>

#include "cos_mon.hpp"


namespace cos_mon {


    namespace {

        const char get_core_utilization_preamble[] = {
            0x7c, 0x08, 0x02, 0xa6,
            0x94, 0x21, 0xff, 0xf0,
            0x93, 0xc1, 0x00, 0x08,
            0x93, 0xe1, 0x00, 0x0c,
            0x90, 0x01, 0x00, 0x14,
            0x3d, 0x80, 0x10, 0x0b,
            0x54, 0x60, 0x18, 0x38,
            0x39, 0x8c, 0x0d, 0x88,
            0x7c, 0x6c, 0x02, 0x14,
            0x4b, 0xff, 0x80, 0x85
        };

        //__OSGetStatistics     0x020294f4
        // get_core_utilization 0x020298d4
        // offset = +0x3e0
    }

    double (*get_core_utilization)(unsigned core);

    double
    safe_get_core_utilization(unsigned core)
    {
        if (get_core_utilization)
            return get_core_utilization(core);
        else
            return -1;
    }


    void
    initialize()
    {
        const char* addr = reinterpret_cast<const char*>(0x1045cd4);

        // sanity check: function preamble should match
        if (!std::memcmp(addr,
                         get_core_utilization_preamble,
                         sizeof get_core_utilization_preamble))
            std::memcpy(&get_core_utilization, &addr, sizeof addr);
    }


    void
    finalize()
    {}


    const char*
    get_report(float)
    {
        static char buf[128];

        double c0 = safe_get_core_utilization(0);
        double c1 = safe_get_core_utilization(1);
        double c2 = safe_get_core_utilization(2);

        std::snprintf(buf, sizeof buf,
                      "Core 0: %2.0f%% | Core 1: %2.0f%% | Core 2: %2.0f%%",
                      c0, c1, c2);
        return buf;
    }


}
