#pragma once

#include "../core/type_traits.h"
#include "../containers/sparse_set.h"

namespace fecs {

    template<typename T>
    class runner{
    public:
        using sparse_set_t = sparse_set<T>;

        runner(sparse_set_t* sst)
            : _pool(sst) {}

        template<typename Func>
        requires std::is_invocable_v<Func, T&> || std::is_invocable_v<Func, entity_t, T&>
        void for_each(Func func){
            _pool->for_each(func);
        }
        
    private:
        sparse_set_t* _pool;

    };

}