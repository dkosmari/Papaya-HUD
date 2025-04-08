/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef OUT_SPAN_HPP
#define OUT_SPAN_HPP

#include <cstdlib>


struct out_span {

    char* data;
    std::size_t size;

    out_span(char* buf, std::size_t buf_size)
        noexcept;

    bool
    eof()
        const noexcept;

    void
    advance(std::size_t amount)
        noexcept;

    void
    append(const char* str)
        noexcept;

    void
    append(const char* str,
           std::size_t n)
        noexcept;

    __attribute__(( __format__ ( __printf__, 2, 3 ) ))
    void
    printf(const char* fmt,
           ...)
        noexcept;

};

#endif
