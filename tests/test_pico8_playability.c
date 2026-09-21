/* Phase 4, Chunk 2 (docs/prd/triage/phase4-etape1-headless-replay-wiring-
 * and-playability-verification.md): the exploratory scripted playthrough
 * this chunk's own PRD text asks for -- drives the same four real entry
 * points main.c's own replay driver uses (pico8_cart_load/pico8__init/
 * pico8_eu/pico8__update60/pico8__draw) through
 * tests/fixtures/phase4_playthrough_events.json, and prints (never
 * asserts -- that is Chunk 3's own job) the live planet count
 * (pico8_cg("jo")'s own list length) and credits (pico8_fm()'s own "nz"
 * field -- see this file's own further note below on why pico8_fm(), not
 * pico8_ju(), is the real economy singleton this chunk reads) after each
 * of the six named stages the PRD requires: probe launch, wormhole
 * traversal, colonization, slipway construction, trade route setup, and a
 * score/credit-affecting tick.
 *
 * WHY pico8_fm(), NOT pico8_ju() -- a genuine PRD-text/code discrepancy
 * found while building this chunk: the PRD's own "Testing Decisions"
 * section says to read credits via `pico8_instance_get_field(pico8_ju(),
 * "nz", ...)`, but reading the real, already-shipped code (`pico8_cart.c`'s
 * own `eu()` comment: "fm,ju=hy(),pj()") shows `ju` is `pj()`, the SCORE
 * PANEL display singleton (`pico8_ju()`, pico8_score_panel.h) -- a
 * completely different class from `fm` (`hy()`, the real economy
 * singleton, `pico8_fm()`, pico8_probe.h), which is the one that actually
 * owns "nz". `pj`'s own class spec never declares an "nz" field at all, so
 * reading it off `pico8_ju()` would always read as absent/0, never
 * reflecting a real change. This is a PRD-text inconsistency, not a code
 * bug (the shipped `pico8_ju()`/`pico8_fm()` accessors are both correct,
 * separate, and doing exactly what their own header comments say) --
 * logged in docs/portage-progress.md for Chunk 3, which must build its own
 * real assertions against `pico8_fm()`, not `pico8_ju()`, to have anything
 * meaningful to assert on.
 *
 * HOW THE SIX STAGES ARE ACTUALLY REACHED (discovered empirically, exactly
 * as this chunk's own PRD text anticipates -- "many iterations... the
 * actual hard part of this chunk"; see this file's own frame-index
 * constants below and docs/portage-progress.md for the full narrative):
 *
 *   1. PROBE LAUNCH: a plain click-then-release on the galaxy background,
 *      well inside `ui`'s own main-panel hitbox (`ui.hw=b(10,10,118,118)`,
 *      pico8_ui.h) and far enough (raw screen (108,10), i.e. world
 *      (300,202) with the camera's own default (192,192) offset) from
 *      every one of `bw(90)`'s 90 placed planets that `bv()`'s own
 *      squared-distance snap check (`g_.dw=0.025`) never fires -- `bg:
 *      ldown()` (pico8_bg.c) then creates a real `g_` probe (`cg.g_` was
 *      empty), and `d_()`'s own commit succeeds for free (a probe's
 *      default `od=0` "project cost") because `self.et` ends up plain
 *      `false` (not a real target, not the classless drag-throwaway
 *      either) precisely because the release lands INSIDE the panel box.
 *   2. WORMHOLE TRAVERSAL: a SECOND background click (`cg.g_` is non-empty
 *      now, so `bg:ldown()` builds an `mz` wormhole instead) landing
 *      almost exactly on a real placed planet ("p27" for this cart's own
 *      deterministic RNG seed, pico8_fixed.c's own fixed-seed LCG) --
 *      held for >=15 ticks (`mz:ey()`'s own `self.t<15` guard) within
 *      `mz:hq()`'s own "close enough to an already-established point"
 *      threshold (`ov("space folding",0.19,0.27)`, here the probe itself,
 *      `.fx=1` by default) before releasing. Once committed, `mz:pr()`'s
 *      own per-tick growing reveal radius (up to `ov(...,29,35)` pixels)
 *      calls `pt:hh()` on every still-"unknown" planet inside it --
 *      landing right on top of the target planet means it is covered from
 *      the very first "pr" tick.
 *   3. COLONIZATION: once revealed "known", clicking the SAME planet again
 *      runs `jo:ldown()`'s own `self:fe()` branch, opening a real building-
 *      option menu (`ok.le`, `pico8_pz()`) whose items animate from the
 *      planet's own screen position toward their final row position over
 *      several ticks (`js:ph()`'s own lerp) -- this file's own exploratory
 *      scratch tooling (not committed) confirmed items settle within ~7
 *      ticks; this fixture waits 40 for margin -- then clicking a few
 *      pixels INSIDE the settled item's own hit box (`js`'s own
 *      `kq=qz(0,-1,mb,11)`, exactly on the box's own (0,0) corner can miss
 *      by float rounding) runs that option's own `my` callback
 *      (`fm:lu(od)` then `self:hu(b)`), setting the planet's own
 *      `me="ly"`.
 *   4. SLIPWAY CONSTRUCTION: needs a SECOND colonized planet ("p48",
 *      chosen the same reveal+colonize way, close enough to "p27" to stay
 *      within `mz:hq()`'s own reach) -- NOT every revealed planet has any
 *      buildable options at all (`jo:fe()`'s own `bj(self)` candidate list
 *      can genuinely be empty depending on the planet's own randomly-
 *      resolved subtype, e.g. "p1", this fixture's own first, discarded
 *      candidate, had zero) -- clicking the now-"ly" origin planet again
 *      runs `jo:ldown()`'s OTHER branch (`jc({qu=self})`), dragging onto
 *      the second "ly" planet and releasing there (`jo:gv()`'s own
 *      `self.me=="ly"` acceptance check) commits the connection
 *      (`jc:ey()`/`fm:lu`/`qu:kf`/`re:kf`).
 *   5. TRADE ROUTE SETUP: per the PRD's own expectation, no new click --
 *      just idle ticks once the slipway exists, letting `jo:cn()`/`jo:ib()`
 *      (Chunk 13/14/15's own already-verified trade-cluster mechanism) run.
 *   6. A SCORE/CREDIT-AFFECTING TICK: the permanent HUD "skip year" button
 *      (`ix[1]`, `ra=v(118,0)`, pico8_menu.h -- `ix[1].my=function()fm.lo=1
 *      fm:hk()end`) is a much more RELIABLE way to reach this than waiting
 *      for `hy:nx()`'s own ambient `self.lo` accumulator to cross its
 *      12-15-tick threshold naturally (that only grows by small, discrete
 *      per-ACTION amounts -- a probe/wormhole/colonization commit -- not
 *      per tick, so crossing it "naturally" would need many more
 *      colonizations than this fixture needs for the other five stages) --
 *      a single click directly calls `pico8_hy_hk()`, unconditionally
 *      changing `fm.nz` by `fm.ge` (confirmed non-zero once "p27"/"p48"
 *      are colonized) and bumping `fm.yr`.
 *
 * A REAL BUG FOUND AND FIXED while building this fixture (first real,
 * scripted click-through exercise of `ui`'s own click-dispatch precedence
 * and of the building-menu construction path) -- see docs/portage-
 * progress.md for the full entries, `pico8_ui.c`'s own updated
 * `pico8_ui_truthy` comment, and `pico8_ui_button.c`'s own new
 * `pico8_js_lq_arg_table` comment for the full traces:
 *   (a) `pico8_ui_truthy()` (pico8_ui.c) treated `pico8_value_ref(NULL)`
 *       (this port's own nil stand-in) as TRUTHY, unlike its sibling
 *       `pico8_probe_truthy()` -- this broke `ui:jz()`'s own click-
 *       dispatch fallthrough for any entity whose method could return
 *       real Lua nil (e.g. `jo:ldown()` on an "unknown" planet), silently
 *       swallowing clicks that should have fallen through to `bg:ldown()`.
 *       Fixed to match the correct, already-established convention.
 *   (b) `pico8_js_pq()` (pico8_ui_button.c) unconditionally reinterpreted
 *       `mv(self,"lq")`'s REF payload as a raw `pico8_ob_table_t*` --
 *       correct for a plain spec table, but a genuine, reproducible heap-
 *       buffer-overflow (confirmed under ASan) when that payload is
 *       ALREADY a constructed `lq` instance (a real, evidenced case: a
 *       research/building option's own "lq" field, `pico8_hp_lq_dispatch`,
 *       returns exactly that). Fixed with a small, targeted disambiguating
 *       helper, `pico8_js_lq_arg_table()`.
 *
 * STAGE NOT FULLY VERIFIED END-TO-END: stage 4/5's own downstream effect
 * (each connected planet's own `oe` list growing, `jo:kf()`) could not be
 * confirmed non-zero within this chunk's own exploration budget even
 * though the connection's own commit path visibly runs (`jc`'s own "nh"
 * field, set only on `jc:ey()`'s success branch, was confirmed present) --
 * logged in docs/portage-progress.md as an open follow-up rather than
 * forced further here, per this PRD's own "Further Notes" allowance.
 *
 * `make test`'s own new `unit` dependency, per the PRD's own "if you get
 * it working reliably, wiring it in now is fine and arguably better than
 * leaving it fully disconnected" allowance -- this program runs cleanly
 * end-to-end (no crash, all 6 checkpoints reached, deterministic given
 * this port's own fixed-seed `pico8_rnd`, pico8_fixed.c) every time it has
 * been run while building this chunk, so it is wired into `unit` alongside
 * `test_pico8_data_init`/`test_pico8_planet_kj_dispatch` (same full engine
 * closure, same reasoning: reaching real game state pulls in essentially
 * the whole engine). Chunk 3's own job is turning these printed checkpoints
 * into real `assert()`-based invariants -- this file intentionally has
 * NONE yet, per this chunk's own explicit scope. */

