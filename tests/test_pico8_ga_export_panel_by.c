/* Reverse REF-cast audit (docs/prd/triage/reverse-direction-ref-cast-
 * audit.md, docs/audit-ref-cast-reverse.md): the one site the audit did
 * not find safe by construction, pinned to the value a player actually
 * reads off the screen.
 *
 * WHAT THE AUDIT FOUND. `jo:ib`'s own `local ib=self.by or {}`
 * (pico8/lua.lua:500) is ported as a plain `REF -> pico8_list_t*` cast
 * (src/pico8_planet_trade.c), and so are the five other reads of the
 * "by" field -- `jo:hb`'s and `jo:gh`'s export counters
 * (src/pico8_planet_display.c), `pico8_db`'s (src/pico8_economy_score.c)
 * and `pico8_dh`'s (src/pico8_economy_tiers.c). Five of the six producers that
 * can fill "by" make a real list. The sixth did not: the `ga` class spec
 * string carries `by=o(),` (src/pico8_planet_family.c), and ob()'s own
 * unregistered-identifier fallback parses that into a
 * pico8_ob_table_t -- an ORDERED TABLE, handed to six sites that read it
 * as a list.
 *
 * WHY THIS WAS NOT A VISIBLE BUG, AND WHY THAT IS THE POINT. The two
 * structs place `count` at the same byte offset, and the table ob()
 * builds for `by=o()` is EMPTY, so every one of those six reads got the
 * right answer -- 0 -- out of the wrong type. The whole family this PRD
 * exists for is exactly that: quiet, correct-by-coincidence, and one
 * non-empty table away from printing fragments of keys as numbers. The
 * fix is at the producer, the same swap pico8_ui_init() already makes for
 * `le=o()` and pico8_hy_pq() for `invented=o()`: install a real empty
 * heap pico8_list_t* in the class defaults right after declaring the
 * class, so no consumer has to know.
 *
 * WHAT THIS TEST LOCKS DOWN, and honestly what it cannot. Its SUBJECT is
 * the visible value: it builds a real `ga` planet, runs the real
 * pico8_jo_gh() popup and reads the "exports:" panel line off the button
 * pz() built -- "0/2" for a fresh planet, then "2/2" once two export
 * routes are in its "by" the way `jo:ib`'s own `add(ib,a)` puts them
 * there. Those two strings are what a player sees and what this test is
 * for.
 *
 * Neither of them, however, is what fails against the pre-fix code, and
 * that is worth stating rather than hiding: with an EMPTY ob_table both
 * panel lines already came out right, because `count` sits at the same
 * byte offset in both structs. No observable in the whole program could
 * separate the two representations here -- which is exactly why six
 * sites read a table as a list for an entire phase and nobody noticed.
 * What is red before the fix is the PRECONDITION in the middle of the
 * test: that the REF the port hands out for "by" really is the
 * representation all six of its readers assume. That is a contract
 * assertion on the producer, not an assertion that some guard ran; the
 * behaviour assertions around it are what the contract is FOR, and they
 * are what will catch the day `by` starts holding something with entries
 * in it.
 *
 * Built plain, not under a sanitizer, on purpose: this direction
 * UNDER-reads (16-byte elements out of a 32-byte-element array) and can
 * never leave the allocation, so there is no memory error for ASan to
 * find and an ASan target here would assert nothing the plain one does
 * not. That is the whole difference from tests/test_pico8_planet_gh_row_
 * lq.c, whose forward-direction bug overflows and therefore needs one.
 *
 * Full engine closure in the Makefile, like every other test that reaches
 * real game state: pico8_cart_load() pulls in essentially the whole
 * port. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_class.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
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

/* The `js` buttons pz() just built, read back the ordinary way (same
 * accessor tests/test_pico8_planet_gh_row_lq.c uses). */
static const pico8_list_t *ok_le(void) {
    pico8_instance_t *ok = pico8_ok();
    pico8_value_t le;
    assert(ok != NULL);
    assert(pico8_instance_get_field(ok, "le", &le));
    assert(le.kind == PICO8_VALUE_REF && le.as.ref != NULL);
    return (const pico8_list_t *)le.as.ref;
}

static const pico8_instance_t *button_lq(pico8_value_t button_val) {
    const pico8_instance_t *button;
    pico8_value_t lq_val;

    assert(button_val.kind == PICO8_VALUE_REF && button_val.as.ref != NULL);
    button = (const pico8_instance_t *)button_val.as.ref;
    assert(pico8_instance_get_field(button, "lq", &lq_val));
    assert(lq_val.kind == PICO8_VALUE_REF && lq_val.as.ref != NULL);
    return (const pico8_instance_t *)lq_val.as.ref;
}

