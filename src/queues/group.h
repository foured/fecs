#pragma once

#include <algorithm>
#include <stddef.h>
#include <array>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../core/type_index.h"
#include "../core/types.h"
#include "../core/type_traits.h"
#include "../containers/sparse_set.h"
#include "../util/log.h"
#include "group_view.h"

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

        group(const pools_array& pools)
            : _pools(pools) {
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
            const auto& entities = min_pool->get_keys();

            if(entities.empty()){
                return;
            }

            _next_index = 0;

            for(size_t i = 0; i < entities.size(); i++) {
                if(contains(entities[i])){
                    entity_t contained = min_pool->get_key_by_index(i);
                    for(pool* p : _pools) {
                        entity_t target = p->get_key_by_index(_next_index);
                        p->swap(contained, target);
                    }
                    _next_index++;
                }
            }
        }

        void trigger_emplace(entity_t entity) override{
            if(contains(entity)){
                entity_t target = _pools[0]->get_key_by_index(_next_index);
                for(pool* p : _pools){
                    p->swap(target, entity);
                }
                _next_index++;
            }
        }

        void trigger_remove(entity_t entity) override{
            if(contains(entity)){
                size_t last_packed_index = --_next_index;
                entity_t target = _pools[0]->get_key_by_index(last_packed_index);
                for (pool *p : _pools) {
                    p->swap(entity, target);
                    remove_by_self(p, entity);
                }
            }
        }

        template<typename Func>
        requires std::is_invocable_v<Func, Ts&...>
        void for_each(Func func) {
            for_each_impl(func, components::sequence);
        }

        template<typename... Types>
        requires (unique_types<Types...>) && (sizeof...(Types) > 1) && (components::template contains_all<Types...>)
        group_view<Types...> view() {
            using group_view_t = group_view<Types...>;
            typename group_view_t::pools_array arr = { find_pool<Types>(components::sequence)... };

            return group_view_t(arr, &_next_index);
        }
        
    private:
        pools_array _pools;
        size_t _next_index = 0;

        entity_t find_swapable(pool* p, size_t start_index){
            const auto& entities = p->get_keys();

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
        auto get_pool() const {
            using component_t = typename components::template get<index>;
            return static_cast<sparse_set<component_t>*>(_pools[index]);
        }

        template<size_t... indicies>
        auto pack_components(size_t idx, std::index_sequence<indicies...>) const{
            return std::forward_as_tuple(get_pool<indicies>()->get_ref_directly(idx)...);
        }

        template<typename Func, size_t... Is>
        void for_each_impl(Func func, std::index_sequence<Is...>) {
            for(size_t i = 0; i < _next_index; ++i){
                func(get_pool<Is>()->get_ref_directly(i)...);
            }
        }

        template<typename T, size_t... Is>
        requires components::template contains_all<T>
        pool* find_pool(std::index_sequence<Is...>) const {
            using pool_t = sparse_set<T>*;

            pool* result = nullptr;

            (([&] {
                if constexpr (std::is_same_v<pool_t, decltype(get_pool<Is>())>) {
                    result = get_pool<Is>();
                }
            }()), ...);

            return result;
        }

    };

}