// SPDX-License-Identifier: GPL-3.0-or-later

#include <wups.h>

#include "cfg.hpp"

#include "logging.hpp"
#include "overlay.hpp"
#include "wupsxx/bool_item.hpp"
#include "wupsxx/category.hpp"
#include "wupsxx/color_item.hpp"
#include "wupsxx/storage.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PACKAGE);


namespace cfg {

    using wups::config::color;


    namespace keys {
        const char* bandwidth = "bandwidth";
        const char* bg_color  = "bg_color";
        const char* cpu       = "cpu";
        const char* enabled   = "enabled";
        const char* fg_color  = "foreground";
        const char* fps       = "fps";
        const char* fs        = "fs";
        const char* time      = "time";
    }


    namespace labels {
        const char* bandwidth = "Network bandwidth";
        const char* bg_color  = "Background color";
        const char* cpu       = "CPU utilization";
        const char* enabled   = "Enabled";
        const char* fg_color  = "Foreground color";
        const char* fps       = "Frames per second";
        const char* fs        = "Filesystem";
        const char* time      = "Time";
    }


    namespace defaults {
        const bool  bandwidth = true;
        const color bg_color  = {0x00, 0x00, 0x00, 0xb0};
        const bool  cpu       = true;
        const bool  enabled   = true;
        const color fg_color  = {0xff, 0xff, 0x60};
        const bool  fps       = true;
        const bool  fs        = true;
        const bool  time      = true;
    }


    bool bandwidth = defaults::bandwidth;
    color bg_color = defaults::bg_color;
    bool cpu       = defaults::cpu;
    bool enabled   = defaults::enabled;
    color fg_color = defaults::fg_color;
    bool fps       = defaults::fps;
    bool fs        = defaults::fs;
    bool time      = defaults::time;


    WUPSConfigAPICallbackStatus
    menu_open(WUPSConfigCategoryHandle root_handle)
    {
        wups::config::category root{root_handle};

        root.add(wups::config::bool_item::create(keys::enabled,
                                                 labels::enabled,
                                                 enabled,
                                                 defaults::enabled,
                                                 "yes", "no"));

        root.add(wups::config::bool_item::create(keys::time,
                                                 labels::time,
                                                 time,
                                                 defaults::time,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(keys::fps,
                                                 labels::fps,
                                                 fps,
                                                 defaults::fps,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(keys::cpu,
                                                 labels::cpu,
                                                 cpu,
                                                 defaults::cpu,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(keys::bandwidth,
                                                 labels::bandwidth,
                                                 bandwidth,
                                                 defaults::bandwidth,
                                                 "on", "off"));

        root.add(wups::config::bool_item::create(keys::fs,
                                                 labels::fs,
                                                 fs,
                                                 defaults::fs,
                                                 "on", "off"));

        root.add(wups::config::color_item::create(keys::fg_color,
                                                  labels::fg_color,
                                                  fg_color,
                                                  defaults::fg_color,
                                                  false));

        root.add(wups::config::color_item::create(keys::bg_color,
                                                  labels::bg_color,
                                                  bg_color,
                                                  defaults::bg_color,
                                                  true));

        return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
    }


    void
    menu_close()
    {
        cfg::save();

        if (enabled)
            overlay::create();
        else
            overlay::destroy();

        overlay::reset();
    }


    void
    init()
    {
        static bool done = false;

        if (done)
            return;

        WUPSConfigAPIOptionsV1 options{ .name = PACKAGE_NAME };
        auto status = WUPSConfigAPI_Init(options, menu_open, menu_close);
        if (status != WUPSCONFIG_API_RESULT_SUCCESS) {
            logging::printf("Error initializing WUPS config API: %s\n",
                            WUPSConfigAPI_GetStatusStr(status));
        } else
            logging::printf("WUPS config API initialized\n");

        load();

        done = true;
    }


    void
    load()
    {
        using wups::storage::load_or_init;
        using wups::storage::load_or_init_str;

        try {

#define LOI(x) load_or_init(keys::x, x, defaults::x)
            LOI(bandwidth);
            LOI(cpu);
            LOI(enabled);
            LOI(fps);
            LOI(fs);
            LOI(time);
#undef LOI
            load_or_init_str(keys::fg_color, fg_color,
                             defaults::fg_color, to_string(defaults::fg_color, false));
            load_or_init_str(keys::bg_color, bg_color,
                             defaults::bg_color, to_string(defaults::bg_color, true));
        }
        catch (std::exception& e) {
            logging::printf("error loading config: %s\n", e.what());
        }
    }


    void
    save()
    {
        try {
            wups::storage::save();
        }
        catch (std::exception& e) {
            logging::printf("error saving config: %s\n", e.what());
        }
    }
}
