#pragma once

#include <cassert>
#include <type_traits>
#include <unordered_map>

#include "type_traits.h"
#include "types.h"
#include "sparce_set.h"
#include "type_index.h"
#include "group.h"
#include "util/log.h"

namespace fecs {

    class pools_registory{
    public:

        template<typename Component, typename... Args>
        typename std::enable_if_t<std::is_constructible_v<Component, Args&&...>, void>
        add_component(entity_t entity, Args&&... args) {
            using sparce_t = sparce_set<Component>;

            pool* pool_ptr = find_or_create_pool<Component>();

            sparce_t* sparce_ptr = static_cast<sparce_t*>(pool_ptr);

            sparce_ptr->emplace(entity, std::forward<Args>(args)...);
        }

        pool* get_pool(id_index_t comp_index) {
            if(_pools_map.contains(comp_index)){
                return _pools_map[comp_index];
            }
            return nullptr;
        }

        template<typename Component>
        void remove_component(entity_t entity){
            pool* p = find_pool<Component>();
            if(p != nullptr){
                p->remove(entity);
            }
        }

        template<typename... Ts>
        requires unique_types<Ts...> && (sizeof...(Ts) > 1)
        void create_group(){
            using group_t = group<Ts...>;
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
                _groups_map.emplace(id_index, new group_t(std::move(pools)));

            FECS_ASSERT_M(inserted, "Error while cereating group");

            group_descriptor* descriptor = iter->second;

            descriptor->pack_pools();
        }
        
        template<typename Component>
        pool* find_pool(){
            return find_pool(type_index<Component>::value());
        }

        pool* find_pool(id_index_t associated_component){
            auto it = _pools_map.find(associated_component);
            if(it != _pools_map.end()){
                return it->second;
            }
            FECS_LOG_WARN << "Returning nullptr in pools_registory::find_pool" << FECS_NL;
            return nullptr;
        }

        template<typename... Ts>
        requires unique_types<Ts...> && (sizeof...(Ts) > 1)
        group_descriptor* find_group(){
            using group_t = group<Ts...>;
            auto it = _groups_map.find(type_index<group_t>::value());
            if(it != _groups_map.end()){
                return it->second;
            }
            FECS_LOG_WARN << "Returning nullptr in pools_registory::find_group" << FECS_NL;
            return nullptr;
        }

    private:
        std::unordered_map<id_index_t, pool*> _pools_map;
        std::unordered_map<id_index_t, group_descriptor*> _groups_map;

        template<typename T>
        pool* find_or_create_pool(){
            id_index_t index = type_index<T>::value();

            auto [iter, inserted] = _pools_map.try_emplace(
                index, new sparce_set<T>()
            );

            return iter->second;
        }

    };

}