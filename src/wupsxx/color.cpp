// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>
#include <stdexcept>

#include "color.hpp"


namespace wups::config {

    color::color(const std::string& str)
    {
        if (str.empty() || str[0] != '#')
            throw std::invalid_argument{"color must start with '#' character"};

        std::size_t pos = 0;
        std::uint32_t val = std::stoul(str.substr(1), &pos, 16);
        if (pos != str.size() - 1)
            throw std::invalid_argument{"color string \"" + str +
                                        "\" contains invalid chars at pos=" + std::to_string(pos)};

        auto len = str.size();

        color c;
        switch (len) {
            // TODO: support '#rgb' and '#rgba' formats
        case 1 + 6: // #RRGGBB
            r = (val >> 16) & 0xff;
            g = (val >>  8) & 0xff;
            b = (val >>  0) & 0xff;
            a = 0xff;
            break;
        case 1 + 8: // #RRGGBBAA
            r = (val >> 24) & 0xff;
            g = (val >> 16) & 0xff;
            b = (val >>  8) & 0xff;
            a = (val >>  0) & 0xff;
            break;
        default:
            throw std::invalid_argument{"invalid length for color string: \"" + str + "\""};
        }
    }


    std::string
    to_string(color c,
              bool with_alpha,
              bool uppercase)
    {
        char buf[10];
        if (with_alpha) {
            if (uppercase)
                std::snprintf(buf, sizeof buf,
                              "#%02X%02X%02X%02X",
                              c.r, c.g, c.b, c.a);
            else
                std::snprintf(buf, sizeof buf,
                              "#%02x%02x%02x%02x",
                              c.r, c.g, c.b, c.a);
        } else {
            if (uppercase)
                std::snprintf(buf, sizeof buf,
                              "#%02X%02X%02X",
                              c.r, c.g, c.b);
            else
                std::snprintf(buf, sizeof buf,
                              "#%02x%02x%02x",
                              c.r, c.g, c.b);
        }

        return buf;
    }


} // namespace wups::config
