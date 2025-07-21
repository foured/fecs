#pragma once

#include <limits>
#include <type_traits>
#include <utility>
#include <vector>
#include <array>

#include "../core/type_traits.h"
#include "../util/log.h"
#include "pool.h"

#define SPARSE_MAX_SIZE 512

namespace fecs {

    template<typename Key, typename T, size_t chunk_size = 512>
    requires is_index_type<Key> && (!std::is_pointer_v<T>)
    class sparse_set_template : public pool_template<Key> {
    public:
        using pool_t = pool_template<Key>;
        using sparse = std::array<size_t, chunk_size>;
        using packed_t = std::vector<T>;

        using iterator = typename packed_t::iterator;
        using const_iterator = typename packed_t::const_iterator;
        using reverse_iterator = typename packed_t::reverse_iterator;
        using const_reverse_iterator = typename packed_t::const_reverse_iterator;

        explicit sparse_set_template(size_t reservation = 10){
            _packed.reserve(reservation);
            _keys.reserve(reservation);
            resize_sparses(1);
        }

        template<typename... Args>
        requires std::is_constructible_v<T, Args...>
        size_t emplace(Key key, Args&&... args)
        {
            size_t index = get_index(key);
            if(index == error_index) {
                index = _packed.size();
                _packed.emplace_back(std::forward<Args>(args)...);
                _keys.push_back(key);
                set_index(key, index);

                if(_owner != nullptr){
                    _owner->trigger_emplace(key);
                }
            }
            else{
                _packed.emplace(_packed.begin() + index, std::forward<Args>(args)...);
            }
            return index;
        }

        template<typename... Args>
        requires std::is_constructible_v<T, Args...>
        size_t try_emplace(Key key, Args&&... args)
        {
            size_t index = get_index(key);
            if(index == error_index) {
                index = _packed.size();
                _packed.emplace_back(std::forward<Args>(args)...);
                _keys.push_back(key);
                set_index(key, index);

                if(_owner != nullptr){
                    _owner->trigger_emplace(key);
                }
            }
            return index;
        }

        void remove(Key key) override {
            size_t index = get_index(key);
            if (index == error_index) return;

            if(_owner != nullptr){
                _owner->trigger_remove(key);
            }
            else{
                remove_by_self(key);
            }
        }

        void swap(Key k1, Key k2) override {
            size_t i1 = get_index(k1);
            size_t i2 = get_index(k2);

            if(i1 == error_index || i2 == error_index || i1 == i2){
                return;
            }

            std::swap(_packed[i1], _packed[i2]);
            std::swap(_keys[i1], _keys[i2]);

            set_index(k1, i2);
            set_index(k2, i1);
        }

        bool contains(Key key) const override{
            size_t page = key / chunk_size;

            if(page >= _sparses.size()){
                return false;
            }
            size_t offset = key % chunk_size;

            return _sparses[page][offset] != error_index;
        }

        bool contains(size_t page, size_t offset) const override{
            if(page > _sparses.size()) return false;
            return _sparses[page][offset != error_index];
        }

        size_t size() const override{
            return _packed.size();
        }

        void shrink_to_fit() override {
            _packed.shrink_to_fit();
            _keys.shrink_to_fit();
        }

        template<typename Func>
        requires std::is_invocable_v<Func, T&>
        void for_each(Func func){
            const size_t s = size();
            for(size_t i = 0; i < s; ++i){
                func(_packed[i]);
            }
        }

        T* get_ptr(Key key) {
            size_t index = get_index(key);

            if(index == error_index) {
                return nullptr;
            }

            return &_packed[index];
        }

        T& get_ref(Key key) {
            size_t index = get_index(key);

            FECS_ASSERT(index != error_index);

            return _packed[index];
        }

        T& get_ref_directly_e(Key key) {
            size_t page = key / chunk_size;
            size_t offset = key % chunk_size;

            return _packed[_sparses[page][offset]];
        }

        T& get_ref_directly_e(size_t page, size_t offset) {
            return _packed[_sparses[page][offset]];
        }

        T& get_ref_directly(size_t idx) {
            return _packed[idx];
        }

        T* get_ptr_directly(size_t idx) {
            return &_packed[idx];
        }

        iterator begin() {
            return _packed.begin();
        }

        iterator end() {
            return _packed.end();
        }

        const_iterator begin() const {
            return _packed.cbegin();
        }

        const_iterator end() const {
            return _packed.cend();
        }

        reverse_iterator rbegin() {
            return _packed.rbegin();
        }

        reverse_iterator rend() {
            return _packed.rend();
        }

        const_reverse_iterator rbegin() const {
            return _packed.crbegin();
        }

        const_reverse_iterator rend() const {
            return _packed.crend();
        }

    protected:
        using pool_t::_keys;
        using pool_t::_owner;

        void remove_by_self(Key key) override {
            size_t index = get_index(key);
            if (index == error_index) return;

            size_t last_index = _packed.size() - 1; 
            if (index != last_index) {
                std::swap(_packed[index], _packed[last_index]);
                std::swap(_keys[index], _keys[last_index]);

                Key moved_key = _keys[index];
                set_index(moved_key, index);
            }

            _packed.pop_back();
            _keys.pop_back();

            set_index(key, error_index);
        }

    private:            
        static constexpr size_t error_index = std::numeric_limits<size_t>::max();

        packed_t _packed;
        std::vector<sparse> _sparses;

        void set_index(Key key, size_t index){
            size_t page = key / chunk_size;

            if(page >= _sparses.size()) {
                resize_sparses(page + 1);
            }
            size_t offset = key % chunk_size;

            _sparses[page][offset] = index;
        }

        size_t get_index(Key key) const {
            size_t page = key / chunk_size;

            if(page >= _sparses.size()){
                return error_index;
            }
            size_t offset = key % chunk_size;

            return _sparses[page][offset];
        }  

        void resize_sparses(size_t new_size){
            size_t init_size = _sparses.size();
            _sparses.resize(new_size);
            for(size_t i = init_size; i < _sparses.size(); i++){
                _sparses[i].fill(error_index);
            }
        }

    };

    template<typename T>
    using sparse_set = sparse_set_template<entity_t, T, SPARSE_MAX_SIZE>;

}