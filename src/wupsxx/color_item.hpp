// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WUPSXX_COLOR_ITEM_HPP
#define WUPSXX_COLOR_ITEM_HPP

#include <cstdint>
#include <memory>

#include "color.hpp"
#include "item.hpp"
#include "var_watch.hpp"


namespace wups::config {

    class color_item : public item {

        var_watch<color> variable;
        const color default_value;
        bool has_alpha;
        bool editing;
        unsigned edit_idx;

    public:

        color_item(const std::optional<std::string>& key,
                   const std::string& label,
                   color& variable,
                   color default_value,
                   bool has_alpha = false);

        static
        std::unique_ptr<color_item>
        create(const std::optional<std::string>& key,
               const std::string& label,
               color& variable,
               color default_value,
               bool has_alpha = false);


        virtual int get_display(char* buf, std::size_t size) const override;

        virtual int get_selected_display(char* buf, std::size_t size) const override;

        virtual void restore() override;

        virtual bool is_movement_allowed() const override;

        virtual void on_input(WUPSConfigSimplePadData input,
                              WUPS_CONFIG_SIMPLE_INPUT repeat);


    private:

        void on_changed();

    };

} // namespace wups::config


#endif
