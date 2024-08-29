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

#if 0
        // Waiting on https://github.com/wiiu-env/WiiUPluginSystem/pull/76

        // Don't bother doing anything else if the config menu is open.
        BOOL isMenuOpen = false;
        WUPSConfigAPI_GetMenuOpen(&isMenuOpen);
        if (isMenuOpen)
            return result;
#endif

        // Note: when proc mode is loose, all samples ar identical, so we just check one
        const std::int32_t num_samples = VPADGetButtonProcMode(channel) ? result : 1;

        // Check if shortcut was pressed
        if (holds_alternative<wups::config::vpad_combo>(cfg::toggle_shortcut)) {
            auto& shortcut = get<wups::config::vpad_combo>(cfg::toggle_shortcut);
            for (std::int32_t idx = 0; idx < num_samples; ++idx) {
                if (buf[idx].trigger & shortcut.buttons) {
                    if ((buf[idx].hold & shortcut.buttons) == shortcut.buttons) {
                        overlay::toggle(); // user activated the shortcut
                        break;
                    }
                }
            }
        }

        if (cfg::button_rate) {
            // We only care from HOME to R stick (skip sync and emulated) buttons.
            const std::uint32_t buttons_begin = 0x000002;
            const std::uint32_t buttons_end   = 0x080000;

            unsigned counter = 0;
            for (std::int32_t idx = 0; idx < num_samples; ++idx)
                if (buf[idx].trigger)
                    for (auto button = buttons_begin; button < buttons_end; button <<= 1)
                        if (buf[idx].trigger & button)
                            ++counter;

            // only touch the atomic counter if there's someting to count
            if (counter)
                button_presses += counter;
        }

        return result;
    }

    WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);

} // namespace pad_mon
