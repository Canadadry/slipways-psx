/* The placeholder `mo` that pico8_entity_init() leaves until `jl()` replaces
 * it with a `qb`, read through pico8_mo_qb(). The frozen
 * values were taken on the code where the placeholder was still a
 * class-less instance and must not be re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "p8/assets.h"
#include "engine/bootstrap.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"
#include "game/scroll.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "engine/vec.h"

#define GD_NEAR 1
#define GD_FAR 0
#define LJ_QA_X 0
#define LJ_QA_Y 0
#define QB_RA 192

static p8_memory_t mem;
static p8_graphics_t gfx;
static p8_input_t input;

static int ents_listed(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return 1;
        }
    }
    return 0;
}

/* Not a qb; no method answers and the camera stays put; hv and cv are
 * empty. */
static void assert_placeholder(void) {
    const char *buckets[] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};
    entity_t *mo = main_camera();
    int i;

    assert(mo != NULL && pico8_mo_qb() == NULL);
    p8_camera(&gfx, 5, 6);
    assert(mo->class != pico8_qb_struct_class());
    assert(mo->class->states[ENTITY_STATE_NORMAL] == NULL && mo->class->draw == NULL);
    assert(gfx.camera_x == 5 && gfx.camera_y == 6);
    assert(live_entities()->count == 0);
    for (i = 0; i < 5; i++) {
        assert(entities_by_message(buckets[i])->count == 0);
    }
}

static void assert_a_qb(void) {
    const vec_t *ra;

    assert(pico8_mo_qb() != NULL && pico8_mo_qb()->ra != NULL);
    ra = pico8_mo_qb()->ra;
    assert(ra->x == p8_fixed_from_int(QB_RA) && ra->y == p8_fixed_from_int(QB_RA));
}

static void test_placeholder_after_init(void) {
    entity_registry_init();
    assert_placeholder();
    assert(!ents_listed(live_entities(), main_camera()));
}

typedef struct {
    entity_t header;
    const vec_t *ra;
} ra_only_t;

static const entity_class_t g_ra_only_class = {0};

/* `gd` reads `mo.ra` as (0,0): 64 off is inside the 76 margin, 77 is not. */
static void test_gd_on_placeholder(void) {
    vec_t near_ra = vec_make(p8_fixed_from_int(64), p8_fixed_from_int(64));
    vec_t far_ra = vec_make(p8_fixed_from_int(64 + 77), p8_fixed_from_int(64));
    ra_only_t near_e = {{.class = &g_ra_only_class, .pos = &near_e.ra}, NULL};
    ra_only_t far_e = {{.class = &g_ra_only_class, .pos = &far_e.ra}, NULL};
    int near_hit;
    int far_hit;

    entity_registry_init();
    near_e.ra = &near_ra;
    far_e.ra = &far_ra;
    near_hit = entity_geom_is_on_screen(&near_e.header);
    far_hit = entity_geom_is_on_screen(&far_e.header);
    printf("[gd] near %d far %d\n", near_hit, far_hit);
    fflush(stdout);
    assert(near_hit == GD_NEAR && far_hit == GD_FAR);
}

/* `ok`'s frame on the placeholder, `ok` built by hand since `jl()` has not
 * run: `qa` is `je.ra + mo.ra`. */
static void test_lj_on_placeholder(void) {
    pico8_ui_t *ok;
    const vec_t *qa;

    entity_registry_init();
    ok = pico8_ui_new();
    pico8_ui_lj(&mem, ok);
    assert(ok->qa != NULL);
    qa = ok->qa;
    printf("[lj qa] (%d,%d)\n", (int)qa->x, (int)qa->y);
    fflush(stdout);
    assert(qa->x == LJ_QA_X && qa->y == LJ_QA_Y);
}

static void test_shutdown_and_init_cycle(void) {
    entity_registry_shutdown();
    assert(main_camera() == NULL);
    entity_registry_init();
    assert_placeholder();
    entity_registry_init();
    entity_registry_init();
    assert_placeholder();
    entity_registry_shutdown();
    entity_registry_shutdown();
    assert(main_camera() == NULL);
}

/* `jl()` puts a `qb` in its place; pico8_entity_init() puts it back. */
static void test_replaced_by_jl(void) {
    reset_world();
    assert_a_qb();
    entity_registry_init();
    assert_placeholder();
    reset_world();
    bootstrap_shutdown();
    assert(main_camera() == NULL);
    reset_world();
    assert_a_qb();
}

static int is_a_struct_of(const void *e, const entity_class_t *cls) {
    return e != NULL &&
           ((const entity_t *)e)->class == cls;
}

/* A struct of its own class, one object for the whole process. A `qb`
 * struct once `jl()` has run. */
static void test_placeholder_is_a_struct(void) {
    const void *placeholder;

    entity_registry_init();
    placeholder = main_camera();
    assert(is_a_struct_of(placeholder, main_camera_placeholder_class()));
    reset_world();
    assert(is_a_struct_of(main_camera(), pico8_qb_struct_class()));
    entity_registry_init();
    assert(main_camera() == placeholder);
    bootstrap_shutdown();
}

/* A shaken camera: `ra` is off `p`, and off its own diagonal. `gd` reads
 * the camera's `ra`, never its `p`, and each axis on its own. */
static void test_gd_under_a_shaken_camera(void) {
    static const vec_t shaken_p = {0, 0};
    static vec_t shaken_ra;
    vec_t in_ra = vec_make(p8_fixed_from_int(200 + 64), p8_fixed_from_int(100 + 64));
    vec_t below_ra = vec_make(p8_fixed_from_int(200 + 64), p8_fixed_from_int(100 + 64 + 77));
    ra_only_t in_e = {{.class = &g_ra_only_class, .pos = &in_e.ra}, NULL};
    ra_only_t below_e = {{.class = &g_ra_only_class, .pos = &below_e.ra}, NULL};
    pico8_qb_t *qb;
    const vec_t *saved_p;
    const vec_t *saved_ra;

    reset_world();
    qb = pico8_mo_qb();
    assert(qb != NULL);
    saved_p = qb->p;
    saved_ra = qb->ra;
    shaken_ra = vec_make(p8_fixed_from_int(200), p8_fixed_from_int(100));
    qb->p = &shaken_p;
    qb->ra = &shaken_ra;
    in_e.ra = &in_ra;
    below_e.ra = &below_ra;
    assert(entity_geom_is_on_screen(&in_e.header) == GD_NEAR);
    assert(entity_geom_is_on_screen(&below_e.header) == GD_FAR);
    qb->p = saved_p;
    qb->ra = saved_ra;
    bootstrap_shutdown();
}

int main(void) {
    p8_mem_init(&mem);
    p8_gfx_init(&gfx);
    assert(p8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(p8_load_map_json(&mem, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&gfx);
    dispatch_context_set_mem(&mem);
    dispatch_context_set_input(&input);

    pico8_cart_load();

    test_placeholder_after_init();
    test_gd_on_placeholder();
    test_lj_on_placeholder();
    test_shutdown_and_init_cycle();
    test_replaced_by_jl();
    test_placeholder_is_a_struct();
    test_gd_under_a_shaken_camera();

    printf("test_pico8_mo: all tests passed\n");
    return 0;
}
