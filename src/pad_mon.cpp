/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
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

        // Don't bother doing anything else if the config menu is open.
        WUPSConfigAPIMenuStatus menu_status{};
        WUPSConfigAPI_Menu_GetStatus(&menu_status);
        if (menu_status == WUPSCONFIG_API_MENU_STATUS_OPENED)
            return result;


        if (cfg::enabled && cfg::button_rate) {

            // Note: when proc mode is loose, all button samples are identical to the most recent
            const int32_t num_samples = VPADGetButtonProcMode(channel) ? result : 1;

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
                  WPADStatus* status)
    {
        real_WPADRead(channel, status);

        if (status && status->error)
            return;

        // Don't bother doing anything else if the config menu is open.
        WUPSConfigAPIMenuStatus menu_status{};
        WUPSConfigAPI_Menu_GetStatus(&menu_status);
        if (menu_status == WUPSCONFIG_API_MENU_STATUS_OPENED)
            return;

#if 0
        // TODO: gotta redo the tracking for button presses.
        if (cfg::enabled && cfg::button_rate) {
            unsigned counter = 0;
            const auto& state = wups::utils::wpad::get_button_state(channel);
            counter += std::popcount(state.core.trigger);

            using wups::utils::wpad::nunchuk_button_state;
            if (auto* ext = std::get_if<nunchuk_button_state>(&state.ext))
                counter += std::popcount(ext->trigger);

            using wups::utils::wpad::classic_button_state;
            if (auto* ext = std::get_if<classic_button_state>(&state.ext))
                counter += std::popcount(ext->trigger);

            using wups::utils::wpad::pro_button_state;
            if (auto* ext = std::get_if<pro_button_state>(&state.ext))
                counter += std::popcount(ext->trigger);

            if (counter)
                button_presses += counter;
        }
#endif

    }

    WUPS_MUST_REPLACE(WPADRead, WUPS_LOADER_LIBRARY_PADSCORE, WPADRead);


} // namespace pad_mon
