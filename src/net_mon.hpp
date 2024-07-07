// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NET_MON_HPP
#define NET_MON_HPP

namespace net_mon {

    void initialize();

    void finalize();


    const char* get_report(float dt);


}

#endif
