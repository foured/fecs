#pragma once

#include <algorithm>
#include <stddef.h>
#include <array>
#include <tuple>
#include <type_traits>
#include <utility>

#include "type_index.h"
#include "types.h"
#include "sparce_set.h"
#include "type_traits.h"
#include "util/log.h"

namespace fecs {

    struct group_descriptor : public pool::owner {
        virtual ~group_descriptor() = default;
        virtual bool contains(entity_t entity) const = 0;
        virtual bool own(id_index_t id_index) const = 0;
        virtual void pack_pools() = 0;
    };

    template<typename... Ts>
    requires unique_types<Ts...> && (sizeof...(Ts) > 1)
    class group : public group_descriptor {
    public:
        using components = type_list<Ts...>;        
        using pools_array = std::array<pool*, components::size>;

        group(pools_array&& pools)
            : _pools(std::move(pools)) {
                for(pool* p : _pools){
                   set_ownership(p);
                }
            }

        bool contains(entity_t entity) const override{
            for(const pool* p : _pools){
                if(!p->contains(entity)){
                    return false;
                }
            }
            return true;
        }

        bool own(id_index_t id_index) const override{
            return ((type_index<Ts>::value() == id_index) || ...);
        }

        void pack_pools() override {
            pool* min_pool = *std::min_element(_pools.begin(), _pools.end(),
                [](const pool* a, const pool* b) {
                    return a->size() < b->size();
                });
            const auto& entities = min_pool->get_entities();

            if(entities.empty()){
                return;
            }

            _next_index = 0;

            for(size_t i = 0; i < entities.size(); i++) {
                if(contains(entities[i])){
                    entity_t contained = min_pool->get_entity_by_index(i);
                    for(pool* p : _pools) {
                        entity_t target = p->get_entity_by_index(_next_index);
                        p->swap(contained, target);
                    }
                    _next_index++;
                }
            }
        }

        void trigger_emplace(entity_t entity) override{
            if(contains(entity)){
                entity_t target = _pools[0]->get_entity_by_index(_next_index);
                for(pool* p : _pools){
                    p->swap(target, entity);
                }
                _next_index++;
            }
        }

        void trigger_remove(entity_t entity) override{
            if(contains(entity)){
                size_t last_packed_index = --_next_index;
                entity_t target = _pools[0]->get_entity_by_index(last_packed_index);
                for (pool *p : _pools) {
                    p->swap(entity, target);
                    remove_by_self(p, entity);
                }
            }
        }

        template<typename Func>
        requires std::is_invocable_v<Func, entity_t, Ts&...> || std::is_invocable_v<Func, Ts&...>
        void for_each(Func func) {
            constexpr auto seq = std::make_index_sequence<components::size>{};
            for(size_t i = 0; i < _next_index; i++){
                if constexpr (std::is_invocable_v<Func, entity_t, Ts&...>){
                    entity_t entity = _pools[0]->get_entity_by_index(i);
                    std::apply(func, 
                        std::tuple_cat(
                            std::make_tuple(entity),
                            pack_components(i, seq)));
                }
                else {
                    std::apply(func, pack_components(i, seq));
                }
            }
        }

    private:
        pools_array _pools;
        size_t _next_index = 0;

        entity_t find_swapable(pool* p, size_t start_index){
            const auto& entities = p->get_entities();

            if (start_index >= entities.size()) {
                return error_entity;
            }

            auto iter = std::find_if(entities.begin() + start_index, entities.end(), 
            [&](const entity_t e){
                return !contains(e);
            });

            if(iter != entities.end()){
                return *iter;
            }
            return error_entity;
        }

        template<size_t index>
        auto get_pool() {
            using component_t = typename components::template get<index>;
            return static_cast<sparce_set<component_t>*>(_pools[index]);
        }

        template<size_t... indicies>
        auto pack_components(size_t idx, std::index_sequence<indicies...>){
            return std::forward_as_tuple(get_pool<indicies>()->get_ref_directly(idx)...);
        }

    };

}