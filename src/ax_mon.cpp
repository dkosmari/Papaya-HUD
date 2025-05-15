/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * Audio Monitoring
 */

#include <optional>

#include <coreinit/dynload.h>
#include <sndcore2/core.h>
#include <sndcore2/device.h>

#include <wups.h>

#include <wupsxx/logger.hpp>

#include "ax_mon.hpp"


enum MIXSoundMode {
    MIX_SOUND_MODE_MONO     = 0,
    MIX_SOUND_MODE_STEREO   = 1,
    MIX_SOUND_MODE_SURROUND = 2,
    MIX_SOUND_MODE_5_1      = 4,
};


namespace ax_mon {

    namespace {

        [[maybe_unused]]
        void
        print_mode(out_span& out,
                   AXDeviceMode mode)
        {
            // TODO: send a PR to WUT for these
            switch (mode) {
                case 0:
                    out.append("stereo");
                    break;
                case 1: // virtual surround on gamepad
                    out.append("surround");
                    break;
                case 3: // surround on TV
                    out.append("5.1");
                    break;
                case 5:
                    out.append("mono");
                    break;
                default:
                    out.printf("%d", mode);
            }
        }


        void*
        get_core_func(const char* name)
        {
            OSDynLoad_Module m;
            void* result = nullptr;

            m = nullptr;
            if (!OSDynLoad_IsModuleLoaded("snd_core", &m)) {
                if (!OSDynLoad_FindExport(m,
                                          OS_DYNLOAD_EXPORT_FUNC,
                                          name,
                                          &result))
                    return result;
            }

            m = nullptr;
            if (!OSDynLoad_IsModuleLoaded("sndcore2", &m)) {
                if (!OSDynLoad_FindExport(m,
                                          OS_DYNLOAD_EXPORT_FUNC,
                                          name,
                                          &result)) {
                    // wups::logger::printf("%s found on sndcore2\n", name);
                    return result;
                }
            }
            // wups::logger::printf("%s not found\n", name);
            return nullptr;
        }


        [[maybe_unused]]
        void*
        get_user_func(const char* name)
        {
            OSDynLoad_Module m;
            void* result = nullptr;

            m = nullptr;
            if (!OSDynLoad_IsModuleLoaded("snd_user", &m)) {
                if (!OSDynLoad_FindExport(m,
                                          OS_DYNLOAD_EXPORT_FUNC,
                                          name,
                                          &result))
                    return result;
            }

            m = nullptr;
            if (!OSDynLoad_IsModuleLoaded("snduser2", &m)) {
                if (!OSDynLoad_FindExport(m,
                                          OS_DYNLOAD_EXPORT_FUNC,
                                          name,
                                          &result))
                    return result;
            }
            // wups::logger::printf("%s not found\n", name);
            return nullptr;
        }


        using AXIsInit_func           = BOOL(void);
        using AXGetCurrentParams_func = void(AXInitParams*);
        using AXGetDeviceMode_func    = AXResult(AXDeviceType, AXDeviceMode*);
        using AXGetDeviceVolume_func  = AXResult(AXDeviceType, uint32_t, uint16_t*);
        using AXGetDspLoad_func       = float(void);
        using AXGetPpcLoad_func       = float(void);
        using AXGetNumVoices_func     = uint32_t(void);
        using AXGetNumDspVoices_func  = uint32_t(void);
        using MIXGetSoundMode_func    = MIXSoundMode(void);


        std::optional<BOOL>
        dyn_AXIsInit()
        {
            void* ptr = get_core_func("AXIsInit");
            if (!ptr)
                return std::nullopt;
            auto func = reinterpret_cast<AXIsInit_func*>(ptr);
            return func();
        }


        bool
        dyn_AXGetCurrentParams(AXInitParams* params)
        {
            void* ptr = get_core_func("AXGetCurrentParams");
            if (!ptr)
                return false;
            auto func = reinterpret_cast<AXGetCurrentParams_func*>(ptr);
            func(params);
            return true;
        }


        std::optional<AXResult>
        dyn_AXGetDeviceMode(AXDeviceType type,
                            AXDeviceMode *mode)
        {
            void* ptr = get_core_func("AXGetDeviceMode");
            if (!ptr)
                return {};
            auto func = reinterpret_cast<AXGetDeviceMode_func*>(ptr);
            return func(type, mode);
        }


        [[maybe_unused]]
        std::optional<AXResult>
        dyn_AXGetDeviceVolume(AXDeviceType type,
                              uint32_t id,
                              uint16_t* volume)
        {
            void* ptr = get_core_func("AXGetDeviceVolume");
            if (!ptr)
                return {};
            auto func = reinterpret_cast<AXGetDeviceVolume_func*>(ptr);
            return func(type, id, volume);
        }


        std::optional<float>
        dyn_AXGetDspLoad()
        {
            void* ptr = get_core_func("AXGetDspLoad");
            if (!ptr)
                return {};
            auto func = reinterpret_cast<AXGetDspLoad_func*>(ptr);
            return func();
        }


