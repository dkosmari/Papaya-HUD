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
#include <nsysccr/cdc.h>
#include <wups.h>

#include <wupsxx/cafe_glyphs.h>
#include <wupsxx/logger.hpp>

#include "pad_mon.hpp"

#include "cfg.hpp"
#include "overlay.hpp"
#include "utils.hpp"


using std::array;
using std::int32_t;
using std::uint32_t;
using std::uint16_t;


namespace logger = wups::logger;


namespace pad_mon {

    enum VPADBatteryLevel : unsigned {
        VPAD_BATTERY_CHARGING,
        VPAD_BATTERY_LEVEL_EMPTY,
        VPAD_BATTERY_LEVEL_0, // no bars
        VPAD_BATTERY_LEVEL_1,
        VPAD_BATTERY_LEVEL_2,
        VPAD_BATTERY_LEVEL_3,
        VPAD_BATTERY_LEVEL_4, // max bars
    };


    enum WPADBatteryLevel : unsigned {
        WPAD_BATTERY_LEVEL_0, // no bars
        WPAD_BATTERY_LEVEL_1,
        WPAD_BATTERY_LEVEL_2,
        WPAD_BATTERY_LEVEL_3,
        WPAD_BATTERY_LEVEL_4, // max bars

        // hack to easily handle Pro Controller
        PRO_BATTERY_CHARGING = 0x100,
        PRO_BATTERY_WIRED    = 0x200,
    };


    const char*
    vpad_charge_to_bar(unsigned level)
        noexcept
    {
        switch (level) {
            case VPAD_BATTERY_CHARGING:
                return "C";
            case VPAD_BATTERY_LEVEL_EMPTY:
                return "X";
            case VPAD_BATTERY_LEVEL_0:
                return "\u3000";
            case VPAD_BATTERY_LEVEL_1:
                return "▂";
            case VPAD_BATTERY_LEVEL_2:
                return "▄";
            case VPAD_BATTERY_LEVEL_3:
                return "▆";
            case VPAD_BATTERY_LEVEL_4:
                return "█";
            default:
                return "?";
        }
    }


    const char*
    vpad_charge_to_percent(unsigned level)
        noexcept
    {
        switch (level) {
            case VPAD_BATTERY_CHARGING:
                return "C";
            case VPAD_BATTERY_LEVEL_EMPTY:
                return "X";
            case VPAD_BATTERY_LEVEL_0:
                return "0%";
            case VPAD_BATTERY_LEVEL_1:
                return "25%";
            case VPAD_BATTERY_LEVEL_2:
                return "50%";
            case VPAD_BATTERY_LEVEL_3:
                return "75%";
            case VPAD_BATTERY_LEVEL_4:
                return "100%";
            default:
                return "?";
        }
    }


    const char*
    wpad_charge_to_bar(unsigned level)
        noexcept
    {
        switch (level) {
            case WPAD_BATTERY_LEVEL_0:
                return "\u3000";
            case WPAD_BATTERY_LEVEL_1:
                return "▂";
            case WPAD_BATTERY_LEVEL_2:
                return "▄";
            case WPAD_BATTERY_LEVEL_3:
                return "▆";
            case WPAD_BATTERY_LEVEL_4:
                return "█";
            default:
                if (level & PRO_BATTERY_CHARGING)
                    return "C";
                if (level & PRO_BATTERY_WIRED)
                    return "W";
                return "?";
        }
    }


    const char*
    wpad_charge_to_percent(unsigned level)
        noexcept
    {
        switch (level) {
            case WPAD_BATTERY_LEVEL_0:
                return "0%";
            case WPAD_BATTERY_LEVEL_1:
                return "25%";
            case WPAD_BATTERY_LEVEL_2:
                return "50%";
            case WPAD_BATTERY_LEVEL_3:
                return "75%";
            case WPAD_BATTERY_LEVEL_4:
                return "100%";
            default:
                if (level & PRO_BATTERY_CHARGING)
                    return "C";
                if (level & PRO_BATTERY_WIRED)
                    return "W";
                return "?";
        }
    }


    std::array<unsigned, 7> pro_power;


    alignas(0x20)
    std::atomic_uint button_presses = 0;

    struct alignas(0x20) vpad_state_t {
        std::atomic_uint battery = 0;
        std::atomic_bool attached = false;

        void
        reset()
            noexcept
        {
            battery = 0;
            attached = false;
        }
    };

    std::array<vpad_state_t, 2> vpad_states;


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


    struct alignas(0x20) wpad_state_t {
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


    std::array<wpad_state_t, 7> wpad_states;


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
        for (auto& vpad : vpad_states)
            vpad.reset();
        for (auto& wpad : wpad_states)
            wpad.reset();
    }


