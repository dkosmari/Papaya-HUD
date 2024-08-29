/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef OVERLAY_HPP
#define OVERLAY_HPP


namespace overlay {


    extern bool gx2_init;


    void initialize();
    void finalize();

    void create_or_reset();
    void destroy();
    void reset();


    void on_acquired_foreground();

    void on_release_foreground();


    void render();


    void toggle();
    void process_toggle_request_from_gx2();
}


#endif
