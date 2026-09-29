/* A display row's "lq" holds a `pico8_list_t*`, whose elements `js:pq()`
 * takes as the row's positionals; the bug read it as a `pico8_ob_table_t*`.
 * Lua has one table type and this port has two: before the type tag the
 * structs were ABI-compatible but element-incompatible, so the copy loop
 * walked 32-byte entries over a 16-byte-element array and overflowed the
 * allocation.
 *
 * Built under AddressSanitizer, because the resulting SIGSEGV depends on
 * allocator layout and is not a sound oracle: on the replay that reported
 * it, -O0 exits 139 while -O1 and -O2 exit 0, and ASan reports the
 * overflow in all three. The target runs with detect_leaks=0.
 *
 * The field assertions below are a second, sanitizer-independent oracle:
 * under the blind cast the entry count came out of the list's `capacity`
 * slot and the values from past the end of the allocation, so neither
 * could be right by construction. */
/* TROU-7 */

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
#include "../src/pico8_type_tag.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

/* The `js` button instances `pz()` just built. */
static const pico8_list_t *ok_le(void) {
    const void *ok = pico8_ok();
    pico8_value_t le;
    assert(ok != NULL);
    assert(pico8_entity_get_field(ok, "le", &le));
    assert(le.kind == PICO8_VALUE_REF && le.as.ref != NULL);
    return (const pico8_list_t *)le.as.ref;
}

/* A button's `lq` field, after `js:pq()` replaced the raw row override
 * with a real row. */
static const pico8_lq_t *button_lq(pico8_value_t button_val) {
    const void *button;
    pico8_value_t lq_val;

    assert(button_val.kind == PICO8_VALUE_REF && button_val.as.ref != NULL);
    button = button_val.as.ref;
    assert(pico8_entity_get_field(button, "lq", &lq_val));
    assert(lq_val.kind == PICO8_VALUE_REF && lq_val.as.ref != NULL);
    return (const pico8_lq_t *)lq_val.as.ref;
}

/* The row's item at 1-based `index`, as a table lookup would find it. */
static int lq_item(const pico8_lq_t *lq, int index, pico8_value_t *out) {
    if (index < 1 || index > lq->items.count) {
        return 0;
    }
    *out = lq->items.items[index - 1];
    return 1;
}

static int lq_index_is_string(const pico8_lq_t *lq, int index, const char *expected) {
    pico8_value_t v;
    if (!lq_item(lq, index, &v)) {
        return 0;
    }
    return v.kind == PICO8_VALUE_STRING && v.as.string != NULL && strcmp(v.as.string, expected) == 0;
}

/* What `jo:gh` reaches when the cart calls it, built without a boot: `r`
 * (from pico8_data_init), the jo/lq/js classes, and the `ok` singleton `pz()`
 * fills. */
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
}

static void test_gh_exports_row_lq_receives_its_four_items(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_jo_t *planet;
    static pico8_kw_t kw;
    const pico8_jb_row_t *kw_row;
    pico8_vec_t *ra;
    pico8_vec_t *gq;
    const pico8_list_t *le;
    const pico8_lq_t *exports_lq = NULL;
    pico8_value_t entry;
    int i;

    setup_panel_state(&mem, &gfx);

    planet = pico8_jo_new(NULL);
    assert(planet != NULL);

    /* 99 is deliberately not one of `jo.cq`'s resources, so `jo:gh` takes
     * the else branch and builds the 4-item "exports:" row rather than the
     * 3-item "makes:" one. */
    kw.has_qe = 1;
    kw.qe = pico8_fixed_from_int(99);
    kw.mu = pico8_fixed_from_int(1);
    planet->kw = &kw;

    /* `jo:gh` reads these for `pz()`'s row placement. */
    ra = malloc(sizeof *ra);
    *ra = pico8_vec_v(pico8_fixed_from_int(64), pico8_fixed_from_int(64));
    planet->ra = ra;
    gq = malloc(sizeof *gq);
    *gq = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(9));
    planet->gq = gq;

    /* The call under test: this is where the overflow happened. */
    pico8_jo_gh(planet);

    le = ok_le();
    /* The icon row, whose "lq" really is an ob_table, plus the "exports:"
     * row. */
    assert(le->count >= 2);

    for (i = 0; i < le->count; i++) {
        const pico8_lq_t *lq = button_lq(le->items[i]);
        assert(pico8_type_tag_of(lq) == PICO8_TYPE_TAG_STRUCT && lq->header.cls == pico8_lq_struct_class());
        if (lq_index_is_string(lq, 1, "exports:")) {
            exports_lq = lq;
        }
    }
    assert(exports_lq != NULL);

    /* The four items the row holds, in order and at the right element
     * stride: "exports:", "<#by>/<mu>", the `{qe=kw.qe}` of `kw`, and the
     * `{false,1}` tuple `jo:gh` appends to every row after the first. */
    assert(lq_index_is_string(exports_lq, 1, "exports:"));
    assert(lq_index_is_string(exports_lq, 2, "0/1"));

    assert(lq_item(exports_lq, 3, &entry));
    assert(entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL);
    kw_row = entry.as.ref;
    assert(kw_row->has_qe && kw_row->qe == pico8_fixed_from_int(99) && kw_row->count == 0);

    assert(lq_item(exports_lq, 4, &entry));
    assert(entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL);

    /* No phantom fifth item from an over-long copy loop. */
    assert(!lq_item(exports_lq, 5, &entry));

    /* `lq:pq()` ran over exactly those four: one `fs` entry per item. */
    assert(pico8_entity_get_field(exports_lq, "fs", &entry));
    assert(entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL);
    assert(((const pico8_list_t *)entry.as.ref)->count == 4);
}

int main(void) {
    test_gh_exports_row_lq_receives_its_four_items();

    printf("test_pico8_planet_gh_row_lq: all tests passed\n");
    return 0;
}
