//
// Created by Ivan on 29.07.2025.
//

#ifndef ENTITY_BUILDER_H
#define ENTITY_BUILDER_H

#include "../core/registry.h"

namespace fecs {

    class entity_builder {
    public:
        explicit entity_builder(registry& registry) : _registry(registry) {
            _entity = _registry.create_entity();
        }

        template<typename Component, typename... Args>
        requires std::is_constructible_v<Component, Args&&...>
        void add_component(Args&&... args) {
            _registry.add_component<Component>(_entity, std::forward<Args>(args)...);
        }

        template<typename Component, typename... Args>
        requires std::is_constructible_v<Component, Args&&...>
        entity_builder& with(Args&&... args) {
            add_component<Component>(std::forward<Args>(args)...);
            return *this;
        }

        [[nodiscard]] entity_t get() const {
            return _entity;
        }

    private:
        registry& _registry;
        entity_t _entity;

    };

} // fecs

#endif //ENTITY_BUILDER_H
