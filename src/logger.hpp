/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGGER_HPP
#define LOGGER_HPP


namespace logger {

    void initialize();

    void finalize();

    __attribute__(( __format__ (__printf__, 1, 2) ))
    void printf(const char* fmt, ...);


    // Keeps the logger initialized while it's constructed.
    struct guard {
        guard();
        ~guard();
    };

} // namespace logger

#endif
