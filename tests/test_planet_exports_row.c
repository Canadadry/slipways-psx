/* The inspection panel's "exports:" row must reach its layout row with its
 * four items, in order, and not one more: the bug copied a data row's items
 * at the wrong element stride, took its count from the wrong slot and
 * walked past the end of the allocation. The copy into the row's cells, and
 * the layout that gives each cell one placed item, fail the same way if
 * they take one item too many.
 *
 * Built under AddressSanitizer, because the resulting SIGSEGV depends on
 * allocator layout and is not a sound oracle: on the replay that reported
 * it, -O0 exits 139 while -O1 and -O2 exit 0, and ASan reports the
 * overflow in all three.
 *
 * The field assertions below are a second, sanitizer-independent oracle. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/bootstrap.h"
#include "engine/entity_id.h"
#include "game/data_init.h"
#include "engine/entity_class.h"
#include "engine/dispatch_context.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/layout.h"
#include "p8/vram.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "engine/vec.h"

/* The buttons message_box_open_box just built. */
static const entity_id_list_t *main_ui_ephemerals(void) {
    const ui_t *ok = main_ui();
    assert(ok != NULL);
    assert(ok->ephemerals != NULL);
    return ok->ephemerals;
}

/* A button's layout row, after button_construct replaced the raw row keys
 * with a real row. */
static const layout_row_t *button_layout_row(const void *button) {
    const layout_row_t *lq;

    assert(button != NULL);
    assert(((const entity_t *)button)->class == button_class());
    lq = ((const button_t *)button)->layout_row;
    assert(lq != NULL);
    return lq;
}

/* The row's item at 1-based `index`, as a table lookup would find it. */
static int layout_row_item(const layout_row_t *lq, int index, layout_row_cell_t *out) {
    if (index < 1 || index > lq->items_count) {
        return 0;
    }
    *out = lq->items[index - 1];
    return 1;
}

static int layout_row_index_is_string(const layout_row_t *lq, int index, const char *expected) {
    layout_row_cell_t v;
    if (!layout_row_item(lq, index, &v)) {
        return 0;
    }
    return v.kind == LAYOUT_ROW_CELL_STRING && strcmp(v.as.text, expected) == 0;
}

/* What planet_inspection_panel reaches in a game, built without a boot:
 * the resource codes (data_init), the planet, layout row and button
 * classes, and the main user interface message_box_open_box fills. */
static void setup_panel_state(p8_vram_t *mem, p8_graphics_t *gfx) {
    p8_vram_init(mem);
    p8_gfx_init(gfx);
    dispatch_context_set_gfx(gfx);
    dispatch_context_set_mem(mem);

    data_init();
    reset_world(NULL);
    layout_init();
    button_init();
    planet_init();
}

static void test_inspection_panel_exports_row_layout_row_receives_its_four_items(void) {
    p8_vram_t memory;
    p8_graphics_t graphics;
    planet_t *planet;
    static production_t production;
    const data_row_t *production_row;
    vec_t *planet_pos;
    const entity_id_list_t *ephemerals;
    const layout_row_t *exports_layout_row = NULL;
    layout_row_cell_t entry;
    int index;

    setup_panel_state(&memory, &graphics);

    planet = planet_new(NULL);
    assert(planet != NULL);

    /* 99 is deliberately not one of the planet's made resources, so the
     * panel takes the else branch and builds the 4-item "exports:" row
     * rather than the "makes:" one. */
    production.has_resource = 1;
    production.resource = p8_fixed_from_int(99);
    production.amount = p8_fixed_from_int(1);
    planet->production = &production;

    /* The panel reads it, and the archetype's row_step, for the box's row
     * placement. */
    planet_pos = malloc(sizeof *planet_pos);
    *planet_pos = vec_make(p8_fixed_from_int(64), p8_fixed_from_int(64));
    planet->pos = planet_pos;

    /* The call under test: this is where the overflow happened. */
    planet_inspection_panel(planet);

    ephemerals = main_ui_ephemerals();
    /* The icon row, plus the "exports:" row. */
    assert(ephemerals->count >= 2);

    for (index = 0; index < ephemerals->count; index++) {
        const layout_row_t *lq = button_layout_row(entity_id_list_at(ephemerals, index));
        assert(lq != NULL && lq->header.class == layout_row_class());
        if (layout_row_index_is_string(lq, 1, "exports:")) {
            exports_layout_row = lq;
        }
    }
    assert(exports_layout_row != NULL);

    /* The four items the row holds, in order and at the right element
     * stride: "exports:", "<outgoing routes>/<amount>", the production's
     * resource tuple, and the {false, 1} gap the panel appends to every row
     * after the first. */
    assert(layout_row_index_is_string(exports_layout_row, 1, "exports:"));
    assert(layout_row_index_is_string(exports_layout_row, 2, "0/1"));

    assert(layout_row_item(exports_layout_row, 3, &entry));
    assert(entry.kind == LAYOUT_ROW_CELL_ROW && entry.as.row != NULL);
    production_row = entry.as.row;
    assert(production_row->has_resource && production_row->resource == p8_fixed_from_int(99) && production_row->count == 0);

    assert(layout_row_item(exports_layout_row, 4, &entry));
    assert(entry.kind == LAYOUT_ROW_CELL_ROW && entry.as.row != NULL);

    /* No phantom fifth item from an over-long copy loop. */
    assert(!layout_row_item(exports_layout_row, 5, &entry));

    /* layout_row_construct ran over exactly those four: one placed item
     * each. */
    assert(exports_layout_row->placed_items != NULL);
    assert(exports_layout_row->placed_items_count == 4);
}

int main(void) {
    test_inspection_panel_exports_row_layout_row_receives_its_four_items();

    printf("test_planet_exports_row: all tests passed\n");
    return 0;
}
