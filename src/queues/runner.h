#pragma once

#include "../core/type_traits.h"
#include "../containers/sparce_set.h"

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