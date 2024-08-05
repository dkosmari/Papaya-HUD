/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef COS_MON_HPP
#define COS_MON_HPP

namespace cos_mon {

    void initialize();
    void finalize();
    void reset();
    const char* get_report(float dt);

}

#endif
