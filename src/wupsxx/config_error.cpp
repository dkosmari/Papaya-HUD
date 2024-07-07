// SPDX-License-Identifier: GPL-3.0-or-later

#include <wups/config_api.h>

#include "config_error.hpp"


using namespace std::literals;


namespace wups::config {

    config_error::config_error(WUPSConfigAPIStatus status,
                               const std::string& msg) :
        std::runtime_error{msg + ": "s + WUPSConfigAPI_GetStatusStr(status)}
    {}

} // namespace wups::config
