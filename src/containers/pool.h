#pragma once

#include <stddef.h>
#include <vector>

#include "../core/types.h"

namespace fecs {

    class pool {
    public:

        class owner{
        public:
            virtual ~owner() = default;

            virtual void trigger_emplace(entity_t entity) = 0;
            virtual void trigger_remove(entity_t entity) = 0;

        protected:
            void set_ownership(pool* p){
                p->_owner = this;
            }

            void remove_by_self(pool* p, entity_t entity){
                p->remove_by_self(entity);
            }

        };

        using entities_container = std::vector<entity_t>;

        virtual ~pool() = default;

        const entities_container& get_entities() const{
            return _entities;
        }

        entity_t get_entity_by_index(size_t index) const{
            return _entities[index];
        }

        virtual void remove(entity_t entity) = 0;
        virtual size_t size() const = 0;
        virtual bool contains(entity_t entity) const = 0;
        virtual bool contains(size_t page, size_t offset) const = 0;
        virtual void swap(entity_t ent1, entity_t ent2) = 0;
        virtual void shrink_to_fit() = 0;

    protected:
        friend class owner;

        entities_container _entities;
        owner* _owner = nullptr;

        virtual void remove_by_self(entity_t entity) = 0;

    };

}