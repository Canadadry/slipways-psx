/* The placeholder `je` that pico8_ui_init() leaves until `jl()` replaces it
 * with an `nf`, read only through the dual-form field reader. The frozen
 * values were taken on the code where the placeholder was still a
 * class-less instance and must not be re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_bootstrap.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_class.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mouse_reader.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define LJ_QA_X 0
#define LJ_QA_Y 0

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

/* `cv` may be read only once pico8_entity_init() has run. */
static int registered(const void *e, int entity_live) {
    const char *buckets[] = {"kj", "ev", "hd", "hk", "hb"};
    const char *tags[] = {"je", "nf", "ui", "qb", "ke"};
    int i;

    if (listed(pico8_hv(), e)) {
        return 1;
    }
    for (i = 0; i < 5; i++) {
        if ((entity_live && listed(pico8_cv(buckets[i]), e)) || listed(pico8_cg(tags[i]), e)) {
            return 1;
        }
    }
    return 0;
}

/* Every name a reader of `je` asks for, and a few it never does, is absent;
 * no method answers; nothing registers it. */
static void assert_placeholder(int entity_live) {
    const char *names[] = {"ra", "l", "r", "me", "t", "ph"};
    pico8_struct_header_t *je = pico8_je();
    pico8_value_t v;
    int i;

    assert(je != NULL);
    for (i = 0; i < 6; i++) {
        assert(!pico8_entity_get_field(je, names[i], &v));
    }
    assert(!pico8_mv(je, "ph", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &v));
    assert(!pico8_mv(je, "kj", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &v));
    assert(!registered(je, entity_live));
}

static void assert_an_nf(void) {
    pico8_value_t v;

    assert(pico8_entity_get_field(pico8_je(), "ra", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    assert(((const pico8_vec_t *)v.as.ref)->x == 0 && ((const pico8_vec_t *)v.as.ref)->y == 0);
    assert(pico8_entity_get_field(pico8_je(), "l", &v) && pico8_entity_get_field(pico8_je(), "r", &v));
}

/* Between pico8_cart_load() and pico8__init(). */
static void test_placeholder_after_cart_load(void) {
    assert_placeholder(0);
}

/* `ok`'s frame on the placeholder, `ok` built by hand since `jl()` has not
 * run: no press, `qa` is `mo.ra` alone. */
static void test_lj_on_placeholder(void) {
    pico8_ui_t *ok;
    pico8_value_t v;
    const pico8_vec_t *qa;

    pico8_entity_init();
    assert_placeholder(1);
    ok = pico8_ui_new(NULL);
    pico8_ui_lj(&mem, ok);
    assert(!pico8_entity_get_field(ok, "lheld", &v) && !pico8_entity_get_field(ok, "rheld", &v));
    assert(pico8_entity_get_field(ok, "qa", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL);
    qa = v.as.ref;
    printf("[lj qa] (%d,%d)\n", (int)qa->x, (int)qa->y);
    fflush(stdout);
    assert(qa->x == LJ_QA_X && qa->y == LJ_QA_Y);
}

/* Shutdown clears it; every init brings it back; a whole cart reload too. */
static void test_shutdown_and_init_cycle(void) {
    pico8_ui_shutdown();
    assert(pico8_je() == NULL);
    pico8_ui_init();
    assert_placeholder(0);
    pico8_ui_init();
    pico8_ui_init();
    assert_placeholder(0);
    pico8_cart_shutdown();
    assert(pico8_je() == NULL);
    pico8_cart_load();
    assert_placeholder(0);
}

/* `jl()` puts an `nf` in its place; pico8_ui_init() puts it back. */
static void test_replaced_by_jl(void) {
    pico8__init();
    assert_an_nf();
    pico8_ui_init();
    assert_placeholder(1);
}

/* The path of a caller that never loads the cart. */
static void test_jl_without_cart_load(void) {
    pico8_cart_shutdown();
    pico8_bootstrap_shutdown();
    pico8_jl();
    assert_an_nf();
    pico8_bootstrap_shutdown();
    assert(pico8_je() == NULL);
    pico8_jl();
    assert_an_nf();
    pico8_bootstrap_shutdown();
}

static int is_a_struct_of(const void *e, const pico8_struct_class_t *cls) {
    return e != NULL && pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)e)->cls == cls;
}

/* A struct of its own class, one object for the whole process; an `nf`
 * struct once `jl()` has run. */
static void test_placeholder_is_a_struct(void) {
    const void *placeholder;

    pico8_cart_load();
    placeholder = pico8_je();
    assert(is_a_struct_of(placeholder, pico8_je_placeholder_struct_class()));
    pico8_jl();
    assert(is_a_struct_of(pico8_je(), pico8_nf_struct_class()));
    pico8_ui_init();
    assert(pico8_je() == placeholder);
    pico8_cart_shutdown();
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

    test_placeholder_after_cart_load();
    test_lj_on_placeholder();
    test_shutdown_and_init_cycle();
    test_replaced_by_jl();
    test_jl_without_cart_load();
    test_placeholder_is_a_struct();

    printf("test_pico8_je: all tests passed\n");
    return 0;
}
