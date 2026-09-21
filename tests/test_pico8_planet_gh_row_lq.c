/* Permanent regression test for the `jo:gh()` -> `pz()` -> `js(b)` ->
 * `js:pq()` -> `lq(mv(self,"lq"))` type-confusion crash (docs/plan.md's
 * own "Notes de portage" journal; reported from a real raylib playtest as
 * a hard SIGSEGV on mouse RELEASE over a developed planet).
 *
 * WHAT WENT WRONG (the defect this test locks down): `pico8_jo_gh_row()`
 * (src/pico8_planet_display.c) stores a raw `pico8_list_t*` in a display
 * row's "lq" field -- real Lua's `{lq={"exports:","0/1",kw}}` is just an
 * array-shaped table, and Lua has only ONE table type. This port does
 * not: `pico8_list_t` ({items;count;capacity} of 16-byte
 * `pico8_value_t`) and `pico8_ob_table_t` ({entries;count;capacity} of
 * 32-byte `pico8_ob_entry_t`) were, before the type tag, byte-for-byte
 * ABI-compatible but element-INCOMPATIBLE. `js:pq()` hands that REF
 * straight to `pico8_lq_new()` -> `pico8_class_instantiate()`, whose copy
 * loop walks `overrides->entries[i]` at 32-byte stride over a 16-byte-
 * element array: a heap-buffer-overflow for every row with >= 3 items
 * (capacity starts at 4 and doubles, so `2*count > capacity` always), and
 * the garbage key/value pairs it copies in blow up later in `lq:mn()`.
 *
 * WHAT THIS TEST DOES: builds the minimal real game state that reaches
 * that row -- one real `jo` planet with `kw={mu=1,qe="x"}` (jo:gh's own
 * `if self.kw.mu>0` "exports:" row, src/pico8_planet_display.c:310-347,
 * pico8/lua.lua:442-445) -- calls the real `pico8_jo_gh()`, then reads
 * the resulting `ok.le` buttons back through the ordinary public entity
 * interface and asserts the "exports:" button's own `lq` instance really
 * received the FOUR items the row actually holds, in order, at the right
 * element stride.
 *
 * WHY IT IS BUILT UNDER AddressSanitizer (Makefile target, -fsanitize=
 * address, the same "a sanitizer report is a hard failure" precedent
 * `test_pico8_fixed_ubsan` already establishes): the SIGSEGV this bug
 * produced is ALLOCATOR-LAYOUT DEPENDENT and therefore NOT a sound
 * oracle. Measured on the original 9758-frame replay of the reported
 * crash: plain -O0 exits 139, but plain -O1 and -O2 exit 0 -- while ASan
 * reports the heap-buffer-overflow in all three. A test asserting on a
 * crash (or on exit code 139) would silently pass forever. ASan turns the
 * overflow itself -- not its downstream symptom -- into a hard,
 * layout-independent failure. This port leaks by design (no GC, see
 * pico8_value.h's own ownership note), so the target runs with
 * ASAN_OPTIONS=detect_leaks=0.
 *
 * The field assertions below are a second, sanitizer-independent oracle:
 * under the old blind cast the copied "entry count" was read out of the
 * list's own `capacity` slot and the keys/values came from past the end
 * of the allocation, so neither the entry count nor the item values could
 * come out right by construction.
 *
 * Full engine closure in the Makefile (like tests/test_pico8_planet_kj_
 * dispatch.c and tests/test_pico8_empty_planet_cleared.c before it):
 * `pico8_cart_load()` pulls in essentially the whole port. */

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
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_planet_display.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

/* `ok.le` -- the list of `js` button instances pz() just built (see
 * pico8_msgbox.c's own pico8_pz). */
static const pico8_list_t *ok_le(void) {
    pico8_instance_t *ok = pico8_ok();
    pico8_value_t le;
    assert(ok != NULL);
    assert(pico8_instance_get_field(ok, "le", &le));
    assert(le.kind == PICO8_VALUE_REF && le.as.ref != NULL);
    return (const pico8_list_t *)le.as.ref;
}

/* A button's own `lq` field, after js:pq() replaced the raw row override
 * with a real `lq` instance (`self.lq=lq(mv(self,"lq"))`). */
