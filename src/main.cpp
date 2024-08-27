/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <optional>

#include <coreinit/memdefaultheap.h> // DEBUG

#include <wups.h>

#include "cfg.hpp"
#include "gx2_mon.hpp"
#include "logger.hpp"
#include "overlay.hpp"

#include "coreinit_allocator.h" // DEBUG

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


WUPS_PLUGIN_NAME(PACKAGE_NAME);
WUPS_PLUGIN_VERSION(PACKAGE_VERSION);
WUPS_PLUGIN_DESCRIPTION("Show a HUD.");
WUPS_PLUGIN_AUTHOR("Daniel K. O.");
WUPS_PLUGIN_LICENSE("GPLv3+");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PACKAGE);


std::optional<logger::guard> app_log_guard;


INITIALIZE_PLUGIN()
{
    logger::guard log_guard;

    cfg::init();
    overlay::initialize();


    // MEMAllocator test_alloc;
    // MEMInitAllocatorForDefaultHeap(&test_alloc);

    // void* ptr;

    // ptr = MEMAllocFromAllocator(&test_alloc, 16);
    // logger::printf("[1] ptr = %p\n", ptr);
    // MEMFreeToAllocator(&test_alloc, ptr);

    // ptr = MEMAllocFromDefaultHeap(16);
    // logger::printf("[2] ptr = %p\n", ptr);
    // MEMFreeToDefaultHeap(ptr);
}


DEINITIALIZE_PLUGIN()
{
    logger::guard log_guard;

    overlay::finalize();
    logger::finalize();
}


ON_APPLICATION_START()
{
    app_log_guard.emplace();
    gx2_mon::on_application_start();
}


ON_APPLICATION_REQUESTS_EXIT()
{
    overlay::destroy();
}


ON_APPLICATION_ENDS()
{
    gx2_mon::on_application_ends();
    app_log_guard.reset();
}


ON_ACQUIRED_FOREGROUND()
{
    overlay::on_acquired_foreground();
}


ON_RELEASE_FOREGROUND()
{
    overlay::on_release_foreground();
}
