/* Releasing the mouse over a BUILT planet must open its info panel with no
 * extra row: `jo:lz` nils the planet's `ep`, so `pz` prepends no description
 * row. Regression test for the phantom sprite-#0 row drawn on top of the
 * planet; the trap itself is kept at the two guards that were incomplete,
 * in src/pico8_msgbox.c and src/pico8_layout.c.
 *
 * The test reaches the panel the way a player does -- scripted mouse input
 * only, through the real pico8__update60()/pico8__draw() chain, like
 * tests/test_pico8_playability.c and tests/test_pico8_empty_planet_cleared.c.
 *
 * Fixture layout, for anyone regenerating
 * tests/fixtures/built_planet_panel_events.json: frames 0..223 are
 * tests/fixtures/phase4_playthrough_events.json verbatim (it really drops a
 * building on planets "p27" and "p48", i.e. really runs `jo:hu` -> `jo:lz`),
 * then each click block is 3 hovering frames + 4 held + 1 released +
 * settling frames parked at (2,120) so the mouse sprite never overdraws the
 * area scanned for sprite #0. In order: 22 frames on the still-uncolonized
 * "p1" at screen (67,14), then 10 on the colonized "p27" at (88,37) that are
 * swallowed dismissing p1's panel (any press with a panel open closes it),
 * then 22 more on "p27" -- the panel under test. Both screen positions are
 * the planets' own live `ra` minus the camera's (192,192) offset, derived
 * the way tests/test_pico8_empty_planet_cleared.c derives its own. */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
#include "../src/pico8_cheat.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_planet.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_value.h"
#include "../src/pico8_vec.h"

#define BUILT_PLANET_EVENTS_PATH "tests/fixtures/built_planet_panel_events.json"

/* Last settled frame of the click-and-release on the still-"known" planet
 * "p1", then of the one on the colonized "p27" -- see the fixture layout
 * above. */
#define KNOWN_PANEL_FRAME 245
#define BUILT_PANEL_FRAME 277

/* `jo:gh`'s own rows for this planet: the label row, the "needs:" row and
 * the "exports:" row. Nothing else -- `jo:lz` nilled `ep`, so `pz` prepends
 * nothing. */
#define BUILT_PANEL_EXPECTED_ROWS 3

/* Sprite #0's bottom pixel row, the part that survived overdraw on the
 * faulty build. Row 7 and not row 0: row 0 is all zeros, so it would match
 * blank background at every position and make the search vacuous. */
#define SPRITE0_ROW_PIXELS 8
#define SPRITE0_BOTTOM_ROW_Y 7

static pico8_mem_t mem;
static pico8_gfx_t gfx;
static pico8_input_t input;

static int screen_px(int x, int y) {
    return pico8_peek(&mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x);
}

/* The rows `pz()` just built, as the panel singleton holds them. */
static const pico8_list_t *panel_rows(void) {
    const void *ok = pico8_ok();
    pico8_value_t le;
    assert(ok != NULL);
    assert(pico8_entity_get_field(ok, "le", &le));
    assert(le.kind == PICO8_VALUE_REF && le.as.ref != NULL);
    return (const pico8_list_t *)le.as.ref;
}

/* Row `index`'s first drawable item (`lq[1]`) as a string, or NULL when that
 * item is not a string at all -- which is what a nilled `ep` left behind. */
static const char *row_first_item_string(const pico8_list_t *rows, int index) {
    const void *button;
    const pico8_lq_t *lq;
    pico8_value_t lq_val, item;

    assert(index >= 0 && index < rows->count);
    assert(rows->items[index].kind == PICO8_VALUE_REF && rows->items[index].as.ref != NULL);
    button = rows->items[index].as.ref;
    assert(pico8_entity_get_field(button, "lq", &lq_val));
    assert(lq_val.kind == PICO8_VALUE_REF && lq_val.as.ref != NULL);
    lq = (const pico8_lq_t *)lq_val.as.ref;

    if (lq->items.count < 1) {
        return NULL;
    }
    item = lq->items.items[0];
    return item.kind == PICO8_VALUE_STRING ? item.as.string : NULL;
}

static const char *state_of(const void *e) {
    pico8_value_t v;
    if (e != NULL && pico8_entity_get_field(e, "me", &v) && v.kind == PICO8_VALUE_STRING) {
        return v.as.string;
    }
    return "(none)";
}

