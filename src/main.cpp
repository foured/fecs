#include "sparce_set.h"
#include "type_index.h"
#include "types.h"
#include "util/log.h"
#include "type_traits.h"
#include "group.h"
#include "pools_registory.h"

struct pos {
    pos(int v) : x(v) {}
    int x;
};
struct size {
    size(int v) : y(v) {}
    int y;
};

int main() {
    fecs::pools_registory registory;
    registory.create_group<pos, size>();

    fecs::entity_t e0 = 0;
    fecs::entity_t e1 = 1;

    registory.add_component<pos>(e0, 1);
    registory.add_component<pos>(e1, 2);
    registory.add_component<size>(e0, 11);
    registory.add_component<size>(e1, 22);

    fecs::group_descriptor* gd = registory.find_group<pos, size>();
    fecs::group<pos, size>* g = static_cast<fecs::group<pos, size>*>(gd);
    g->for_each([](pos& p, size& s) {
        p.x++;
        s.y++;
        FECS_LOG << p.x << " " << s.y << FECS_NL;
    });

    g->for_each([](fecs::entity_t e, pos& p, size& s) {
        p.x++;
        s.y++;
        FECS_LOG << e << " " << p.x << " " << s.y << FECS_NL;
    });

    return 0;
}

