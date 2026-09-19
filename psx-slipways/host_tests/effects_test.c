#include "t.h"
#include "../effects.h"
#include "../fixed.h"
#include "../rng.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Brute-force search for a seed whose FIRST rng_next_range(rng, 0, FX_ONE)
 * draw lands under EFFECTS_FLOW_SPAWN_CHANCE (~8% of draws do), used to
 * build a fixture that actually exercises the "the RNG draw succeeds" side
 * of the active-gate tests below, rather than relying on a hand-picked
 * magic seed. The chance is ~8% per seed, so this terminates quickly. */
static uint32_t find_seed_with_successful_draw(void) {
    for (uint32_t seed = 1; seed < 1000000u; seed++) {
        rng_t rng;
        rng_seed(&rng, seed);
        fixed_t draw = (fixed_t)rng_next_range(&rng, 0u, (uint32_t)FX_ONE);
        if (draw < EFFECTS_FLOW_SPAWN_CHANCE) {
            return seed;
        }
    }
    return 0;
}

/* Proves effects_flow_try_spawn mirrors Lua's left-to-right "and"
 * short-circuit in "rnd()<0.08 and self.hg[t.id]" (jc:ch,
 * pico8/lua.lua:657): the RNG draw always happens, even when active=0. A
 * seed whose draw is known (via find_seed_with_successful_draw) to succeed
 * when active=1 must still report no spawn when active=0 is passed with
 * that SAME seed -- if the implementation instead skipped the draw
 * whenever active were checked first, this distinction wouldn't be
 * provable; an unlucky-draw seed could pass this test by accident. */
static void test_active_zero_never_spawns_even_when_draw_would_succeed(void) {
    uint32_t seed = find_seed_with_successful_draw();
    T_CHECK(seed != 0u);

    rng_t rng_active;
    rng_seed(&rng_active, seed);
    effects_flow_particle_t p_active;
    int spawned_active = effects_flow_try_spawn(0, 0, FX_ONE, FX_ONE, 1, &rng_active, &p_active);
    T_CHECK(spawned_active == 1);

    rng_t rng_inactive;
    rng_seed(&rng_inactive, seed);
    effects_flow_particle_t p_inactive;
    int spawned_inactive = effects_flow_try_spawn(0, 0, FX_ONE, FX_ONE, 0, &rng_inactive, &p_inactive);
    T_CHECK(spawned_inactive == 0);
}

/* Mirrors planetgen_test.c's test_same_seed_is_deterministic pattern: two
 * independent rng_t instances seeded identically must produce an identical
 * spawn/no-spawn sequence and identical particle fields whenever a spawn
 * does occur, across many calls. */
static void test_same_seed_is_deterministic(void) {
    rng_t rng_a, rng_b;
    rng_seed(&rng_a, 12345u);
    rng_seed(&rng_b, 12345u);

    for (int i = 0; i < 200; i++) {
        effects_flow_particle_t pa, pb;
        int result_a = effects_flow_try_spawn(0, 0, 10 * FX_ONE, 20 * FX_ONE, 1, &rng_a, &pa);
        int result_b = effects_flow_try_spawn(0, 0, 10 * FX_ONE, 20 * FX_ONE, 1, &rng_b, &pb);

        T_CHECK(result_a == result_b);
        if (result_a && result_b) {
            T_CHECK(pa.x == pb.x);
            T_CHECK(pa.y == pb.y);
            T_CHECK(pa.vx == pb.vx);
            T_CHECK(pa.vy == pb.vy);
            T_CHECK(pa.lifetime == pb.lifetime);
        }
    }
}

/* v=d:nn() (km:nn(), pico8/lua.lua:113-114) is a normalize -- the
 * particle's velocity must have magnitude 1 (one screen-space unit per
 * tick). Checked as vx^2+vy^2 close to FX_ONE, within a small fixed-point
 * tolerance (FX_ONE/1000, i.e. within 0.001 of exactly 1) to allow for the
 * integer division/sqrt truncation inherent to fixed-point arithmetic --
 * mirrors screen_map_test.c's tolerance-bounded round-trip style. */
static void test_spawned_velocity_is_unit_length(void) {
    uint32_t seed = find_seed_with_successful_draw();
    rng_t rng;
    rng_seed(&rng, seed);

    effects_flow_particle_t p;
    int spawned = effects_flow_try_spawn(0, 0, 3 * FX_ONE, 4 * FX_ONE, 1, &rng, &p);
    T_CHECK(spawned == 1);

    fixed_t mag_sq = fixed_add(fixed_mul(p.vx, p.vx), fixed_mul(p.vy, p.vy));
    fixed_t tolerance = FX_ONE / 1000;
    fixed_t diff = mag_sq > FX_ONE ? mag_sq - FX_ONE : FX_ONE - mag_sq;
    T_CHECK(diff < tolerance);
}

/* Spawn position = from + v:pa() (km:pa(), pico8/lua.lua:115-116: the
 * 90-degree rotation (x,y)->(-y,x) applied to the unit velocity), i.e.
 * out->x == from_x + (-vy), out->y == from_y + vx. Tolerance is a handful
 * of raw fixed_t units (well under 0.0001 in real terms) to absorb the
 * fixed-point division/sqrt rounding in effects_length/effects_fixed_div,
 * not a loose approximation of the formula itself. */
static void test_spawn_position_is_perpendicular_offset_from_source(void) {
    uint32_t seed = find_seed_with_successful_draw();
    rng_t rng;
    rng_seed(&rng, seed);

    fixed_t from_x = 0, from_y = 0;
    effects_flow_particle_t p;
    int spawned = effects_flow_try_spawn(from_x, from_y, 5 * FX_ONE, 12 * FX_ONE, 1, &rng, &p);
    T_CHECK(spawned == 1);

    fixed_t tolerance = 4;
    fixed_t expected_x = fixed_add(from_x, -p.vy);
    fixed_t expected_y = fixed_add(from_y, p.vx);
    fixed_t diff_x = p.x > expected_x ? p.x - expected_x : expected_x - p.x;
    fixed_t diff_y = p.y > expected_y ? p.y - expected_y : expected_y - p.y;

    T_CHECK(diff_x <= tolerance);
    T_CHECK(diff_y <= tolerance);
}

/* Degenerate zero-length segment (from == to): source never exercises this
 * (see effects.h's doc comment), but this defensive case must never spawn
 * regardless of active/RNG state -- checked across many calls, both
 * active=1 (with a seed known to draw at least one success elsewhere) and
 * active=0, at a fixed nonzero coordinate and at the origin. */
static void test_from_equals_to_never_spawns(void) {
    rng_t rng;
    rng_seed(&rng, 1u);

    for (int i = 0; i < 500; i++) {
        effects_flow_particle_t p;
        int spawned = effects_flow_try_spawn(7 * FX_ONE, -3 * FX_ONE, 7 * FX_ONE, -3 * FX_ONE, 1, &rng, &p);
        T_CHECK(spawned == 0);
    }

    rng_t rng2;
    rng_seed(&rng2, 42u);
    for (int i = 0; i < 500; i++) {
        effects_flow_particle_t p;
        int spawned = effects_flow_try_spawn(0, 0, 0, 0, 0, &rng2, &p);
        T_CHECK(spawned == 0);
    }
}

