/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <wups.h>

#include "cfg.hpp"
#include "logging.hpp"
#include "overlay.hpp"

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


INITIALIZE_PLUGIN()
{
    logging::initialize();

    cfg::init();

    overlay::initialize();
}


DEINITIALIZE_PLUGIN()
{
    overlay::finalize();
    logging::finalize();
}


ON_APPLICATION_REQUESTS_EXIT()
{
    // logging::printf("ON_APPLICATION_REQUESTS_EXIT\n");
    overlay::destroy();
}


ON_ACQUIRED_FOREGROUND()
{
    // logging::printf("ON_ACQUIRED_FOREGROUND\n");
    overlay::on_acquired_foreground();
}


ON_RELEASE_FOREGROUND()
{
    // logging::printf("ON_RELEASE_FOREGROUND\n");
    overlay::on_release_foreground();
}