        std::optional<float>
        dyn_AXGetPpcLoad()
        {
            void* ptr = get_core_func("AXGetPpcLoad");
            if (!ptr)
                return {};
            auto func = reinterpret_cast<AXGetPpcLoad_func*>(ptr);
            return func();
        }


        std::optional<uint32_t>
        dyn_AXGetNumVoices()
        {
            void* ptr = get_core_func("AXGetNumVoices");
            if (!ptr)
                return {};
            auto func = reinterpret_cast<AXGetNumVoices_func*>(ptr);
            return func();
        }


        std::optional<uint32_t>
        dyn_AXGetNumDspVoices()
        {
            void* ptr = get_core_func("AXGetNumDspVoices");
            if (!ptr)
                return {};
            auto func = reinterpret_cast<AXGetNumDspVoices_func*>(ptr);
            return func();
        }


        [[maybe_unused]]
        std::optional<MIXSoundMode>
        dyn_MIXGetSoundMode()
        {
            void* ptr = get_user_func("MIXGetSoundMode");
            if (!ptr)
                return {};
            auto func = reinterpret_cast<MIXGetSoundMode_func*>(ptr);
            return func();
        }

    } // namespace


    void
    initialize()
    {}


    void
    finalize()
    {}


    void
    reset()
    {
        finalize();
        initialize();
    }


    void
    on_application_start()
    {
        // TODO: clear function pointer caches.
    }


    void
    get_report(out_span& out,
               float)
    {
        const char* sep = "";

        auto ax_is_init = dyn_AXIsInit();
        if (!ax_is_init || !*ax_is_init) {
            out.append("No audio");
            return;
        }

        out.append("audio: ");
        alignas(0x40) AXInitParams params{};
        dyn_AXGetCurrentParams(&params);
        switch (params.renderer) {
            case AX_INIT_RENDERER_32KHZ:
                out.append("32k㎐");
                break;
            case AX_INIT_RENDERER_48KHZ:
                out.append("48k㎐");
                break;
            default:
                out.append("unknown");
        }
        switch (params.pipeline) {
            case AX_INIT_PIPELINE_SINGLE:
                // out.append("(1)");
                break;
            case AX_INIT_PIPELINE_FOUR_STAGE:
                out.append("(4)");
                break;
            default:
                out.append("(?)");
        }
        sep = ", ";


        AXDeviceMode mode;
        auto status_mode_tv = dyn_AXGetDeviceMode(AX_DEVICE_TYPE_TV, &mode);
        if (status_mode_tv && !*status_mode_tv) {
            out.append(sep);
            sep = ", ";
            out.append("TV: ");
            print_mode(out, mode);
        }

        auto status_mode_drc = dyn_AXGetDeviceMode(AX_DEVICE_TYPE_DRC, &mode);
        if (status_mode_drc && !*status_mode_drc) {
            out.append(sep);
            sep = ", ";
            out.append("DRC: ");
            print_mode(out, mode);
        }

        auto dsp_load = dyn_AXGetDspLoad();
        if (dsp_load) {
            out.append(sep);
            sep = ", ";
            out.printf("DSP: %2.1f%%", *dsp_load);
        }

        auto ppc_load = dyn_AXGetPpcLoad();
        if (ppc_load) {
            out.append(sep);
            sep = ", ";
            out.printf("PPC: %2.1f%%", *ppc_load);
        }

        auto num_dsp_voices = dyn_AXGetNumDspVoices();
        auto num_voices = dyn_AXGetNumVoices();
        if (num_dsp_voices && num_voices) {
            out.append(sep);
            sep = ", ";
            out.printf("Voices: %u/%u", *num_dsp_voices, *num_voices);
        }

#if 0
        uint16_t volume;
        auto status_vol_tv = dyn_AXGetDeviceVolume(AX_DEVICE_TYPE_TV, 0, &volume);
        if (status_vol_tv && !*status_vol_tv) {
            out.append(sep);
            sep = ", ";
            out.printf("TV vol: %2.0f", (100.0 * volume / 0x8000));
            // out.printf("TV vol: %04X", volume);
        }
        auto status_vol_drc = dyn_AXGetDeviceVolume(AX_DEVICE_TYPE_DRC, 0, &volume);
        if (status_vol_drc && !*status_vol_drc) {
            out.append(sep);
            sep = ", ";
            out.printf("DRC vol: %2.0f", (100.0 * volume / 0x8000));
            // out.printf("DRC vol: %04X", volume);
        }
#endif

#if 0
        auto mix_mode = dyn_MIXGetSoundMode();
        if (mix_mode) {
            out.append(sep);
            sep = ", ";
            out.append("mix: ");
            switch (*mix_mode) {
                case MIX_SOUND_MODE_MONO:
                    out.append("mono");
                    break;
                case MIX_SOUND_MODE_STEREO:
                    out.append("stereo");
                    break;
                case MIX_SOUND_MODE_SURROUND:
                    out.append("surround");
                    break;
                case MIX_SOUND_MODE_5_1:
                    out.append("5.1");
                    break;
                default:
                    out.printf("%d?", int(*mix_mode));
            }
        }
#endif

    }

} // namespace ax_mon