/* t.ra+=t.v (pico8/lua.lua:674): position advances by exactly the stored
 * velocity per call. Uses a real spawned particle (non-trivial vx/vy, not
 * (0,0) or an axis-aligned trivial vector) so the assertion actually
 * exercises both components. */
static void test_step_advances_position_by_velocity(void) {
    uint32_t seed = find_seed_with_successful_draw();
    rng_t rng;
    rng_seed(&rng, seed);

    effects_flow_particle_t p;
    int spawned = effects_flow_try_spawn(0, 0, 3 * FX_ONE, 4 * FX_ONE, 1, &rng, &p);
    T_CHECK(spawned == 1);

    fixed_t x0 = p.x, y0 = p.y, vx = p.vx, vy = p.vy;

    int alive = effects_flow_step_particle(&p);
    (void)alive;

    T_CHECK(p.x == fixed_add(x0, vx));
    T_CHECK(p.y == fixed_add(y0, vy));
}

/* t.l-=1 (pico8/lua.lua:675), ported as lifetime -= FX_ONE (not a plain
 * integer 1) -- see effects.h's doc comment. Checked as EXACT fixed-point
 * subtraction, not an approximate/tolerance-bounded check. */
static void test_step_decrements_lifetime_by_exactly_fx_one(void) {
    effects_flow_particle_t p = {0};
    p.x = 10 * FX_ONE;
    p.y = 20 * FX_ONE;
    p.vx = FX_ONE / 2;
    p.vy = -FX_ONE / 3;
    p.lifetime = 5 * FX_ONE + (FX_ONE / 4);

    fixed_t lifetime0 = p.lifetime;
    effects_flow_step_particle(&p);

    T_CHECK(p.lifetime == fixed_sub(lifetime0, FX_ONE));
}

/* A particle survives every call while lifetime > 0, and reports dead on
 * the first call that brings it to <= 0 -- verified at the exact tick
 * count, not just "eventually dies". lifetime = 3*FX_ONE: call 1 leaves
 * 2*FX_ONE (alive), call 2 leaves FX_ONE (alive), call 3 leaves 0 (dead,
 * matching source's t.l<=0 test). */
static void test_particle_survives_until_lifetime_crosses_zero(void) {
    effects_flow_particle_t p = {0};
    p.x = 0;
    p.y = 0;
    p.vx = FX_ONE;
    p.vy = 0;
    p.lifetime = 3 * FX_ONE;

    int alive1 = effects_flow_step_particle(&p);
    T_CHECK(alive1 == 1);
    T_CHECK(p.lifetime == 2 * FX_ONE);

    int alive2 = effects_flow_step_particle(&p);
    T_CHECK(alive2 == 1);
    T_CHECK(p.lifetime == FX_ONE);

    int alive3 = effects_flow_step_particle(&p);
    T_CHECK(alive3 == 0);
    T_CHECK(p.lifetime == 0);
}

/* Defensive edge case (not explicitly required by the chunk spec, but
 * worth confirming): a particle whose lifetime is already <= 0 BEFORE any
 * call still reports dead (a sane, non-crashing result) the very first
 * time it's stepped, rather than e.g. reporting alive because no
 * "crossing" happened during this particular call. */
static void test_already_dead_particle_reports_dead_on_first_step(void) {
    effects_flow_particle_t p = {0};
    p.x = 0;
    p.y = 0;
    p.vx = 0;
    p.vy = 0;
    p.lifetime = 0;

    int alive = effects_flow_step_particle(&p);
    T_CHECK(alive == 0);
    T_CHECK(p.lifetime == -FX_ONE);

    effects_flow_particle_t p2 = {0};
    p2.lifetime = -5 * FX_ONE;
    int alive2 = effects_flow_step_particle(&p2);
    T_CHECK(alive2 == 0);
}

/* Chunk 4: full spawn-to-death lifecycle test -- the end-to-end integration
 * proof that effects_flow_try_spawn's spawn geometry and
 * effects_flow_step_particle's iterative stepping are mutually consistent:
 * a particle spawned by the first function, then repeatedly stepped by the
 * second until it reports dead, must travel the correct distance in the
 * correct direction and land near `to` -- NOT exactly at `to` (see
 * effects.h's doc comment on effects_flow_try_spawn and the PRD's
 * "Velocity and lifetime" trace: the real endpoint is `to +
 * perp(unit(to-from))`, one perpendicular unit off from `to` itself, not
 * exact arrival). This does NOT re-test what
 * test_spawned_velocity_is_unit_length /
 * test_spawn_position_is_perpendicular_offset_from_source (chunk 2) or
 * test_step_advances_position_by_velocity /
 * test_particle_survives_until_lifetime_crosses_zero (chunk 3) already
 * independently prove in isolation -- it proves those two formulas compose
 * correctly when chained spawn-then-repeated-step, which none of the
 * earlier tests (each of which only calls one of the two functions, or
 * steps a hand-built fixture rather than a real spawned particle) exercise.
 *
 * Segment: from=(2,3), to=(11,15) (screen-space FX_ONE units) -- a
 * 9-12-15-scaled 3-4-5 right triangle (dx=9, dy=12), distinct from the
 * (3,4,5) and (5,12,13) pairs chunk 2's own tests already use. Chosen so
 * dx^2+dy^2=225 is a PERFECT SQUARE: effects_length's integer sqrt returns
 * exactly 15*FX_ONE with zero rounding error, so the tick-count
 * hand-derivation below is exact arithmetic, not an approximation.
 *
 * Hand-derivation (worked out before writing the assertions):
 *
 *   lifetime_initial = effects_length(dx,dy) = 15*FX_ONE exactly. dx =
 *   9*FX_ONE, dy = 12*FX_ONE (both exact, since from/to are chosen as
 *   exact-integer FX_ONE multiples), so effects_length's raw squared sum
 *   is dx^2+dy^2 = FX_ONE^2*(81+144) = FX_ONE^2*225, and the bit-by-bit
 *   integer sqrt of a perfect square is exact: sqrt = FX_ONE*15.
 *
 *   Each effects_flow_step_particle call subtracts exactly FX_ONE from
 *   lifetime (fixed_sub is exact integer subtraction, and lifetime stays
 *   an exact multiple of FX_ONE throughout: 15,14,...,1,0 -- no
 *   fractional remainder ever appears). A call reports alive (1) while
 *   the POST-decrement lifetime is >0, and dead (0) the first time it is
 *   <=0 (source's own `t.l<=0` test). So calls 1..14 report alive
 *   (lifetime 14*FX_ONE down to 1*FX_ONE), and call 15 reports dead
 *   (lifetime exactly 0*FX_ONE) -- ticks_to_death = 15 =
 *   ceil(lifetime_initial/FX_ONE), with no off-by-one ambiguity here
 *   since 15*FX_ONE is an exact multiple of FX_ONE (nothing to round up
 *   from).
 *
 *   Final position: spawn = (from_x - vy, from_y + vx) (km:pa()'s
 *   perpendicular offset, effects_flow_try_spawn's own formula); after
 *   N=15 steps, final = spawn + N*(vx,vy). Expected real-valued endpoint
 *   = (to_x - vy, to_y + vx) (the SAME perpendicular formula applied to
 *   `to` instead of `from`, per the PRD's corrected "one perpendicular
 *   unit off from `to`, not exact arrival" wording) -- computed here from
 *   the particle's own p.vx/p.vy fields, exactly like chunk 2's
 *   test_spawn_position_is_perpendicular_offset_from_source already does,
 *   not a separately-rederived continuous value. Algebraically:
 *   diff_x = final_x - expected_x
 *          = (from_x - vy + N*vx) - (to_x - vy) = N*vx - dx
 *   diff_y = final_y - expected_y
 *          = (from_y + vx + N*vy) - (to_y + vx) = N*vy - dy
 *   -- the perpendicular-offset term cancels exactly between spawn and
 *   expected, leaving only the accumulated per-tick unit-vector
 *   truncation error: effects_fixed_div (effects.c) truncates towards
 *   zero, so each of vx/vy is within 1 raw fixed_t unit of its ideal real
 *   value, and that loss accumulates additively over N ticks (each step
 *   adds the SAME already-truncated vx/vy again). Worst-case bound:
 *   |diff| < N raw units = 15 raw units for this segment.
 *
 *   Tolerance: 20 raw fixed_t units (~0.0003 in real terms, i.e.
 *   20/FX_ONE). Comfortably above the 15-raw-unit worst-case bound just
 *   derived, with a few units of margin for a future reader adapting this
 *   segment (this bound scales with ticks_to_death, so a much larger
 *   segment would need a larger tolerance too) -- small enough to still
 *   be a meaningful geometric check (FX_ONE is 65536, so 20 is a tiny
 *   fraction of it), generous enough to absorb the fixed-point
 *   division/sqrt truncation inherent to this arithmetic, matching
 *   chunk 1's fixed_sqrt irrational-value tolerance and
 *   screen_map_test.c's round-trip tolerance-bounded style. */
