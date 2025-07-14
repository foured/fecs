#pragma once

#include <cassert>
#include <type_traits>
#include <unordered_map>
#include <memory>

#include "type_traits.h"
#include "types.h"
#include "sparce_set.h"
#include "type_index.h"
#include "group.h"
#include "view.h"
#include "runner.h"
#include "util/log.h"

namespace fecs {

    class registory{
    public:

        template<typename Component, typename... Args>
        typename std::enable_if_t<std::is_constructible_v<Component, Args&&...>, void>
        add_component(entity_t entity, Args&&... args) {
            using sparce_t = sparce_set<Component>;

            pool* pool_ptr = find_or_create_pool<Component>();

            sparce_t* sparce_ptr = static_cast<sparce_t*>(pool_ptr);

            sparce_ptr->emplace(entity, std::forward<Args>(args)...);
        }

        template<typename Component>
        void remove_component(entity_t entity){
            pool* p = find_pool<Component>();
            if(p != nullptr){
                p->remove(entity);
            }
        }

        // Packs all components of types for very fast accsess
        template<typename... Ts>
        requires unique_types<Ts...> && (sizeof...(Ts) > 1)
        void create_group(){
            using group_t = fecs::group<Ts...>;
            id_index_t id_index = type_index<group_t>::value();

            if(_groups_map.contains(id_index)){
                return;
            }
            
            for(auto& [key, group_d] : _groups_map){
                if(((group_d->own(type_index<Ts>::value())) || ...)){
                    FECS_ASSERT_M(false, "Groups conflict: only one group can own compoent");
                }
            }

            typename group_t::pools_array pools = { find_or_create_pool<Ts>()... };
            
            auto[iter, inserted] = 
                _groups_map.emplace(id_index, std::make_unique<group_t>(std::move(pools)));

            FECS_ASSERT_M(inserted, "Error while cereating group");

            group_descriptor* descriptor = iter->second.get();

            descriptor->pack_pools();
        }
        
        // Different 'iterators'

        template<typename... Ts>
        requires unique_types<Ts...> && (sizeof...(Ts) > 1)
        fecs::group<Ts...>* group(){
            using group_t = fecs::group<Ts...>;
            auto it = _groups_map.find(type_index<group_t>::value());
            if(it != _groups_map.end()){
                return static_cast<group_t*>(it->second.get());
            }
            FECS_ASSERT_M(false, "Before using registory::grup you have to registory::create_group")
        }

        template<typename... Ts>
        requires unique_types<Ts...> && (sizeof...(Ts) > 1)
        fecs::view<Ts...> view() {
            using view_t = fecs::view<Ts...>;

            typename view_t::pools_array arr { find_pool<Ts>()... };
            return view_t(std::move(arr));
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
        
        template<typename Component>
        sparce_set<Component>* find_pool(){
            return static_cast<sparce_set<Component>*>(find_pool(type_index<Component>::value()));
        }

        pool* find_pool(id_index_t associated_component){
            auto it = _pools_map.find(associated_component);
            if(it != _pools_map.end()){
                return it->second.get();
            }
            FECS_LOG_WARN << "Returning nullptr in pools_registory::find_pool" << FECS_NL;
            return nullptr;
        }

    private:
        std::unordered_map<id_index_t, std::unique_ptr<pool>> _pools_map;
        std::unordered_map<id_index_t, std::unique_ptr<group_descriptor>> _groups_map;

        template<typename T>
        pool* find_or_create_pool() {
            id_index_t index = type_index<T>::value();

            auto [iter, inserted] = _pools_map.try_emplace(
                index, std::make_unique<sparce_set<T>>()
            );

            return iter->second.get();
        }

    };

}