#include "types.h"
#include "util/log.h"
#include "type_traits.h"
#include "group.h"
#include "pools_registory.h"

struct pos{
    float x;
};  

struct size{
    float v;
};

struct rotation {};

int main() {
    fecs::entity_t e0 = 0;
    fecs::entity_t e1 = 1;
    fecs::entity_t e2 = 2;
    fecs::entity_t e3 = 3;
    fecs::entity_t e4 = 4;

    fecs::pools_registory registory;

    registory.create_group<pos, size>();
    fecs::group_descriptor* gd = registory.find_group<pos, size>();

    registory.add_component<pos>(e0);
    registory.add_component<pos>(e1);
    registory.add_component<pos>(e2);
    registory.add_component<pos>(e4);
    registory.add_component<size>(e1);
    registory.add_component<size>(e2);
    registory.add_component<size>(e3);
    registory.add_component<size>(e4);

    gd->print();

    registory.remove_component<pos>(e1);

    gd->print();

    return 0;
}