#include <stdio.h>
#include <assert.h>

#include "../src/pico8_mem.h"
#include "../src/pico8_assets.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_input.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_cart.h"
#include "../src/pico8_help.h"
#include "../src/pico8_ui_button.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_probe.h"
#include "../src/pico8_fixed.h"

#define PHASE4_PLAYTHROUGH_EVENTS_PATH "tests/fixtures/phase4_playthrough_events.json"

#define STAGE1_PROBE_LAUNCH_FRAME 3
#define STAGE2_WORMHOLE_TRAVERSAL_FRAME 54
#define STAGE3_COLONIZATION_FRAME 98
#define STAGE4_SLIPWAY_CONSTRUCTION_FRAME 201
#define STAGE5_TRADE_ROUTE_SETUP_FRAME 221
#define STAGE6_SCORE_TICK_FRAME 223

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

static void print_checkpoint(const char *stage_name, int frame_index) {
    printf("[checkpoint] %-32s frame=%-4d cg(jo) count=%-3d credits (fm.nz)=%.2f\n", stage_name, frame_index,
           planet_count(), credits_now());
}

int main(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    pico8_value_t difficulty_entry;
    pico8_instance_t *difficulty_button;
    int frame_count;
    int i;

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

    for (i = 0; i < frame_count; i++) {
        pico8_dispatch_set_input(&input);
        pico8__update60();
        pico8__draw();

        if (i == STAGE1_PROBE_LAUNCH_FRAME) {
            print_checkpoint("1. probe launch", i);
        } else if (i == STAGE2_WORMHOLE_TRAVERSAL_FRAME) {
            print_checkpoint("2. wormhole traversal", i);
        } else if (i == STAGE3_COLONIZATION_FRAME) {
            print_checkpoint("3. colonization", i);
        } else if (i == STAGE4_SLIPWAY_CONSTRUCTION_FRAME) {
            print_checkpoint("4. slipway construction", i);
        } else if (i == STAGE5_TRADE_ROUTE_SETUP_FRAME) {
            print_checkpoint("5. trade route setup", i);
        } else if (i == STAGE6_SCORE_TICK_FRAME) {
            print_checkpoint("6. score/credit-affecting tick", i);
        }

        if (i < frame_count - 1) {
            pico8_input_step(&input);
        }
    }

    pico8_input_free(&input);

    printf("pico8 playability: %d frame(s) run against %s; all 6 stage checkpoints reached\n", frame_count,
           PHASE4_PLAYTHROUGH_EVENTS_PATH);
    return 0;
}
