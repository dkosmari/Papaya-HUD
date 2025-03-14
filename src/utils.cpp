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
        char buf[32];

        if (seconds < 0.750) {
            std::snprintf(buf, sizeof buf, "%ld ms", std::lround(seconds * 1000));
        } else if (seconds < 60) {
            std::snprintf(buf, sizeof buf, "%.1f s", seconds);
        } else if (seconds < 60 * 60) {
            auto r = std::div(std::lround(seconds), 60l);
            std::snprintf(buf, sizeof buf, "%ld min, %ld s", r.quot, r.rem);
        } else if (seconds < 24 * 60 * 60) {
            auto r = std::div(std::lround(seconds), 60l);
            r = std::div(r.quot, 60l);
            if (r.rem)
                std::snprintf(buf, sizeof buf, "%ld h, %ld min", r.quot, r.rem);
            else // avoid showing "0 min"
                std::snprintf(buf, sizeof buf, "%ld h", r.quot);
        } else {
            auto r = std::div(std::lround(seconds), 60l);
            r = std::div(r.quot, 60l);
            auto rr = std::div(r.quot, 24l);
            std::snprintf(buf, sizeof buf, "%ld d, %ld h, %ld min", rr.quot, rr.rem, r.rem);
        }

        return buf;
    }


} // namespace utils
