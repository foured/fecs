#pragma once

#include <limits>
#include <stdint.h>

namespace fecs {

    using entity_t = uint32_t;

    constexpr entity_t error_entity = std::numeric_limits<entity_t>::max();

    template<typename... Ts>
    class pack_part{};

    template<typename... Ts>
    class view_part{};

}