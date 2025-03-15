/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <algorithm>            // clamp()
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "utils.hpp"


namespace utils {

    const char*
    percent_to_bar(float p)
    {
        static const std::array bars{
            "\u3000", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"
        };
        long idx = std::floor(bars.size() * p / 100.0);
        idx = std::clamp<long>(idx, 0l, bars.size() - 1);
        return bars[idx];
    }


    std::string
    format_bytes(float bytes)
    {
        static const std::array suffixes = {
            "B",
            "KiB",
            "MiB",
            "GiB",
            "TiB"
        };

        std::size_t suffix_idx = 0;
        while (bytes >= 768) {
            if (++suffix_idx >= suffixes.size()) {
                --suffix_idx;
                break;
            }
            bytes /= 1024;
        }

        char buf[64];
        std::snprintf(buf, sizeof buf,
                      "%.1f %s",
                      bytes,
                      suffixes[suffix_idx]);
        return buf;
    }


    std::string
    format_seconds(float seconds)
    {
        using std::div;
        using std::lround;
        using std::to_string;
        using std::string;

        if (seconds < 0.75)
            return to_string(lround(seconds * 1000)) + " ms";

        // If less than a minute.
        if (seconds < 60) {
            char buf[16];
            std::snprintf(buf, sizeof buf, "%.1f s", seconds);
            return buf;
        }

        // If less than an hour.
        if (seconds < 60 * 60) {
            auto min_sec = div(lround(seconds), 60l);
            string result = to_string(min_sec.quot) + " min";
            if (min_sec.rem) // avoid showing "0 s"
                result += ", " + to_string(min_sec.rem) + " s";
            return result;
        }

        // If less than a day.
        if (seconds < 24 * 60 * 60) {
            auto minutes = lround(seconds) / 60l;
            auto hr_min = div(minutes, 60l);
            string result = to_string(hr_min.quot) + " h";
            if (hr_min.rem) // avoid showing "0 min"
                result += ", " + to_string(hr_min.rem) + " min";
            return result;
        }

        // General case: a day or more.
        auto minutes = lround(seconds) / 60l;
        auto hr_min = div(minutes, 60l);
        auto d_h = div(hr_min.quot, 24l);

        string result = to_string(d_h.quot) + " d";
        if (d_h.rem) // avoid showing "0 h"
            result += ", " + to_string(d_h.rem) + " h";
        if (hr_min.rem) // avoid showing "0 min"
            result += ", " + to_string(hr_min.rem) + " min";
        return result;

    }

} // namespace utils
