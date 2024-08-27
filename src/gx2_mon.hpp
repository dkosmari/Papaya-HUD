/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef GX2_MON_HPP
#define GX2_MON_HPP


namespace gx2_mon {

    namespace perf {
        const char* get_report(float dt);
    }

    namespace fps {
        const char* get_report(float dt);
    }

    void initialize();
    void finalize();
    void reset();

    void on_application_start();
    void on_application_ends();

}

#endif
