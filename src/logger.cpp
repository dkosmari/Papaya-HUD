/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cstdarg>
#include <cstdio>               // vsnprintf()
#include <mutex>
#include <string>

#include <coreinit/dynload.h>
#include <whb/log.h>
#include <whb/log_module.h>

#include "logger.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


using namespace std::literals;


// Note: this wrapper around WHBLogModule*() allows for multiple initialize/finalize.
// Every initialize must be paired with a finalize.
namespace whb::log_module {

    std::mutex mut;
    unsigned refs = 0;

    void
    initialize()
    {
        std::lock_guard guard{mut};
        if (refs == 0)
            WHBLogModuleInit();
        ++refs;
    }

    void
    finalize()
    {
        std::lock_guard guard{mut};
        if (refs == 0)
            return; // must have called finalize() one too many times
        if (refs == 1) // last finalize()
            WHBLogModuleDeinit();
        --refs;
    }

} // namespace whb::log_module


namespace logger {

    std::mutex mut;


    void
    initialize()
    {
        whb::log_module::initialize();
    }


    void
    finalize()
    {
        whb::log_module::finalize();
    }


    void
    printf(const char* fmt, ...)
    {
        std::string buf(256, '\0');
        std::string xfmt = std::string("[" PACKAGE_NAME "] ") + fmt;

        std::va_list args;

        va_start(args, fmt);
        int sz = std::vsnprintf(buf.data(), buf.size(), xfmt.c_str(), args);
        va_end(args);

        if (sz > 0 && static_cast<unsigned>(sz) >= buf.size()) {
            buf.resize(sz + 1); // remember, room for null char

            va_start(args, fmt);
            std::vsnprintf(buf.data(), buf.size(), xfmt.c_str(), args);
            va_end(args);
        }

        if (sz > 0) {
            std::lock_guard guard{mut};
            WHBLogWrite(buf.c_str());
        }
    }


    guard::guard()
    {
        initialize();
    }


    guard::~guard()
    {
        finalize();
    }

} // namespace logger
