#pragma once

#include <cassert>
#include <type_traits>
#include <unordered_map>
#include <memory>

#include "type_traits.h"
#include "types.h"
#include "../containers/sparse_set.h"
#include "../core/type_index.h"
#include "../queues/group.h"
#include "../queues/view.h"
#include "../queues/runner.h"
#include "../util/log.h"

namespace fecs {

    class registry{
    public:

        // Entities management

        entity_t create_entity() {
            return _entity_counter++;
        }

        void destroy_entity(entity_t entity) {
            for (const auto& p : _pools) {
                if (p->contains(entity)) {
                    p->remove(entity);
                }
            }
        }

        // Components management

        template<typename Component, typename... Args>
        requires std::is_constructible_v<Component, Args&&...>
        void add_component(entity_t entity, Args&&... args) {
            using sparse_t = sparse_set<Component>;

            pool* pool_ptr = find_or_create_pool<Component>();

            sparse_t* sparse_ptr = static_cast<sparse_t*>(pool_ptr);

            sparse_ptr->emplace(entity, std::forward<Args>(args)...);
        }

        template<typename Component>
        void remove_component(entity_t entity){
            auto p = find_pool<Component>();
            if(p != nullptr){
                p->remove(entity);
            }
        }

        template<typename Component>
        bool has_component(entity_t entity) {
            auto p = find_pool<Component>();
            if (p == nullptr) {
                return false;
            }
            return p->contains(entity);
        }

        // Packs all components of types for very fast accsess
        template<typename... Ts>
        requires unique_types<Ts...> && (sizeof...(Ts) > 1)
        void create_group(){
            using group_t = fecs::group<Ts...>;
            id_index_t id_index = type_index<group_t>::value();

            if(_groups.contains(id_index)){
                return;
            }
            
            for(auto& g_uptr : _groups){
                if(((g_uptr->own(type_index<Ts>::value())) || ...)){
                    FECS_ASSERT_M(false, "Groups conflict: only one group can own compoent");
                }
            }

            typename group_t::pools_array pools = { find_or_create_pool<Ts>()... };
            
            size_t index = _groups.emplace(id_index, std::make_unique<group_t>(pools));

            _groups.get_ref_directly(index)->pack_pools();
        }
        
        // Different 'iterators'

        template<typename... Ts>
        requires unique_types<Ts...> && (sizeof...(Ts) > 1)
        fecs::group<Ts...>* group(){
            using group_t = fecs::group<Ts...>;
            auto group_u_ptr = _groups.get_ptr(type_index<group_t>::value());
            if (group_u_ptr != nullptr) {
                return static_cast<group_t*>(group_u_ptr->get());
            }
            FECS_ASSERT_M(false, "Before using registory::group you have to registory::create_group");
        }

        template<typename... Ts>
        requires unique_types<Ts...> && (sizeof...(Ts) > 1)
        fecs::view<Ts...> view() {
            using view_t = fecs::view<Ts...>;

            typename view_t::pools_array arr { find_pool<Ts>()... };
            return view_t(arr); 
        }

        template<typename T>
        fecs::runner<T> runner(){
            return fecs::runner<T>(find_pool<T>());
        }

        template<typename T, typename Func>
        requires std::is_invocable_v<Func, T&>
        void direct_for_each(Func func) {
            find_pool<T>()->for_each(func);
        }
        
        // Help methods

        void shrink_to_fit() {
            for (std::unique_ptr<pool>& p : _pools) {
                p->shrink_to_fit();
            }
        }

        template<typename Component>
        sparse_set<Component>* find_pool(){
            return static_cast<sparse_set<Component>*>(find_pool(type_index<Component>::value()));
        }

        pool* find_pool(id_index_t associated_component){
            std::unique_ptr<pool>* p = _pools.get_ptr(associated_component);
            if (p != nullptr) {
                return p->get();
            }
            FECS_LOG_WARN << "Returning nullptr in pools_registry::find_pool" << FECS_NL;
            return nullptr;
        }

    private:
        sparse_set_template<id_index_t, std::unique_ptr<pool>> _pools;
        sparse_set_template<id_index_t, std::unique_ptr<group_descriptor>> _groups;
        entity_t _entity_counter = 0;

        template<typename T>
        pool* find_or_create_pool() {
            id_index_t t_index = type_index<T>::value();

            size_t index = _pools.try_emplace(t_index, std::make_unique<sparse_set<T>>());

            return _pools.get_ref_directly(index).get();
        }

    };

}