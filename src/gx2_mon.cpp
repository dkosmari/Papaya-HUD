// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>

#include <wups.h>

#include "gx2_mon.hpp"

#include "cfg.hpp"
#include "overlay.hpp"


namespace gx2_mon {

    unsigned frame_counter = 0;


    void
    initialize()
    {
        frame_counter = 0;
    }


    void
    finalize()
    {}


    const char*
    get_report(float dt)
    {
        static char buf[32];

        const float fps = frame_counter / dt;
        frame_counter = 0;

        std::snprintf(buf, sizeof buf, "%04.1f fps", fps);
        return buf;
    }

}


DECL_FUNCTION(void, GX2SwapScanBuffers, void)
{
    if (cfg::enabled)
        overlay::render();

    ++gx2_mon::frame_counter;
    real_GX2SwapScanBuffers();
}


WUPS_MUST_REPLACE(GX2SwapScanBuffers, WUPS_LOADER_LIBRARY_GX2, GX2SwapScanBuffers);
