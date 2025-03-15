/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FS_MON_HPP
#define FS_MON_HPP

#include <string>


namespace fs_mon {

    void initialize();
    void finalize();
    void reset();
    std::string get_report(float dt);

}

#endif
