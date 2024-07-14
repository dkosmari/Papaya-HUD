// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIME_MON_HPP
#define TIME_MON_HPP

namespace time_mon {

    void initialize();
    void finalize();
    void reset();
    const char* get_report(float dt);

}


#endif
