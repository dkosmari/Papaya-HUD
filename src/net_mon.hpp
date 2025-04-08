/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NET_MON_HPP
#define NET_MON_HPP

#include "out_span.hpp"


namespace net_mon {

    void
    initialize();

    void
    finalize();

    void
    reset();

    void
    get_report(out_span& out,
               float dt);

}

#endif
