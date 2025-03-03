/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <algorithm>            // clamp()
#include <array>
#include <cmath>

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
        while (bytes >= 1000) {
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


} // namespace utils
