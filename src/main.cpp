/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <optional>

#include <wups.h>

#include <wupsxx/logger.hpp>
#include <wupsxx/shortcut.hpp>

#include "cfg.hpp"
#include "gx2_mon.hpp"
#include "overlay.hpp"
#include "time_mon.hpp"

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


std::optional<wups::logger::guard> app_log_guard;


INITIALIZE_PLUGIN()
{
    wups::logger::set_prefix(PACKAGE_NAME);
    wups::logger::guard log_guard;

    try {
        wups::shortcut::initialize(PACKAGE_NAME);

        cfg::initialize();
        overlay::initialize();
    }
    catch (std::exception& e) {
        wups::logger::printf("init error: %s\n", e.what());
    }
}


DEINITIALIZE_PLUGIN()
{
    wups::logger::guard log_guard;

    cfg::finalize();
    overlay::finalize();

    wups::shortcut::finalize();
}


ON_APPLICATION_START()
{
    app_log_guard.emplace();
    gx2_mon::on_application_start();
    time_mon::on_application_start();
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
