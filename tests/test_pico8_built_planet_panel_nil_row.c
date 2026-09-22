/* Phase 5 bug fix: a garbage 8x8 icon (`spr(0)`) drawn over a BUILT planet
 * when its info panel opens.
 *
 * WHAT A HUMAN SEES: press and release the mouse over a planet that has a
 * building on it. Real PICO-8 opens the planet's info panel -- the label
 * row, then one row per "needs:"/"exports:" line. This port drew one extra
 * row above all of those, right on top of the planet, holding an 8x8 block
 * of raw sprite-sheet ramp data (PICO-8 sprite #0).
 *
 * WHY IT HAPPENED (three real lines of pico8/lua.lua, chained):
 *
 *   1. `jo:lz(hp)` (pico8/lua.lua:461-463) is `self.hn,self.ep=hp` -- two
 *      variables, ONE value, so Lua 5.2 (manual 3.3.3) assigns nil to
 *      `self.ep`. Placing a building therefore NILS the planet's own
 *      description string. The port reproduces that faithfully
 *      (src/pico8_planet.c, `pico8_value_ref(NULL)` -- this port's own
 *      documented nil stand-in, src/pico8_value.h).
 *   2. `pz(p,bs)` (pico8/lua.lua:280-281) prepends a description row only
 *      `if p.ep then`. In Lua that is FALSE for a built planet, so there
 *      is no such row at all. src/pico8_msgbox.c's own truthiness helper
 *      reported this port's nil as TRUE, so the row was built anyway,
 *      holding nil.
 *   3. `lq:mn(f)` (pico8/lua.lua:300-312) turns a non-string, non-table
 *      item into a sprite only in its `elseif f then` branch -- again
 *      false for nil. src/pico8_layout.c's own truthiness helper had the
 *      same gap, so the nil item became a sprite whose id fell back to 0.
 *
 * HOW THIS TEST REACHES IT: exactly the way a player does, and by the same
 * idiom as tests/test_pico8_playability.c / test_pico8_empty_planet_
 * cleared.c / test_pico8_slipway_drag.c -- scripted mouse input only,
 * through the real pico8__update60()/pico8__draw() dispatch chain.
 * tests/fixtures/built_planet_panel_events.json is the phase4 playthrough
 * script (which really colonizes planets "p27" and "p48" by dropping a
 * building on them, i.e. really runs `jo:hu` -> `jo:lz`) followed by plain
 * click-and-releases:
 *   - first on the still-uncolonized "known" planet "p1" (`jo:ldown` ->
 *     `jo:fe` -> `pz`, whose `ep` is a real string) -- the CONTROL panel;
 *   - then twice on the COLONIZED planet "p27": the first of those two is
 *     swallowed dismissing the control panel (any press with a panel open
 *     closes it), the second really runs `jo:ldown` -> `jc({qu=self})` and,
 *     on release over the same planet, `jo:gv` -> `jo:gh` -> `pz` -- the
 *     panel under test.
 * The cursor is parked far away for the settling frames, so the mouse
 * sprite does not overdraw the area under test. Frame layout, for anyone
 * regenerating the fixture: frames 0..223 are phase4_playthrough_events.
 * json verbatim, then each click block is 3 hovering frames + 4 held
 * frames + 1 released frame + settling frames parked at (2,120) -- 22
 * frames on "p1" at screen (67,14), 10 dismissing on "p27" at screen
 * (88,37), then 22 more on "p27". Both screen positions are the planets'
 * own live `ra` minus the camera's own (192,192) offset, the same way
 * tests/test_pico8_empty_planet_cleared.c derives its own.
 *
 * WHAT IT ASSERTS, both on observable output rather than on any internal
 * flag:
 *   - the built planet's panel carries exactly the rows `jo:gh` built for
 *     it, with no extra row prepended, and its first row really starts
 *     with the planet's own label string;
 *   - sprite #0's own bottom pixel row -- read back out of the loaded
 *     sprite sheet through pico8_sget(), not hard-coded -- appears
 *     NOWHERE in the 128x128 framebuffer. That run is what `spr(0)` put on
 *     screen (measured at (89,46) on the faulty build, and nowhere at all
 *     once fixed).
 *   - CONTROL, so the fix cannot be "delete the description row": the
 *     uncolonized planet's panel still DOES lead with its own description
 *     string, because its `ep` was never nilled.
 *
 * Full engine source list in the Makefile for the same reason
 * test_pico8_playability's is: pico8_cart_load() pulls in the whole port. */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_class.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_help.h"
#include "../src/pico8_input.h"
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
 * ("p1"), and of the one on the COLONIZED planet ("p27") -- in that order,
 * see this file's own top comment. */
