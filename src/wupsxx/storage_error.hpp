// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WUPSXX_STORAGE_ERROR_HPP
#define WUPSXX_STORAGE_ERROR_HPP

#include <stdexcept>
#include <string>

#include <wups/storage.h>


namespace wups::storage {

    struct storage_error : std::runtime_error {

        WUPSStorageError code;

        storage_error(const std::string& msg, WUPSStorageError status);

    };

} // namespace wups::storage

#endif
