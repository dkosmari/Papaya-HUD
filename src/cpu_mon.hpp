/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CPU_MON_HPP
#define CPU_MON_HPP

namespace cpu_mon {

    void initialize();
    void finalize();
    void reset();
    const char* get_report(float dt);

}

#endif
