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

    std::string format_seconds(float seconds);


    inline
    std::string
    concat()
    {
        return {};
    }


    template<typename... Args>
    std::string
    concat(const std::string& head, const Args&... tail)
    {
        const std::string concat_tail = concat(tail...);

        if (head.empty())
            return concat_tail;
        if (concat_tail.empty())
            return head;

        return head + "┃" + concat_tail;
    }

} // namespace utils

#endif