static const pico8_instance_t *button_lq(pico8_value_t button_val) {
    const pico8_instance_t *button;
    pico8_value_t lq_val;

    assert(button_val.kind == PICO8_VALUE_REF && button_val.as.ref != NULL);
    button = (const pico8_instance_t *)button_val.as.ref;
    assert(pico8_instance_get_field(button, "lq", &lq_val));
    assert(lq_val.kind == PICO8_VALUE_REF && lq_val.as.ref != NULL);
    return (const pico8_instance_t *)lq_val.as.ref;
}

static int lq_index_is_string(const pico8_instance_t *lq, int index, const char *expected) {
    pico8_value_t v;
    if (!pico8_ob_table_get_index(lq->fields, index, &v)) {
        return 0;
    }
    return v.kind == PICO8_VALUE_STRING && v.as.string != NULL && strcmp(v.as.string, expected) == 0;
}

static void test_gh_exports_row_lq_receives_its_four_items(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_instance_t *planet;
    pico8_ob_table_t *kw;
    pico8_vec_t *ra;
    pico8_vec_t *gq;
    const pico8_list_t *le;
    const pico8_instance_t *exports_lq = NULL;
    pico8_value_t entry;
    int i;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);

    pico8_cart_load();
    pico8__init();

    planet = pico8_jo_new(NULL);
    assert(planet != NULL);

    /* self.kw={mu=1,qe="x"} -- "x" is deliberately NOT one of jo.cq's own
     * resources, so jo:gh takes the `else` branch and builds the
     * "exports:" row (4 items once the `for i=2,#bs do add(bs[i].lq,
     * {false,1}) end` tuple is appended), not the 3-item "makes:" one. */
    kw = pico8_ob_table_new();
    pico8_ob_table_set_name(kw, "mu", pico8_value_fixed(pico8_fixed_from_int(1)));
    pico8_ob_table_set_name(kw, "qe", pico8_value_string("x"));
    pico8_ob_table_set_name(planet->fields, "kw", pico8_value_ref(kw));

    /* jo:gh reads self.ra/self.gq for pz()'s own row placement. */
    ra = malloc(sizeof *ra);
    *ra = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    pico8_ob_table_set_name(planet->fields, "ra", pico8_value_ref(ra));
    gq = malloc(sizeof *gq);
    *gq = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(9));
    pico8_ob_table_set_name(planet->fields, "gq", pico8_value_ref(gq));

    /* THE call under test. Before the type tag this overflowed the row's
     * own 4-element pico8_list_t allocation inside
     * pico8_class_instantiate() (ASan: heap-buffer-overflow, READ of size
     * 16, 0 bytes after a 64-byte region). */
    pico8_jo_gh(planet);

    le = ok_le();
    /* bs[0] (the icon row, whose "lq" really IS a pico8_ob_table_t) plus
     * the "exports:" row. */
    assert(le->count >= 2);

    for (i = 0; i < le->count; i++) {
        const pico8_instance_t *lq = button_lq(le->items[i]);
        assert(lq->cls == pico8_lq_class());
        if (lq_index_is_string(lq, 1, "exports:")) {
            exports_lq = lq;
        }
    }
    assert(exports_lq != NULL);

    /* The four items the row genuinely holds, in order, each copied in at
     * the RIGHT element stride:
     *   1 "exports:"  2 "<#by>/<mu>"  3 the kw table  4 {false,1}
     * (src/pico8_planet_display.c's own `lb` list plus jo:gh's own
     * `add(bs[i].lq,{false,1})` tuple; pico8/lua.lua:442-448). */
    assert(lq_index_is_string(exports_lq, 1, "exports:"));
    assert(lq_index_is_string(exports_lq, 2, "0/1"));

    assert(pico8_ob_table_get_index(exports_lq->fields, 3, &entry));
    assert(entry.kind == PICO8_VALUE_REF && entry.as.ref == (void *)kw);

    assert(pico8_ob_table_get_index(exports_lq->fields, 4, &entry));
    assert(entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL);

    /* ...and no phantom fifth item invented by an over-long copy loop. */
    assert(!pico8_ob_table_get_index(exports_lq->fields, 5, &entry));

    /* lq:pq() really ran over exactly those four items: one `fs` entry per
     * item (src/pico8_layout.c's own pico8_lq_pq). */
    assert(pico8_instance_get_field(exports_lq, "fs", &entry));
    assert(entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL);
    assert(((const pico8_list_t *)entry.as.ref)->count == 4);
}

int main(void) {
    test_gh_exports_row_lq_receives_its_four_items();

    printf("test_pico8_planet_gh_row_lq: all tests passed\n");
    return 0;
}
