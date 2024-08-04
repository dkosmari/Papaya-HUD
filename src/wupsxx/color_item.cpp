// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>

#include "color_item.hpp"

#include "storage.hpp"
// #include "../logging.hpp"


namespace wups::config {

    color_item::color_item(const std::optional<std::string>& key,
                           const std::string& label,
                           color& variable,
                           color default_value,
                           bool has_alpha) :
        item{key, label},
        variable{variable},
        default_value{default_value},
        has_alpha{has_alpha},
        editing{false},
        edit_idx{0}
    {}


    std::unique_ptr<color_item>
    color_item::create(const std::optional<std::string>& key,
                       const std::string& label,
                       color& variable,
                       color default_value,
                       bool has_alpha)
    {
        return std::make_unique<color_item>(key, label,
                                            variable, default_value,
                                            has_alpha);
    }


    int
    color_item::get_display(char* buf, std::size_t size)
        const
    {
        auto s = to_string(variable.value(), has_alpha);
        std::snprintf(buf, size, "%s", s.c_str());
        return 0;
    }


    int
    color_item::get_selected_display(char* buf, std::size_t size)
        const
    {
        if (!editing)
            return get_display(buf, size);

        auto s = to_string(variable.value(), has_alpha);
        const char* left_bracket = "[";
        const char* right_bracket = "]";
        s.insert(1 + 2 + edit_idx * 2, right_bracket);
        s.insert(1 + 0 + edit_idx * 2, left_bracket);
        std::snprintf(buf, size, "%s", s.c_str());
        return 0;
    }


    void
    color_item::restore()
    {
        variable = default_value;
        on_changed();
    }


    bool
    color_item::is_movement_allowed()
        const
    {
        return !editing;
    }


    void
    color_item::on_input(WUPSConfigSimplePadData input,
                         WUPS_CONFIG_SIMPLE_INPUT repeat)
    {
        item::on_input(input, repeat);

        if (input.buttons_d & WUPS_CONFIG_BUTTON_A)
            editing = !editing;
        if (input.buttons_d & WUPS_CONFIG_BUTTON_B)
            editing = false;

        if (!editing)
            return;

        const unsigned max_edit_idx = has_alpha ? 3 : 2;

        const auto left_mask = WUPS_CONFIG_BUTTON_LEFT | WUPS_CONFIG_BUTTON_L;
        if (input.buttons_d & left_mask || repeat & left_mask)
            if (edit_idx > 0)
                --edit_idx;

        const auto right_mask = WUPS_CONFIG_BUTTON_RIGHT | WUPS_CONFIG_BUTTON_R;
        if (input.buttons_d & right_mask || repeat & right_mask)
            if (edit_idx < max_edit_idx)
                ++edit_idx;

        color c = variable.value();
        auto& channel = c[edit_idx];

        if (input.buttons_d & WUPS_CONFIG_BUTTON_UP ||
            repeat & WUPS_CONFIG_BUTTON_UP)
            if (channel < 0xff)
                ++channel;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_DOWN ||
            repeat & WUPS_CONFIG_BUTTON_DOWN)
            if (channel > 0)
                --channel;

        variable = c;
        on_changed();
    }


    void
    color_item::on_changed()
    {
        if (!key)
            return;
        if (!variable.changed())
            return;

        try {
            storage::store(*key, variable.value());
            variable.reset();
        }
        catch (std::exception& e) {
            // logging::printf("Error storing %s: %s\n", key->c_str(), e.what());
        }
    }

} // namespace wups::config