static void test_full_spawn_to_death_lifecycle(void) {
    uint32_t seed = find_seed_with_successful_draw();
    rng_t rng;
    rng_seed(&rng, seed);

    fixed_t from_x = 2 * FX_ONE, from_y = 3 * FX_ONE;
    fixed_t to_x = 11 * FX_ONE, to_y = 15 * FX_ONE;

    effects_flow_particle_t p;
    int spawned = effects_flow_try_spawn(from_x, from_y, to_x, to_y, 1, &rng, &p);
    T_CHECK(spawned == 1);
    T_CHECK(p.lifetime == 15 * FX_ONE);

    /* Expected real-valued endpoint, from the SAME perpendicular formula
     * effects_flow_try_spawn itself uses, applied to `to` instead of
     * `from`, using the particle's own (already spawn-truncated) vx/vy --
     * see the derivation above for why this is the right comparison. */
    fixed_t expected_x = fixed_add(to_x, -p.vy);
    fixed_t expected_y = fixed_add(to_y, p.vx);

    int alive = 1;
    int ticks = 0;
    int max_ticks = 1000; /* safety bound against a runaway loop; the hand
                            * derivation above expects exactly 15 */
    while (alive && ticks < max_ticks) {
        alive = effects_flow_step_particle(&p);
        ticks++;
    }

    T_CHECK(ticks == 15);
    T_CHECK(p.lifetime == 0);

    fixed_t tolerance = 20;
    fixed_t diff_x = p.x > expected_x ? p.x - expected_x : expected_x - p.x;
    fixed_t diff_y = p.y > expected_y ? p.y - expected_y : expected_y - p.y;

    T_CHECK(diff_x <= tolerance);
    T_CHECK(diff_y <= tolerance);
}

/* Chunk 5 (final): the Monte-Carlo spawn-rate regression this PRD's own
 * Implementation Chunks item 5 / Testing Decisions call for -- "many
 * thousands of effects_flow_try_spawn calls with active=1 from a fixed
 * seed, asserting the empirical spawn rate falls within an
 * implementation-chosen tolerance band around 0.08" -- explicitly mirroring
 * docs/prd/psx-rules-discovery.md's own empty-roll Monte-Carlo regression
 * (rules_discovery_test.c's
 * test_empty_rate_matches_045_within_tolerance_over_many_rolls), itself
 * mirroring docs/plan.md Phase 1 step 2's BFS-connectivity-rate precedent.
 * Same posture: ONE rng_t (seeded once), threaded across every call, not
 * re-seeded per call -- matching how a real per-frame sequence of calls
 * would consume the RNG. A fixed, non-degenerate from/to pair is used --
 * the specific geometry doesn't matter for the spawn-rate statistic, since
 * effects_flow_try_spawn always draws its RNG value first, unconditionally,
 * before the geometry is ever touched (see effects.h's/effects.c's "always
 * consume a draw" trace) -- but a real segment, not a from==to case that
 * would never spawn regardless of the draw, is used for realism.
 *
 * UNLIKE rules_discovery's empty-roll (jf(0.8,0.5), a mean-reverting random
 * WALK whose stationary rate is not analytically obvious and measured
 * ~0.436 against a nominal 0.45 -- a real, expected ~0.014 gap, not a bug),
 * effects_flow_try_spawn's spawn trial is a plain i.i.d. Bernoulli(0.08)
 * draw every call: `rng_next_range(rng, 0, FX_ONE) < EFFECTS_FLOW_SPAWN_CHANCE`,
 * with no persistent bias/state carried between calls (effects_flow_try_spawn
 * takes no state struct, unlike rules_discovery_empty_roll_t -- see
 * effects.c). So this test's empirical rate is expected to converge MUCH
 * closer to exactly 0.08 than rules_discovery's own ~0.436-vs-0.45 gap --
 * confirmed by exploration below, not just assumed from the PRD's reasoning.
 *
 * Choosing N and the tolerance band (empirical, per this PRD -- exploration
 * below, not part of the final test itself):
 *   - A standalone exploration harness (same shape as this test: one rng_t
 *     per seed, threaded across N calls to effects_flow_try_spawn with
 *     active=1 and this same fixed non-degenerate from/to pair) was run
 *     across 30 distinct seeds at N=50,000/100,000/200,000. At N=50,000:
 *     mean rate 0.07996 (within 0.00004 of the nominal 0.08 -- essentially
 *     exact), min 0.07772, max 0.08338 (a 0.00566 seed-to-seed spread). At
 *     N=100,000: mean 0.07987, min 0.07882, max 0.08233 (spread tightens to
 *     0.00351 -- variance shrinking with N, exactly as expected for i.i.d.
 *     Bernoulli draws, unlike a mean-reverting walk's stationary spread,
 *     which would not shrink the same way). This confirms the "i.i.d., not
 *     a walk" analysis above empirically, not just theoretically: the
 *     empirical rate lands within a few thousandths of 0.08 at every N
 *     tried, nowhere near rules_discovery's own ~0.014 gap, and nowhere
 *     near the "below 0.05 or above 0.12" danger zone that would flag a
 *     real chunk 1-4 spawn-trial bug rather than ordinary sampling noise.
 *   - N=100,000 was chosen over N=50,000 for its tighter seed-to-seed spread
 *     (0.00351 vs 0.00566), at negligible extra cost -- the whole
 *     exploration sweep above (30 seeds x 3 N values, plus a threshold
 *     sensitivity check) runs in well under a second on the host.
 *   - A sensitivity check (deliberately comparing the RNG draw against
 *     mis-set thresholds in place of the real EFFECTS_FLOW_SPAWN_CHANCE, at
 *     N=50,000, single seed) confirmed the empirical rate tracks a wrong
 *     threshold almost exactly 1:1 (0.07 -> ~0.0697, 0.075 -> ~0.0746,
 *     0.09 -> ~0.0898, 0.12 -> ~0.1187) -- unlike rules_discovery's walk,
 *     where a threshold change is damped by mean-reversion, this i.i.d.
 *     generator is, if anything, an EASIER target for a tolerance band to
 *     usefully catch a regression on, not a harder one.
 *   - Tolerance: +/-0.01 around 0.08 (i.e. [0.07, 0.09]). The observed
 *     seed-to-seed spread at N=100,000 (max single-sided deviation ~0.00233
 *     from 0.08) sits comfortably inside this band -- margin to the nearest
 *     edge (~0.01) is roughly 4.3x that maximum observed deviation,
 *     matching rules_discovery's own "3-7x the observed spread" margin
 *     choice -- while still tight enough to catch a threshold-constant bug
 *     of a magnitude the sensitivity check above shows would be readily
 *     visible (e.g. an accidental 0.07 threshold, empirical rate ~0.0697,
 *     already falls outside [0.07, 0.09] on its own at this precision).
 *     This test itself uses ONE fixed seed (987654321u, confirmed during
 *     exploration to land at 0.07833 -- comfortably inside the band, not a
 *     cherry-picked edge case), so it is fully deterministic and non-flaky
 *     by construction; the tolerance band exists to bound the GENERATOR's
 *     true stochastic behavior, not to average out test-to-test variance. */
