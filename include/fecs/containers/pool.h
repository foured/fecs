#pragma once

#include <stddef.h>
#include <vector>

#include "../core/types.h"

namespace fecs {

    template<typename Key>
    requires is_index_type<Key>
    class pool_template {
    public:

        class owner{
        public:
            virtual ~owner() = default;

            virtual void trigger_emplace(Key key) = 0;
            virtual void trigger_remove(Key key) = 0;

        protected:
            void set_ownership(pool_template* p){
                p->_owner = this;
            }

            static void remove_by_self(pool_template* p, Key key){
                p->remove_by_self(key);
            }

        };

        using keys_container = std::vector<Key>;

        virtual ~pool_template() = default;

        [[nodiscard]] const keys_container& get_keys() const{
            return _keys;
        }

        [[nodiscard]] Key get_key_by_index(const size_t index) const {
            return _keys[index];
        }

        virtual void remove(Key key) = 0;
        [[nodiscard]] virtual size_t size() const = 0;
        [[nodiscard]] virtual bool contains(Key key) const = 0;
        [[nodiscard]] virtual bool contains(size_t page, size_t offset) const = 0;
        virtual void swap(Key k1, Key k2) = 0;
        virtual void shrink_to_fit() = 0;

    protected:
        friend class owner;

        keys_container _keys;
        owner* _owner = nullptr;

        virtual void remove_by_self(Key key) = 0;

    };

    using pool = pool_template<entity_t>;

}