#pragma once

#include <type_traits>
#include <tuple>
#include <utility>

namespace fecs {

    template<typename...>
    constexpr bool is_unique = true;

    template<typename T, typename... Rest>
    constexpr bool is_unique<T, Rest...> =
        (!std::is_same_v<T, Rest> && ...) && is_unique<Rest...>;

    template<typename... Ts>
    concept unique_types = is_unique<Ts...>;

    template<typename T, typename... Ts>
    constexpr bool contains_type = (std::is_same_v<T, Ts> || ...);

    template<typename... Ts>
    struct type_list {
        using types = std::tuple<Ts...>;
        template<size_t index>
        using get = std::tuple_element_t<index, types>;
        static constexpr size_t size = sizeof...(Ts);
        static constexpr auto sequence = std::make_index_sequence<size>{};
        template<typename... Types>
        static constexpr bool contains_all = (contains_type<Types, Ts...> && ...);
    };

    template<typename... As, typename... Bs>
    constexpr bool are_type_lists_elements_equal(type_list<As...>, type_list<Bs...>){
        return sizeof...(As) == sizeof...(Bs) 
            && (contains_type<As, Bs...> && ...)
            && (contains_type<Bs, As...> && ...);
    }

}