static void test_spawn_rate_matches_008_within_tolerance_over_many_calls(void) {
    const long NUM_CALLS = 100000;
    const double EXPECTED_RATE = 0.08;
    const double TOLERANCE = 0.01;

    rng_t rng;
    rng_seed(&rng, 987654321u);

    fixed_t from_x = 2 * FX_ONE, from_y = 3 * FX_ONE;
    fixed_t to_x = 11 * FX_ONE, to_y = 15 * FX_ONE;

    long spawn_count = 0;
    for (long i = 0; i < NUM_CALLS; i++) {
        effects_flow_particle_t p;
        if (effects_flow_try_spawn(from_x, from_y, to_x, to_y, 1, &rng, &p)) {
            spawn_count++;
        }
    }

    double empirical_rate = (double)spawn_count / (double)NUM_CALLS;
    printf("     %ld/%ld calls spawned (%.5f), expected ~%.2f +/- %.2f\n", spawn_count, NUM_CALLS,
           empirical_rate, EXPECTED_RATE, TOLERANCE);

    T_CHECK(empirical_rate > EXPECTED_RATE - TOLERANCE);
    T_CHECK(empirical_rate < EXPECTED_RATE + TOLERANCE);
}

/* Chunk 3: effects_kick_start sets the exact literal starting amplitude
 * jo:kd() ports (pico8/lua.lua:469: self.ma=1.25). 1.25*FX_ONE is an exact
 * double-to-fixed_t conversion (1.25*65536==81920.0, no fractional part),
 * so this is checked as EXACT equality, not a tolerance-bounded check. */
static void test_kick_start_sets_amplitude_to_125_fx_one(void) {
    effects_kick_t k;
    effects_kick_start(&k);

    T_CHECK(k.amplitude == (fixed_t)(1.25 * (double)FX_ONE));
}

/* Chunk 3: effects_kick_step decays amplitude by exactly 0.1*FX_ONE per
 * call (ma(p,e)'s own e.ma=max(0,e.ma-0.1), pico8/lua.lua:349), checked
 * from a non-starting amplitude so this doesn't accidentally also depend on
 * effects_kick_start's own value being correct. */
static void test_kick_step_decays_amplitude_by_exact_amount(void) {
    effects_kick_t k;
    k.amplitude = 50000;

    effects_kick_step(&k);

    T_CHECK(k.amplitude == 50000 - (fixed_t)(0.1 * (double)FX_ONE));
}

/* Chunk 3: the traced 13-step lifecycle (docs/prd/triage/psx-juice-effects.md,
 * "### 2. Planet "kick" wobble" -- "1.25 - 13*0.1 = -0.05, floored ... the
 * 12th call leaves 0.05, still nonzero"). Hand-traced here in fixed_t terms
 * (not just restating the PRD's real-number arithmetic), starting from
 * EFFECTS_KICK_START_AMPLITUDE == 81920 and
 * EFFECTS_KICK_DECAY_PER_STEP == (fixed_t)(0.1*FX_ONE) == 6553 (0.1*65536
 * == 6553.6, truncated): 81920 - 12*6553 == 3284 (still > 0) after the
 * 12th step, and 3284 - 6553 == -3269, floored to exactly 0, after the
 * 13th. Also confirms a 14th step leaves it at 0 (floored, not negative)
 * -- the amplitude never goes negative once it bottoms out. */
static void test_kick_amplitude_reaches_zero_after_exactly_13_steps(void) {
    effects_kick_t k;
    effects_kick_start(&k);

    for (int step = 1; step <= 12; step++) {
        effects_kick_step(&k);
        T_CHECK(k.amplitude > 0);
    }

    T_CHECK(k.amplitude == 3284);

    effects_kick_step(&k);
    T_CHECK(k.amplitude == 0);

    /* Floored, stays at 0 -- does not go negative on further steps. */
    effects_kick_step(&k);
    T_CHECK(k.amplitude == 0);
}

