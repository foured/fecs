#pragma once

#include "type_traits"
#include "sparce_set.h"
#include <type_traits>

namespace fecs {

    template<typename T>
    class runner{
    public:
        using sparce_set_t = sparce_set<T>;

        runner(sparce_set_t* sst)
            : _pool(sst) {}

        template<typename Func>
        requires std::is_invocable_v<Func, T&>
        void for_each(Func func){
            _pool->for_each(func);
        }
        
    private:
        sparce_set_t* _pool;

    };

}