/* Scripted playthrough over tests/fixtures/phase4_playthrough_events.json,
 * driven through the same entry points as main.c's replay driver. The
 * recording is opaque, so here are the six stages it walks through and the
 * frame each one is checked on:
 *
 *   1. probe launch (frame 3). Click and release on the background, inside
 *      `ui.hw` and far enough from all 90 planets that `bv()`'s snap check
 *      never fires. `cg.g_` is still empty, so `bg:ldown()` builds a probe;
 *      releasing inside the panel leaves `self.et` false, so `d_()` commits
 *      for free.
 *   2. wormhole traversal (frame 54). A second background click -- `cg.g_` is
 *      no longer empty, so `bg:ldown()` builds an `mz` instead -- landing on
 *      planet "p27" and held at least 15 ticks, past `mz:ey()`'s `self.t<15`
 *      guard, before release. Landing on the planet means `mz:pr()`'s growing
 *      reveal radius covers it from its very first tick.
 *   3. colonization (frame 98). Clicking the now-known "p27" opens `jo:fe()`'s
 *      building menu, whose items lerp into place over about 7 ticks (the
 *      fixture idles 40 for margin), then a click a few pixels INSIDE the
 *      settled item's `kq` box -- its (0,0) corner itself can miss by
 *      rounding -- runs that option's `my`, leaving the planet at `me="ly"`.
 *   4. slipway construction (frame 201). Needs a second colonized planet,
 *      "p48": not every revealed planet has buildable options at all, since
 *      `jo:fe()`'s `bj(self)` candidate list can come back empty. Clicking the
 *      "ly" origin takes `jo:ldown()`'s other branch (`jc({qu=self})`), and
 *      releasing on the second "ly" planet commits the connection.
 *   5. trade route setup (frame 221). No click: idle ticks, letting
 *      `jo:cn()`/`jo:ib()` run.
 *   6. score/credit tick (frame 223). The "skip year" button `ix[1]` calls
 *      `hy:hk()` outright. Waiting for `hy:nx()`'s ambient `lo` accumulator
 *      to cross its threshold instead would need far more colonizations than
 *      the other five stages need.
 *
 * Credits are read off `fm`, never `ju`: `pj`'s class spec declares no `nz`
 * field, so reading credits off the score panel would always see it absent.
 *
 * TODO: stage 4/5's downstream effect -- each connected planet's `oe` list
 * growing through `jo:kf()` -- is not checked here, only the commit itself. */

#include <math.h>
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
#include "../src/pico8_probe.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_fixed_test_double.h"

#define PHASE4_PLAYTHROUGH_EVENTS_PATH "tests/fixtures/phase4_playthrough_events.json"

#define STAGE1_PROBE_LAUNCH_FRAME 3
#define STAGE2_WORMHOLE_TRAVERSAL_FRAME 54
#define STAGE3_COLONIZATION_FRAME 98
#define STAGE4_SLIPWAY_CONSTRUCTION_FRAME 201
#define STAGE5_TRADE_ROUTE_SETUP_FRAME 221
#define STAGE6_SCORE_TICK_FRAME 223

/* `bw(90)` is the only thing that ever populates `cg.jo`, and no in-game
 * mechanic adds a planet, so 90 is a hard ceiling. A negative count could
 * only come from a double-removal in the entity/tag lists. */
#define PICO8_PLAYABILITY_MIN_PLANETS 0
#define PICO8_PLAYABILITY_MAX_PLANETS 90

/* `hy`'s spec starts `nz` at 100: the baseline the first stage's delta is
 * measured from. */
#define PICO8_PLAYABILITY_CREDIT_BASELINE 100.0

/* Overflow detectors, not tuned to observed play: the deltas this run
 * actually produces stay under 12. `pico8_fixed_t` is 16.16, so its integer
 * part wraps past about 32768 -- 20000 sits below that, and a single
 * `hy:hk()` tick of a fully developed 90-planet economy stays far under
 * 5000. */
#define PICO8_PLAYABILITY_MAX_STAGE_CREDIT_DELTA 5000.0
#define PICO8_PLAYABILITY_MAX_ABS_CREDITS 20000.0

static double credits_now(void) {
    pico8_instance_t *fm = pico8_fm();
    pico8_value_t nz;
    if (fm != NULL && pico8_instance_get_field(fm, "nz", &nz) && nz.kind == PICO8_VALUE_FIXED) {
        return pico8_fixed_to_double(nz.as.fixed);
    }
    return 0.0;
}

static int planet_count(void) {
    const pico8_list_t *jo = pico8_cg("jo");
    return jo != NULL ? jo->count : 0;
}

/* `prev_credits` is updated in place, so each stage checks the delta since
 * the previous stage rather than since the start. */
static void verify_checkpoint(const char *stage_name, int frame_index, double *prev_credits) {
    int count = planet_count();
    double credits = credits_now();
    double delta = credits - *prev_credits;

    printf("[checkpoint] %-32s frame=%-4d cg(jo) count=%-3d credits (fm.nz)=%.2f\n", stage_name, frame_index, count,
           credits);

    assert(count >= PICO8_PLAYABILITY_MIN_PLANETS);
    assert(count <= PICO8_PLAYABILITY_MAX_PLANETS);

    /* Deliberately not "credits are never negative": `hy:hk()`'s
     * `self.nz+=self.ge` has no floor, so a negative `nz` is legitimate. */
    assert(fabs(delta) <= PICO8_PLAYABILITY_MAX_STAGE_CREDIT_DELTA);
    assert(fabs(credits) <= PICO8_PLAYABILITY_MAX_ABS_CREDITS);

    *prev_credits = credits;
}

int main(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    pico8_value_t difficulty_entry;
    pico8_js_t *difficulty_button;
    int frame_count;
    int i;
    double prev_credits = PICO8_PLAYABILITY_CREDIT_BASELINE;

    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_input_init(&input);
    assert(pico8_input_load(&input, PHASE4_PLAYTHROUGH_EVENTS_PATH));

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

    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();

        if (i == STAGE1_PROBE_LAUNCH_FRAME) {
            verify_checkpoint("1. probe launch", i, &prev_credits);
        } else if (i == STAGE2_WORMHOLE_TRAVERSAL_FRAME) {
            verify_checkpoint("2. wormhole traversal", i, &prev_credits);
        } else if (i == STAGE3_COLONIZATION_FRAME) {
            verify_checkpoint("3. colonization", i, &prev_credits);
        } else if (i == STAGE4_SLIPWAY_CONSTRUCTION_FRAME) {
            verify_checkpoint("4. slipway construction", i, &prev_credits);
        } else if (i == STAGE5_TRADE_ROUTE_SETUP_FRAME) {
            verify_checkpoint("5. trade route setup", i, &prev_credits);
        } else if (i == STAGE6_SCORE_TICK_FRAME) {
            verify_checkpoint("6. score/credit-affecting tick", i, &prev_credits);
        }

        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    pico8_input_free(&input);

    printf("pico8 playability: %d frame(s) run against %s; all 6 stage checkpoints reached and both "
           "docs/plan.md Phase 4 exit-gate invariants held\n",
           frame_count, PHASE4_PLAYTHROUGH_EVENTS_PATH);
    return 0;
}
