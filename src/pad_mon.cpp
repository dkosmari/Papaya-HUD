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


    // Simple class to track buttons triggered and released.
    template<typename T>
    struct button_tracker {

        T hold    = 0;
        T trigger = 0;
        T release = 0;


        void
        reset()
            noexcept
        {
            hold    = 0;
            trigger = 0;
            release = 0;
        }


        void
        update(T buttons)
            noexcept
        {
            T changed = hold ^ buttons;
            hold      = buttons;
            trigger   = changed &  buttons;
            release   = changed & ~buttons;
        }

    };


    struct wiimote_state_t {
        button_tracker<uint16_t> core;
        button_tracker<uint32_t> ext;
        uint8_t ext_type = WPAD_EXT_CORE;

        void
        reset()
            noexcept
        {
            core.reset();
            ext.reset();
        }

        void
        update_ext_type(uint8_t et)
            noexcept
        {
            if (ext_type != et) {
                ext.reset();
                ext_type = et;
            }
        }
    };


    std::array<wiimote_state_t, 7> wiimote_states;


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

        if (!cfg::enabled.value || !cfg::button_rate.value)
            return result;

        // Don't bother doing anything else if the config menu is open.
        WUPSConfigAPIMenuStatus menu_status{};
        WUPSConfigAPI_Menu_GetStatus(&menu_status);
        if (menu_status == WUPSCONFIG_API_MENU_STATUS_OPENED)
            return result;

        // Note: when proc mode is loose, all button samples are identical to the most recent.
        bool is_loose = !VPADGetButtonProcMode(channel);
        int num_samples = is_loose ? 1 : result;

        unsigned counter = 0;
        for (int idx = num_samples - 1; idx >= 0; --idx)
            counter += std::popcount(buf[idx].trigger & vpad_mask);

        if (counter)
            button_presses += counter;

        return result;
    }

    WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);


    DECL_FUNCTION(void,
                  WPADRead,
                  WPADChan channel,
                  WPADStatus* status)
    {
        real_WPADRead(channel, status);

        if (!status)
            return;
        if (status->error)
            return;
        if (!cfg::enabled.value || !cfg::button_rate.value)
            return;

        // Don't bother doing anything else if the config menu is open.
        WUPSConfigAPIMenuStatus menu_status{};
        WUPSConfigAPI_Menu_GetStatus(&menu_status);
        if (menu_status == WUPSCONFIG_API_MENU_STATUS_OPENED)
            return;

        if (channel < 0 || channel >= wiimote_states.size())
            return;

        auto& wiimote = wiimote_states[channel];

        wiimote.update_ext_type(status->extensionType);

        using std::popcount;
        unsigned counter = 0;

        switch (status->extensionType) {
            case WPAD_EXT_CORE:
            case WPAD_EXT_MPLUS:
            case WPAD_EXT_NUNCHUK:
            case WPAD_EXT_MPLUS_NUNCHUK:
                wiimote.core.update(status->buttons);
                counter += popcount(wiimote.core.trigger);
                break;

            case WPAD_EXT_CLASSIC:
            case WPAD_EXT_MPLUS_CLASSIC:
                wiimote.core.update(status->buttons);
                wiimote.ext.update(reinterpret_cast<WPADStatusClassic*>(status)->buttons);
                counter += popcount(wiimote.core.trigger);
                counter += popcount(wiimote.ext.trigger);
                break;

            case WPAD_EXT_PRO_CONTROLLER:
                wiimote.ext.update(reinterpret_cast<WPADStatusPro*>(status)->buttons);
                counter += popcount(wiimote.ext.trigger);
                break;
        }

        if (counter)
            button_presses += counter;

    }

    WUPS_MUST_REPLACE(WPADRead, WUPS_LOADER_LIBRARY_PADSCORE, WPADRead);

} // namespace pad_mon
