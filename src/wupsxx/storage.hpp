// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WUPSXX_STORAGE_HPP
#define WUPSXX_STORAGE_HPP

#include <expected>
#include <string>
#include <utility>

#include <wups/storage.h>

#include "storage_error.hpp"
#include "concepts.hpp"


namespace wups::storage {

    template<typename T>
    std::expected<T, storage_error>
    load(const std::string& key)
    {
        T value;
        auto status = WUPSStorageAPI::Get(key,
                                          value,
                                          WUPSStorageAPI::GetOptions::RESIZE_EXISTING_BUFFER);
        if (status != WUPS_STORAGE_ERROR_SUCCESS)
            return std::unexpected{storage_error{"error loading key \"" + key + "\"",
                                                 status}};
        return value;
    }


    template<concepts::duration T>
    std::expected<T, storage_error>
    load(const std::string& key)
    {
        auto value = load<int>(key);
        if (!value)
            return std::unexpected{value.error()};
        return T{*value};
    }


    template<typename T>
    void
    store(const std::string& key, const T& value)
    {
        auto status = WUPSStorageAPI::Store(key, value);
        if (status != WUPS_STORAGE_ERROR_SUCCESS)
            throw storage_error{"error storing key \"" + key + "\"",
                                status};
    }


    template<concepts::duration T>
    void
    store(const std::string& key, const T& value)
    {
        return store<int>(key, value.count());
    }



    // This will either load the variable from the config, or initialize
    // it (and the config) with the default value.
    template<typename T,
             typename U>
    void
    load_or_init(const std::string& key,
                 T& variable,
                 U&& init)
    {
        auto status = load<T>(key);
        if (status)
            variable = std::move(*status);
        else {
            if (status.error().code != WUPS_STORAGE_ERROR_NOT_FOUND)
                throw status.error();
            variable = std::forward<U>(init);
            store(key, variable);
        }
    }


    // same as above, but ensure storage is in string format
    template<typename T,
             typename U>
    void
    load_or_init_str(const std::string& key,
                     T& variable,
                     U&& init,
                     const std::string& init_str)
    {
        auto status = load<std::string>(key);
        if (status)
            variable = T{std::move(*status)};
        else {
            if (status.error().code != WUPS_STORAGE_ERROR_NOT_FOUND)
                throw status.error();
            variable = std::forward<U>(init);
            store(key, init_str);
        }
    }


    void save();

    void reload();

} // namespace wups::storage

#endif
