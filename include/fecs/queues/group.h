#pragma once

#include <algorithm>
#include <array>
#include <tuple>
#include <type_traits>
#include <utility>

#include "group_base.h"
#include "../core/type_index.h"
#include "../core/types.h"
#include "../core/type_traits.h"
#include "../containers/sparse_set.h"
#include "group_part.h"

namespace fecs {

    template<typename... Ts>
    class pack_part{};

    template<typename... Ts>
    class view_part{};

    template<typename, typename>
    struct group_args_descriptor;

    template<typename... Ts>
    requires unique_types<Ts...> && (sizeof...(Ts) > 1)
    struct group_args_descriptor<pack_part<Ts...>, view_part<>> { };

    template<typename... PTs, typename... VTs>
    requires unique_types<PTs..., VTs...> && (sizeof...(PTs) > 1)
    struct group_args_descriptor<pack_part<PTs...>, view_part<VTs...>> { };

    template<typename, typename>
    class group;

    template<typename... PTs, typename... VTs>
    requires unique_types<PTs..., VTs...> && (sizeof...(PTs) > 1)
    class group<pack_part<PTs...>, view_part<VTs...>> : public group_base<PTs...> {
    public:
        using group_base_t = group_base<PTs...>;
        using p_components = typename group_base_t::components;
        using p_pools_array = typename group_base_t::pools_array;
        using v_components = type_list<VTs...>;
        using v_pools_array = std::array<pool*, v_components::size>;

        group(const p_pools_array& p_pools, const v_pools_array& v_pools) \
        : group_base_t(p_pools), _v_pools(v_pools) {}

        template<typename Func>
        requires std::is_invocable_v<Func, PTs&..., VTs&...> || std::is_invocable_v<Func, entity_t, PTs&..., VTs&...>
        void for_each(Func func) {
            for_each_impl(func, p_components::sequence, v_components::sequence);
        }

    private:
        using group_base_t::_pools;
        using group_base_t::_next_index;

        v_pools_array _v_pools;

        template<size_t index>
        auto get_view_pool() const {
            using component_t = typename v_components::template get<index>;
            return static_cast<sparse_set<component_t>*>(_v_pools[index]);
        }

        template<typename Func, size_t... PIs, size_t... VIs>
        void for_each_impl(Func func, std::index_sequence<PIs...>, std::index_sequence<VIs...>) {
            pool* first_pool = _pools[0];

            if constexpr (std::is_invocable_v<Func, PTs&..., VTs&...>) {
                for (size_t i = 0; i < _next_index; ++i) {
                    const entity_t e = first_pool->get_key_by_index(i);
                    const size_t page = e / SPARSE_MAX_SIZE;
                    const size_t offset = e % SPARSE_MAX_SIZE;

                    bool passed = true;
                    for (size_t j = 0; j < v_components::size; ++j) {
                        if (!_v_pools[j]->contains(page, offset)) {
                            passed = false;
                            break;
                        }
                    }
                    if (passed) {
                        func(group_base_t::template get_pool<PIs>()->get_ref_directly(i)...,
                                                    get_view_pool<VIs>()->get_ref_directly_e(page, offset)...);
                    }
                }
            }
            else {
                for (size_t i = 0; i < _next_index; ++i) {
                    const entity_t e = first_pool->get_key_by_index(i);
                    const size_t page = e / SPARSE_MAX_SIZE;
                    const size_t offset = e % SPARSE_MAX_SIZE;

                    bool passed = true;
                    for (size_t j = 0; j < v_components::size; ++j) {
                        if (!_v_pools[j]->contains(page, offset)) {
                            passed = false;
                            break;
                        }
                    }
                    if (passed) {
                        func(e,
                            group_base_t::template get_pool<PIs>()->get_ref_directly(i)...,
                                                    get_view_pool<VIs>()->get_ref_directly_e(page, offset)...);
                    }
                }
            }
        }

    };

    template<typename... Ts>
    requires unique_types<Ts...> && (sizeof...(Ts) > 1)
    class group<pack_part<Ts...>, view_part<>> : public group_base<Ts...> {
    public:
        using group_base_t = group_base<Ts...>;
        using p_pools_array = typename group_base_t::pools_array;
        using p_components = typename group_base_t::components;

        group(const p_pools_array& pools) : group_base_t(pools) {}

        template<typename Func>
        requires std::is_invocable_v<Func, Ts&...> || std::is_invocable_v<Func, entity_t, Ts&...>
        void for_each(Func func) {
            for_each_impl(func, p_components::sequence);
        }

    private:
        using group_base_t::_pools;
        using group_base_t::_next_index;

        template<typename Func, size_t... Is>
        void for_each_impl(Func func, std::index_sequence<Is...>) {
            if constexpr (std::is_invocable_v<Func, Ts&...>) {
                for (size_t i = 0; i < _next_index; ++i) {
                    func(group_base_t::template get_pool<Is>()->get_ref_directly(i)...);
                }
            }
            else {
                pool* first_pool = _pools[0];
                for (size_t i = 0; i < _next_index; ++i) {
                    func(first_pool->get_key_by_index(i), group_base_t::template get_pool<Is>()->get_ref_directly_e(i)...);
                }
            }
        }
    };


