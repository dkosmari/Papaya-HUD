/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * This is the "overlay" that shows the HUD.
 *
 * Currently we just (ab)use the Notification module, showing a persistent dynamic
 * notification. At every update interval, we update the notification text. We don't have
 * to write any code to actually render things, but we pay a hefty price: updating a
 * notification requires locking one or more mutexes in the Notification module; not a
 * good thing to have hooked into `GX2SwapScanBuffers()`.
 *
 * This will be later replaced by an actual overlay rendering implementation, requiring no
 * mutexes. Then we can have more advanced rendering, like line graphs and histograms.
 */

#include <atomic>

#include <coreinit/cache.h>
#include <coreinit/time.h>

#include <notifications/notifications.h>

#include <wupsxx/cafe_glyphs.h>
#include <wupsxx/logger.hpp>

#include "overlay.hpp"

#include "ax_mon.hpp"
#include "cfg.hpp"
#include "cpu_mon.hpp"
#include "fs_mon.hpp"
#include "gx2_mon.hpp"
#include "net_mon.hpp"
#include "pad_mon.hpp"
#include "time_mon.hpp"
#include "utils.hpp"


// #define TEST_TIME


namespace overlay {

    namespace logger = wups::logger;


    bool gx2_init = false;
    std::atomic_bool toggle_requested = false;

    std::atomic<NotificationModuleHandle> notif_handle{0};

    OSTime last_sample_time;


    namespace {

        void
        on_notif_finished(NotificationModuleHandle h, void*)
        {
            if (h == notif_handle.load())
                notif_handle.store(0);
        }


        NMColor
        convert(wups::color c)
        {
            return {c.r, c.g, c.b, c.a};
        }

    }


    void
    initialize()
    {
        NotificationModule_InitLibrary();
    }


    void
    finalize()
    {
        destroy();
        NotificationModule_DeInitLibrary();
    }


    void
    create_or_reset()
    {
        // Don't create anything until GX2Init() is called.
        if (!gx2_init)
            return;

        auto handle = notif_handle.load();
        if (!handle) {
            auto status = NotificationModule_AddDynamicNotificationEx(CAFE_GLYPH_HELP,
                                                                      &handle,
                                                                      convert(cfg::color_fg.value),
                                                                      convert(cfg::color_bg.value),
                                                                      on_notif_finished,
                                                                      nullptr,
                                                                      false);
            if (status != NOTIFICATION_MODULE_RESULT_SUCCESS) {
                logger::printf("Failed to create overlay notification: %s\n",
                               NotificationModule_GetStatusStr(status));
                return;
            }
            notif_handle.store(handle);
        }

        reset();
    }


    void
    destroy()
    {
        ax_mon::finalize();
        time_mon::finalize();
        gx2_mon::finalize();
        cpu_mon::finalize();
        net_mon::finalize();
        fs_mon::finalize();
        pad_mon::finalize();

        auto handle = notif_handle.load();
        if (!handle)
            return;

        auto status = NotificationModule_FinishDynamicNotification(handle, 0);
        if (status != NOTIFICATION_MODULE_RESULT_SUCCESS) {
            logger::printf("Failed to finish notification: %s\n",
                           NotificationModule_GetStatusStr(status));
            return;
        }
    }


    void
    reset()
    {
        last_sample_time = OSGetSystemTime();

        auto handle = notif_handle.load();
        if (handle) {
            NotificationModule_UpdateDynamicNotificationTextColor(handle,
                                                                  convert(cfg::color_fg.value));
            NotificationModule_UpdateDynamicNotificationBackgroundColor(handle,
                                                                        convert(cfg::color_bg.value));
        }

        ax_mon::reset();
        time_mon::reset();
        gx2_mon::reset();
        cpu_mon::reset();
        net_mon::reset();
        fs_mon::reset();
        pad_mon::reset();

    }


    void
    on_acquired_foreground()
    {
        if (cfg::enabled.value)
            create_or_reset();
    }


    void
    on_release_foreground()
    {
        ax_mon::finalize();
        time_mon::finalize();
        gx2_mon::finalize();
        cpu_mon::finalize();
        net_mon::finalize();
        fs_mon::finalize();
        pad_mon::finalize();
    }


    void
    render()
    {
        auto handle = notif_handle.load();
        if (!handle)
            return;

        const OSTime update_interval = OSMillisecondsToTicks(cfg::interval.value.count());

        OSTime now = OSGetSystemTime();
        // If it's time to update the notification
        if (now - last_sample_time >= update_interval) {
            try {
                static char buffer[256];
                out_span output{buffer, sizeof buffer};
                buffer[0] = '\0';

                const char* separator = "";

                auto append_separator = [&output, &separator]
                {
                    output.append(separator);
                    separator = utils::field_separator;
                };

                const float dt = (now - last_sample_time) / float(OSTimerClockSpeed);

                if (cfg::time.value || cfg::uptime.value || cfg::play_time.value) {
                    append_separator();
                    time_mon::get_report(output, dt);
                }

                if (cfg::gpu_fps.value) {
                    append_separator();
                    gx2_mon::fps::get_report(output, dt);
                }

                if (cfg::gpu_resolution.value) {
                    append_separator();
                    gx2_mon::resolution::get_report(output, dt);
                }

                if (cfg::gpu_busy.value) {
                    append_separator();
                    gx2_mon::perf::get_report(output, dt);
                }

                if (cfg::cpu_busy.value) {
                    append_separator();
                    cpu_mon::get_report(output, dt);
                }

                if (cfg::net_bw.value || cfg::net_cfg.value) {
                    append_separator();
                    net_mon::get_report(output, dt);
                }

                if (cfg::fs_perf.value) {
                    append_separator();
                    fs_mon::get_report(output, dt);
                }

                if (cfg::audio.value) {
                    append_separator();
                    ax_mon::get_report(output, dt);
                }

                if (cfg::button_rate.value || cfg::battery.value) {
                    append_separator();
                    pad_mon::get_report(output, dt);
                }

                // WORKAROUND: NotificationsModule doesn't like empty text.
                if (buffer[0] == '\0')
                    output.append(CAFE_GLYPH_HELP);

                NotificationModule_UpdateDynamicNotificationText(handle, buffer);

                last_sample_time = now;

#ifdef TEST_TIME
                // check how long it takes to calculate all fields
                now = OSGetSystemTime();
                OSTime delta = now - last_sample_time;
                logger::printf("Overlay render time = %lld (%f us)\n",
                               delta,
                               (double)OSTicksToMicroseconds(delta));
#endif
            }
            catch (std::exception& e) {
                logger::printf("Error in overlay::render(): %s\n", e.what());
            }

        } // if it's time to update
    }


    void
    toggle()
    {
        toggle_requested = true;
        OSMemoryBarrier();
    }


    void
    process_toggle_request_from_gx2()
    {
        if (!toggle_requested) [[likely]]
            return;
        toggle_requested = false;
        cfg::enabled.value = !cfg::enabled.value;
        cfg::save();
        if (cfg::enabled.value)
            overlay::create_or_reset();
        else
            overlay::destroy();
    }

} // namespace overlay