static const char *lq_index_string(const pico8_instance_t *lq, int index) {
    pico8_value_t v;
    if (!pico8_ob_table_get_index(lq->fields, index, &v)) {
        return NULL;
    }
    return (v.kind == PICO8_VALUE_STRING) ? v.as.string : NULL;
}

static int lq_index_is_string(const pico8_instance_t *lq, int index, const char *expected) {
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

/* `self.by` as the rest of the port sees it: whatever REF the field
 * holds. Read through the public field interface, not through the class
 * defaults, so the test says nothing about WHERE the value comes from. */
static void *planet_by_ref(const pico8_instance_t *planet) {
    pico8_value_t by;
    assert(pico8_instance_get_field(planet, "by", &by));
    assert(by.kind == PICO8_VALUE_REF);
    return by.as.ref;
}

static void test_ga_export_line_counts_the_routes_in_by(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_instance_t *planet;
    pico8_ob_table_t *kw;
    pico8_vec_t *ra;
    pico8_vec_t *gq;
    pico8_list_t *by;
    pico8_ob_table_t *route_a;
    pico8_ob_table_t *route_b;
    const pico8_list_t *le;
    const pico8_instance_t *exports_lq = NULL;
    int i;

    boot(&mem, &gfx);

    planet = pico8_ga_new(NULL);
    assert(planet != NULL);

    /* kw={mu=2,qe="x"}: "x" is deliberately not one of jo.cq's own
     * resources, so jo:gh takes the `else` branch and builds the
     * "exports: <#by>/<mu>" row rather than the "makes:" one
     * (src/pico8_planet_display.c, pico8/lua.lua:442-445). */
    kw = pico8_ob_table_new();
    pico8_ob_table_set_name(kw, "mu", pico8_value_fixed(pico8_fixed_from_int(2)));
    pico8_ob_table_set_name(kw, "qe", pico8_value_string("x"));
    pico8_ob_table_set_name(planet->fields, "kw", pico8_value_ref(kw));

    ra = malloc(sizeof *ra);
    *ra = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    pico8_ob_table_set_name(planet->fields, "ra", pico8_value_ref(ra));
    gq = malloc(sizeof *gq);
    *gq = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(9));
    pico8_ob_table_set_name(planet->fields, "gq", pico8_value_ref(gq));

    /* A freshly built `ga` exports nothing yet, and the panel says so. */
    pico8_jo_gh(planet);
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        const pico8_instance_t *lq = button_lq(le->items[i]);
        if (lq_index_is_string(lq, 1, "exports:")) {
            exports_lq = lq;
        }
    }
    assert(exports_lq != NULL);
    assert(lq_index_is_string(exports_lq, 2, "0/2"));

    /* Now two export routes, added exactly the way jo:ib's own
     * `add(ib,a)` adds them (src/pico8_planet_trade.c) -- straight into
     * whatever `self.by` already holds, which is the class-level value
     * the `ga` declaration installed. That is the read this audit is
     * about: the field's producer and its six consumers have to agree on
     * one representation, and the panel line below is where a
     * disagreement would surface. */
    by = planet_by_ref(planet);
    assert(by != NULL);
    /* The contract those six readers all assume, asserted once, on the
     * value the port actually hands out -- see this file's top comment.
     * Red before pico8_planet_family_init() swapped the `by=o()` default
     * for a real list; the panel assertions on either side of it were
     * green either way. */
    assert(pico8_type_tag_of(by) == PICO8_TYPE_TAG_LIST);
    route_a = pico8_ob_table_new();
    route_b = pico8_ob_table_new();
    pico8_list_add(by, pico8_value_ref(route_a));
    pico8_list_add(by, pico8_value_ref(route_b));

    exports_lq = NULL;
    pico8_jo_gh(planet);
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        const pico8_instance_t *lq = button_lq(le->items[i]);
        if (lq_index_is_string(lq, 1, "exports:")) {
            exports_lq = lq;
        }
    }
    assert(exports_lq != NULL);

    /* The visible result: two routes out of a capacity of two. Not "0/2"
     * (the count read past a representation the consumer misunderstood),
     * not an absurd number assembled from a neighbouring key. */
    assert(lq_index_is_string(exports_lq, 2, "2/2"));
}

int main(void) {
    test_ga_export_line_counts_the_routes_in_by();

    printf("test_pico8_ga_export_panel_by: all tests passed\n");
    return 0;
}