    void
    get_report(out_span& out,
               float dt)
    {
        const char* separator = "";

        if (cfg::button_rate.value) {
            unsigned presses = std::atomic_exchange(&button_presses, 0u);
            float presses_rate = presses / dt;
            out.printf("%.1f bps", presses_rate);
            separator = utils::field_separator;
        }

        if (cfg::battery.value) {

            out.append(separator);
            out.append("bat:");

            const char* icon = CAFE_GLYPH_GAMEPAD;

            separator = " ";
            for (unsigned i = 0; i < 2; ++i) {
                if (vpad_states[i].attached.load(std::memory_order::acquire)) {
                    unsigned battery = vpad_states[i].battery.load(std::memory_order::relaxed);
                    out.append(separator);
                    out.append(icon);
                    if (cfg::battery_percent.value)
                        out.append(vpad_charge_to_percent(battery));
                    else
                        out.append(vpad_charge_to_bar(battery));
                    // logger::printf("vpad%u: %u\n", i, battery);
                }
            }

            icon = CAFE_GLYPH_WIIMOTE;
            for (unsigned i = 0; i < 7; ++i) {
                auto channel = static_cast<WPADChan>(i);
                WPADExtensionType ext;
                if (WPADProbe(channel, &ext))
                    continue;

                unsigned battery = WPADGetBatteryLevel(channel);

                if (ext == WPAD_EXT_PRO_CONTROLLER)
                    if (pro_power[channel])
                        battery = pro_power[channel];

                out.append(separator);
                out.append(icon);
                if (cfg::battery_percent.value)
                    out.append(wpad_charge_to_percent(battery));
                else
                    out.append(wpad_charge_to_bar(battery));
                // logger::printf("wpad%u: %u\n", i, battery);
            }

        }
    }


    // Note: we use this mask to ignore emulated buttons
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

        if (error && *error == VPAD_READ_INVALID_CONTROLLER)
            if (channel >= 0 && channel < 2)
                vpad_states[channel].attached.store(false,
                                                    std::memory_order::release);

        if (result <= 0)
            return result;

        if (error && *error != VPAD_READ_SUCCESS)
            return result;

        if (!cfg::enabled.value)
            return result;

        vpad_states[channel].attached.store(true, std::memory_order::release);

        // Don't bother doing anything else if the config menu is open.
        WUPSConfigAPIMenuStatus menu_status{};
        WUPSConfigAPI_Menu_GetStatus(&menu_status);
        if (menu_status == WUPSCONFIG_API_MENU_STATUS_OPENED)
            return result;

        if (cfg::button_rate.value) {
            // Note: when proc mode is loose, all button samples are identical to the most recent.
            bool is_loose = !VPADGetButtonProcMode(channel);
            int num_samples = is_loose ? 1 : result;

            unsigned counter = 0;
            for (int idx = num_samples - 1; idx >= 0; --idx)
                counter += std::popcount(buf[idx].trigger & vpad_mask);

            if (counter)
                button_presses.fetch_add(counter, std::memory_order::relaxed);
        }

        if (cfg::battery.value)
            vpad_states[channel].battery.store(buf[0].battery, std::memory_order::relaxed);

        return result;
    }

    WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);


    DECL_FUNCTION(void, __VPADBASEAttachCallback,
                  CCRCDCRegisterCallbackData *data,
                  unsigned status)
    {
        real___VPADBASEAttachCallback(data, status);
        if (data)
#if 1
            vpad_states[data->chan].attached.store(!!status, std::memory_order::release);
#else
            vpad_states[data->chan].attached.store(!!data->attached, std::memory_order::release);
#endif
    }

    WUPS_MUST_REPLACE_PHYSICAL(__VPADBASEAttachCallback,
                               (0x0200146c + 0x31000000 - 0xee0100),
                               (0x0200146c - 0x00000000 - 0xee0100));


    DECL_FUNCTION(void, WPADRead,
                  WPADChan channel,
                  WPADStatus* status)
    {
        real_WPADRead(channel, status);

        if (!status)
            return;
        if (status->error)
            return;
        if (!cfg::enabled.value)
            return;

        // Don't bother doing anything else if the config menu is open.
        WUPSConfigAPIMenuStatus menu_status{};
        WUPSConfigAPI_Menu_GetStatus(&menu_status);
        if (menu_status == WUPSCONFIG_API_MENU_STATUS_OPENED)
            return;

        if (channel < 0 || channel >= wpad_states.size())
            return;

        if (cfg::button_rate.value) {

            auto& wiimote = wpad_states[channel];

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
                button_presses.fetch_add(counter, std::memory_order::relaxed);

        }

        if (cfg::battery.value) {
            // The only way to track the charging/wired status of Pro Controller is to
            // peek into the WPADStatusProController fields.
            if (status->extensionType == WPAD_EXT_PRO_CONTROLLER) {
                auto pro_status = reinterpret_cast<const WPADStatusProController*>(status);
                unsigned flags = 0;
                if (pro_status->charging)
                    flags |= PRO_BATTERY_CHARGING;
                if (pro_status->wired)
                    flags |= PRO_BATTERY_WIRED;
                pro_power[channel] = flags;
            }
        }
    }

    WUPS_MUST_REPLACE(WPADRead, WUPS_LOADER_LIBRARY_PADSCORE, WPADRead);

} // namespace pad_mon
