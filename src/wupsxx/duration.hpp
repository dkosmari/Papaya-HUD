// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WUPSXX_DURATION_HPP
#define WUPSXX_DURATION_HPP

#include <chrono>
#include <string>
#include <type_traits>


namespace wups::concepts {

    namespace detail {

        template<typename T>
        struct is_duration : std::false_type {};

        template<typename R, typename P>
        struct is_duration<std::chrono::duration<R, P>> : std::true_type {};

        template<typename T>
        constexpr inline bool is_duration_v = is_duration<T>::value;
    }

    template<typename T>

    concept duration = detail::is_duration_v<T>;

} // namespace wups::concepts


namespace wups::config {

    template<typename R, typename P>
    using duration = std::chrono::duration<R, P>;


    template<concepts::duration D>
    std::string to_string(D d);

}


#endif
