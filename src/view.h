#pragma once

#include "pool.h"
#include "sparce_set.h"
#include "type_traits.h"
#include "types.h"
#include <algorithm>
#include <type_traits>
#include <utility>

#include "benchmarks/timer.hpp"

namespace fecs {

    // struct view_descriptor{
    //     virtual ~view_descriptor() = default;
    //     //virtual bool contains(entity_t entity) const = 0;
    // };

    template <typename... Ts>
    requires unique_types<Ts...> && (sizeof...(Ts) > 1)
    class view { 
    public:
        using components = type_list<Ts...>;
        using pools_array = std::array<pool*, components::size>;

        view(pools_array&& pools)
            : _pools(std::move(pools)) {
                _min_pool = get_min_pool();
            }

        template<typename Func>
        requires std::is_invocable_v<Func, Ts&...>
        void for_each(Func func){
            for_each_impl(func, components::sequence);
        }

    private:
        pools_array _pools;
        pool* _min_pool;

        bool contains(entity_t entity) const{
            for(size_t i = 0; i < components::size; ++i){
                if(!_pools[i]->contains(entity)){
                    return false;
                }
            }
            return true;
        }

        template<size_t index>
        auto get_pool() const {
            using component_t = typename components::template get<index>;
            return static_cast<sparce_set<component_t>*>(_pools[index]);
        }

        pool* get_min_pool() const {
            return *std::min_element(_pools.begin(), _pools.end(), 
                [](const pool* p1, const pool* p2){
                    return p1->size() < p2->size();
                });
        }

        template<typename Func, size_t... It>
        void for_each_impl(Func func, std::index_sequence<It...>){
            const auto& ents = _min_pool->get_entities();
            const size_t s = ents.size();
            pool* tp = nullptr;
            size_t page, offset;
            entity_t e;
            for(size_t i = 0; i < s; ++i){
                e = ents[i];
                page = e / SPARCE_MAX_SIZE;
                offset = e % SPARCE_MAX_SIZE;
                for(size_t j = 0; j < components::size; ++j){
                    tp = _pools[j];
                    if(tp != _min_pool && !tp->contains(page, offset)){
                        continue;
                    }
                }
                func(get_pool<It>()->get_ref_directly_e(page, offset)...);
            }
        }

    };

}