// SPDX-License-Identifier: GPL-3.0-or-later

#include <wups/storage.h>

#include "storage.hpp"


namespace wups::storage {


    template<>
    std::expected<config::color, storage_error>
    load<config::color>(const std::string& key)
    {
        auto res = load<std::string>(key);
        if (!res)
            return std::unexpected{res.error()};
        return config::color{*res};
    }


    void
    store(const std::string& key, const config::color& c)
    {
        store<std::string>(key, to_string(c));
    }


    void
    save()
    {
        auto status = WUPSStorageAPI::SaveStorage();
        if (status != WUPS_STORAGE_ERROR_SUCCESS)
            throw storage_error{"error saving storage", status};
    }


    void
    reload()
    {
        auto status = WUPSStorageAPI::ForceReloadStorage();
        if (status != WUPS_STORAGE_ERROR_SUCCESS)
            throw storage_error{"error reloading storage", status};
    }


} // namespace wups::storage