/* Chunk 3: effects_kick_offset is a thin wrapper around fixed_polar --
 * ma(p,e)'s own `qm(e.ma, e.t*0.2)` (pico8/lua.lua:350). Confirms, at a
 * handful of hand-picked (amplitude, age_ticks) pairs, that
 * effects_kick_offset(k, age_ticks, ...) produces EXACTLY the same output
 * as calling fixed_polar(amplitude, age_ticks*0.2_in_Q16.16, ...) directly
 * -- exact equality is possible (no tolerance needed) since both paths run
 * the identical fixed_polar computation on the identical resolved
 * arguments. The angular-rate constant (0.2 turns/tick) is recomputed here
 * independently of effects.c's own EFFECTS_KICK_ANGULAR_TURNS_PER_TICK
 * (which is file-local, not visible from this test anyway) -- both
 * expressions are the same literal double-to-fixed_t translation of
 * source's own `0.2` constant, deterministic and lossless-of-difference
 * across translation units, so this is a real independent check, not a
 * tautology.
 *
 * Pairs are chosen so magnitude and turns are clearly distinguishable from
 * each other (never equal, never simply related) -- if effects_kick_offset
 * ever swapped fixed_polar's argument order (magnitude for turns or vice
 * versa), at least one of these pairs would produce a visibly different
 * result than the correctly-ordered expected computation. */
static void test_kick_offset_matches_fixed_polar_directly(void) {
    fixed_t angular_rate = (fixed_t)(0.2 * (double)FX_ONE); /* turns/tick */

    struct {
        fixed_t amplitude;
        fixed_t age_ticks;
    } cases[] = {
        {0, 7 * FX_ONE},
        {(fixed_t)(1.25 * (double)FX_ONE), 0},
        {5 * FX_ONE, FX_ONE},
        {50000, 3 * FX_ONE + FX_ONE / 4},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        effects_kick_t k;
        k.amplitude = cases[i].amplitude;

        fixed_t actual_dx, actual_dy;
        effects_kick_offset(&k, cases[i].age_ticks, &actual_dx, &actual_dy);

        fixed_t expected_dx, expected_dy;
        fixed_t turns = fixed_mul(cases[i].age_ticks, angular_rate);
        fixed_polar(cases[i].amplitude, turns, &expected_dx, &expected_dy);

        T_CHECK(actual_dx == expected_dx);
        T_CHECK(actual_dy == expected_dy);
    }
}

/* Chunk 4: effects_idle_bob_delta(0) == 0 -- turns = fixed_mul(0, rate) is
 * exactly 0 regardless of the turn-rate constant's own truncation (anything
 * times 0 is 0), and fixed_sin(0) is an exact table-boundary sample
 * (already confirmed exact in rng_and_fixed_test.c's
 * test_fixed_sin_cos_known_turns), so this is checked as EXACT equality,
 * not tolerance-bounded, unlike the nonzero-t cases below. */
static void test_idle_bob_delta_zero_ticks_is_zero(void) {
    T_CHECK(effects_idle_bob_delta(0) == 0);
}

/* Chunk 4: effects_idle_bob_delta matches na()'s own `sin(t*0.005)*0.06`
 * (pico8/lua.lua:578) at several hand-picked age_ticks values, including
 * t=137 (not a "nice" round number, and not any exact fixed_sin table
 * boundary either -- table samples land every 1/1024 turn, 137*0.005=0.685
 * turns is not a multiple of that), so this genuinely exercises the
 * interpolation path, not just table-boundary cases -- matching this
 * chunk's own testing mandate (docs/prd/triage/psx-juice-effects.md,
 * Implementation Chunks item 4) directly.
 *
 * Expected values are derived independently here via libm at test-authorship
 * time (`-sin(2*pi*(t*0.005))*0.06`, PICO-8's own sign-flipped-sine
 * convention -- see effects.h's doc comment on effects_idle_bob_delta and
 * fixed.h's fixed_sin block comment), not copied from the PRD text (which
 * states the formula but never this function's numeric values at these
 * particular t's) and not recomputed through this port's own truncated
 * EFFECTS_IDLE_BOB_ANGULAR_TURNS_PER_TICK/EFFECTS_IDLE_BOB_AMPLITUDE
 * constants (which would make this test tautological against a wrong
 * constant value).
 *
 * Tolerance: this function chains an EXTRA truncated constant
 * (EFFECTS_IDLE_BOB_ANGULAR_TURNS_PER_TICK, 327 vs the real 327.68) ahead
 * of fixed_sin, unlike rng_and_fixed_test.c's own
 * test_fixed_sin_cos_irrational_spot_check (which feeds fixed_sin a turns
 * value built directly via fixed_from_double, with no separate per-tick
 * rate constant in between) -- so this needs a wider budget than that
 * test's own EPSILON (0.001), not the same one reused unchanged. The rate
 * constant's own truncation error (0.005 - 327/65536 =~ 0.0000103760
 * turns/tick) accumulates linearly with t: at t=283 (the largest value
 * tested here), that's ~0.0029364 turns of phase drift, i.e. ~0.01845
 * radians, and since d(sin)/d(angle) <= 1, at most ~0.01845 of "raw sine"
 * error before scaling by this function's own 0.06 amplitude -- ~0.0011.
 * Adding a small margin for the table's own linear-interpolation error and
 * the amplitude/fixed_mul truncations (each independently already shown
 * negligible, well under 0.0001, by rng_and_fixed_test.c's own EPSILON
 * budget scaled down by this function's 0.06 amplitude), 0.002 is
 * comfortably above the ~0.0011 worst case among the t values tested here
 * while still tight enough to catch a genuinely wrong rate/amplitude
 * constant or a sign error. */
static void test_idle_bob_delta_matches_formula_at_several_t_values(void) {
    double IDLE_BOB_TOLERANCE = 0.002;

    long t_values[] = {50, 137, 283, -64};

    for (size_t i = 0; i < sizeof(t_values) / sizeof(t_values[0]); i++) {
        double t = (double)t_values[i];
        double expected = -sin(2.0 * M_PI * (t * 0.005)) * 0.06;

        fixed_t age_ticks = fixed_from_double(t);
        double actual = fixed_to_double(effects_idle_bob_delta(age_ticks));

        T_CHECK(fabs(actual - expected) < IDLE_BOB_TOLERANCE);
    }
}

/* Chunk 5: the sequence of values returned over 25 calls, starting from a
 * fresh reset (nh=1, matching jc:ey's own self.nh=1), climbs 1,2,...,19,20
 * then holds at 20 forever -- source's own read-before-increment,
 * capped-at-20 counter (pico8/lua.lua:669-670). Hand-traced here, not just
 * restated from the PRD: call N returns the pre-increment value of nh, which
 * starts at 1 and increments by exactly 1 per call while nh<20, so call N
 * returns N for N in [1,20] (20 calls climb 1..20, incrementing after each
 * of the first 19 since only those satisfy nh<20 at read time -- the 20th
 * call reads nh==20 and, since 20<20 is false, does NOT increment further);
 * every call after that (calls 21-25) reads nh==20 again and again returns
 * 20, never incrementing past the cap. */
static void test_reveal_ramp_step_sequence_climbs_then_holds_at_20(void) {
    int nh = 1;
    int expected[25] = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
                         14, 15, 16, 17, 18, 19, 20, 20, 20, 20, 20, 20};

    for (int call = 0; call < 25; call++) {
        int value = effects_reveal_ramp_step(&nh);
        T_CHECK(value == expected[call]);
    }

    T_CHECK(nh == 20);
}