#define KNOWN_PANEL_FRAME 245
#define BUILT_PANEL_FRAME 277

/* jo:gh's own rows for this planet, counted from the real Lua
 * (pico8/lua.lua:418-435): the label row, the "needs:" row and the
 * "exports:" row. Nothing else -- `jo:lz` nilled `ep`, so `pz` prepends
 * nothing. */
#define BUILT_PANEL_EXPECTED_ROWS 3

/* Sprite #0 is an 8x8 corner of the sprite sheet's own colour ramp; its
 * bottom row is the part that survived overdraw on the faulty build. */
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
    pico8_instance_t *ok = pico8_ok();
    pico8_value_t le;
    assert(ok != NULL);
    assert(pico8_instance_get_field(ok, "le", &le));
    assert(le.kind == PICO8_VALUE_REF && le.as.ref != NULL);
    return (const pico8_list_t *)le.as.ref;
}

/* Row `index`'s first drawable item (`lq[1]`), as a string, or NULL when
 * that item is not a string at all -- which is exactly what a nilled `ep`
 * left behind. */
static const char *row_first_item_string(const pico8_list_t *rows, int index) {
    const pico8_instance_t *button;
    const pico8_instance_t *lq;
    pico8_value_t lq_val, item;

    assert(index >= 0 && index < rows->count);
    assert(rows->items[index].kind == PICO8_VALUE_REF && rows->items[index].as.ref != NULL);
    button = (const pico8_instance_t *)rows->items[index].as.ref;
    assert(pico8_instance_get_field(button, "lq", &lq_val));
    assert(lq_val.kind == PICO8_VALUE_REF && lq_val.as.ref != NULL);
    lq = (const pico8_instance_t *)lq_val.as.ref;

    if (!pico8_ob_table_get_index(lq->fields, 1, &item)) {
        return NULL;
    }
    return item.kind == PICO8_VALUE_STRING ? item.as.string : NULL;
}

static const char *state_of(const pico8_instance_t *e) {
    pico8_value_t v;
    if (e != NULL && pico8_instance_get_field(e, "me", &v) && v.kind == PICO8_VALUE_STRING) {
        return v.as.string;
    }
    return "(none)";
}

static const pico8_instance_t *planet_by_id(const char *id) {
    const pico8_list_t *jo = pico8_cg("jo");
    int i;
    assert(jo != NULL);
    for (i = 0; i < jo->count; i++) {
        const pico8_instance_t *p = (const pico8_instance_t *)jo->items[i].as.ref;
        pico8_value_t v;
        if (pico8_instance_get_field(p, "id", &v) && v.kind == PICO8_VALUE_STRING && strcmp(v.as.string, id) == 0) {
            return p;
        }
    }
    return NULL;
}

/* Scans the whole framebuffer for the 8 pixels `spr(0)` would lay down for
 * sprite #0's bottom row. Returns the number of places it appears. */
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
    pico8_instance_t *difficulty_button;
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

    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8__update60();
        pico8__draw();

        if (i == BUILT_PANEL_FRAME) {
            const pico8_list_t *rows = panel_rows();

            /* The click really landed on a planet a building was really
             * built on: that is what nilled its `ep`. */
            assert(strcmp(state_of(planet_by_id("p27")), "ly") == 0);

            built_rows = rows->count;
            built_first_label = row_first_item_string(rows, 0);
            built_spr0_hits = sprite0_bottom_row_hits_on_screen();

            printf("[built] frame=%d released over colonized planet p27: panel has %d row(s), first row leads with "
                   "\"%s\", sprite #0 appears %d time(s) on screen\n",
                   i, built_rows, built_first_label != NULL ? built_first_label : "(not a string)", built_spr0_hits);
        } else if (i == KNOWN_PANEL_FRAME) {
            const pico8_list_t *rows = panel_rows();

            const pico8_instance_t *p1 = planet_by_id("p1");
            pico8_value_t ep;

            assert(strcmp(state_of(p1), "known") == 0);
            assert(pico8_instance_get_field(p1, "ep", &ep));
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

    /* 1. No extra row: a built planet's `ep` is nil, so `pz` prepends
     *    nothing. */
    assert(built_rows == BUILT_PANEL_EXPECTED_ROWS);

    /* 2. The panel's first row is jo:gh's own label row, a real string --
     *    not the nil a prepended description row would hold. */
    assert(built_first_label != NULL);

    /* 3. And nothing on screen is sprite #0. */
    assert(built_spr0_hits == 0);

    /* 4. CONTROL, so "delete the description row" cannot pass as a fix:
     *    an uncolonized planet's `ep` was never nilled, so `pz` really
     *    does still prepend it, and the panel leads with that very
     *    string. */
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
