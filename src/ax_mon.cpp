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
#include <coreinit/time.h>
#include <sndcore2/core.h>
#include <sndcore2/device.h>

#include <wups.h>

#include <wupsxx/logger.hpp>

#include "ax_mon.hpp"

#include "cfg.hpp"


enum MIXSoundMode {
    MIX_SOUND_MODE_MONO     = 0,
    MIX_SOUND_MODE_STEREO   = 1,
    MIX_SOUND_MODE_SURROUND = 2,
    MIX_SOUND_MODE_5_1      = 4,
};


// TODO: send a PR to WUT

struct AXProfileInterval {
    OSTime start;
    OSTime finish;
};
WUT_CHECK_OFFSET(AXProfileInterval, 0, start);
WUT_CHECK_OFFSET(AXProfileInterval, 8, finish);
WUT_CHECK_SIZE(AXProfileInterval,  16);


struct RealAXProfile {

    AXProfileInterval frame;
    AXProfileInterval auxCallback;
    AXProfileInterval frameCallback;
    AXProfileInterval finalMixCallback;

    uint32_t numVoices;
    uint32_t numDSPVoices;

    AXProfileInterval DSP;
    AXProfileInterval PPC;

    AXProfileInterval post;

    WUT_UNKNOWN_BYTES(8);
    OSTime   postLatency; // TODO: unused?

};

WUT_CHECK_OFFSET(RealAXProfile, 0, frame);
WUT_CHECK_OFFSET(RealAXProfile, 16, auxCallback);
WUT_CHECK_OFFSET(RealAXProfile, 32, frameCallback);
WUT_CHECK_OFFSET(RealAXProfile, 48, finalMixCallback);
WUT_CHECK_OFFSET(RealAXProfile, 64, numVoices);
WUT_CHECK_OFFSET(RealAXProfile, 68, numDSPVoices);
WUT_CHECK_OFFSET(RealAXProfile, 72, DSP);
WUT_CHECK_OFFSET(RealAXProfile, 88, PPC);
WUT_CHECK_OFFSET(RealAXProfile, 104, post);
WUT_CHECK_OFFSET(RealAXProfile, 128, postLatency);

WUT_CHECK_SIZE(RealAXProfile,  8 * 17);


namespace ax_mon {

    namespace {

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
            if (!OSDynLoad_IsModuleLoaded("sndcore2", &m)) {
                if (!OSDynLoad_FindExport(m,
                                          OS_DYNLOAD_EXPORT_FUNC,
                                          name,
                                          &result)) {
                    // wups::logger::printf("%s found on sndcore2\n", name);
                    return result;
                }
            }

            m = nullptr;
            if (!OSDynLoad_IsModuleLoaded("snd_core", &m)) {
                if (!OSDynLoad_FindExport(m,
                                          OS_DYNLOAD_EXPORT_FUNC,
                                          name,
                                          &result))
                    return result;
            }

            // wups::logger::printf("%s not found\n", name);
            return nullptr;
        }


        using AXIsInit_func_t           = BOOL(void);
        using AXGetCurrentParams_func_t = void(AXInitParams*);
        using AXGetDeviceMode_func_t    = AXResult(AXDeviceType, AXDeviceMode*);


        AXIsInit_func_t*           AXIsInit_func;
        AXGetCurrentParams_func_t* AXGetCurrentParams_func;
        AXGetDeviceMode_func_t*    AXGetDeviceMode_func;


        void
        lookup_functions()
        {
            AXIsInit_func =
                reinterpret_cast<AXIsInit_func_t*>(get_core_func("AXIsInit"));
            AXGetCurrentParams_func =
                reinterpret_cast<AXGetCurrentParams_func_t*>(get_core_func("AXGetCurrentParams"));
            AXGetDeviceMode_func =
                reinterpret_cast<AXGetDeviceMode_func_t*>(get_core_func("AXGetDeviceMode"));
        }


        void
        clear_functions()
        {
            AXIsInit_func           = nullptr;
            AXGetCurrentParams_func = nullptr;
            AXGetDeviceMode_func    = nullptr;
        }


