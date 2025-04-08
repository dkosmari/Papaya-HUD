/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef PAD_MON_HPP
#define PAD_MON_HPP

#include "out_span.hpp"


namespace pad_mon {

    void
    initialize();

    void
    finalize();

    void
    reset();

    void
    get_report(out_span& out,
               float dt);

} // namespace pad_mon

#endif
