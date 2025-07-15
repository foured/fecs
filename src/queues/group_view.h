#pragma once

#include <array>

#include "../core/type_traits.h"
#include "../containers/sparce_set.h"

namespace fecs {

    template<typename... Ts>
    requires unique_types<Ts...> && (sizeof...(Ts) > 1)
    class group_view {
    public:
        using components = type_list<Ts...>;
        using pools_array = std::array<pool*, components::size>;

        group_view(const pools_array& pools, size_t* next_index)
            : _pools(pools), _next_index(next_index) { }

        template<typename Func>
        requires std::is_invocable_v<Func, Ts&...>
        void for_each(Func func){
            for_each_impl(func, components::sequence);
        }

    private:
        pools_array _pools;
        size_t* _next_index;

        template<size_t index>
        auto get_pool() const {
            using component_t = typename components::template get<index>;
            return static_cast<sparce_set<component_t>*>(_pools[index]);
        }

        template<typename Func, size_t... Is>
        void for_each_impl(Func func, std::index_sequence<Is...>) {
            for(size_t i = 0; i < *_next_index; ++i){
                func(get_pool<Is>()->get_ref_directly(i)...);
            }
        }

    };

}