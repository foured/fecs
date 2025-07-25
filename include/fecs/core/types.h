#pragma once

#include <limits>
#include <cstdint>

#include "type_traits.h"

namespace fecs {

    template<typename T>
    requires std::is_unsigned_v<T>
    class unsigned_wrapper {
    public:
        constexpr unsigned_wrapper() = default;
        constexpr unsigned_wrapper(T value) : _value(value) {}

        constexpr unsigned_wrapper(const unsigned_wrapper&) = default;
        constexpr unsigned_wrapper(unsigned_wrapper&&) noexcept = default;

        constexpr unsigned_wrapper& operator=(const unsigned_wrapper&) = default;
        constexpr unsigned_wrapper& operator=(unsigned_wrapper&&) noexcept = default;

        constexpr unsigned_wrapper& operator+=(T rhs) { _value += rhs; return *this; }
        constexpr unsigned_wrapper& operator-=(T rhs) { _value -= rhs; return *this; }
        constexpr unsigned_wrapper& operator*=(T rhs) { _value *= rhs; return *this; }
        constexpr unsigned_wrapper& operator/=(T rhs) { _value /= rhs; return *this; }
        constexpr unsigned_wrapper& operator%=(T rhs) { _value %= rhs; return *this; }

        constexpr unsigned_wrapper& operator++() { ++_value; return *this; }
        constexpr unsigned_wrapper operator++(int) { unsigned_wrapper tmp = *this; ++(*this); return tmp; }

        constexpr unsigned_wrapper& operator--() { --_value; return *this; }
        constexpr unsigned_wrapper operator--(int) { unsigned_wrapper tmp = *this; --(*this); return tmp; }

        constexpr operator T() const noexcept { return _value; }
        constexpr size_t to_size_t() const noexcept { return static_cast<size_t>(_value); }

        constexpr T get() const noexcept { return _value; }

        friend constexpr unsigned_wrapper operator+(unsigned_wrapper lhs, T rhs) { return lhs += rhs; }
        friend constexpr unsigned_wrapper operator-(unsigned_wrapper lhs, T rhs) { return lhs -= rhs; }
        friend constexpr unsigned_wrapper operator*(unsigned_wrapper lhs, T rhs) { return lhs *= rhs; }
        friend constexpr unsigned_wrapper operator/(unsigned_wrapper lhs, T rhs) { return lhs /= rhs; }
        friend constexpr unsigned_wrapper operator%(unsigned_wrapper lhs, T rhs) { return lhs %= rhs; }

        friend constexpr bool operator==(unsigned_wrapper lhs, unsigned_wrapper rhs) { return lhs._value == rhs._value; }
        friend constexpr bool operator!=(unsigned_wrapper lhs, unsigned_wrapper rhs) { return !(lhs == rhs); }
        friend constexpr bool operator<(unsigned_wrapper lhs, unsigned_wrapper rhs) { return lhs._value < rhs._value; }
        friend constexpr bool operator<=(unsigned_wrapper lhs, unsigned_wrapper rhs) { return lhs._value <= rhs._value; }
        friend constexpr bool operator>(unsigned_wrapper lhs, unsigned_wrapper rhs) { return lhs._value > rhs._value; }
        friend constexpr bool operator>=(unsigned_wrapper lhs, unsigned_wrapper rhs) { return lhs._value >= rhs._value; }

    private:
        T _value{};
    };

    //using entity_t = unsigned_wrapper<uint32_t>;
    using entity_t = uint32_t;

    constexpr entity_t error_entity = std::numeric_limits<entity_t>::max();

    template<typename... Ts>
    class pack_part{};

    template<typename... Ts>
    class view_part{};

    template<typename, typename>
    struct queue_args_descriptor;

    template<typename... Ts>
    requires unique_types<Ts...> && (sizeof...(Ts) > 1)
    struct queue_args_descriptor<pack_part<Ts...>, view_part<>> { };

    template<typename... PTs, typename... VTs>
    requires unique_types<PTs..., VTs...> && (sizeof...(PTs) > 1)
    struct queue_args_descriptor<pack_part<PTs...>, view_part<VTs...>> { };

}

namespace std {
    template <typename T>
    struct is_unsigned<fecs::unsigned_wrapper<T>> : std::is_unsigned<T> {};
}