/* Chunk 5: an in-progress/never-reset counter (nh starting at its `jc`
 * prototype default of 20, pico8/lua.lua:619, i.e. a drag that was never
 * successfully released) always reads the last, fully-settled ramp entry --
 * every call returns 20, with no climb at all, and nh never moves past 20. */
static void test_reveal_ramp_step_default_20_never_climbs(void) {
    int nh = 20;

    for (int call = 0; call < 5; call++) {
        int value = effects_reveal_ramp_step(&nh);
        T_CHECK(value == 20);
        T_CHECK(nh == 20);
    }
}

/* Chunk 5: EFFECTS_SLIPWAY_REVEAL_RAMP's 20 entries match `k_`
 * (pico8/lua.lua:618) exactly, cross-checked directly against the actual
 * source line (re-read at pico8/lua.lua:618 during this chunk's
 * implementation, not just copied from the PRD's own transcription of it):
 * `k_=o(0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1)`. */
static void test_reveal_ramp_table_matches_traced_k_table(void) {
    int expected[EFFECTS_SLIPWAY_REVEAL_RAMP_LENGTH] = {0, 0, 7, 7, 10, 10, 9, 9, 9, 9,
                                                          9, 9, 9, 9, 4,  4,  4, 2, 2, 1};

    for (int i = 0; i < EFFECTS_SLIPWAY_REVEAL_RAMP_LENGTH; i++) {
        T_CHECK(EFFECTS_SLIPWAY_REVEAL_RAMP[i] == expected[i]);
    }
}

/* Chunk 6: effects_probe_pulse_radius matches `mz:kz(t)`'s own
 * `base_radius*(min(t/15,1)+sin(t/40)*0.1)` at the PRD's own specified
 * checkpoints -- `t=0`, `t=15` (the ramp-up caps here), `t=40` (one full
 * breathing period, since PICO-8's turns-based sin() gives sin(t/40) a true
 * period of 40 ticks -- see this PRD's own corrected Implementation
 * Decisions note), `t=80` (two full breathing periods). Expected values are
 * derived independently here via libm at test-authorship time
 * (`base*(min(t/15,1) + (-sin(2*pi*(t/40)))*0.1)`, PICO-8's own
 * sign-flipped-sine convention -- see fixed.h's fixed_sin block comment),
 * not copied from the PRD text (which states the formula but never this
 * function's numeric values at these particular t's) and not recomputed
 * through this port's own truncated EFFECTS_PROBE_PULSE_RAMP_RECIP_15/
 * EFFECTS_PROBE_PULSE_BREATH_RECIP_40/EFFECTS_PROBE_PULSE_BREATH_AMPLITUDE
 * constants (which would make this test tautological against a wrong
 * constant value) -- matching test_idle_bob_delta_matches_formula_at_
 * several_t_values's own established derivation style exactly.
 *
 * base_radius = 29*FX_ONE (the traced "space folding" un-researched literal,
 * mz:kz's own `ov("space folding",29,35)` -- an arbitrary-but-traced choice
 * for this test; this function takes base_radius as a plain parameter, no
 * tech lookup happens here, see effects.h's doc comment).
 *
 * Tolerance: this function chains TWO truncated reciprocal constants
 * (EFFECTS_PROBE_PULSE_RAMP_RECIP_15, EFFECTS_PROBE_PULSE_BREATH_RECIP_40)
 * ahead of fixed_sin, each losing a small fraction of a turn per tick of
 * `t`, so error grows with `t` -- independently confirmed (not just assumed)
 * by computing this port's own actual fixed-point output against the libm
 * ideal at every checkpoint during test authorship: the largest observed gap
 * is ~0.0089 (at t=80, where the breathing term's own phase drift is largest
 * relative to its own ~2.9-unit swing, 0.1*base_radius). 0.015 sits
 * comfortably above that observed worst case while remaining tight enough to
 * catch a genuinely wrong reciprocal/amplitude constant or a sign error --
 * matching test_idle_bob_delta_matches_formula_at_several_t_values's own
 * "margin above an independently-derived worst-case bound" tolerance
 * posture. */
static void test_probe_pulse_radius_matches_formula_at_prd_checkpoints(void) {
    double PULSE_TOLERANCE = 0.015;
    double base = 29.0;

    double t_values[] = {0, 15, 40, 80};

    for (size_t i = 0; i < sizeof(t_values) / sizeof(t_values[0]); i++) {
        double t = t_values[i];
        double ramp = t / 15.0;
        if (ramp > 1.0) {
            ramp = 1.0;
        }
        double breath = -sin(2.0 * M_PI * (t / 40.0)) * 0.1;
        double expected = base * (ramp + breath);

        fixed_t t_fx = fixed_from_double(t);
        double actual = fixed_to_double(effects_probe_pulse_radius(fixed_from_double(base), t_fx));

        T_CHECK(fabs(actual - expected) < PULSE_TOLERANCE);
    }
}

/* Chunk 6: effects_probe_pulse_radius(base, 0) == 0 exactly -- ramp is
 * exactly 0 (fixed_mul(0, anything) is exactly 0), and breath is
 * base*sin(0)*0.1 which is also exactly 0 since fixed_sin(0) is an exact
 * table-boundary sample (already confirmed exact in rng_and_fixed_test.c's
 * test_fixed_sin_cos_known_turns) -- so this is checked as EXACT equality,
 * unlike the tolerance-bounded nonzero-t checkpoints above, matching
 * test_idle_bob_delta_zero_ticks_is_zero's own identical t=0 exactness
 * precedent. */
static void test_probe_pulse_radius_zero_ticks_is_zero(void) {
    T_CHECK(effects_probe_pulse_radius(fixed_from_double(29.0), 0) == 0);
}

/* Chunk 6: effects_probe_sweep_radius matches `mz:ej(t)`'s own
 * `min(sqrt(t/15),1)*frozen_radius` at the PRD's own specified checkpoints --
 * `t=0`, `t=7.5` (a genuinely fractional tick, per `mz:kj`'s own `self:ej
 * (self.t-dt)` re-call pattern -- see effects.h's doc comment on this
 * function pair), `t=15` (the cap engages exactly here), `t=30` (well past
 * the cap, confirming it holds). Expected values derived independently via
 * libm at test-authorship time (`min(sqrt(t/15),1)*frozen`), not copied from
 * the PRD text and not recomputed through this port's own truncated
 * EFFECTS_PROBE_SWEEP_RECIP_15 constant.
 *
 * frozen_radius = 35*FX_ONE (the traced "space folding"-researched literal --
 * again arbitrary-but-traced; this function takes frozen_radius as a plain
 * parameter, see effects.h's doc comment).
 *
 * Tolerance: 0.001, comfortably above the largest observed actual-vs-ideal
 * gap at these checkpoints (~0.00053, at t=7.5 and t=15, confirmed during
 * test authorship the same way as the pulse test above) while tight enough
 * to catch a genuinely wrong reciprocal constant, a missing/wrong clamp, or
 * a sign error -- this function chains only ONE truncated reciprocal
 * constant (EFFECTS_PROBE_SWEEP_RECIP_15) ahead of fixed_sqrt, unlike the
 * pulse function's two, so its error budget is correspondingly tighter. */
