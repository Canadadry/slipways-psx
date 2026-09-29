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

#include "../src/pico8_bootstrap.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_planet_family.h"
#include "../src/pico8_trade.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

/* The `js` buttons `pz()` just built. */
static const pico8_ptrs_t *ok_le(void) {
    const pico8_ui_t *ok = pico8_ok();
    assert(ok != NULL);
    assert(ok->le != NULL);
    return ok->le;
}

static const pico8_lq_t *button_lq(const void *button) {
    const pico8_lq_t *lq;

    assert(button != NULL);
    assert(((const pico8_struct_header_t *)button)->cls == pico8_js_struct_class());
    lq = ((const pico8_js_t *)button)->lq;
    assert(lq != NULL);
    return lq;
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

/* What `jo:gh` reaches when the cart calls it on a `ga`, built without a
 * boot: `r` (from pico8_data_init), the jo/ga/lq/js classes, and the `ok`
 * singleton `pz()` fills. */
static void setup_panel_state(pico8_mem_t *mem, pico8_gfx_t *gfx) {
    pico8_mem_init(mem);
    pico8_gfx_init(gfx);
    pico8_dispatch_set_gfx(gfx);
    pico8_dispatch_set_mem(mem);

    pico8_data_init();
    pico8_jl();
    pico8_layout_init();
    pico8_js_init();
    pico8_jo_init();
    pico8_planet_family_init();
}

/* Read on the instance's member, not the class defaults, so the test
 * says nothing about where the value comes from. */
static void *planet_by_ref(const pico8_ga_t *planet) {
    return planet->jo.by;
}

static void test_ga_export_line_counts_the_routes_in_by(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_ga_t *planet;
    static pico8_kw_t kw;
    pico8_vec_t *ra;
    pico8_vec_t *gq;
    pico8_list_t *by;
    pico8_trade_t *route_a;
    pico8_trade_t *route_b;
    const pico8_ptrs_t *le;
    const pico8_lq_t *exports_lq = NULL;
    int i;

    setup_panel_state(&mem, &gfx);

    planet = pico8_ga_new(NULL);
    assert(planet != NULL);

    /* 99 is deliberately not one of `jo.cq`'s resources, so `jo:gh` takes
     * the else branch and builds the "exports:" row, not the "makes:" one. */
    kw.has_qe = 1;
    kw.qe = pico8_fixed_from_int(99);
    kw.mu = pico8_fixed_from_int(2);
    planet->jo.kw = &kw;

    ra = malloc(sizeof *ra);
    *ra = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    planet->jo.ra = ra;
    gq = malloc(sizeof *gq);
    *gq = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(9));
    planet->jo.gq = gq;

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
     * out. Without the `by` swap, ga's init aborts first. */
    assert(pico8_type_tag_of(by) == PICO8_TYPE_TAG_LIST);
    route_a = pico8_trade_new(NULL);
    route_b = pico8_trade_new(NULL);
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
