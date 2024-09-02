/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * Gamepad/Wiimote Monitoring
 */

#include <array>
#include <atomic>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <coreinit/thread.h>
#include <padscore/kpad.h>
#include <padscore/wpad.h>
#include <vpad/input.h>
#include <wups.h>

#include "pad_mon.hpp"

#include "cfg.hpp"
#include "logger.hpp"
#include "overlay.hpp"
#include "wpad_status.h"


using std::array;
using std::int32_t;
using std::uint32_t;
using std::uint16_t;


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


    constexpr uint32_t vpad_mask =
        VPAD_BUTTON_UP      | VPAD_BUTTON_DOWN    |
        VPAD_BUTTON_LEFT    | VPAD_BUTTON_RIGHT   |
        VPAD_BUTTON_L       | VPAD_BUTTON_R       |
        VPAD_BUTTON_ZL      | VPAD_BUTTON_ZR      |
        VPAD_BUTTON_A       | VPAD_BUTTON_B       |
        VPAD_BUTTON_X       | VPAD_BUTTON_Y       |
        VPAD_BUTTON_PLUS    | VPAD_BUTTON_MINUS   |
        VPAD_BUTTON_HOME    | VPAD_BUTTON_TV      |
        VPAD_BUTTON_STICK_L | VPAD_BUTTON_STICK_R |
        VPAD_BUTTON_SYNC;


    DECL_FUNCTION(int32_t, VPADRead,
                  VPADChan channel,
                  VPADStatus* buf,
                  uint32_t count,
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

        // Note: when proc mode is loose, all button samples are identical to the most recent
        const int32_t num_samples = VPADGetButtonProcMode(channel) ? result : 1;

        // Check for shortcut activation.
        for (int32_t idx = num_samples - 1; idx >= 0; --idx) {
            if (wups::config::vpad_update(channel, buf[idx])) {
                if (wups::config::vpad_triggered(channel, cfg::toggle_shortcut))
                    overlay::toggle();
            }
        }


        if (cfg::enabled && cfg::button_rate) {
            // We only care from HOME to R stick (skip sync and emulated) buttons.

            unsigned counter = 0;
            for (int32_t idx = num_samples - 1; idx >= 0; --idx)
                counter += std::popcount(buf[idx].trigger & vpad_mask);

            if (counter)
                button_presses += counter;
        }

        return result;
    }

    WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);


    DECL_FUNCTION(void,
                  WPADRead,
                  WPADChan channel,
                  WPADStatus *status)
    {
        real_WPADRead(channel, status);
        if (status->error)
            return;

#if 0
        // Waiting on https://github.com/wiiu-env/WiiUPluginSystem/pull/76

        // Don't bother doing anything else if the config menu is open.
        BOOL isMenuOpen = false;
        WUPSConfigAPI_GetMenuOpen(&isMenuOpen);
        if (isMenuOpen)
            return;
#endif

        if (wups::config::wpad_update(channel, status)) {

            if (wups::config::wpad_triggered(channel, cfg::toggle_shortcut))
                overlay::toggle();

            if (cfg::enabled && cfg::button_rate) {
                unsigned counter = 0;
                const auto& state = wups::config::wpad_get_button_state(channel);
                counter += std::popcount(state.core.trigger);

                using wups::config::wpad_nunchuk_button_state;
                if (auto* ext = std::get_if<wpad_nunchuk_button_state>(&state.ext))
                    counter += std::popcount(ext->trigger);

                using wups::config::wpad_classic_button_state;
                if (auto* ext = std::get_if<wpad_classic_button_state>(&state.ext))
                    counter += std::popcount(ext->trigger);

                using wups::config::wpad_pro_button_state;
                if (auto* ext = std::get_if<wpad_pro_button_state>(&state.ext))
                    counter += std::popcount(ext->trigger);

                if (counter)
                    button_presses += counter;
            }
        }

    }

    WUPS_MUST_REPLACE(WPADRead, WUPS_LOADER_LIBRARY_PADSCORE, WPADRead);


#if 0
    // TODO: Trine seems to peek directly into the ring buffer, gotta figure out how to
    // use these.

    DECL_FUNCTION(void,
                  WPADSetAutoSamplingBuf,
                  WPADChan chan,
                  void* buf,
                  uint32_t length)
    {
        logger::printf("called WPADSetAutoSamplingBuf(%d, %p, %u)\n",
                       chan,
                       buf,
                       length);
        real_WPADSetAutoSamplingBuf(chan, buf, length);
    }

    WUPS_MUST_REPLACE(WPADSetAutoSamplingBuf, WUPS_LOADER_LIBRARY_PADSCORE,
                      WPADSetAutoSamplingBuf);


    DECL_FUNCTION(WPADSamplingCallback,
                  WPADSetSamplingCallback,
                  WPADChan chan,
                  WPADSamplingCallback callback)
    {
        logger::printf("called WPADSetSamplingCallback(%d, %p)\n",
                       chan,
                       callback);
        return real_WPADSetSamplingCallback(chan, callback);
    }

    WUPS_MUST_REPLACE(WPADSetSamplingCallback, WUPS_LOADER_LIBRARY_PADSCORE,
                      WPADSetSamplingCallback);
#endif


} // namespace pad_mon
