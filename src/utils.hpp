/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

namespace utils {

    const char* percent_to_bar(float p);

    std::string format_bytes(float bytes);

} // namespace utils

#endif
