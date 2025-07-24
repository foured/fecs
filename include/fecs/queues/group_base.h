//
// Created by Ivan on 23.07.2025.
//

#ifndef GROUP_BASE_H
#define GROUP_BASE_H

#include "../containers/pool.h"
#include "../core/type_index.h"
#include "ranges"

namespace fecs {

    enum class group_mode : uint8_t {
        owning,
        watching
    };

    class group_descriptor : public pool::owner {
    public:
        ~group_descriptor() override = default;
        [[nodiscard]] virtual bool own(id_index_t id_index) const = 0;
        virtual void pack_pools() = 0;

        [[nodiscard]] const size_t * get_next_index_ptr() const {
            return &_next_index;
        }

        template<typename T>
        [[nodiscard]] bool own() const {
            return own(type_index<T>::value());
        }

        template<typename... Ts>
        [[nodiscard]] bool own_all() const {
            return (own(type_index<Ts>::value()) && ...);
        }

    protected:
        size_t _next_index = 0;


    };

    template<typename... Ts>
    requires unique_types<Ts...> && (sizeof...(Ts) > 1)
    class group_base : public group_descriptor {
    public:
        using components = type_list<Ts...>;
        using pools_array = std::array<pool*, components::size>;

        explicit group_base(const pools_array& pools)
            : _pools(pools) {
            for(pool* p : _pools){
                set_ownership(p);
            }
        }

        [[nodiscard]] bool contains(const entity_t entity) const {
            for(const pool* p : _pools){
                if(!p->contains(entity)){
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] bool own(id_index_t id_index) const override{
            return ((type_index<Ts>::value() == id_index) || ...);
        }

        void pack_pools() override {
            const pool* min_pool = *std::min_element(_pools.begin(), _pools.end(),
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
                const entity_t target = _pools[0]->get_key_by_index(last_packed_index);
                for (pool *p : _pools) {
                    p->swap(entity, target);
                    remove_by_self(p, entity);
                }
            }
        }

    protected:
        pools_array _pools;

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


#endif //GROUP_BASE_H
