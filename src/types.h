#pragma once

#include <limits>
#include <stdint.h>

namespace fecs {

    using entity_t = uint32_t;

    constexpr entity_t error_entity = std::numeric_limits<entity_t>::max();

}