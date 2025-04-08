/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "out_span.hpp"


using std::size_t;


out_span::out_span(char* buf, size_t buf_size)
    noexcept :
    data{buf},
    size{buf_size}
{}


bool
out_span::eof()
    const noexcept
{
    return size <= 1;
}


void
out_span::advance(size_t amount)
    noexcept
{
    if (amount > size)
        amount = size;
    size -= amount;
    data += amount;
}


void
out_span::append(const char* str)
    noexcept
{
    if (eof())
        return;

    while (size > 1 && *str) {
        *data++ = *str++;
        --size;
    }
    if (size > 0)
        *data = '\0';
}


void
out_span::append(const char* str,
                 size_t n)
    noexcept
{
    if (eof())
        return;

    while (size > 1 && *str && n--) {
        *data++ = *str++;
        --size;
    }
    if (size > 0)
        *data = '\0';
}


void
out_span::printf(const char* fmt,
                 ...)
    noexcept
{
    if (eof())
        return;
    va_list args;
    va_start(args, fmt);
    int written = std::vsnprintf(data, size, fmt, args);
    va_end(args);
    if (written <= 0)
        return;
    advance(written);
}
