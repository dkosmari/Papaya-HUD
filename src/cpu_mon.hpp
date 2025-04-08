/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CPU_MON_HPP
#define CPU_MON_HPP

#include "out_span.hpp"


namespace cpu_mon {

    void
    initialize();

    void
    finalize();

    void
    reset();

    void
    get_report(out_span& out,
               float dt);

} // namespace cpu_mon

#endif
