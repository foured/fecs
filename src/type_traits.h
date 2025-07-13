#pragma once

#include <type_traits>

template<typename...>
constexpr bool is_unique = true;

template<typename T, typename... Rest>
constexpr bool is_unique<T, Rest...> =
    (!std::is_same_v<T, Rest> && ...) && is_unique<Rest...>;

template<typename... Ts>
concept unique_types = is_unique<Ts...>;