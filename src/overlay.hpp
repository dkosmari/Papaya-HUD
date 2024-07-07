// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OVERLAY_HPP
#define OVERLAY_HPP


namespace overlay {

    void initialize();
    void finalize();

    void create();
    void destroy();

    void reset();

    void on_frame();

    void render();

}


#endif
