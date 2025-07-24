#pragma once

namespace fecs {

    using id_index_t = size_t;

    namespace details{

        struct type_index final {
            [[nodiscard]] static id_index_t next() noexcept{
                static size_t value{};
                return value++;
            }
        };

    }

    template<typename T>
    struct type_index final {
        [[nodiscard]] static id_index_t value() noexcept{
            static const id_index_t value = details::type_index::next();
            return value; 
        }
    };

}