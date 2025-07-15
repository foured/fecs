#include "core/registory.h"
#include "core/type_traits.h"

#include "benchmarks/b1.hpp"

/*
1:
    u: 18004666 ns
    avg: 18.0047
    g: 7871458 ns
    avg: 7.87146
    v: 33177542 ns
    avg: 33.1775
    r: 6885708 ns
    avg: 6.88571
    d: 5526833 ns
    avg: 5.52683
2:
    u: 19277959 ns
    avg: 19.278
    g: 7843916 ns
    avg: 7.84392
    v: 34813625 ns
    avg: 34.8136
    r: 5980416 ns
    avg: 5.98042
    d: 5640458 ns
    avg: 5.64046
3:
    u: 19359958 ns
    avg: 19.36
    g: 7961292 ns
    avg: 7.96129
    v: 20551334 ns
    avg: 20.5513
    r: 5352958 ns
    avg: 5.35296
    d: 5519125 ns
    avg: 5.51912
4: (win, release, msvc)
    u: 10658600 ns
    avg: 10.6586
    g: 2775000 ns
    avg: 2.775
    v: 5109400 ns
    avg: 5.1094
    r: 1266500 ns
    avg: 1.2665
    d: 1194300 ns
    avg: 1.1943
*/

struct position {
    float x = 0;
};

struct rotation {
    float x = 1;
};

struct ragdoll {
    float x = 2;
};

struct renderable {
    float x = 3;
};

int main() {
    //b1(1'000'000);
    
    fecs::registory registory;

    fecs::entity_t e1 = registory.create_entity();
    registory.add_component<position>(e1);
    registory.add_component<rotation>(e1);
    registory.add_component<ragdoll>(e1);
    registory.add_component<renderable>(e1);

    registory.create_group<position, rotation, ragdoll, renderable>();

    auto g = registory.group<position, rotation, ragdoll, renderable>();
    auto gv = g->view<position, renderable, rotation>();

    gv.for_each([](position& p, renderable& re, rotation& ro) {
            std::cout << p.x << " " << re.x << " " << ro.x << "\n";
        });

    return 0;
}