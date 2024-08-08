/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * Time Monitoring
 *
 * Just show a clock. This helps users to not lose track of time while playing.
 */

#include <cstdio>

#include <coreinit/time.h>

#include "time_mon.hpp"

#include "cfg.hpp"


namespace time_mon {


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
        static char buf[64];

        OSTime now = OSGetTime();
        OSCalendarTime cal;
        OSTicksToCalendarTime(now, &cal);

        int h = cal.tm_hour;
        int m = cal.tm_min;

        if (cfg::time_24h)
            std::snprintf(buf, sizeof buf,
                          "\ue007 %02d:%02d",
                          h, m);
        else {
            const char* suffix = h >=12 ? "pm" : "am";
            h = (h + 11) % 12 + 1;
            std::snprintf(buf, sizeof buf,
                          "\ue007 %d:%02d %s",
                          h, m, suffix);
        }

        return buf;
    }

} // namespace time_mon