static const void *planet_by_id(const char *id) {
    const pico8_list_t *jo = pico8_cg("jo");
    int i;
    assert(jo != NULL);
    for (i = 0; i < jo->count; i++) {
        const void *p = jo->items[i].as.ref;
        pico8_value_t v;
        if (pico8_entity_get_field(p, "id", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, id) == 0) {
            return p;
        }
    }
    return NULL;
}

/* How many places sprite #0's bottom row appears in the framebuffer. The
 * wanted run is read back out of the loaded sprite sheet, never hard-coded. */
static int sprite0_bottom_row_hits_on_screen(void) {
    int wanted[SPRITE0_ROW_PIXELS];
    int i, x, y, hits = 0;

    for (i = 0; i < SPRITE0_ROW_PIXELS; i++) {
        wanted[i] = pico8_sget(&mem, i, SPRITE0_BOTTOM_ROW_Y);
    }

    for (y = 0; y < PICO8_SCREEN_HEIGHT; y++) {
        for (x = 0; x + SPRITE0_ROW_PIXELS <= PICO8_SCREEN_WIDTH; x++) {
            int match = 1;
            for (i = 0; i < SPRITE0_ROW_PIXELS; i++) {
                if (screen_px(x + i, y) != wanted[i]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                fprintf(stderr, "[spr0] sprite #0's bottom row is on screen at (%d,%d)\n", x, y);
                hits++;
            }
        }
    }
    return hits;
}

int main(void) {
    pico8_value_t difficulty_entry;
    pico8_js_t *difficulty_button;
    int frame_count;
    int i;
    int built_rows = -1;
    int built_spr0_hits = -1;
    const char *built_first_label = NULL;
    const char *known_first_label = NULL;
    const char *known_planet_ep = NULL;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, BUILT_PLANET_EVENTS_PATH));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    difficulty_button = pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref);
    pico8_eu(difficulty_button);

    frame_count = input.frame_count;
    assert(frame_count == BUILT_PANEL_FRAME + 1);

    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();

        if (i == BUILT_PANEL_FRAME) {
            const pico8_list_t *rows = panel_rows();

            /* The click landed on a planet a building was really built on:
             * that is what nilled its `ep`. */
            assert(strcmp(state_of(planet_by_id("p27")), "ly") == 0);

            built_rows = rows->count;
            built_first_label = row_first_item_string(rows, 0);
            built_spr0_hits = sprite0_bottom_row_hits_on_screen();

            printf("[built] frame=%d released over colonized planet p27: panel has %d row(s), first row leads with "
                   "\"%s\", sprite #0 appears %d time(s) on screen\n",
                   i, built_rows, built_first_label != NULL ? built_first_label : "(not a string)", built_spr0_hits);
        } else if (i == KNOWN_PANEL_FRAME) {
            const pico8_list_t *rows = panel_rows();

            const void *p1 = planet_by_id("p1");
            pico8_value_t ep;

            assert(strcmp(state_of(p1), "known") == 0);
            assert(pico8_entity_get_field(p1, "ep", &ep));
            assert(ep.kind == PICO8_VALUE_STRING);
            known_planet_ep = ep.as.string;

            assert(rows->count >= 1);
            known_first_label = row_first_item_string(rows, 0);

            printf("[known] frame=%d clicked still-uncolonized planet p1 (ep=\"%s\"): panel has %d row(s), first row "
                   "leads with \"%s\"\n",
                   i, known_planet_ep, rows->count, known_first_label != NULL ? known_first_label : "(not a string)");
        }

        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    /* No extra row: a built planet's `ep` is nil, so `pz` prepends nothing. */
    assert(built_rows == BUILT_PANEL_EXPECTED_ROWS);

    /* Its first row is `jo:gh`'s own label row, a real string -- not the nil
     * a prepended description row would hold. */
    assert(built_first_label != NULL);

    /* And nothing on screen is sprite #0. */
    assert(built_spr0_hits == 0);

    /* CONTROL, so "delete the description row" cannot pass as a fix: an
     * uncolonized planet's `ep` was never nilled, so `pz` really does still
     * prepend it, and its panel leads with that very string. */
    assert(known_planet_ep != NULL);
    assert(known_first_label != NULL);
    assert(strcmp(known_first_label, known_planet_ep) == 0);

    pico8_input_free(&input);

    printf("pico8 built planet panel: %d frame(s) run against %s; releasing over a built planet opens a panel with no "
           "spurious description row and no sprite #0 drawn on the planet, while an unbuilt planet keeps its own "
           "description row\n",
           frame_count, BUILT_PLANET_EVENTS_PATH);
    return 0;
}
