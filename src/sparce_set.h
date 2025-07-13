#pragma once

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>
#include <array>

#include "types.h"
#include "util/log.h"
#include "pool.h"

namespace fecs {

    template<typename T>
    class sparce_set : public pool {
    public:

        sparce_set(size_t reservation = 100){
            _packed.reserve(reservation);
            _entities.reserve(reservation);
        }

        template<typename... Args>
        typename std::enable_if_t<std::is_constructible_v<T, Args...>, void>
        emplace(entity_t entity, Args&&... args)
        {
            size_t index = get_index(entity);
            if(index == error_index) {
                index = _packed.size();
                _packed.emplace_back(std::forward<Args>(args)...);
                _entities.push_back(entity);
                set_index(entity, index);

                if(_owner != nullptr){
                    _owner->trigger_emplace(entity);
                }
            }
            else{
                _packed.emplace(_packed.begin() + index, std::forward<Args>(args)...);
            }
        }

        T* get_ptr(entity_t entity) {
            size_t index = get_index(entity);

            if(index == error_index) {
                return nullptr;
            }

            return &_packed[index];
        }

        T& get_ref(entity_t entity) {
            size_t index = get_index(entity);

            FECS_ASSERT(index != error_index);

            return _packed[index];
        }

        void remove(entity_t entity) override {
            size_t index = get_index(entity);
            if (index == error_index) return;

            if(_owner != nullptr){
                _owner->trigger_remove(entity);
            }
            else{
                remove_by_self(entity);
            }
        }

        void swap(entity_t ent1, entity_t ent2) override {
            size_t i1 = get_index(ent1);
            size_t i2 = get_index(ent2);

            if(i1 == error_index || i2 == error_index || i1 == i2){
                return;
            }

            std::swap(_packed[i1], _packed[i2]);
            std::swap(_entities[i1], _entities[i2]);

            set_index(ent1, i2);
            set_index(ent2, i1);
        }

        bool contains(entity_t entity) const override{
            return get_index(entity) != error_index;
        }

        size_t size() const override{
            return _packed.size();
        }

    protected:
        void remove_by_self(entity_t entity) override {
            size_t index = get_index(entity);
            if (index == error_index) return;

            size_t last_index = _packed.size() - 1; 
            if (index != last_index) {
                std::swap(_packed[index], _packed[last_index]);
                std::swap(_entities[index], _entities[last_index]);

                entity_t moved_entity = _entities[index];
                set_index(moved_entity, index);
            }

            _packed.pop_back();
            _entities.pop_back();

            set_index(entity, error_index);
        }

    private:
        static constexpr size_t sparce_max_size = 512;
        static constexpr size_t error_index = std::numeric_limits<size_t>::max();
        
        using sparce = std::array<size_t, sparce_max_size>; 

        std::vector<T> _packed;
        std::vector<sparce> _sparces;

        void set_index(entity_t entity, size_t index){
            size_t page = entity / sparce_max_size;
            size_t offset = entity % sparce_max_size;

            if(page >= _sparces.size()){
                resize_sparces(page + 1);
            }

            _sparces[page][offset] = index;
        }

        size_t get_index(entity_t entity) const {
            size_t page = entity / sparce_max_size;
            size_t offset = entity % sparce_max_size;

            if(page >= _sparces.size()){
                return error_index;
            }

            return _sparces[page][offset];
        }  

        void resize_sparces(size_t new_size){
            size_t init_size = _sparces.size();
            _sparces.resize(new_size);
            for(size_t i = init_size; i < _sparces.size(); i++){
                _sparces[i].fill(error_index);
            }
        }

    };

}