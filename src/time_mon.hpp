/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TIME_MON_HPP
#define TIME_MON_HPP

#include "out_span.hpp"


namespace time_mon {

    void
    initialize();

    void
    finalize();

    void
    reset();

    void
    on_application_start();

    void
    get_report(out_span& out,
               float dt);

} // namespace time_mon

#endif