static void test_probe_sweep_radius_matches_formula_at_prd_checkpoints(void) {
    double SWEEP_TOLERANCE = 0.001;
    double frozen = 35.0;

    double t_values[] = {0, 7.5, 15, 30};

    for (size_t i = 0; i < sizeof(t_values) / sizeof(t_values[0]); i++) {
        double t = t_values[i];
        double root = sqrt(t / 15.0);
        if (root > 1.0) {
            root = 1.0;
        }
        double expected = root * frozen;

        fixed_t t_fx = fixed_from_double(t);
        double actual = fixed_to_double(effects_probe_sweep_radius(fixed_from_double(frozen), t_fx));

        T_CHECK(fabs(actual - expected) < SWEEP_TOLERANCE);
    }
}

/* Chunk 6: effects_probe_sweep_radius(frozen, 0) == 0 exactly --
 * fixed_mul(0, RECIP_15) is exactly 0, fixed_sqrt(0) is exactly 0 (fixed.h's
 * own fixed_sqrt contract: x=0 is not the x<0 defensive case, and the
 * bit-by-bit loop returns exactly 0 for n=0), and fixed_mul(0, frozen) is
 * exactly 0 -- checked as EXACT equality, matching this pair's own
 * zero-ticks-is-zero precedent for the pulse function above. */
static void test_probe_sweep_radius_zero_ticks_is_zero(void) {
    T_CHECK(effects_probe_sweep_radius(fixed_from_double(35.0), 0) == 0);
}

/* Chunk 6: for t comfortably past 15, effects_probe_sweep_radius holds
 * EXACTLY at frozen_radius -- source's own min(sqrt(t/15),1) cap, once
 * t/15's sqrt reaches or exceeds 1, never exceeds frozen_radius no matter
 * how much further t grows. Checked at several t values well past 15,
 * confirming the cap holds rather than drifting (e.g. from further sqrt
 * growth an unclamped implementation would wrongly show), and as EXACT
 * equality (root clamps to precisely FX_ONE, and fixed_mul(FX_ONE, frozen)
 * is exact by fixed_mul's own definition), not tolerance-bounded. `t=15`
 * itself is deliberately NOT included here -- EFFECTS_PROBE_SWEEP_RECIP_15's
 * own truncation (4369/65536 == 0.0666656494, a hair below the true
 * 1/15 == 0.0666666667) makes the computed ratio at exactly t=15 a hair
 * BELOW 1.0 (not clamped, root landing a few raw units short of FX_ONE) --
 * the PRD checkpoint test above already covers t=15 with the appropriate
 * tolerance for exactly this reason; this test's job is only the
 * comfortably-past-the-cap region where the clamp itself, not truncation
 * noise near the boundary, is what's under test. */
static void test_probe_sweep_radius_caps_exactly_at_frozen_radius(void) {
    fixed_t frozen = fixed_from_double(35.0);

    long t_values[] = {20, 100, 10000};
    for (size_t i = 0; i < sizeof(t_values) / sizeof(t_values[0]); i++) {
        fixed_t t_fx = t_values[i] * FX_ONE;
        T_CHECK(effects_probe_sweep_radius(frozen, t_fx) == frozen);
    }
}

/* Chunk 7: rate<=0 never spawns, for any RNG state -- effects.h's doc
 * comment on effects_spiral_spawn_count establishes this returns 0
 * immediately without even drawing, but the OBSERVABLE contract under test
 * here is simply "never spawns," checked across many calls threading a
 * single rng_t (so if the implementation ever did draw despite rate<=0,
 * this would still catch any accidental nonzero count, not just the
 * no-draw implementation detail). Checked at rate exactly 0 and at a
 * negative rate (source's own `while rnd()<n do` never enters its body for
 * n<0 either, same as n==0, since rnd() is always >=0). */
static void test_spiral_spawn_count_nonpositive_rate_never_spawns(void) {
    rng_t rng;
    rng_seed(&rng, 42u);

    for (int i = 0; i < 1000; i++) {
        T_CHECK(effects_spiral_spawn_count(0, &rng) == 0);
    }

    rng_t rng2;
    rng_seed(&rng2, 777u);
    for (int i = 0; i < 1000; i++) {
        T_CHECK(effects_spiral_spawn_count(-FX_ONE, &rng2) == 0);
    }
}

/* Chunk 7: mirrors planetgen_test.c's own test_same_seed_is_deterministic
 * pattern -- two independent rng_t instances seeded identically must
 * produce an identical sequence of returned counts across many calls, at a
 * fixed fractional rate (3.5*FX_ONE, chosen so both the "always spawns 3"
 * guaranteed floor and the "maybe a 4th" fractional trial are exercised
 * every call, not just a whole-number rate that would never touch the
 * fractional-trial code path at all). */
static void test_spiral_spawn_count_same_seed_is_deterministic(void) {
    rng_t rng_a, rng_b;
    rng_seed(&rng_a, 12345u);
    rng_seed(&rng_b, 12345u);

    fixed_t rate = 3 * FX_ONE + FX_ONE / 2;

    for (int i = 0; i < 500; i++) {
        int count_a = effects_spiral_spawn_count(rate, &rng_a);
        int count_b = effects_spiral_spawn_count(rate, &rng_b);
        T_CHECK(count_a == count_b);
        /* Every call at this rate guarantees at least the integer floor
         * (3) and never more than one beyond it (4) -- see effects.h's doc
         * comment on effects_spiral_spawn_count. */
        T_CHECK(count_a == 3 || count_a == 4);
    }
}

