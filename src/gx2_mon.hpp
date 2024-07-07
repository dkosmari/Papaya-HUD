// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GX2_MON_HPP
#define GX2_MON_HPP


namespace gx2_mon {


    void initialize();

    void finalize();


    const char* get_report(float dt);

}

#endif
