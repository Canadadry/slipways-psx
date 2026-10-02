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
static const entity_list_t *ok_le(void) {
    const pico8_ui_t *ok = pico8_ok();
    assert(ok != NULL);
    assert(ok->le != NULL);
    return ok->le;
}

static const layout_row_t *button_lq(const void *button) {
    const layout_row_t *lq;

    assert(button != NULL);
    assert(((const entity_t *)button)->class == pico8_js_struct_class());
    lq = ((const pico8_js_t *)button)->lq;
    assert(lq != NULL);
    return lq;
}

static const char *lq_index_string(const layout_row_t *lq, int index) {
    layout_row_cell_t v;
    if (index < 1 || index > lq->items_count) {
        return NULL;
    }
    v = lq->items[index - 1];
    return (v.kind == LAYOUT_ROW_CELL_STRING) ? v.as.text : NULL;
}

static int lq_index_is_string(const layout_row_t *lq, int index, const char *expected) {
    const char *s = lq_index_string(lq, index);
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
    pico8_js_init();
    planet_init();
    pico8_planet_family_init();
}

/* Read on the instance's member, not the class defaults, so the test
 * says nothing about where the value comes from. */
static entity_list_t *planet_by_ref(const planet_t *planet) {
    return planet->outgoing_routes;
}

static void test_ga_export_line_counts_the_routes_in_by(void) {
    p8_memory_t mem;
    p8_graphics_t gfx;
    planet_t *planet;
    static production_t kw;
    vec_t *ra;
    entity_list_t *by;
    trade_route_t *route_a;
    trade_route_t *route_b;
    const entity_list_t *le;
    const layout_row_t *exports_lq = NULL;
    int i;

    setup_panel_state(&mem, &gfx);

    planet = pico8_ga_new(NULL);
    assert(planet != NULL);

    /* 99 is deliberately not one of `jo.cq`'s resources, so `jo:gh` takes
     * the else branch and builds the "exports:" row, not the "makes:" one. */
    kw.has_resource = 1;
    kw.resource = p8_fixed_from_int(99);
    kw.amount = p8_fixed_from_int(2);
    planet->production = &kw;

    ra = malloc(sizeof *ra);
    *ra = vec_make(p8_fixed_from_int(64), p8_fixed_from_int(64));
    planet->pos = ra;

    /* A fresh `ga` exports nothing, against a capacity of `kw.mu`. */
    planet_inspection_panel(planet);
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        const layout_row_t *lq = button_lq(le->items[i]);
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
    route_a = trade_new(game_zone(), NULL);
    route_b = trade_new(game_zone(), NULL);
    entity_list_add(by, &route_a->header);
    entity_list_add(by, &route_b->header);

    exports_lq = NULL;
    planet_inspection_panel(planet);
    le = ok_le();
    for (i = 0; i < le->count; i++) {
        const layout_row_t *lq = button_lq(le->items[i]);
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
