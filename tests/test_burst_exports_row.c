/* The "exports:" row of `jo:gh` counts the routes held in the `by` a `ga`
 * receives at construction, read on the member itself. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/bootstrap.h"
#include "game/data_init.h"
#include "engine/entity_class.h"
#include "engine/dispatch_context.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/planet_family.h"
#include "game/trade.h"
#include "engine/game_zone.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "engine/vec.h"

/* The `js` buttons `pz()` just built. */
static const entity_list_t *main_ui_ephemerals(void) {
    const ui_t *ok = main_ui();
    assert(ok != NULL);
    assert(ok->ephemerals != NULL);
    return ok->ephemerals;
}

static const layout_row_t *button_layout_row(const void *button) {
    const layout_row_t *lq;

    assert(button != NULL);
    assert(((const entity_t *)button)->class == button_class());
    lq = ((const button_t *)button)->layout_row;
    assert(lq != NULL);
    return lq;
}

static const char *layout_row_index_string(const layout_row_t *lq, int index) {
    layout_row_cell_t v;
    if (index < 1 || index > lq->items_count) {
        return NULL;
    }
    v = lq->items[index - 1];
    return (v.kind == LAYOUT_ROW_CELL_STRING) ? v.as.text : NULL;
}

static int layout_row_index_is_string(const layout_row_t *lq, int index, const char *expected) {
    const char *s = layout_row_index_string(lq, index);
    return s != NULL && strcmp(s, expected) == 0;
}

/* What `jo:gh` reaches when the cart calls it on a `ga`, built without a
 * boot: `r` (from pico8_data_init), the jo/ga/lq/js classes, and the `ok`
 * singleton `pz()` fills. */
static void setup_panel_state(p8_memory_t *mem, p8_graphics_t *gfx) {
    p8_mem_init(mem);
    p8_gfx_init(gfx);
    dispatch_context_set_gfx(gfx);
    dispatch_context_set_mem(mem);

    data_init();
    reset_world();
    layout_init();
    button_init();
    planet_init();
    planet_family_init();
}

/* Read on the instance's member, not the class defaults, so the test
 * says nothing about where the value comes from. */
static entity_list_t *planet_by_ref(const planet_t *planet) {
    return planet->outgoing_routes;
}

static void test_burst_export_line_counts_the_outgoing_routes(void) {
    p8_memory_t memory;
    p8_graphics_t graphics;
    planet_t *planet;
    static production_t production;
    vec_t *planet_pos;
    entity_list_t *outgoing;
    trade_route_t *route_a;
    trade_route_t *route_b;
    const entity_list_t *ephemerals;
    const layout_row_t *exports_layout_row = NULL;
    int index;

    setup_panel_state(&memory, &graphics);

    planet = burst_new(NULL);
    assert(planet != NULL);

    /* 99 is deliberately not one of `jo.cq`'s resources, so `jo:gh` takes
     * the else branch and builds the "exports:" row, not the "makes:" one. */
    production.has_resource = 1;
    production.resource = p8_fixed_from_int(99);
    production.amount = p8_fixed_from_int(2);
    planet->production = &production;

    planet_pos = malloc(sizeof *planet_pos);
    *planet_pos = vec_make(p8_fixed_from_int(64), p8_fixed_from_int(64));
    planet->pos = planet_pos;

    /* A fresh `ga` exports nothing, against a capacity of `kw.mu`. */
    planet_inspection_panel(planet);
    ephemerals = main_ui_ephemerals();
    for (index = 0; index < ephemerals->count; index++) {
        const layout_row_t *lq = button_layout_row(ephemerals->items[index]);
        if (layout_row_index_is_string(lq, 1, "exports:")) {
            exports_layout_row = lq;
        }
    }
    assert(exports_layout_row != NULL);
    assert(layout_row_index_is_string(exports_layout_row, 2, "0/2"));

    /* Two routes, added the way `jo:ib`'s `add(ib,a)` adds them: straight
     * into whatever `self.by` already holds, here the value the `ga`
     * declaration installed. */
    outgoing = planet_by_ref(planet);
    assert(outgoing != NULL);
    route_a = trade_new(game_zone(), NULL);
    route_b = trade_new(game_zone(), NULL);
    entity_list_add(outgoing, &route_a->header);
    entity_list_add(outgoing, &route_b->header);

    exports_layout_row = NULL;
    planet_inspection_panel(planet);
    ephemerals = main_ui_ephemerals();
    for (index = 0; index < ephemerals->count; index++) {
        const layout_row_t *lq = button_layout_row(ephemerals->items[index]);
        if (layout_row_index_is_string(lq, 1, "exports:")) {
            exports_layout_row = lq;
        }
    }
    assert(exports_layout_row != NULL);

    /* Two routes out of a capacity of two -- not "0/2", and not a number
     * assembled out of a neighbouring key. */
    assert(layout_row_index_is_string(exports_layout_row, 2, "2/2"));
}

int main(void) {
    test_burst_export_line_counts_the_outgoing_routes();

    printf("test_burst_exports_row: all tests passed\n");
    return 0;
}
