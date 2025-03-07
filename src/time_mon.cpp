/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
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
#include "utils.hpp"


namespace time_mon {

    OSTime app_start_time;


    void
    initialize()
    {}


    void
    finalize()
    {}


    void
    reset()
    {}


    void
    on_application_start()
    {
        app_start_time = OSGetSystemTime();
    }


    std::string
    get_report(float)
    {
        std::string clock_str;
        if (cfg::time.value) {

            OSTime now = OSGetTime();
            OSCalendarTime cal;
            OSTicksToCalendarTime(now, &cal);

            int h = cal.tm_hour;
            int m = cal.tm_min;

            static char buf[64];

            if (cfg::time_24h.value)
                std::snprintf(buf, sizeof buf,
                              "%02d:%02d",
                              h, m);
            else {
                const char* suffix = h >=12 ? "pm" : "am";
                h = (h + 11) % 12 + 1;
                std::snprintf(buf, sizeof buf,
                              "%d:%02d %s",
                              h, m, suffix);
            }

            clock_str = buf;
        }

        std::string uptime_str;
        if (cfg::uptime.value)
            uptime_str = "up: "
                         + utils::format_seconds(float(OSGetSystemTime()) / OSTimerClockSpeed);

        std::string play_time_str;
        if (cfg::play_time.value) {
            OSTime play_duration = OSGetSystemTime() - app_start_time;
            play_time_str = "play: "
                            + utils::format_seconds(float(play_duration) / OSTimerClockSpeed);
        }

        return utils::concat(clock_str, uptime_str, play_time_str);
    }

} // namespace time_mon
