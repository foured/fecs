//
// Created by Ivan on 24.07.2025.
//

#ifndef GROUP_VIEW_H
#define GROUP_VIEW_H

#include "../containers/sparse_set.h"

namespace fecs {

    template<typename, typename>
    class group_slice;

    template<typename... Ts>
    class group_slice<pack_part<Ts...>, view_part<>> {
    public:
        using p_components = type_list<Ts...>;
        using p_pools_array = std::array<pool*, p_components::size>;

        group_slice(const p_pools_array& pools, const size_t* next_index) : _pools(pools), _next_index(next_index)  {}

        template<typename Func>
        requires std::is_invocable_v<Func, Ts&...> || std::is_invocable_v<Func, entity_t, Ts&...>
        void for_each(Func func) {
            for_each_impl(func, p_components::sequence);
        }

    private:
        p_pools_array _pools;
        const size_t *const _next_index = nullptr;

        template<size_t index>
        auto get_pool() const {
            using component_t = typename p_components::template get<index>;
            return static_cast<sparse_set<component_t>*>(_pools[index]);
        }

        template<typename Func, size_t... Is>
        void for_each_impl(Func func, std::index_sequence<Is...>) const {
            if constexpr (std::is_invocable_v<Func, Ts&...>) {
                for (size_t i = 0; i < *_next_index; ++i) {
                    func(get_pool<Is>()->get_ref_directly(i)...);
                }
            }
            else {
                const pool* first_pool = _pools[0];
                for (size_t i = 0; i < *_next_index; ++i) {
                    func(first_pool->get_key_by_index(i), get_pool<Is>()->get_ref_directly(i)...);
                }
            }
        }

    };

    template<typename... PTs, typename... VTs>
    class group_slice<pack_part<PTs...>, view_part<VTs...>> {
    public:
        using p_components = type_list<PTs...>;
        using v_components = type_list<VTs...>;
        using p_pools_array = std::array<pool*, p_components::size>;
        using v_pools_array = std::array<pool*, p_components::size>;

        group_slice(const p_pools_array& p_pools, const v_pools_array& v_pools, const size_t* next_index)
            : _p_pools(p_pools), _v_pools(v_pools), _next_index(next_index)  {}

        template<typename Func>
        requires std::is_invocable_v<Func, PTs&..., VTs&...> || std::is_invocable_v<Func, entity_t, PTs&..., VTs&...>
        void for_each(Func func) {
            for_each_impl(func, p_components::sequence, v_components::sequence);
        }

    private:
        p_pools_array _p_pools;
        v_pools_array _v_pools;
        const size_t *const _next_index = nullptr;

        template<size_t index>
        auto get_pack_pool() const {
            using component_t = typename p_components::template get<index>;
            return static_cast<sparse_set<component_t>*>(_p_pools[index]);
        }

        template<size_t index>
        auto get_view_pool() const {
            using component_t = typename v_components::template get<index>;
            return static_cast<sparse_set<component_t>*>(_v_pools[index]);
        }

        template<typename Func, size_t... PIs, size_t... VIs>
        void for_each_impl(Func func, std::index_sequence<PIs...>, std::index_sequence<VIs...>) {
            const pool* first_pool = _p_pools[0];

            if constexpr (std::is_invocable_v<Func, PTs&..., VTs&...>) {
                for (size_t i = 0; i < *_next_index; ++i) {
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
                        func(get_pack_pool<PIs>()->get_ref_directly(i)...,
                             get_view_pool<VIs>()->get_ref_directly_e(page, offset)...);
                    }
                }
            }
            else {
                for (size_t i = 0; i < *_next_index; ++i) {
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
                             get_pack_pool<PIs>()->get_ref_directly(i)...,
                             get_view_pool<VIs>()->get_ref_directly_e(page, offset)...);
                    }
                }
            }
        }

    };

}

#endif //GROUP_VIEW_H
