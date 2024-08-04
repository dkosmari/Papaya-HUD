// SPDX-License-Identifier: GPL-3.0-or-later

#include "duration.hpp"


namespace wups::config {


    template<>
    std::string
    to_string<std::chrono::seconds>(std::chrono::seconds s)
    {
        return std::to_string(s.count()) + "s";
    }

    template<>
    std::string
    to_string<std::chrono::milliseconds>(std::chrono::milliseconds s)
    {
        return std::to_string(s.count()) + "ms";
    }


}
