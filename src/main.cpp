#include "sparce_set.h"
#include "type_index.h"
#include "types.h"
#include "util/log.h"
#include "type_traits.h"
#include "group.h"
#include "pools_registory.h"

struct pos {};
struct size {};

int main() {
    fecs::pools_registory registory;
    registory.create_group<pos, size>();

    fecs::group_descriptor* gd = registory.find_group<pos, size>();
    fecs::group<pos, size>* g = static_cast<fecs::group<pos, size>*>(gd);

    return 0;
}

