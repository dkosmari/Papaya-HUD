// SPDX-License-Identifier: GPL-3.0-or-later

#include <atomic>
#include <string>

#include <coreinit/time.h>

#include <notifications/notifications.h>

#include "overlay.hpp"

#include "cfg.hpp"
#include "cos_mon.hpp"
#include "fs_mon.hpp"
#include "gx2_mon.hpp"
#include "logging.hpp"
#include "net_mon.hpp"
#include "nintendo_glyphs.h"
#include "time_mon.hpp"


// #define TEST_TIME


namespace overlay {

    bool gx2_init = false;

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
        convert(wups::config::color c)
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
            auto status = NotificationModule_AddDynamicNotificationEx(NIN_GLYPH_HELP,
                                                                      &handle,
                                                                      convert(cfg::color_fg),
                                                                      convert(cfg::color_bg),
                                                                      on_notif_finished,
                                                                      nullptr,
                                                                      false);
            if (status != NOTIFICATION_MODULE_RESULT_SUCCESS) {
                logging::printf("Failed to create overlay notification: %s\n",
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
        time_mon::finalize();
        gx2_mon::finalize();
        cos_mon::finalize();
        net_mon::finalize();
        fs_mon::finalize();

        auto handle = notif_handle.load();
        if (!handle)
            return;

        auto status = NotificationModule_FinishDynamicNotification(handle, 0);
        if (status != NOTIFICATION_MODULE_RESULT_SUCCESS) {
            logging::printf("Failed to finish notification: %s\n",
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
                                                                  convert(cfg::color_fg));
            NotificationModule_UpdateDynamicNotificationBackgroundColor(handle,
                                                                        convert(cfg::color_bg));
        }

        time_mon::reset();
        gx2_mon::reset();
        cos_mon::reset();
        net_mon::reset();
        fs_mon::reset();

    }


    void
    on_acquired_foreground()
    {
        if (cfg::enabled)
            create_or_reset();
    }


    void
    on_release_foreground()
    {
        time_mon::finalize();
        gx2_mon::finalize();
        cos_mon::finalize();
        net_mon::finalize();
        fs_mon::finalize();
    }


    void
    render()
    {
        auto handle = notif_handle.load();
        if (!handle)
            return;

        // TODO: make update_interval configurable
        const OSTime update_interval = OSSecondsToTicks(1);

        OSTime now = OSGetSystemTime();
        if (now - last_sample_time >= update_interval) {
            // Note: text is static to reduce allocations during rendering.
            // This function is only ever called from the main rendering thread.
            static std::string text;
            text.clear();
            const char* sep = "";

            const float dt = (now - last_sample_time) / float(OSTimerClockSpeed);

            if (cfg::time) {
                text += sep;
                text += time_mon::get_report(dt);
                sep = " | ";
            }

            if (cfg::gpu_fps) {
                text += sep;
                text += gx2_mon::fps::get_report(dt);
                sep = " | ";
            }

            if (cfg::gpu_perf) {
                text += sep;
                text += gx2_mon::perf::get_report(dt);
                sep = " | ";
            }

            if (cfg::cpu_busy) {
                text += sep;
                text += cos_mon::get_report(dt);
                sep = " | ";
            }

            if (cfg::net_bw) {
                text += sep;
                text += net_mon::get_report(dt);
                sep = " | ";
            }

            if (cfg::fs_read) {
                text += sep;
                text += fs_mon::get_report(dt);
                sep = " | ";
            }

            // WORKAROUND: NotificationsModule doesn't like empty text.
            if (text.empty())
                text = NIN_GLYPH_HELP;

            NotificationModule_UpdateDynamicNotificationText(handle, text.c_str());

            last_sample_time = now;

#ifdef TEST_TIME
            // check that we aren't taking that much time to do it
            now = OSGetSystemTime();
            OSTime delta = now - last_sample_time;
            logging::printf("Overlay render time = %lld (%f us)\n",
                            delta,
                            (double)OSTicksToMicroseconds(delta));
#endif
        }
    }

}