/* Chunk 7 (final): the Monte-Carlo spawn-count regression this PRD's own
 * Implementation Chunks item 7 calls for -- "a fixed fractional rate (e.g.
 * 1.5), confirms the empirical mean lands within a documented tolerance of
 * the rate itself," explicitly mirroring
 * docs/prd/psx-rules-discovery.md's own empty-roll Monte-Carlo regression
 * (rules_discovery_test.c's
 * test_empty_rate_matches_045_within_tolerance_over_many_rolls), itself
 * mirroring this file's own identical
 * test_spawn_rate_matches_008_within_tolerance_over_many_calls precedent
 * above. Same posture: ONE rng_t (seeded once), threaded across every
 * call, not re-seeded per call.
 *
 * UNLIKE rules_discovery's empty-roll (a mean-reverting random walk whose
 * stationary rate is not analytically obvious), effects_spiral_spawn_count
 * is a sum of i.i.d. Bernoulli trials each call (see effects.h's doc
 * comment: floor(rate/FX_ONE) guaranteed spawns plus one more with
 * probability equal to rate's own fractional part), so its expected value
 * is EXACTLY `rate` (in real terms) for any rate>=0 -- confirmed
 * empirically below, not just assumed from that analysis.
 *
 * Choosing N and the tolerance band (empirical, per this PRD -- exploration
 * below, not part of the final test itself):
 *   - A standalone exploration harness (same shape as this test: one rng_t
 *     per seed, threaded across N calls to effects_spiral_spawn_count at
 *     this same fixed rate 1.5*FX_ONE) was run across 20 distinct seeds at
 *     N=20,000/50,000/100,000. At N=100,000: average-of-seed-means 1.49970
 *     (within 0.0003 of the nominal 1.5 -- essentially exact), min
 *     1.49756, max 1.50243 (a 0.00487 seed-to-seed spread, i.e. at most
 *     ~0.0025 single-sided deviation from 1.5 at any of the 20 seeds
 *     tried) -- tighter than N=50,000's own 0.00930 spread and N=20,000's
 *     0.01230, variance shrinking with N exactly as expected for a sum of
 *     i.i.d. Bernoulli trials, not a mean-reverting walk's stationary
 *     spread (which would not shrink the same way) -- matching
 *     rules_discovery_test.c's own identical "confirmed empirically, not
 *     just assumed" posture for its i.i.d.-vs-walk distinction.
 *   - N=100,000 was chosen over N=20,000/50,000 for its tighter
 *     seed-to-seed spread, at negligible extra cost -- the whole
 *     exploration sweep (20 seeds x 3 N values) runs in well under a
 *     second on the host.
 *   - Tolerance: +/-0.015 around 1.5. The observed seed-to-seed spread at
 *     N=100,000 (max single-sided deviation ~0.0025 from 1.5) sits
 *     comfortably inside this band -- margin to the nearest edge (0.015)
 *     is exactly 6x that maximum observed deviation, matching
 *     rules_discovery's/this file's own established "3-7x the observed
 *     spread" margin choice -- while still tight enough to catch a
 *     genuinely wrong loop condition or decrement amount (e.g. an
 *     off-by-one in the FX_ONE decrement would shift the mean by a
 *     magnitude far larger than this band). This test itself uses ONE
 *     fixed seed (56789u, confirmed by this test's own run to land at
 *     1.49867 -- comfortably inside the band, not a cherry-picked edge
 *     case), so
 *     it is fully deterministic and non-flaky by construction; the
 *     tolerance band exists to bound the GENERATOR's true stochastic
 *     behavior, not to average out test-to-test variance. */
static void test_spiral_spawn_count_mean_matches_rate_within_tolerance_over_many_calls(void) {
    const long NUM_CALLS = 100000;
    const double EXPECTED_MEAN = 1.5;
    const double TOLERANCE = 0.015;

    rng_t rng;
    rng_seed(&rng, 56789u);

    fixed_t rate = FX_ONE + FX_ONE / 2; /* 1.5*FX_ONE */

    long total_spawned = 0;
    for (long i = 0; i < NUM_CALLS; i++) {
        total_spawned += effects_spiral_spawn_count(rate, &rng);
    }

    double empirical_mean = (double)total_spawned / (double)NUM_CALLS;
    printf("     %ld calls, mean count %.5f, expected ~%.2f +/- %.3f\n", NUM_CALLS, empirical_mean,
           EXPECTED_MEAN, TOLERANCE);

    T_CHECK(empirical_mean > EXPECTED_MEAN - TOLERANCE);
    T_CHECK(empirical_mean < EXPECTED_MEAN + TOLERANCE);
}

int main(void) {
    t_run("active_zero_never_spawns_even_when_draw_would_succeed",
          test_active_zero_never_spawns_even_when_draw_would_succeed);
    t_run("same_seed_is_deterministic", test_same_seed_is_deterministic);
    t_run("spawned_velocity_is_unit_length", test_spawned_velocity_is_unit_length);
    t_run("spawn_position_is_perpendicular_offset_from_source",
          test_spawn_position_is_perpendicular_offset_from_source);
    t_run("from_equals_to_never_spawns", test_from_equals_to_never_spawns);
    t_run("step_advances_position_by_velocity", test_step_advances_position_by_velocity);
    t_run("step_decrements_lifetime_by_exactly_fx_one",
          test_step_decrements_lifetime_by_exactly_fx_one);
    t_run("particle_survives_until_lifetime_crosses_zero",
          test_particle_survives_until_lifetime_crosses_zero);
    t_run("already_dead_particle_reports_dead_on_first_step",
          test_already_dead_particle_reports_dead_on_first_step);
    t_run("full_spawn_to_death_lifecycle", test_full_spawn_to_death_lifecycle);
    t_run("spawn_rate_matches_008_within_tolerance_over_many_calls",
          test_spawn_rate_matches_008_within_tolerance_over_many_calls);
    t_run("kick_start_sets_amplitude_to_125_fx_one", test_kick_start_sets_amplitude_to_125_fx_one);
    t_run("kick_step_decays_amplitude_by_exact_amount", test_kick_step_decays_amplitude_by_exact_amount);
    t_run("kick_amplitude_reaches_zero_after_exactly_13_steps",
          test_kick_amplitude_reaches_zero_after_exactly_13_steps);
    t_run("kick_offset_matches_fixed_polar_directly", test_kick_offset_matches_fixed_polar_directly);
    t_run("idle_bob_delta_zero_ticks_is_zero", test_idle_bob_delta_zero_ticks_is_zero);
    t_run("idle_bob_delta_matches_formula_at_several_t_values",
          test_idle_bob_delta_matches_formula_at_several_t_values);
    t_run("reveal_ramp_step_sequence_climbs_then_holds_at_20",
          test_reveal_ramp_step_sequence_climbs_then_holds_at_20);
    t_run("reveal_ramp_step_default_20_never_climbs",
          test_reveal_ramp_step_default_20_never_climbs);
    t_run("reveal_ramp_table_matches_traced_k_table",
          test_reveal_ramp_table_matches_traced_k_table);
    t_run("probe_pulse_radius_matches_formula_at_prd_checkpoints",
          test_probe_pulse_radius_matches_formula_at_prd_checkpoints);
    t_run("probe_pulse_radius_zero_ticks_is_zero", test_probe_pulse_radius_zero_ticks_is_zero);
    t_run("probe_sweep_radius_matches_formula_at_prd_checkpoints",
          test_probe_sweep_radius_matches_formula_at_prd_checkpoints);
    t_run("probe_sweep_radius_zero_ticks_is_zero", test_probe_sweep_radius_zero_ticks_is_zero);
    t_run("probe_sweep_radius_caps_exactly_at_frozen_radius",
          test_probe_sweep_radius_caps_exactly_at_frozen_radius);
    t_run("spiral_spawn_count_nonpositive_rate_never_spawns",
          test_spiral_spawn_count_nonpositive_rate_never_spawns);
    t_run("spiral_spawn_count_same_seed_is_deterministic",
          test_spiral_spawn_count_same_seed_is_deterministic);
    t_run("spiral_spawn_count_mean_matches_rate_within_tolerance_over_many_calls",
          test_spiral_spawn_count_mean_matches_rate_within_tolerance_over_many_calls);
    return t_report();
}
