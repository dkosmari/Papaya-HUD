// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>

#include <coreinit/time.h>

#include "time_mon.hpp"


namespace time_mon {


    void
    initialize()
    {}


    void
    finalize()
    {}


    const char* get_report(float)
    {
        static char buf[64];

        OSTime now = OSGetTime();
        OSCalendarTime cal;
        OSTicksToCalendarTime(now, &cal);
        std::snprintf(buf, sizeof buf,
                      "%02d:%02d:%02d",
                      cal.tm_hour, cal.tm_min, cal.tm_sec);
        return buf;
    }


}
