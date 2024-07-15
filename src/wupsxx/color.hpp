// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WUPSXX_COLOR_HPP
#define WUPSXX_COLOR_HPP

#include <compare>
#include <cstdint>
#include <string>


namespace wups::config {

    struct color {

        std::uint8_t r = 0;
        std::uint8_t g = 0;
        std::uint8_t b = 0;
        std::uint8_t a = 0xff;


        constexpr color() noexcept = default;

        constexpr
        color(std::uint8_t r,
              std::uint8_t g,
              std::uint8_t b,
              std::uint8_t a = 0xff)
            noexcept :
            r{r},
            g{g},
            b{b},
            a{a}
        {}


        explicit
        color(const std::string& str);


        std::uint8_t
        operator [](unsigned idx)
            const noexcept
        {
            switch (idx) {
            case 0:
                return r;
            case 1:
                return g;
            case 2:
                return b;
            case 3:
            default:
                return a;
            }
        }


        std::uint8_t&
        operator [](unsigned idx)
            noexcept
        {
            switch (idx) {
            case 0:
                return r;
            case 1:
                return g;
            case 2:
                return b;
            case 3:
            default:
                return a;
            }
        }


        constexpr
        bool
        operator ==(const color& other) const noexcept = default;


        std::strong_ordering
        operator <=>(const color& other) const noexcept = default;

    };


    std::string to_string(color c, bool with_alpha = true, bool uppercase = true);


} // namespace wups::config


#endif
