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


    void
    get_report(out_span& out,
               float)
    {
        const char* separator = "";

        if (cfg::time.value) {

            OSTime now = OSGetTime();
            OSCalendarTime cal;
            OSTicksToCalendarTime(now, &cal);

            int h = cal.tm_hour;
            int m = cal.tm_min;

            if (cfg::time_24h.value)
                out.printf("%02d:%02d", h, m);
            else {
                const char* suffix = h >=12 ? "pm" : "am";
                h = (h + 11) % 12 + 1;
                out.printf("%d:%02d %s",
                           h, m, suffix);
            }
            separator = utils::field_separator;
        }

        if (cfg::uptime.value) {
            float up_seconds = float(OSGetSystemTime()) / OSTimerClockSpeed;
            out.printf("%sup: ", separator);
            utils::format_seconds(out, up_seconds);
            separator = utils::field_separator;
        }

        if (cfg::play_time.value) {
            OSTime play_duration = OSGetSystemTime() - app_start_time;
            float play_seconds = float(play_duration) / OSTimerClockSpeed;
            out.printf("%splay: ", separator);
            utils::format_seconds(out, play_seconds);
        }
    }

} // namespace time_mon