    // template<typename... Ts>
    // requires unique_types<Ts...> && (sizeof...(Ts) > 1)
    // class group : public group_descriptor {
    // public:
    //     using components = type_list<Ts...>;
    //     using pools_array = std::array<pool*, components::size>;
    //
    //     group(const pools_array& pools)
    //         : _pools(pools) {
    //         for(pool* p : _pools){
    //             set_ownership(p);
    //         }
    //     }
    //
    //     bool contains(entity_t entity) const {
    //         for(const pool* p : _pools){
    //             if(!p->contains(entity)){
    //                 return false;
    //             }
    //         }
    //         return true;
    //     }
    //
    //     bool own(id_index_t id_index) const override{
    //         return ((type_index<Ts>::value() == id_index) || ...);
    //     }
    //
    //     void pack_pools() override {
    //         pool* min_pool = *std::min_element(_pools.begin(), _pools.end(),
    //             [](const pool* a, const pool* b) {
    //                 return a->size() < b->size();
    //             });
    //         const auto& entities = min_pool->get_keys();
    //
    //         if(entities.empty()){
    //             return;
    //         }
    //
    //         _next_index = 0;
    //
    //         for(size_t i = 0; i < entities.size(); i++) {
    //             if(contains(entities[i])){
    //                 entity_t contained = min_pool->get_key_by_index(i);
    //                 for(pool* p : _pools) {
    //                     entity_t target = p->get_key_by_index(_next_index);
    //                     p->swap(contained, target);
    //                 }
    //                 _next_index++;
    //             }
    //         }
    //     }
    //
    //     void trigger_emplace(entity_t entity) override{
    //         if(contains(entity)){
    //             entity_t target = _pools[0]->get_key_by_index(_next_index);
    //             for(pool* p : _pools){
    //                 p->swap(target, entity);
    //             }
    //             _next_index++;
    //         }
    //     }
    //
    //     void trigger_remove(entity_t entity) override{
    //         if(contains(entity)){
    //             size_t last_packed_index = --_next_index;
    //             entity_t target = _pools[0]->get_key_by_index(last_packed_index);
    //             for (pool *p : _pools) {
    //                 p->swap(entity, target);
    //                 remove_by_self(p, entity);
    //             }
    //         }
    //     }
    //
    //     template<typename Func>
    //     requires std::is_invocable_v<Func, Ts&...> || std::is_invocable_v<Func, entity_t, Ts&...>
    //     void for_each(Func func) {
    //         for_each_impl(func, components::sequence);
    //     }
    //
    //     template<typename... Types>
    //     requires unique_types<Types...> && (sizeof...(Types) > 1) && components::template contains_all<Types...>
    //     group_part<Types...> part() {
    //         using group_view_t = group_part<Types...>;
    //         typename group_view_t::pools_array arr = { find_pool<Types>(components::sequence)... };
    //
    //         return group_view_t(arr, &_next_index);
    //     }
    //
    // private:
    //     pools_array _pools;
    //     size_t _next_index = 0;
    //
    //     entity_t find_swapable(pool* p, size_t start_index){
    //         const auto& entities = p->get_keys();
    //
    //         if (start_index >= entities.size()) {
    //             return error_entity;
    //         }
    //
    //         auto iter = std::find_if(entities.begin() + start_index, entities.end(),
    //         [&](const entity_t e){
    //             return !contains(e);
    //         });
    //
    //         if(iter != entities.end()){
    //             return *iter;
    //         }
    //         return error_entity;
    //     }
    //
    //     template<size_t index>
    //     auto get_pool() const {
    //         using component_t = typename components::template get<index>;
    //         return static_cast<sparse_set<component_t>*>(_pools[index]);
    //     }
    //
    //     template<size_t... indicies>
    //     auto pack_components(size_t idx, std::index_sequence<indicies...>) const{
    //         return std::forward_as_tuple(get_pool<indicies>()->get_ref_directly(idx)...);
    //     }
    //
    //     template<typename Func, size_t... Is>
    //     void for_each_impl(Func func, std::index_sequence<Is...>) {
    //         if constexpr (std::is_invocable_v<Func, Ts&...>) {
    //             for(size_t i = 0; i < _next_index; ++i){
    //                 func(get_pool<Is>()->get_ref_directly(i)...);
    //             }
    //         }
    //         else {
    //             pool* f_pool = _pools[0];
    //             for(size_t i = 0; i < _next_index; ++i){
    //                 func(f_pool->get_key_by_index(i), get_pool<Is>()->get_ref_directly(i)...);
    //             }
    //         }
    //
    //     }
    //
    //     template<typename T, size_t... Is>
    //     requires components::template contains_all<T>
    //     pool* find_pool(std::index_sequence<Is...>) const {
    //         using pool_t = sparse_set<T>*;
    //
    //         pool* result = nullptr;
    //
    //         (([&] {
    //             if constexpr (std::is_same_v<pool_t, decltype(get_pool<Is>())>) {
    //                 result = get_pool<Is>();
    //             }
    //         }()), ...);
    //
    //         return result;
    //     }
    //
    // };

}