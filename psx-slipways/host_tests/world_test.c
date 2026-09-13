#include "t.h"
#include "../planetgen.h"
#include "../world.h"

static void test_world_init_produces_90_planets(void) {
    world_init();

    const galaxy_t *g = world_get_galaxy();

    T_CHECK(g != NULL);
    T_CHECK(g->count == PLANETGEN_MAX_PLANETS);
}

static void test_world_init_is_deterministic_across_calls(void) {
    world_init();
    galaxy_t first = *world_get_galaxy();

    world_init();
    const galaxy_t *second = world_get_galaxy();

    T_CHECK(first.count == second->count);
    for (int i = 0; i < first.count && i < second->count; i++) {
        T_CHECK(first.positions[i].x == second->positions[i].x);
        T_CHECK(first.positions[i].y == second->positions[i].y);
    }
}

int main(void) {
    t_run("world_init_produces_90_planets", test_world_init_produces_90_planets);
    t_run("world_init_is_deterministic_across_calls", test_world_init_is_deterministic_across_calls);
    return t_report();
}
