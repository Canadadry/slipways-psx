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

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static int ents_listed(const pico8_ents_t *list, const void *e) {
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
    entity_t *mo = pico8_mo();
    int i;

    assert(mo != NULL && pico8_mo_qb() == NULL);
    pico8_camera(&gfx, 5, 6);
    assert(mo->class != pico8_qb_struct_class());
    assert(mo->class->states[ENTITY_STATE_NORMAL] == NULL && mo->class->draw == NULL);
    assert(gfx.camera_x == 5 && gfx.camera_y == 6);
    assert(pico8_hv()->count == 0);
    for (i = 0; i < 5; i++) {
        assert(pico8_cv(buckets[i])->count == 0);
    }
}

static void assert_a_qb(void) {
    const pico8_vec_t *ra;

    assert(pico8_mo_qb() != NULL && pico8_mo_qb()->ra != NULL);
    ra = pico8_mo_qb()->ra;
    assert(ra->x == pico8_fixed_from_int(QB_RA) && ra->y == pico8_fixed_from_int(QB_RA));
}

static void test_placeholder_after_init(void) {
    pico8_entity_init();
    assert_placeholder();
    assert(!ents_listed(pico8_hv(), pico8_mo()));
}

typedef struct {
    entity_t header;
    const pico8_vec_t *ra;
} ra_only_t;

static const entity_class_t g_ra_only_class = {0};

/* `gd` reads `mo.ra` as (0,0): 64 off is inside the 76 margin, 77 is not. */
static void test_gd_on_placeholder(void) {
    pico8_vec_t near_ra = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    pico8_vec_t far_ra = pico8_vec_v(pico8_fixed_from_int(64 + 77), pico8_fixed_from_int(64));
    ra_only_t near_e = {{.class = &g_ra_only_class, .pos = &near_e.ra}, NULL};
    ra_only_t far_e = {{.class = &g_ra_only_class, .pos = &far_e.ra}, NULL};
    int near_hit;
    int far_hit;

    pico8_entity_init();
    near_e.ra = &near_ra;
    far_e.ra = &far_ra;
    near_hit = pico8_gd(&near_e.header);
    far_hit = pico8_gd(&far_e.header);
    printf("[gd] near %d far %d\n", near_hit, far_hit);
    fflush(stdout);
    assert(near_hit == GD_NEAR && far_hit == GD_FAR);
}

/* `ok`'s frame on the placeholder, `ok` built by hand since `jl()` has not
 * run: `qa` is `je.ra + mo.ra`. */
static void test_lj_on_placeholder(void) {
    pico8_ui_t *ok;
    const pico8_vec_t *qa;

    pico8_entity_init();
    ok = pico8_ui_new();
    pico8_ui_lj(&mem, ok);
    assert(ok->qa != NULL);
    qa = ok->qa;
    printf("[lj qa] (%d,%d)\n", (int)qa->x, (int)qa->y);
    fflush(stdout);
    assert(qa->x == LJ_QA_X && qa->y == LJ_QA_Y);
}

static void test_shutdown_and_init_cycle(void) {
    pico8_entity_shutdown();
    assert(pico8_mo() == NULL);
    pico8_entity_init();
    assert_placeholder();
    pico8_entity_init();
    pico8_entity_init();
    assert_placeholder();
    pico8_entity_shutdown();
    pico8_entity_shutdown();
    assert(pico8_mo() == NULL);
}

/* `jl()` puts a `qb` in its place; pico8_entity_init() puts it back. */
static void test_replaced_by_jl(void) {
    pico8_jl();
    assert_a_qb();
    pico8_entity_init();
    assert_placeholder();
    pico8_jl();
    pico8_bootstrap_shutdown();
    assert(pico8_mo() == NULL);
    pico8_jl();
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

    pico8_entity_init();
    placeholder = pico8_mo();
    assert(is_a_struct_of(placeholder, pico8_mo_placeholder_struct_class()));
    pico8_jl();
    assert(is_a_struct_of(pico8_mo(), pico8_qb_struct_class()));
    pico8_entity_init();
    assert(pico8_mo() == placeholder);
    pico8_bootstrap_shutdown();
}

/* A shaken camera: `ra` is off `p`, and off its own diagonal. `gd` reads
 * the camera's `ra`, never its `p`, and each axis on its own. */
static void test_gd_under_a_shaken_camera(void) {
    static const pico8_vec_t shaken_p = {0, 0};
    static pico8_vec_t shaken_ra;
    pico8_vec_t in_ra = pico8_vec_v(pico8_fixed_from_int(200 + 64), pico8_fixed_from_int(100 + 64));
    pico8_vec_t below_ra = pico8_vec_v(pico8_fixed_from_int(200 + 64), pico8_fixed_from_int(100 + 64 + 77));
    ra_only_t in_e = {{.class = &g_ra_only_class, .pos = &in_e.ra}, NULL};
    ra_only_t below_e = {{.class = &g_ra_only_class, .pos = &below_e.ra}, NULL};
    pico8_qb_t *qb;
    const pico8_vec_t *saved_p;
    const pico8_vec_t *saved_ra;

    pico8_jl();
    qb = pico8_mo_qb();
    assert(qb != NULL);
    saved_p = qb->p;
    saved_ra = qb->ra;
    shaken_ra = pico8_vec_v(pico8_fixed_from_int(200), pico8_fixed_from_int(100));
    qb->p = &shaken_p;
    qb->ra = &shaken_ra;
    in_e.ra = &in_ra;
    below_e.ra = &below_ra;
    assert(pico8_gd(&in_e.header) == GD_NEAR);
    assert(pico8_gd(&below_e.header) == GD_FAR);
    qb->p = saved_p;
    qb->ra = saved_ra;
    pico8_bootstrap_shutdown();
}

int main(void) {
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_input_init(&input);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

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
