#include "sparce_set.h"
#include "type_index.h"
#include "types.h"
#include "util/log.h"
#include "type_traits.h"
#include "group.h"
#include "registory.h"

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
*/

int main() {
    b1(1'000'000);

    return 0;
}

