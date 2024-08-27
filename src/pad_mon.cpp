/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * Gamepad Monitoring
 */

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <coreinit/core.h>      // OSGetCoreId()
#include <gx2/state.h>          // GX2GetMainCoreId()
#include <vpad/input.h>
#include <wups.h>

#include "pad_mon.hpp"

#include "cfg.hpp"
#include "overlay.hpp"


namespace pad_mon {


    std::atomic_uint button_presses = 0;


    void
    initialize()
    {
        reset();
    }


    void
    finalize()
    {}


    void
    reset()
    {
        button_presses = 0;
    }


    const char*
    get_report(float dt)
    {
        static char buf[64];

        const unsigned presses = std::atomic_exchange(&button_presses, 0u);

        const float presses_rate = presses / dt;

        std::snprintf(buf, sizeof buf,
                      "%.1f bps",
                      presses_rate);

        return buf;
    }


    DECL_FUNCTION(std::int32_t, VPADRead,
                  VPADChan channel,
                  VPADStatus* buf,
                  std::uint32_t count,
                  VPADReadError* error)
    {
        auto result = real_VPADRead(channel, buf, count, error);
        if (result <= 0)
            return result;
        if (error && *error != VPAD_READ_SUCCESS)
            return result;

        // We only care from HOME to R stick (skip sync and emulated) buttons.
        const std::uint32_t buttons_begin = 0x000002;
        const std::uint32_t buttons_end   = 0x080000;

        unsigned counter = 0;
        if (cfg::button_rate) {
            // TODO: handle Twilight Princess input
            if (buf[0].trigger) {
                for (auto button = buttons_begin; button < buttons_end; button <<= 1)
                    if (buf[0].trigger & button)
                        ++counter;
            }
            // only touch the atomic counter if there's someting to count
            if (counter)
                button_presses += counter;
        }


        // Handle shortcut to toggle the HUD, make sure it's in the right core
        std::uint32_t my_core = OSGetCoreId();
        std::uint32_t gx2_core = GX2GetMainCoreId();
        if (gx2_core == my_core) {
            if (holds_alternative<wups::config::vpad_combo>(cfg::toggle_shortcut)) {
                auto& shortcut = get<wups::config::vpad_combo>(cfg::toggle_shortcut);
                // TODO: handle Twilight Princess input
                if (buf[0].trigger & shortcut.buttons) {
                    if ((buf[0].hold & shortcut.buttons) == shortcut.buttons) {
                        // user activated the shortcut
                        cfg::enabled = !cfg::enabled;
                        if (cfg::enabled)
                            overlay::create_or_reset();
                        else
                            overlay::destroy();
                    }
                }
            }
        } // running in the correct core

        return result;
    }


    WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);

} // namespace pad_mon
