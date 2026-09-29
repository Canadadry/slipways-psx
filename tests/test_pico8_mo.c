/* The placeholder `mo` that pico8_entity_init() leaves until `jl()` replaces
 * it with a `qb`, read only through the by-name field API. The frozen
 * values were taken on the code where the placeholder was still a
 * class-less instance and must not be re-taken. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_entity_geom.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_scroll.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define GD_NEAR 1
#define GD_FAR 0
#define LJ_QA_X 0
#define LJ_QA_Y 0
#define QB_RA 192

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static int listed(const pico8_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i].kind == PICO8_VALUE_REF && list->items[i].as.ref == e) {
            return 1;
        }
    }
    return 0;
}

/* Every name a reader of `mo` asks for, and a few it never does, is absent;
 * no method answers and the camera stays put; hv and cv are empty. */
static void assert_placeholder(void) {
    const char *names[] = {"ra", "p", "v", "ma", "pc", "me", "t", "nu", "md", "ph", "kj"};
    const char *buckets[] = {"kj", "ev", "hd", "hk", "hb"};
    pico8_struct_header_t *mo = pico8_mo();
    pico8_value_t v;
    int i;

    assert(mo != NULL);
    for (i = 0; i < 11; i++) {
        assert(!pico8_entity_get_field(mo, names[i], &v));
    }
    pico8_camera(&gfx, 5, 6);
    assert(!pico8_mv(mo, "md", 1, pico8_value_fixed(pico8_fixed_from_int(1)), pico8_value_ref(NULL), &v));
    assert(!pico8_mv(mo, "ph", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &v));
    assert(gfx.camera_x == 5 && gfx.camera_y == 6);
    assert(pico8_hv()->count == 0);
    for (i = 0; i < 5; i++) {
        assert(pico8_cv(buckets[i])->count == 0);
    }
}

static void assert_a_qb(void) {
    pico8_value_t v;
    const pico8_vec_t *ra;

    assert(pico8_entity_get_field(pico8_mo(), "ra", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    ra = v.as.ref;
    assert(ra->x == pico8_fixed_from_int(QB_RA) && ra->y == pico8_fixed_from_int(QB_RA));
}

static void test_placeholder_after_init(void) {
    pico8_entity_init();
    assert_placeholder();
    assert(!listed(pico8_hv(), pico8_mo()));
}

typedef struct {
    pico8_struct_header_t header;
    pico8_value_t ra;
} ra_only_t;

static int ra_only_field(const void *self, const char *name, pico8_value_t *out_value) {
    if (strcmp(name, "ra") != 0) {
        return 0;
    }
    *out_value = ((const ra_only_t *)self)->ra;
    return 1;
}

static const pico8_struct_class_t g_ra_only_class = {{0}, ra_only_field, NULL};

/* `gd` reads `mo.ra` as (0,0): 64 off is inside the 76 margin, 77 is not. */
static void test_gd_on_placeholder(void) {
    pico8_vec_t near_ra = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    pico8_vec_t far_ra = pico8_vec_v(pico8_fixed_from_int(64 + 77), pico8_fixed_from_int(64));
    ra_only_t near_e = {{PICO8_TYPE_TAG_STRUCT, &g_ra_only_class}, {PICO8_VALUE_REF, {0}}};
    ra_only_t far_e = {{PICO8_TYPE_TAG_STRUCT, &g_ra_only_class}, {PICO8_VALUE_REF, {0}}};
    int near_hit;
    int far_hit;

    pico8_entity_init();
    near_e.ra = pico8_value_ref(&near_ra);
    far_e.ra = pico8_value_ref(&far_ra);
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
    pico8_value_t v;
    const pico8_vec_t *qa;

    pico8_entity_init();
    ok = pico8_ui_new();
    pico8_ui_lj(&mem, ok);
    assert(pico8_entity_get_field(ok, "qa", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    qa = v.as.ref;
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

static int is_a_struct_of(const void *e, const pico8_struct_class_t *cls) {
    return e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == cls;
}

/* A struct of its own class, one object for the whole process, that no
 * generic write reaches: one would stop the run. A `qb` struct once `jl()`
 * has run. */
static void test_placeholder_is_a_struct(void) {
    const void *placeholder;
    pid_t child;
    int status;

    pico8_entity_init();
    placeholder = pico8_mo();
    assert(is_a_struct_of(placeholder, pico8_mo_placeholder_struct_class()));
    pico8_jl();
    assert(is_a_struct_of(pico8_mo(), pico8_qb_struct_class()));
    pico8_entity_init();
    assert(pico8_mo() == placeholder);

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        pico8_entity_set_field(pico8_mo(), "ma", pico8_value_fixed(pico8_fixed_from_int(3)));
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
    pico8_bootstrap_shutdown();
}

static void must_abort(void (*fn)(void *), void *e) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        assert(freopen("/dev/null", "w", stderr) != NULL);
        fn(e);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
}

static void get_ra(void *e) {
    pico8_value_t v;
    (void)pico8_entity_get_field(e, "ra", &v);
}

static void set_ra(void *e) {
    pico8_entity_set_field(e, "ra", pico8_value_ref(NULL));
}

static void mv_md(void *e) {
    pico8_value_t v;
    (void)pico8_mv(e, "md", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &v);
}

/* The by-name accessors take entity structs only: a list stops the run
 * instead of being read as an entity. */
static void test_accessors_refuse_a_table(void) {
    static void (*const fns[])(void *) = {get_ra, set_ra, mv_md};
    pico8_list_t list;
    size_t i;

    pico8_list_init(&list);
    for (i = 0; i < sizeof fns / sizeof fns[0]; i++) {
        must_abort(fns[i], &list);
    }
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
    test_accessors_refuse_a_table();

    printf("test_pico8_mo: all tests passed\n");
    return 0;
}
