/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef GX2_MON_HPP
#define GX2_MON_HPP

#include "out_span.hpp"


namespace gx2_mon {

    namespace perf {

        void
        get_report(out_span& out,
                   float dt);

    } // namespace perf

    namespace fps {

        void
        get_report(out_span&,
                   float dt);

    } // namespace fps


    void
    initialize();

    void
    finalize();

    void
    reset();

    void
    on_application_start();

    void
    on_application_ends();

} // namespace gx2_mon

#endif
