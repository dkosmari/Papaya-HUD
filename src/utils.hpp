/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdlib>

#include "out_span.hpp"


namespace utils {

    constexpr const char* field_separator = "┃";


    const char*
    percent_to_bar(float p);

    void
    format_bytes(out_span& dst,
                 float bytes);

    void
    format_seconds(out_span& dst,
                   float seconds);

} // namespace utils

#endif
