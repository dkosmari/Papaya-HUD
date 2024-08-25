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



} // namespace utils
