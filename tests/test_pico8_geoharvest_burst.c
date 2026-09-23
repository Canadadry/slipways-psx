/* Replays tests/fixtures/geoharvest_burst_events.json headless and checks
 * that it harvests a planet into a live particle burst, a `ga`.
 *
 * The only recipe whose `cw` names `ga` is "harvest" (rr="geoharvesting",
 * fj=-20), the last entry of `gb`: `jo:hu` then replaces the planet by
 * `kv[hp.cw]` (lua.lua:451-455), and `ga:pq` fills `qy` with the burst's
 * particles (`ez(self,140)`, lua.lua:1079).
 *
 * The game starts the way src/main.c starts every replay of the net:
 * `eu()` on `bo[2]` before frame 0, no title screen. The galaxy, and so
 * every position below, is the one that start draws.
 *
 * The fixture's stages and what grounds each check:
 *
 *   1. probe (frames 0-22). A click on empty space at (108,10), more than
 *      sqrt(dw)*128 = 20px from every planet, places a `g_` (lua.lua:552-
 *      557) ; the press held on "p27" at (88,37) then launches an `mz`
 *      probe from it, which reveals "p27" (lua.lua:777-783).
 *   2. research (frames 43-88). 23 clicks on `ix[1]`, one research each
 *      with no colony (lua.lua:1212): space folding (4) + geneseeds (5) +
 *      geoharvesting (14), at `fu` = 1 with no laboratory (lua.lua:1252).
 *   3. inventions (frames 89-172). The tech screen is reopened for each:
 *      its first row twice (level 1, `ek[1]`=2 raises `ew` to 2), then its
 *      third, geoharvesting (level 2 rows follow `nb` order).
 *   4. harvest (press at frame 205). A click on "p27" opens its build menu,
 *      whose last row is "harvest"; `fm:lu(-20)` pays the player 20.
 *   5. burst end (frame 289). `ga:kj` sets `p_` once fewer than 3
 *      particles remain (lua.lua:1087-1093), which removes the `ga`. */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../src/pico8_mem.h"
#include "../src/pico8_assets.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_class.h"
#include "../src/pico8_value.h"
#include "../src/pico8_list.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_help.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_fixed_test_double.h"

#define GEOHARVEST_EVENTS_PATH "tests/fixtures/geoharvest_burst_events.json"

#define BEFORE_HARVEST_FRAME 204
#define HARVEST_FRAME 205
#define BURST_OVER_FRAME 289

#define HARVEST_PAYOUT 20
#define GA_KJ_PARTICLE_SLOTS 3

static pico8_fixed_t fm_fixed(const char *name) {
    pico8_value_t v;
    pico8_instance_t *fm = pico8_fm();
    assert(fm != NULL && pico8_instance_get_field(fm, name, &v) && v.kind == PICO8_VALUE_FIXED);
    return v.as.fixed;
}

static const pico8_list_t *invented(void) {
    pico8_value_t v;
    pico8_instance_t *fm = pico8_fm();
    assert(fm != NULL && pico8_instance_get_field(fm, "invented", &v) && v.kind == PICO8_VALUE_REF &&
           v.as.ref != NULL);
    return (const pico8_list_t *)v.as.ref;
}

/* `ga` sets no `nu`, so it sits in no `cg` bucket: it is looked up in `hv`.
 * Returns NULL when no `ga` is alive. */
static pico8_instance_t *live_ga(void) {
    const pico8_list_t *hv = pico8_hv();
    pico8_class_t *ga_class = pico8_class_lookup("ga");
    int i;
    assert(ga_class != NULL);
    for (i = 0; i < hv->count; i++) {
        pico8_instance_t *e = (pico8_instance_t *)hv->items[i].as.ref;
        if (e != NULL && e->cls == ga_class) {
            return e;
        }
    }
    return NULL;
}

static int particle_count(pico8_instance_t *ga) {
    pico8_value_t qy;
    assert(pico8_instance_get_field(ga, "qy", &qy) && qy.kind == PICO8_VALUE_REF && qy.as.ref != NULL);
    return ((const pico8_list_t *)qy.as.ref)->count;
}

int main(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    int frame_count;
    int i;
    pico8_fixed_t nz_before = 0;
    pico8_value_t difficulty_entry;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, GEOHARVEST_EVENTS_PATH));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_dispatch_set_input(&input);

    pico8_cart_load();
    pico8__init();

    assert(pico8_ob_table_get_index(pico8_bo(), 2, &difficulty_entry));
    assert(difficulty_entry.kind == PICO8_VALUE_REF);
    pico8_eu(pico8_js_new((const pico8_ob_table_t *)difficulty_entry.as.ref));

    frame_count = input.frame_count;
    assert(frame_count > BURST_OVER_FRAME);

    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8__update60();
        pico8__draw();

        if (i == BEFORE_HARVEST_FRAME) {
            const pico8_list_t *inv = invented();
            pico8_value_t last;
            assert(inv->count == 3);
            last = inv->items[inv->count - 1];
            assert(last.kind == PICO8_VALUE_STRING && strcmp(last.as.string, "geoharvesting") == 0);
            assert(live_ga() == NULL);
            nz_before = fm_fixed("nz");
            printf("[research] frame=%d geoharvesting invented third, no ga yet, nz=%g\n", i,
                   pico8_fixed_to_double(nz_before));
        } else if (i == HARVEST_FRAME) {
            pico8_instance_t *ga = live_ga();
            assert(ga != NULL);
            assert(particle_count(ga) >= GA_KJ_PARTICLE_SLOTS);
            assert(fm_fixed("nz") == pico8_fixed_add(nz_before, pico8_fixed_from_int(HARVEST_PAYOUT)));
            printf("[harvest] frame=%d a ga is alive with %d particles, nz=%g\n", i, particle_count(ga),
                   pico8_fixed_to_double(fm_fixed("nz")));
        } else if (i == BURST_OVER_FRAME) {
            assert(live_ga() == NULL);
            printf("[burst over] frame=%d the ga is gone\n", i);
        }

        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    pico8_input_free(&input);

    printf("pico8 geoharvest burst: harvesting a planet leaves a ga that ga:kj draws until its burst dies\n");
    return 0;
}
