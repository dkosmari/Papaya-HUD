// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FS_MON_HPP
#define FS_MON_HPP

namespace fs_mon {

    void initialize();
    void finalize();
    void reset();
    const char* get_report(float dt);

}


#endif
