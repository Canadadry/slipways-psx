/* Replays tests/fixtures/tech_screen_invent_events.json headless and checks
 * that it reaches the tech screen and invents a tech through it.
 *
 * The game starts the way src/main.c starts every replay of the net:
 * `eu()` on `bo[2]` before frame 0, no title screen.
 *
 * The fixture's stages and what grounds each check:
 *
 *   1. start (before frame 0). `eu()` creates `fm` with `jk=0`, `ew=1` and
 *      an empty `invented` (lua.lua:1200-1201).
 *   2. five skipped years (frames 14-23). Each click on `ix[1]` runs
 *      `hy:hk()`, which adds `po[r.k]+1` research to `fm.jk` (lua.lua:1212).
 *   3. tech screen (frame 24). A click on `ix[2]` runs `cr()`, whose `pz()`
 *      replaces `ok.le` with the "level" header plus one row per tech of
 *      the era, affordable or not (`cb(true)`, lua.lua:956-964).
 *   4. invention (frame 46). Once the rows have slid into place, a click
 *      on the first tech row runs its `my`, i.e. `fm:kt(t)` (lua.lua:957),
 *      which spends `gi(t)` research and appends the tech to `invented`
 *      (lua.lua:1447-1448). The first row is affordable at that point:
 *      the check at frame 45 asserts it rather than assuming it. */

#include <stdio.h>
#include <assert.h>

#include "../src/pico8_mem.h"
#include "../src/pico8_assets.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_cheat.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_help.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_class.h"
#include "../src/pico8_value.h"
#include "../src/pico8_list.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_ui.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_tech_gate.h"
#include "../src/pico8_fixed_test_double.h"

#define TECH_SCREEN_EVENTS_PATH "tests/fixtures/tech_screen_invent_events.json"

#define BEFORE_TECH_SCREEN_FRAME 23
#define TECH_SCREEN_SETTLED_FRAME 45
#define INVENT_FRAME 46

static pico8_fixed_t fm_fixed(const char *name) {
    pico8_value_t v;
    pico8_instance_t *fm = pico8_fm();
    assert(fm != NULL && pico8_instance_get_field(fm, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static int invented_count(void) {
    pico8_value_t v;
    pico8_instance_t *fm = pico8_fm();
    assert(fm != NULL && pico8_instance_get_field(fm, "invented", &v) && v.kind == PICO8_VALUE_REF &&
           v.as.ref != NULL);
    return ((const pico8_list_t *)v.as.ref)->count;
}

static const pico8_list_t *ok_le(void) {
    pico8_value_t v;
    pico8_instance_t *ok = pico8_ok();
    if (ok == NULL || !pico8_instance_get_field(ok, "le", &v) || v.kind != PICO8_VALUE_REF || v.as.ref == NULL) {
        return NULL;
    }
    return (const pico8_list_t *)v.as.ref;
}

int main(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    int frame_count;
    pico8_value_t difficulty_entry;
    int i;
    pico8_fixed_t jk_before = 0;
    pico8_fixed_t first_cost = 0;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, TECH_SCREEN_EVENTS_PATH));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref));

    frame_count = input.frame_count;
    assert(frame_count > INVENT_FRAME);

    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();

        if (i == BEFORE_TECH_SCREEN_FRAME) {
            assert(invented_count() == 0);
            assert(fm_fixed("ew") == pico8_fixed_from_int(1));
            assert(fm_fixed("yr") == pico8_fixed_from_int(3406));
            printf("[years] frame=%d yr=3406 jk=%g, nothing invented\n", i,
                   pico8_fixed_to_double(fm_fixed("jk")));
        } else if (i == TECH_SCREEN_SETTLED_FRAME) {
            const pico8_list_t *le = ok_le();
            pico8_list_t techs = pico8_cb(pico8_value_bool(1));
            const pico8_ob_table_t *first;

            assert(techs.count > 0);
            assert(le != NULL && le->count == 1 + techs.count);
            first = (const pico8_ob_table_t *)techs.items[0].as.ref;
            first_cost = pico8_gi(first);
            jk_before = fm_fixed("jk");
            assert(first_cost <= jk_before);
            assert(invented_count() == 0);
            printf("[tech screen] frame=%d %d rows (header + %d techs), first costs %g of jk=%g\n", i, le->count,
                   techs.count, pico8_fixed_to_double(first_cost), pico8_fixed_to_double(jk_before));
            pico8_list_free(&techs);
        } else if (i == INVENT_FRAME) {
            assert(invented_count() == 1);
            assert(fm_fixed("jk") == pico8_fixed_sub(jk_before, first_cost));
            printf("[invent] frame=%d invented=1, jk %g -> %g\n", i, pico8_fixed_to_double(jk_before),
                   pico8_fixed_to_double(fm_fixed("jk")));
        }

        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    pico8_input_free(&input);

    printf("pico8 tech screen: ix[2] opens the tech screen and clicking an affordable tech invents it\n");
    return 0;
}
