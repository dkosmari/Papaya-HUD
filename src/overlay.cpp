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


//#define TEST_TIME


namespace overlay {


    std::atomic<NotificationModuleHandle> notif_handle{0};

    OSTime last_sample_time;


    static
    void
    on_notif_finished(NotificationModuleHandle h, void*)
    {
        if (h == notif_handle.load())
            notif_handle.store(0);
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
    create()
    {
        auto handle = notif_handle.load();
        if (!handle) {
            reset();
            auto status = NotificationModule_AddDynamicNotificationEx("-",
                                                                      &handle,
                                                                      {0xff, 0xff, 0x80, 0xff},
                                                                      {0x00, 0x00, 0x00, 0x80},
                                                                      on_notif_finished,
                                                                      nullptr,
                                                                      false);
            if (status != NOTIFICATION_MODULE_RESULT_SUCCESS) {
                logging::printf("failed to create overlay notification: %s\n",
                                NotificationModule_GetStatusStr(status));
                return;
            }
            notif_handle.store(handle);
        }

        if (cfg::fps)
            gx2_mon::initialize();

        if  (cfg::cpu)
            cos_mon::initialize();

        if (cfg::bandwidth)
            net_mon::initialize();

        if (cfg::fs)
            fs_mon::initialize();
    }


    void
    destroy()
    {
        auto handle = notif_handle.load();
        if (!handle)
            return;

        gx2_mon::finalize();
        cos_mon::finalize();
        net_mon::finalize();
        fs_mon::finalize();

        auto status = NotificationModule_FinishDynamicNotification(handle, 0);
        if (status != NOTIFICATION_MODULE_RESULT_SUCCESS) {
            logging::printf("failed to finish notification: %s\n",
                            NotificationModule_GetStatusStr(status));
            return;
        }
        notif_handle.store(0);
    }


    void
    reset()
    {
        last_sample_time = OSGetSystemTime();
    }


    void
    render()
    {
        auto handle = notif_handle.load();
        if (!handle)
            return;

        const OSTime update_interval = OSSecondsToTicks(1);

        OSTime now = OSGetSystemTime();
        if (now - last_sample_time >= update_interval) {
            // Note: text is static to reduce allocations during rendering.
            // This function is only ever called from the main rendering thread.
            static std::string text;
            text = "";
            const char* sep = "";

            const float dt = (now - last_sample_time) / float(OSTimerClockSpeed);

            if (cfg::fps) {
                text += sep;
                text += gx2_mon::get_report(dt);
                sep = " | ";
            }

            if (cfg::cpu) {
                text += sep;
                text += cos_mon::get_report(dt);
                sep = " | ";
            }

            if (cfg::bandwidth) {
                text += sep;
                text += net_mon::get_report(dt);
                sep = " | ";
            }

            if (cfg::fs) {
                text += sep;
                text += fs_mon::get_report(dt);
                sep = " | ";
            }

            NotificationModule_UpdateDynamicNotificationText(handle, text.c_str());

            last_sample_time = now;

#ifdef TEST_TIME
            // check that we aren't taking that much time to do it
            now = OSGetSystemTime();
            OSTime delta = now - last_sample_time;
            logging::printf("swap time = %lld (%f us)\n",
                            delta,
                            (double)OSTicksToMicroseconds(delta));
#endif
        }
    }

}