        std::optional<BOOL>
        dyn_AXIsInit()
        {
            if (!AXIsInit_func)
                return {};
            return AXIsInit_func();
        }


        bool
        dyn_AXGetCurrentParams(AXInitParams* params)
        {
            if (!AXGetCurrentParams_func)
                return false;
            AXGetCurrentParams_func(params);
            return true;
        }


        std::optional<AXResult>
        dyn_AXGetDeviceMode(AXDeviceType type,
                            AXDeviceMode *mode)
        {
            if (!AXGetDeviceMode_func)
                return {};
            return AXGetDeviceMode_func(type, mode);
        }


        struct ax_stats {

            float total_load;
            float dsp_load;
            float ppc_load;

            unsigned voices;
            unsigned dsp_voices;

        };


        OSTime
        duration(const AXProfileInterval& interval)
        {
            return interval.finish - interval.start;
        }


        ax_stats
        get_stats(RealAXProfile prof)
        {
            static const float deadline = OSMillisecondsToTicks(3);

            ax_stats result;

            float total     = duration(prof.frame);
            float dsp_total = duration(prof.DSP);
            float ppc_total =
                (prof.frameCallback.finish - prof.PPC.start)
                +
                (prof.frame.finish - prof.post.start);

            result.total_load = total / deadline;
            result.dsp_load   = dsp_total / deadline;
            result.ppc_load   = ppc_total / deadline;

            result.voices = prof.numVoices;
            result.dsp_voices = prof.numDSPVoices;

            return result;
        }

    } // namespace


    unsigned prof_version = 0;

    RealAXProfile prof_current alignas(0x40);


    void
    initialize()
    {
        prof_version = 0;
        lookup_functions();
    }


    void
    finalize()
    {
        clear_functions();
        prof_version = 0;
    }


    void
    reset()
    {
        finalize();
        initialize();
    }


    void
    on_application_start()
    {
        reset();
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
            sep = "/";
            print_mode(out, mode);
        }

        auto status_mode_drc = dyn_AXGetDeviceMode(AX_DEVICE_TYPE_DRC, &mode);
        if (status_mode_drc && !*status_mode_drc) {
            out.append(sep);
            sep = ", ";
            print_mode(out, mode);
        } else
            sep = ", ";

        if (cfg::audio_busy.value && prof_version) {
            auto stats = get_stats(prof_current);
            prof_version = 0;
            out.append(sep);
            sep = ", ";
            out.printf("load: %2.1f%% / %2.1f%% / %2.1f%%, voices: %u/%u",
                       100.0f * stats.dsp_load,
                       100.0f * stats.ppc_load,
                       100.0f * stats.total_load,
                       stats.dsp_voices,
                       stats.voices);
        }
    }


    DECL_FUNCTION(uint32_t, AXGetSwapProfile1,
                  RealAXProfile* buf,
                  uint32_t count)
    {
        uint32_t result = real_AXGetSwapProfile1(buf, count);
        if (result > 0
            && cfg::enabled.value
            && cfg::audio.value
            && cfg::audio_busy.value) {
            prof_current = buf[result - 1];
            prof_version = 1;
        }
        return result;
    }

    WUPS_MUST_REPLACE(AXGetSwapProfile1,
                      WUPS_LOADER_LIBRARY_SND_CORE,
                      AXGetSwapProfile);


    DECL_FUNCTION(uint32_t, AXGetSwapProfile2,
                  RealAXProfile* buf,
                  uint32_t count)
    {
        uint32_t result = real_AXGetSwapProfile2(buf, count);
        if (result > 0
            && cfg::enabled.value
            && cfg::audio.value
            && cfg::audio_busy.value) {
            prof_current = buf[result - 1];
            prof_version = 2;
        }
        return result;
    }

    WUPS_MUST_REPLACE(AXGetSwapProfile2,
                      WUPS_LOADER_LIBRARY_SNDCORE2,
                      AXGetSwapProfile);

} // namespace ax_mon
