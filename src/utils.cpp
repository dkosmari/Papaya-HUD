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


using std::size_t;


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


    void
    format_bytes(out_span& out, float bytes)
    {
        static const std::array suffixes = {
            "B",
            "KiB",
            "MiB",
            "GiB",
            "TiB"
        };

        size_t suffix_idx = 0;
        while (bytes >= 768) {
            if (++suffix_idx >= suffixes.size()) {
                --suffix_idx;
                break;
            }
            bytes /= 1024;
        }

        out.printf("%.1f %s", bytes, suffixes[suffix_idx]);
    }


    void
    format_seconds(out_span& out, float seconds)
    {
        using std::div;
        using std::lround;

        if (seconds < 0.75) {
            out.printf("%ld ms", lround(seconds * 1000));
            return;
        }

        // If less than a minute.
        if (seconds < 60) {
            out.printf("%.1f s", seconds);
            return;
        }

        // If less than an hour.
        if (seconds < 60 * 60) {
            auto min_sec = div(lround(seconds), 60l);
            out.printf("%ld min", min_sec.quot);
            if (min_sec.rem) // avoid showing "0 s"
                out.printf(", %ld s", min_sec.rem);
            return;
        }

        // If less than a day.
        if (seconds < 24 * 60 * 60) {
            auto minutes = lround(seconds) / 60l;
            auto hr_min = div(minutes, 60l);
            out.printf("%ld h", hr_min.quot);
            if (hr_min.rem) // avoid showing "0 min"
                out.printf(", %ld min", hr_min.rem);
            return;
        }

        // General case: a day or more.
        auto minutes = lround(seconds) / 60l;
        auto hr_min = div(minutes, 60l);
        auto d_h = div(hr_min.quot, 24l);

        out.printf("%ld d", d_h.quot);
        if (d_h.rem) // avoid showing "0 h"
            out.printf(", %ld h", d_h.rem);
        if (hr_min.rem) // avoid showing "0 min"
            out.printf(", %ld min", hr_min.rem);

    }

} // namespace utils
