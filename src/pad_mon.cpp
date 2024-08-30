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

#include <array>
#include <atomic>
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

        // Check if shortcut was pressed
        if (holds_alternative<wups::config::vpad_combo>(cfg::toggle_shortcut)) {
            const auto& shortcut = get<wups::config::vpad_combo>(cfg::toggle_shortcut);
            for (int32_t idx = num_samples - 1; idx >= 0; --idx) {
                if (buf[idx].trigger & shortcut.buttons) {
                    if (buf[idx].hold == shortcut.buttons) {
                        overlay::toggle(); // user activated the shortcut
                        break;
                    }
                }
            }
        }

        if (cfg::button_rate) {
            // We only care from HOME to R stick (skip sync and emulated) buttons.
            const uint32_t buttons_begin = 0x000002;
            const uint32_t buttons_end   = 0x080000;

            unsigned counter = 0;
            for (int32_t idx = 0; idx < num_samples; ++idx)
                if (buf[idx].trigger)
                    for (auto button = buttons_begin; button < buttons_end; button <<= 1)
                        if (buf[idx].trigger & button)
                            ++counter;

            if (counter)
                button_presses += counter;
        }

        return result;
    }

    WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);


    constexpr auto wpad_core_button_list = {
        WPAD_BUTTON_LEFT,
        WPAD_BUTTON_RIGHT,
        WPAD_BUTTON_DOWN,
        WPAD_BUTTON_UP,
        WPAD_BUTTON_PLUS,
        WPAD_BUTTON_2,
        WPAD_BUTTON_1,
        WPAD_BUTTON_B,
        WPAD_BUTTON_A,
        WPAD_BUTTON_MINUS,
        WPAD_BUTTON_HOME,
    };

    constexpr auto wpad_nunchuk_button_list = {
        WPAD_NUNCHUK_BUTTON_Z,
        WPAD_NUNCHUK_BUTTON_C,
    };

    constexpr auto wpad_classic_button_list = {
        WPAD_CLASSIC_BUTTON_UP,
        WPAD_CLASSIC_BUTTON_LEFT,
        WPAD_CLASSIC_BUTTON_ZR,
        WPAD_CLASSIC_BUTTON_X,
        WPAD_CLASSIC_BUTTON_A,
        WPAD_CLASSIC_BUTTON_Y,
        WPAD_CLASSIC_BUTTON_B,
        WPAD_CLASSIC_BUTTON_ZL,
        WPAD_CLASSIC_BUTTON_R,
        WPAD_CLASSIC_BUTTON_PLUS,
        WPAD_CLASSIC_BUTTON_HOME,
        WPAD_CLASSIC_BUTTON_MINUS,
        WPAD_CLASSIC_BUTTON_L,
        WPAD_CLASSIC_BUTTON_DOWN,
        WPAD_CLASSIC_BUTTON_RIGHT,
    };

    constexpr auto wpad_pro_button_list = {
        WPAD_PRO_BUTTON_UP,
        WPAD_PRO_BUTTON_LEFT,
        WPAD_PRO_TRIGGER_ZR,
        WPAD_PRO_BUTTON_X,
        WPAD_PRO_BUTTON_A,
        WPAD_PRO_BUTTON_Y,
        WPAD_PRO_BUTTON_B,
        WPAD_PRO_TRIGGER_ZL,
        WPAD_PRO_RESERVED,
        WPAD_PRO_TRIGGER_R,
        WPAD_PRO_BUTTON_PLUS,
        WPAD_PRO_BUTTON_HOME,
        WPAD_PRO_BUTTON_MINUS,
        WPAD_PRO_TRIGGER_L,
        WPAD_PRO_BUTTON_DOWN,
        WPAD_PRO_BUTTON_RIGHT,
        WPAD_PRO_BUTTON_STICK_R,
        WPAD_PRO_BUTTON_STICK_L,
    };


    constexpr unsigned max_wiimotes = 7;

    array<uint16_t, max_wiimotes> wpad_core_buttons;
    array<uint16_t, max_wiimotes> wpad_nunchuk_buttons;
    array<uint16_t, max_wiimotes> wpad_classic_buttons;
    array<uint32_t, max_wiimotes> wpad_pro_buttons;

    constexpr uint16_t wpad_core_mask =
                  WPAD_BUTTON_LEFT |
                  WPAD_BUTTON_RIGHT |
                  WPAD_BUTTON_DOWN |
                  WPAD_BUTTON_UP |
                  WPAD_BUTTON_PLUS |
                  WPAD_BUTTON_2 |
                  WPAD_BUTTON_1 |
                  WPAD_BUTTON_B |
                  WPAD_BUTTON_A |
                  WPAD_BUTTON_MINUS |
                  WPAD_BUTTON_HOME;

    constexpr uint16_t wpad_nunchuk_mask =
                  WPAD_NUNCHUK_BUTTON_Z |
                  WPAD_NUNCHUK_BUTTON_C;

    constexpr uint16_t wpad_classic_mask =
                  WPAD_CLASSIC_BUTTON_UP |
                  WPAD_CLASSIC_BUTTON_LEFT |
                  WPAD_CLASSIC_BUTTON_ZR |
                  WPAD_CLASSIC_BUTTON_X |
                  WPAD_CLASSIC_BUTTON_A |
                  WPAD_CLASSIC_BUTTON_Y |
                  WPAD_CLASSIC_BUTTON_B |
                  WPAD_CLASSIC_BUTTON_ZL |
                  WPAD_CLASSIC_BUTTON_R |
                  WPAD_CLASSIC_BUTTON_PLUS |
                  WPAD_CLASSIC_BUTTON_HOME |
                  WPAD_CLASSIC_BUTTON_MINUS |
                  WPAD_CLASSIC_BUTTON_L |
                  WPAD_CLASSIC_BUTTON_DOWN |
                  WPAD_CLASSIC_BUTTON_RIGHT;


    bool
    just_pressed(uint32_t btn,
                 uint32_t prev,
                 uint32_t curr)
    {
        uint32_t masked_curr = btn & curr;
        if (!masked_curr)
            return false;
        uint32_t masked_prev = btn & prev;
        uint32_t diff = masked_curr ^ masked_prev;
        return masked_curr & diff;
    }


    void
    process_wpad_core_buttons(WPADChan chan,
                              const WPADStatus* status)
    {
        auto& prev_buttons = wpad_core_buttons[chan];

        // check if shortcut was pressed
        if (holds_alternative<wups::config::wpad_combo>(cfg::toggle_shortcut)) {
            const auto& shortcut = get<wups::config::wpad_combo>(cfg::toggle_shortcut);
            if (shortcut.ext == WPAD_EXT_CORE) {
                // At least one of the "just pressed" buttons must be in the shortcut.
                if (just_pressed(shortcut.core_buttons,
                                 prev_buttons,
                                 status->buttons)) {
                    // No other button must be held, only the shortcut.
                    if (shortcut.core_buttons == status->buttons)
                        overlay::toggle();
                }
            }
        }

        if (cfg::enabled && cfg::button_rate) {
            // count button presses
            unsigned counter = 0;
            for (auto button : wpad_core_button_list)
                if (just_pressed(button, prev_buttons, status->buttons)) {
                    // logger::printf("just pressed core %04x\n", button);
                    ++counter;
                }

            if (counter)
                button_presses += counter;
        }
    }


    void
    process_wpad_buttons(WPADChan chan,
                         const WPADStatus* status)
    {
        process_wpad_core_buttons(chan, status);
        wpad_core_buttons[chan] = status->buttons & wpad_core_mask;
    }


    void
    process_nunchuk_buttons(WPADChan chan,
                            const WPADNunchukStatus* status)
    {
        process_wpad_core_buttons(chan, &status->core);

        auto& prev_core_buttons = wpad_core_buttons[chan];
        auto& prev_buttons = wpad_nunchuk_buttons[chan];

        // check if shortcut was pressed
        if (holds_alternative<wups::config::wpad_combo>(cfg::toggle_shortcut)) {
            const auto& shortcut = get<wups::config::wpad_combo>(cfg::toggle_shortcut);
            if (shortcut.ext == WPAD_EXT_NUNCHUK
                || shortcut.ext == WPAD_EXT_MPLUS_NUNCHUK) {
                // At least one of the "just pressed" buttons must be in the shortcut.
                if (just_pressed(shortcut.core_buttons,
                                 prev_core_buttons,
                                 status->core.buttons)
                    || just_pressed(shortcut.ext_buttons,
                                    prev_buttons,
                                    status->core.buttons))
                    // No other button must be held, only the shortcut.
                    if (shortcut.core_buttons == (status->core.buttons & wpad_core_mask)
                        && shortcut.ext_buttons == (status->core.buttons & wpad_nunchuk_mask))
                        overlay::toggle();
            }
        }

        if (cfg::enabled && cfg::button_rate) {
            // count button presses
            unsigned counter = 0;
            for (auto button : wpad_nunchuk_button_list)
                if (just_pressed(button,
                                 prev_buttons,
                                 status->core.buttons))
                    ++counter;

            if (counter)
                button_presses += counter;
        }

        prev_core_buttons = status->core.buttons & wpad_core_mask;
        prev_buttons = status->core.buttons & wpad_nunchuk_mask;
    }


    void
    process_classic_buttons(WPADChan chan,
                            const WPADClassicStatus* status)
    {
        process_wpad_core_buttons(chan, &status->core);

        auto& prev_core_buttons = wpad_core_buttons[chan];
        auto& prev_buttons = wpad_classic_buttons[chan];

        // check if shortcut was pressed
        if (holds_alternative<wups::config::wpad_combo>(cfg::toggle_shortcut)) {
            const auto& shortcut = get<wups::config::wpad_combo>(cfg::toggle_shortcut);
            if (shortcut.ext == WPAD_EXT_CLASSIC
                || shortcut.ext == WPAD_EXT_MPLUS_CLASSIC) {
                // At least one of the "just pressed" buttons must be in the shortcut.
                if (just_pressed(shortcut.core_buttons,
                                 prev_core_buttons,
                                 status->core.buttons)
                    || just_pressed(shortcut.ext_buttons,
                                    prev_buttons,
                                    status->buttons))
                    // No other button must be held, only the shortcut.
                    if (shortcut.core_buttons == status->core.buttons
                        && shortcut.ext_buttons == status->buttons)
                        overlay::toggle();
            }
        }

        if (cfg::enabled && cfg::button_rate) {
            // count button presses
            unsigned counter = 0;
            for (auto button : wpad_classic_button_list)
                if (just_pressed(button, prev_buttons, status->buttons)) {
                    logger::printf("just pressed classic %04x\n", button);
                    ++counter;
                }

            if (counter)
                button_presses += counter;
        }

        prev_core_buttons = status->core.buttons & wpad_core_mask;
        prev_buttons = status->buttons & wpad_classic_mask;
    }


    void
    process_pro_buttons(WPADChan chan,
                        const WPADProStatus* status)
    {
        auto& prev_buttons = wpad_pro_buttons[chan];

        // check if shortcut was pressed
        if (holds_alternative<wups::config::wpad_combo>(cfg::toggle_shortcut)) {
            const auto& shortcut = get<wups::config::wpad_combo>(cfg::toggle_shortcut);
            if (shortcut.ext == WPAD_EXT_PRO_CONTROLLER) {
                // At least one of the "just pressed" buttons must be in the shortcut.
                if (just_pressed(shortcut.ext_buttons,
                                 prev_buttons,
                                 status->buttons))
                    // No other button must be held, only the shortcut.
                    if (shortcut.ext_buttons && status->buttons)
                        overlay::toggle();
            }
        }

        if (cfg::enabled && cfg::button_rate) {
            // count button presses
            unsigned counter = 0;
            for (auto button : wpad_pro_button_list)
                if (just_pressed(button, prev_buttons, status->buttons)) {
                    logger::printf("just pressed pro %04x\n", button);
                    ++counter;
                }

            if (counter)
                button_presses += counter;
        }

        prev_buttons = status->buttons;
    }


    DECL_FUNCTION(void,
                  WPADRead,
                  WPADChan chan,
                  WPADStatus *status)
    {
        real_WPADRead(chan, status);
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

        if (chan >= max_wiimotes)
            return;

        switch (status->extensionType) {

        case WPAD_EXT_CORE:
        case WPAD_EXT_MPLUS:
            process_wpad_buttons(chan, status);
            break;

        case WPAD_EXT_NUNCHUK:
        case WPAD_EXT_MPLUS_NUNCHUK:
            process_nunchuk_buttons(chan, reinterpret_cast<const WPADNunchukStatus*>(status));
            break;

        case WPAD_EXT_CLASSIC:
        case WPAD_EXT_MPLUS_CLASSIC:
            process_classic_buttons(chan, reinterpret_cast<const WPADClassicStatus*>(status));
            break;

        case WPAD_EXT_PRO_CONTROLLER:
            process_pro_buttons(chan, reinterpret_cast<const WPADProStatus*>(status));
            break;

        } // switch (status->extensionType)

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
