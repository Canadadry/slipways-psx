/* Six sites read the "by" field as a `pico8_list_t*`, so its producers
 * must all make a real list. A `by=o()` in a class spec does not: `ob()`
 * parses it into an ob_table. Nothing visible separates the two while the
 * table is empty, because `count` sits at the same byte offset in both
 * structs -- which is why the contract is asserted here on the REF itself,
 * between two assertions on what a player reads. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

/* The `js` buttons `pz()` just built. */
static const pico8_list_t *ok_le(void) {
    const void *ok = pico8_ok();
    pico8_value_t le;
    assert(ok != NULL);
    assert(pico8_entity_get_field(ok, "le", &le));
    assert(le.kind == PICO8_VALUE_REF && le.as.ref != NULL);
    return (const pico8_list_t *)le.as.ref;
}

static const pico8_lq_t *button_lq(pico8_value_t button_val) {
    const void *button;
    pico8_value_t lq_val;

    assert(button_val.kind == PICO8_VALUE_REF && button_val.as.ref != NULL);
    button = button_val.as.ref;
    assert(pico8_entity_get_field(button, "lq", &lq_val));
    assert(lq_val.kind == PICO8_VALUE_REF && lq_val.as.ref != NULL);
    return (const pico8_lq_t *)lq_val.as.ref;
}

static const char *lq_index_string(const pico8_lq_t *lq, int index) {
    pico8_value_t v;
    if (index < 1 || index > lq->items.count) {
        return NULL;
    }
    v = lq->items.items[index - 1];
    return (v.kind == PICO8_VALUE_STRING) ? v.as.string : NULL;
}

static int lq_index_is_string(const pico8_lq_t *lq, int index, const char *expected) {
    const char *s = lq_index_string(lq, index);
    return s != NULL && strcmp(s, expected) == 0;
}

static void boot(pico8_mem_t *mem, pico8_gfx_t *gfx) {
    pico8_mem_init(mem);
    pico8_gfx_init(gfx);
    assert(pico8_load_gfx_json(mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(mem, "pico8/map.json"));
    pico8_dispatch_set_gfx(gfx);
    pico8_dispatch_set_mem(mem);
    pico8_cart_load();
    pico8__init();
}

/* Read through the field interface, not the class defaults, so the test
 * says nothing about where the value comes from. */
static void *planet_by_ref(const pico8_ga_t *planet) {
    pico8_value_t by;
    assert(pico8_entity_get_field(planet, "by", &by));
    assert(by.kind == PICO8_VALUE_REF);
    return by.as.ref;
}

static void test_ga_export_line_counts_the_routes_in_by(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_ga_t *planet;
    pico8_ob_table_t *kw;
    pico8_vec_t *ra;
    pico8_vec_t *gq;
    pico8_list_t *by;
    pico8_ob_table_t *route_a;
    pico8_ob_table_t *route_b;
    const pico8_list_t *le;
    const pico8_lq_t *exports_lq = NULL;
    int i;

    boot(&mem, &gfx);

    planet = pico8_ga_new(NULL);
    assert(planet != NULL);

    /* "x" is deliberately not one of `jo.cq`'s resources, so `jo:gh` takes
     * the else branch and builds the "exports:" row, not the "makes:" one. */
    kw = pico8_ob_table_new();
    pico8_ob_table_set_name(kw, "mu", pico8_value_fixed(pico8_fixed_from_int(2)));
    pico8_ob_table_set_name(kw, "qe", pico8_value_string("x"));
    planet->jo.kw = kw;

    ra = malloc(sizeof *ra);
    *ra = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    planet->jo.ra = pico8_value_ref(ra);
    gq = malloc(sizeof *gq);
    *gq = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(9));
    planet->jo.gq = pico8_value_ref(gq);

    /* A fresh `ga` exports nothing, against a capacity of `kw.mu`. */
    pico8_jo_gh(&planet->jo);
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        const pico8_lq_t *lq = button_lq(le->items[i]);
        if (lq_index_is_string(lq, 1, "exports:")) {
            exports_lq = lq;
        }
    }
    assert(exports_lq != NULL);
    assert(lq_index_is_string(exports_lq, 2, "0/2"));

    /* Two routes, added the way `jo:ib`'s `add(ib,a)` adds them: straight
     * into whatever `self.by` already holds, here the value the `ga`
     * declaration installed. */
    by = planet_by_ref(planet);
    assert(by != NULL);
    /* The contract all six readers assume, on the value actually handed
     * out. Without the `by` swap, `jo:gh`'s REF check aborts first. */
    assert(pico8_type_tag_of(by) == PICO8_TYPE_TAG_LIST);
    route_a = pico8_ob_table_new();
    route_b = pico8_ob_table_new();
    pico8_list_add(by, pico8_value_ref(route_a));
    pico8_list_add(by, pico8_value_ref(route_b));

    exports_lq = NULL;
    pico8_jo_gh(&planet->jo);
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        const pico8_lq_t *lq = button_lq(le->items[i]);
        if (lq_index_is_string(lq, 1, "exports:")) {
            exports_lq = lq;
        }
    }
    assert(exports_lq != NULL);

    /* Two routes out of a capacity of two -- not "0/2", and not a number
     * assembled out of a neighbouring key. */
    assert(lq_index_is_string(exports_lq, 2, "2/2"));
}

int main(void) {
    test_ga_export_line_counts_the_routes_in_by();

    printf("test_pico8_ga_export_panel_by: all tests passed\n");
    return 0;
}
