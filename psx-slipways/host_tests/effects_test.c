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

/* Chunk 8: the wormhole's own reference parameters (`g_`,
 * pico8/lua.lua:708-726), reused by every effects_spiral_try_spawn_particle
 * test below -- pn=3,pk=1,du=2,py=0.06,ki=9 -- matching this PRD's own
 * Implementation Chunks item 8 and Testing section exactly. */
#define WORMHOLE_BASE_RADIUS (2 * FX_ONE)
#define WORMHOLE_SECTORS 3
#define WORMHOLE_TANGENTIAL 1
#define WORMHOLE_LIFETIME (9 * FX_ONE)

/* fixed_from_double(0.06) is not a compile-time constant expression (it's an
 * inline function), so the velocity scale is computed inside each test
 * function rather than via a further #define -- matching this file's own
 * existing precedent of computing such values locally when a macro can't
 * hold them (e.g. test_kick_offset_matches_fixed_polar_directly's own local
 * `angular_rate`). */

/* Chunk 8: mirrors test_spiral_spawn_count_same_seed_is_deterministic's own
 * pattern -- two independent rng_t instances seeded identically must
 * produce an identical sequence of spawned particles (every field, not just
 * a subset) across many calls, at a growing age_ticks each call (so the
 * angle-drift term is genuinely exercised across a range of values, not
 * just age_ticks=0 every time). */
static void test_spiral_spawn_particle_same_seed_is_deterministic(void) {
    rng_t rng_a, rng_b;
    rng_seed(&rng_a, 12345u);
    rng_seed(&rng_b, 12345u);

    fixed_t velocity_scale = fixed_from_double(0.06);

    for (int i = 0; i < 500; i++) {
        fixed_t age_ticks = i * FX_ONE;
        effects_spiral_particle_t pa, pb;
        int result_a = effects_spiral_try_spawn_particle(WORMHOLE_BASE_RADIUS, WORMHOLE_SECTORS,
                                                           WORMHOLE_TANGENTIAL, age_ticks, velocity_scale,
                                                           WORMHOLE_LIFETIME, &rng_a, &pa);
        int result_b = effects_spiral_try_spawn_particle(WORMHOLE_BASE_RADIUS, WORMHOLE_SECTORS,
                                                           WORMHOLE_TANGENTIAL, age_ticks, velocity_scale,
                                                           WORMHOLE_LIFETIME, &rng_b, &pb);

        T_CHECK(result_a == 1);
        T_CHECK(result_b == 1);
        T_CHECK(pa.x == pb.x);
        T_CHECK(pa.y == pb.y);
        T_CHECK(pa.vx == pb.vx);
        T_CHECK(pa.vy == pb.vy);
        T_CHECK(pa.life == pb.life);
        T_CHECK(pa.has_accel == pb.has_accel);
    }
}

/* Chunk 8: every spawned particle's initial position lies within the
 * expected radius band. Derived from the formula (`p=qm(du+rnd(),a)`,
 * `rnd()` uniform in [0,1)): the spawn radius itself is in [du, du+1), and
 * fixed_polar's own magnitude-preserving cos/sin pair means
 * sqrt(x^2+y^2) should equal that same radius, up to the small numerical
 * error fixed_sin/fixed_cos's own LUT-interpolation and fixed_sqrt's own
 * truncation introduce -- the same magnitude-check shape
 * test_spawned_velocity_is_unit_length (chunk 2, above) already
 * establishes for a unit-length check; here the expected magnitude is a
 * band, not a single point. TOLERANCE (0.002 real units) is double
 * rng_and_fixed_test.c's own EPSILON (0.001) for a single fixed_polar
 * call, to budget for the extra fixed_sqrt truncation this check chains on
 * top. */
static void test_spiral_spawn_particle_position_within_radius_band(void) {
    double TOLERANCE = 0.002;

    rng_t rng;
    rng_seed(&rng, 999u);

    fixed_t velocity_scale = fixed_from_double(0.06);

    for (int i = 0; i < 500; i++) {
        effects_spiral_particle_t p;
        int spawned = effects_spiral_try_spawn_particle(WORMHOLE_BASE_RADIUS, WORMHOLE_SECTORS,
                                                          WORMHOLE_TANGENTIAL, i * FX_ONE, velocity_scale,
                                                          WORMHOLE_LIFETIME, &rng, &p);
        T_CHECK(spawned == 1);

        fixed_t mag_sq = fixed_add(fixed_mul(p.x, p.x), fixed_mul(p.y, p.y));
        double magnitude = fixed_to_double(fixed_sqrt(mag_sq));

        double base = fixed_to_double(WORMHOLE_BASE_RADIUS);
        T_CHECK(magnitude > base - TOLERANCE);
        T_CHECK(magnitude < base + 1.0 + TOLERANCE);
    }
}

/* Chunk 8: the tangential (pk=1) branch produces a velocity perpendicular
 * to the radial spawn vector -- dot(p,v) near zero. TOLERANCE (0.001)
 * matches rng_and_fixed_test.c's own EPSILON for a single fixed_polar-chain
 * check -- this dot product chains only a couple of extra fixed_mul calls
 * beyond fixed_polar itself (each contributing under 1 raw fixed_t unit of
 * truncation, negligible against 0.001 in real terms), matching this file's
 * established "budget a bit above the independently-derived worst case"
 * tolerance posture. */
static void test_spiral_spawn_particle_tangential_velocity_is_perpendicular(void) {
    double TOLERANCE = 0.001;

    rng_t rng;
    rng_seed(&rng, 424242u);

    fixed_t velocity_scale = fixed_from_double(0.06);

    for (int i = 0; i < 500; i++) {
        effects_spiral_particle_t p;
        int spawned = effects_spiral_try_spawn_particle(WORMHOLE_BASE_RADIUS, WORMHOLE_SECTORS,
                                                          WORMHOLE_TANGENTIAL, i * FX_ONE, velocity_scale,
                                                          WORMHOLE_LIFETIME, &rng, &p);
        T_CHECK(spawned == 1);

        fixed_t dot = fixed_add(fixed_mul(p.x, p.vx), fixed_mul(p.y, p.vy));
        T_CHECK(fabs(fixed_to_double(dot)) < TOLERANCE);
    }
}

/* Chunk 8: the counterpart to the perpendicularity check above -- with
 * tangential=0 (the radial branch), velocity must instead be PARALLEL to
 * position (cross product near zero, not the dot product), confirming the
 * `tangential` flag genuinely selects between the two branches rather than
 * e.g. always rotating regardless of its value (a bug the perpendicularity
 * test alone, run only with tangential=1, could never catch). Same
 * TOLERANCE reasoning as the perpendicularity check above. */
static void test_spiral_spawn_particle_radial_velocity_is_parallel(void) {
    double TOLERANCE = 0.001;

    rng_t rng;
    rng_seed(&rng, 13579u);

    fixed_t velocity_scale = fixed_from_double(0.06);

    for (int i = 0; i < 500; i++) {
        effects_spiral_particle_t p;
        int spawned = effects_spiral_try_spawn_particle(WORMHOLE_BASE_RADIUS, WORMHOLE_SECTORS, 0, i * FX_ONE,
                                                          velocity_scale, WORMHOLE_LIFETIME, &rng, &p);
        T_CHECK(spawned == 1);

        fixed_t cross = fixed_sub(fixed_mul(p.x, p.vy), fixed_mul(p.y, p.vx));
        T_CHECK(fabs(fixed_to_double(cross)) < TOLERANCE);
    }
}

/* Chunk 8: lifetime is always exactly `lifetime` (ki*FX_ONE) -- no
 * randomness in this field at all (unlike position/angle), so this is
 * checked as EXACT equality, not tolerance-bounded, matching this file's
 * established "exact when the source formula has no randomness" precedent
 * (test_step_decrements_lifetime_by_exactly_fx_one and others). Checked
 * across both the tangential and radial branches and a range of age_ticks,
 * confirming life never depends on either. */
static void test_spiral_spawn_particle_lifetime_is_always_exact(void) {
    rng_t rng;
    rng_seed(&rng, 24680u);

    fixed_t velocity_scale = fixed_from_double(0.06);

    for (int i = 0; i < 200; i++) {
        effects_spiral_particle_t p_tangential, p_radial;
        int spawned_t = effects_spiral_try_spawn_particle(WORMHOLE_BASE_RADIUS, WORMHOLE_SECTORS, 1, i * FX_ONE,
                                                            velocity_scale, WORMHOLE_LIFETIME, &rng, &p_tangential);
        int spawned_r = effects_spiral_try_spawn_particle(WORMHOLE_BASE_RADIUS, WORMHOLE_SECTORS, 0, i * FX_ONE,
                                                            velocity_scale, WORMHOLE_LIFETIME, &rng, &p_radial);

        T_CHECK(spawned_t == 1);
        T_CHECK(spawned_r == 1);
        T_CHECK(p_tangential.life == WORMHOLE_LIFETIME);
        T_CHECK(p_radial.life == WORMHOLE_LIFETIME);
    }
}

/* Chunk 8: has_accel/ax/ay are always 0 for this (g_:mm) reference variant
 * -- the wormhole's own particles never set `.a` (only `ga:mm` does, out of
 * scope for this whole PRD -- see effects.h's doc comment on effects_
 * spiral_particle_t). Checked as EXACT equality alongside the lifetime
 * check's own precedent above. */
static void test_spiral_spawn_particle_never_sets_acceleration(void) {
    rng_t rng;
    rng_seed(&rng, 11111u);

    fixed_t velocity_scale = fixed_from_double(0.06);

    effects_spiral_particle_t p;
    int spawned = effects_spiral_try_spawn_particle(WORMHOLE_BASE_RADIUS, WORMHOLE_SECTORS, WORMHOLE_TANGENTIAL, 0,
                                                      velocity_scale, WORMHOLE_LIFETIME, &rng, &p);
    T_CHECK(spawned == 1);
    T_CHECK(p.has_accel == 0);
    T_CHECK(p.ax == 0);
    T_CHECK(p.ay == 0);
}

/* Defensive edge case (not explicitly required by the chunk spec, but
 * worth confirming, matching test_from_equals_to_never_spawns's own
 * "defensive, not source-exercised, but still verified" precedent): a
 * non-positive `sectors` never spawns, for any RNG state, rather than
 * dividing by zero. */
static void test_spiral_spawn_particle_nonpositive_sectors_never_spawns(void) {
    rng_t rng;
    rng_seed(&rng, 55555u);

    fixed_t velocity_scale = fixed_from_double(0.06);

    for (int i = 0; i < 100; i++) {
        effects_spiral_particle_t p;
        int spawned_zero = effects_spiral_try_spawn_particle(WORMHOLE_BASE_RADIUS, 0, WORMHOLE_TANGENTIAL,
                                                               i * FX_ONE, velocity_scale, WORMHOLE_LIFETIME, &rng,
                                                               &p);
        T_CHECK(spawned_zero == 0);

        int spawned_negative = effects_spiral_try_spawn_particle(WORMHOLE_BASE_RADIUS, -3, WORMHOLE_TANGENTIAL,
                                                                   i * FX_ONE, velocity_scale, WORMHOLE_LIFETIME,
                                                                   &rng, &p);
        T_CHECK(spawned_negative == 0);
    }
}

/* Chunk 9: the wormhole's own matrix constants (`g_`, pico8/lua.lua:708-726:
 * `qo=1.02,ro=-0.0525,qg=0.0525,q_=1.02`), reused by every effects_spiral_
 * step_particle test below -- matching this PRD's own Implementation Chunks
 * item 9 and Testing section exactly, and this file's own established
 * "define the reference constants once, reuse across the chunk's tests"
 * precedent (WORMHOLE_BASE_RADIUS et al. above). Not compile-time constant
 * expressions (fixed_from_double is an inline function), so these are plain
 * file-scope non-const globals rather than #defines -- computed once here,
 * matching this file's own "compute locally when a macro can't hold it"
 * precedent noted above WORMHOLE_BASE_RADIUS. */
static fixed_t spiral_m00(void) { return fixed_from_double(1.02); }
static fixed_t spiral_m01(void) { return fixed_from_double(-0.0525); }
static fixed_t spiral_m10(void) { return fixed_from_double(0.0525); }
static fixed_t spiral_m11(void) { return fixed_from_double(1.02); }

/* effects_spiral_try_spawn_particle always zeroes has_accel/ax/ay, so a
 * has_accel=0 fixture for the step tests below is built directly rather than
 * via that spawn function -- matching effects_flow_step_particle's own
 * established "hand-built fixture, not always spawned" precedent
 * (test_step_decrements_lifetime_by_exactly_fx_one and others above). */
static effects_spiral_particle_t make_spiral_particle(fixed_t x, fixed_t y, fixed_t vx, fixed_t vy, fixed_t life) {
    effects_spiral_particle_t p;
    p.x = x;
    p.y = y;
    p.vx = vx;
    p.vy = vy;
    p.life = life;
    p.ax = 0;
    p.ay = 0;
    p.has_accel = 0;
    return p;
}

/* Chunk 9: p.p+=pv (pico8/lua.lua:698) uses the OLD velocity, read BEFORE
 * the matrix update overwrites it -- NOT the post-update velocity. vy=0 and
 * vx=FX_ONE are chosen so the post-update velocity (m00*vx+m01*vy,
 * m10*vx+m11*vy) = (1.02*FX_ONE, 0.0525*FX_ONE) is CLEARLY different from
 * the pre-update (FX_ONE, 0) -- both components differ, not just one -- so a
 * bug that read p->vx/p->vy AFTER the matrix update for the position advance
 * would be caught (position would advance by a visibly different amount).
 * life is kept comfortably above 1*FX_ONE so the return value isn't also
 * under test here (that's covered separately below). */
static void test_step_position_advances_by_pre_update_velocity(void) {
    effects_spiral_particle_t p = make_spiral_particle(5 * FX_ONE, -3 * FX_ONE, FX_ONE, 0, 5 * FX_ONE);

    effects_spiral_step_particle(&p, spiral_m00(), spiral_m01(), spiral_m10(), spiral_m11());

    /* Position must advance by exactly the OLD (FX_ONE, 0), not the
     * post-update velocity. */
    T_CHECK(p.x == fixed_add(5 * FX_ONE, FX_ONE));
    T_CHECK(p.y == fixed_add(-3 * FX_ONE, 0));

    /* Sanity check that pre/post velocity genuinely differ here -- if they
     * didn't, the position check above couldn't distinguish the two
     * evaluation orders at all. */
    T_CHECK(p.vx != FX_ONE);
}

/* Chunk 9: p.v.x=qo*pv.x+ro*pv.y, p.v.y=qg*pv.x+q_*pv.y (pico8/lua.lua:
 * 699-700) -- the general 2x2 matrix transform, checked two independent
 * ways against a nontrivial (vx,vy) with both components nonzero and
 * unequal in magnitude (so a row/column transposition bug -- e.g. swapping
 * m01 and m10 -- would produce a visibly different result than the correct
 * mapping):
 *
 *   1. EXACT equality against the same fixed_mul/fixed_add chain the
 *      function itself is expected to run, built independently here from
 *      the raw m00/m01/m10/m11 values -- matches this file's own
 *      test_kick_offset_matches_fixed_polar_directly precedent (exact
 *      equality is possible since both sides run identical fixed-point
 *      arithmetic on identical resolved arguments).
 *   2. A tolerance-bounded cross-check against libm doubles, computed
 *      wholly independently of both this port's truncated fixed_t matrix
 *      constants AND the fixed_mul/fixed_add chain above -- this is what
 *      actually catches a wrong m00<->m10 or m01<->m10 mapping (check 1
 *      alone would happily "pass" a consistently-swapped mapping, since it
 *      reuses the same swapped assignment on both sides).
 */
static void test_step_velocity_matches_2x2_matrix_transform(void) {
    fixed_t vx = 3 * FX_ONE + FX_ONE / 4;   /* 3.25 */
    fixed_t vy = -(2 * FX_ONE + FX_ONE / 2); /* -2.5 */

    fixed_t m00 = spiral_m00(), m01 = spiral_m01(), m10 = spiral_m10(), m11 = spiral_m11();

    effects_spiral_particle_t p = make_spiral_particle(0, 0, vx, vy, 5 * FX_ONE);
    effects_spiral_step_particle(&p, m00, m01, m10, m11);

    /* Check 1: exact match against the same fixed-point chain. */
    fixed_t expected_vx = fixed_add(fixed_mul(m00, vx), fixed_mul(m01, vy));
    fixed_t expected_vy = fixed_add(fixed_mul(m10, vx), fixed_mul(m11, vy));
    T_CHECK(p.vx == expected_vx);
    T_CHECK(p.vy == expected_vy);

    /* Check 2: independent libm cross-check -- 1.02*3.25+(-0.0525)*(-2.5)
     * = 3.44625, 0.0525*3.25+1.02*(-2.5) = -2.379375. Tolerance 0.001,
     * matching this file's own established EPSILON-scale budget for a
     * couple of chained fixed_mul/fixed_add calls (well under 1 raw
     * fixed_t unit each). */
    double expected_vx_d = 1.02 * 3.25 + (-0.0525) * (-2.5);
    double expected_vy_d = 0.0525 * 3.25 + 1.02 * (-2.5);
    T_CHECK(fabs(fixed_to_double(p.vx) - expected_vx_d) < 0.001);
    T_CHECK(fabs(fixed_to_double(p.vy) - expected_vy_d) < 0.001);
}

/* Chunk 9: p.l-=0.14 (pico8/lua.lua:702) -- checked as EXACT fixed-point
 * subtraction, matching test_step_decrements_lifetime_by_exactly_fx_one's
 * own identical exactness precedent for the trade-flow particle. */
static void test_step_decrements_life_by_exactly_014_fx_one(void) {
    fixed_t life_decay = (fixed_t)(0.14 * (double)FX_ONE);

    effects_spiral_particle_t p = make_spiral_particle(0, 0, FX_ONE / 2, -FX_ONE / 3, 10 * FX_ONE);
    fixed_t life0 = p.life;

    effects_spiral_step_particle(&p, spiral_m00(), spiral_m01(), spiral_m10(), spiral_m11());

    T_CHECK(p.life == fixed_sub(life0, life_decay));
}

/* Chunk 9: the death threshold is life<=1*FX_ONE, NOT life<=0 -- the one
 * place this function's death condition genuinely differs from
 * effects_flow_step_particle's own <=0 threshold (see effects.h's doc
 * comment on this function). life0 is chosen so the boundary crossing
 * happens at a known, hand-derived call: life0 = FX_ONE + 2*life_decay, so
 * call 1 leaves FX_ONE+life_decay (still >FX_ONE, alive) and call 2 leaves
 * EXACTLY FX_ONE (not >FX_ONE, dead) -- the exact boundary this threshold is
 * about, not just "eventually dies". */
static void test_particle_survives_until_life_crosses_one_fx_one(void) {
    fixed_t life_decay = (fixed_t)(0.14 * (double)FX_ONE);
    fixed_t life0 = fixed_add(FX_ONE, 2 * life_decay);

    effects_spiral_particle_t p = make_spiral_particle(0, 0, 0, 0, life0);

    int alive1 = effects_spiral_step_particle(&p, spiral_m00(), spiral_m01(), spiral_m10(), spiral_m11());
    T_CHECK(alive1 == 1);
    T_CHECK(p.life == fixed_add(FX_ONE, life_decay));

    int alive2 = effects_spiral_step_particle(&p, spiral_m00(), spiral_m01(), spiral_m10(), spiral_m11());
    T_CHECK(alive2 == 0);
    T_CHECK(p.life == FX_ONE);
}

/* Chunk 9: a particle whose life lands strictly BETWEEN 0 and 1*FX_ONE
 * (post-decrement) must ALREADY report dead -- the case a wrongly-ported
 * `<=0` threshold (effects_flow_step_particle's own threshold, mistakenly
 * reused here) would get wrong, reporting it alive instead. life0 is chosen
 * so life lands at exactly FX_ONE/2 after one decrement -- squarely inside
 * (0, FX_ONE), nowhere near either boundary, so this isn't just re-testing
 * the exact-FX_ONE boundary the previous test already covers. */
static void test_particle_with_life_between_zero_and_one_fx_one_is_dead(void) {
    fixed_t life_decay = (fixed_t)(0.14 * (double)FX_ONE);
    fixed_t life0 = fixed_add(FX_ONE / 2, life_decay);

    effects_spiral_particle_t p = make_spiral_particle(0, 0, 0, 0, life0);

    int alive = effects_spiral_step_particle(&p, spiral_m00(), spiral_m01(), spiral_m10(), spiral_m11());

    T_CHECK(p.life == FX_ONE / 2);
    T_CHECK(p.life > 0);
    T_CHECK(p.life < FX_ONE);
    T_CHECK(alive == 0);
}

/* Chunk 9 (final): full spawn-to-death lifecycle -- spawn via chunk 8's
 * effects_spiral_try_spawn_particle with lifetime=9*FX_ONE (the wormhole's
 * own ki=9), then step repeatedly via this chunk's function until it
 * reports dead, confirming the tick count matches (ki-1)/0.14 within one
 * tick -- mirroring test_full_spawn_to_death_lifecycle's own (chunk 4)
 * lifecycle-test precedent above.
 *
 * Hand-derivation (worked out independently before writing the assertion,
 * using this same truncated life_decay=9175 this file's own
 * test_step_decrements_life_by_exactly_014_fx_one already established, not
 * copied from the PRD's real-number arithmetic): life0 = 9*FX_ONE = 589824.
 * Each step subtracts exactly life_decay=9175. The first N with
 * 589824 - N*9175 <= FX_ONE(65536) is N=58: at N=57, life =
 * 589824-57*9175=66849 (>65536, alive); at N=58, life=66849-9175=57674
 * (<=65536, dead). (ki-1)/0.14 = 8/0.14 = 57.142857..., which rounds up to
 * 58 -- ticks_to_death=58 matches that within less than one tick, as the PRD
 * requires. */
static void test_spiral_full_spawn_to_death_lifecycle(void) {
    rng_t rng;
    rng_seed(&rng, 2024u);

    fixed_t velocity_scale = fixed_from_double(0.06);

    effects_spiral_particle_t p;
    int spawned = effects_spiral_try_spawn_particle(WORMHOLE_BASE_RADIUS, WORMHOLE_SECTORS, WORMHOLE_TANGENTIAL, 0,
                                                      velocity_scale, WORMHOLE_LIFETIME, &rng, &p);
    T_CHECK(spawned == 1);
    T_CHECK(p.life == WORMHOLE_LIFETIME);

    fixed_t m00 = spiral_m00(), m01 = spiral_m01(), m10 = spiral_m10(), m11 = spiral_m11();

    int alive = 1;
    int ticks = 0;
    int max_ticks = 1000; /* safety bound; hand derivation above expects
                            * exactly 58 */
    while (alive && ticks < max_ticks) {
        alive = effects_spiral_step_particle(&p, m00, m01, m10, m11);
        ticks++;
    }

    T_CHECK(ticks == 58);
    T_CHECK(p.life <= FX_ONE);
}

/* Chunk 9: if(p.a)p.v+=p.a (pico8/lua.lua:701) -- has_accel=1 with nonzero
 * ax/ay must add them to velocity AFTER the matrix transform. Expected
 * velocity is the same matrix-transform chain test_step_velocity_matches_
 * 2x2_matrix_transform's own check 1 already establishes, plus ax/ay added
 * on top -- checked as EXACT equality, matching this file's established
 * "exact when the chain is fully deterministic fixed-point arithmetic"
 * precedent. */
static void test_step_applies_acceleration_when_has_accel_set(void) {
    fixed_t vx = 2 * FX_ONE, vy = -FX_ONE;
    fixed_t ax = FX_ONE / 4, ay = -(FX_ONE / 5);

    fixed_t m00 = spiral_m00(), m01 = spiral_m01(), m10 = spiral_m10(), m11 = spiral_m11();

    effects_spiral_particle_t p = make_spiral_particle(0, 0, vx, vy, 5 * FX_ONE);
    p.ax = ax;
    p.ay = ay;
    p.has_accel = 1;

    effects_spiral_step_particle(&p, m00, m01, m10, m11);

    fixed_t expected_vx = fixed_add(fixed_add(fixed_mul(m00, vx), fixed_mul(m01, vy)), ax);
    fixed_t expected_vy = fixed_add(fixed_add(fixed_mul(m10, vx), fixed_mul(m11, vy)), ay);
    T_CHECK(p.vx == expected_vx);
    T_CHECK(p.vy == expected_vy);
}

/* Chunk 9: the counterpart to the has_accel=1 check above -- has_accel=0
 * must leave velocity as JUST the matrix transform, regardless of whatever
 * garbage sits in ax/ay (here, deliberately large nonzero values that would
 * visibly perturb the result if the conditional were skipped or always
 * applied incorrectly). Confirms the conditional is genuinely checked, not
 * always-applied (this test alone would catch that) nor always-skipped
 * (test_step_applies_acceleration_when_has_accel_set above would catch
 * that) -- the pair together prove the branch is a real conditional. */
static void test_step_ignores_acceleration_garbage_when_has_accel_clear(void) {
    fixed_t vx = 2 * FX_ONE, vy = -FX_ONE;

    fixed_t m00 = spiral_m00(), m01 = spiral_m01(), m10 = spiral_m10(), m11 = spiral_m11();

    effects_spiral_particle_t p = make_spiral_particle(0, 0, vx, vy, 5 * FX_ONE);
    /* "Garbage" ax/ay -- large and nonzero, but has_accel is 0 (see
     * make_spiral_particle), so these must be ignored entirely. */
    p.ax = 999 * FX_ONE;
    p.ay = -777 * FX_ONE;

    effects_spiral_step_particle(&p, m00, m01, m10, m11);

    fixed_t expected_vx = fixed_add(fixed_mul(m00, vx), fixed_mul(m01, vy));
    fixed_t expected_vy = fixed_add(fixed_mul(m10, vx), fixed_mul(m11, vy));
    T_CHECK(p.vx == expected_vx);
    T_CHECK(p.vy == expected_vy);
}

/* Chunk 10: EFFECTS_POPUP_VELOCITY_Y is file-local to effects.c, so this
 * helper re-derives the same literal-to-fixed_t translation independently
 * here -- `v=(0,-0.34)` (`iv`'s own prototype default, pico8/lua.lua:
 * 941-951, the money-spent/income popup). -0.34*FX_ONE (-22282.24) is NOT
 * exactly representable; C's (fixed_t) cast from a negative double
 * truncates TOWARDS ZERO (drops the fractional part, the same direction as
 * this file's own positive-literal truncations elsewhere), giving -22282,
 * NOT -22283 (which a floor-towards-negative-infinity truncation would
 * instead produce). */
static fixed_t popup_velocity_y(void) {
    return (fixed_t)(-0.34 * (double)FX_ONE);
}

/* Chunk 10: effects_popup_start places the popup at the exact caller-
 * supplied (x,y), sets vy to the fixed constant above, and age to 0 --
 * checked as EXACT equality (a plain field-copy, no computation involved
 * at all). */
static void test_popup_start_sets_position_velocity_and_zero_age(void) {
    effects_popup_t p;
    effects_popup_start(&p, 7 * FX_ONE, -3 * FX_ONE);

    T_CHECK(p.x == 7 * FX_ONE);
    T_CHECK(p.y == -3 * FX_ONE);
    T_CHECK(p.vy == popup_velocity_y());
    T_CHECK(p.age == 0);
}

/* Chunk 10: self.ra+=self.v (`iv:ph`, pico8/lua.lua:941-951) -- position
 * advances by exactly the stored (fixed) vy each call, x untouched (v has
 * no x component). Checked across several calls, not just one, confirming
 * the SAME constant applies every time, not just the first. */
static void test_popup_step_advances_y_by_exact_velocity_each_call(void) {
    effects_popup_t p;
    effects_popup_start(&p, 100 * FX_ONE, 50 * FX_ONE);

    fixed_t vy = popup_velocity_y();
    fixed_t expected_y = 50 * FX_ONE;

    for (int i = 0; i < 5; i++) {
        effects_popup_step(&p);
        expected_y = fixed_add(expected_y, vy);
        T_CHECK(p.y == expected_y);
        T_CHECK(p.x == 100 * FX_ONE);
    }
}

/* Chunk 10: dies at exactly the 51st call, NOT the naively-expected 50th
 * (50*0.1==5.0 in real-number terms) -- hand-derived here independently of
 * both the PRD text and effects.c's own EFFECTS_POPUP_AGE_PER_STEP/
 * EFFECTS_POPUP_DEATH_AGE macros (which would make this test tautological
 * against a wrong constant):
 *
 *   0.1*FX_ONE = 0.1*65536 = 6553.6, truncated (towards zero) to 6553 --
 *   the same truncation convention every other literal-constant macro in
 *   this file already follows (see effects.c's own doc comment on
 *   EFFECTS_POPUP_AGE_PER_STEP).
 *   5*FX_ONE = 5*65536 = 327680 (the death threshold -- exact, 5 has no
 *   fractional part).
 *   After N calls, age = N*6553 (each call adds exactly the same already-
 *   truncated 6553, an exact integer, so this is exact arithmetic with no
 *   further rounding per call).
 *   N=50: 50*6553 = 327650 < 327680 -- still alive.
 *   N=51: 51*6553 = 334203 >= 327680 -- dead.
 *
 * So the truncated per-step constant accumulates just enough shortfall (30
 * raw fixed_t units short of the threshold at N=50) to push death one call
 * later than the naive real-number expectation -- exactly the kind of
 * off-by-one this chunk's own task description warns to verify precisely,
 * not assume. */
static void test_popup_dies_at_exactly_51st_step(void) {
    effects_popup_t p;
    effects_popup_start(&p, 0, 0);

    int alive = 1;
    for (int i = 1; i <= 50; i++) {
        alive = effects_popup_step(&p);
        T_CHECK(alive == 1);
    }
    T_CHECK(p.age == 50 * 6553);

    alive = effects_popup_step(&p);
    T_CHECK(alive == 0);
    T_CHECK(p.age == 51 * 6553);
}

/* Chunk 10: effects_popup_fade_index == 0 at spawn (age=0) --
 * (int)(0>>16) is exactly 0, no clamping involved, checked as EXACT
 * equality. */
static void test_popup_fade_index_zero_at_spawn(void) {
    effects_popup_t p;
    effects_popup_start(&p, 0, 0);

    T_CHECK(effects_popup_fade_index(&p) == 0);
}

/* Chunk 10: effects_popup_fade_index == 4 at the LAST alive call (the
 * 50th, per test_popup_dies_at_exactly_51st_step's own hand derivation
 * above) -- age = 50*6553 = 327650 there; (int)(327650>>16) =
 * floor(327650/65536) = 4 (4*65536=262144 <= 327650 < 327680=5*65536),
 * independently re-derived here, not copied from the PRD's own "4 just
 * before death" wording. */
static void test_popup_fade_index_is_4_just_before_death(void) {
    effects_popup_t p;
    effects_popup_start(&p, 0, 0);

    int alive = 1;
    for (int i = 0; i < 50; i++) {
        alive = effects_popup_step(&p);
    }
    T_CHECK(alive == 1);
    T_CHECK(effects_popup_fade_index(&p) == 4);
}

/* Chunk 10: fade index is monotonically non-decreasing across the popup's
 * entire alive lifetime (age only ever increases, and floor(age/FX_ONE) of
 * a monotonically increasing sequence can never decrease), and never
 * strays outside [0,4] -- checked well past death too (several extra calls
 * after effects_popup_step first reports dead), confirming the upper
 * clamp genuinely engages once age exceeds 5*FX_ONE (a real, exercised
 * case -- see effects.h's doc comment on effects_popup_fade_index for why
 * this isn't dead code). */
static void test_popup_fade_index_monotonic_and_bounded_through_and_past_death(void) {
    effects_popup_t p;
    effects_popup_start(&p, 0, 0);

    int prev_index = effects_popup_fade_index(&p);
    T_CHECK(prev_index == 0);

    for (int i = 0; i < 60; i++) {
        effects_popup_step(&p);
        int index = effects_popup_fade_index(&p);
        T_CHECK(index >= 0);
        T_CHECK(index <= 4);
        T_CHECK(index >= prev_index);
        prev_index = index;
    }

    /* After 60 calls (well past the 51st-call death), age has grown
     * comfortably past 5*FX_ONE -- confirms the clamp holds, not just that
     * it happened not to be exercised at exactly the boundary. */
    T_CHECK(prev_index == 4);
}

/* Chunk 11: EFFECTS_GROWBAR_STEP is file-local to effects.c, so this helper
 * re-derives the same literal-to-fixed_t translation independently here --
 * `self.gz+=0.2` (`pj:kj`, pico8/lua.lua:854, the end-game reveal's
 * frame-grow bars). 0.2*FX_ONE (13107.2) is NOT exactly representable; C's
 * (fixed_t) cast from a positive double truncates towards zero, giving
 * 13107, matching effects.c's own EFFECTS_GROWBAR_STEP. */
static fixed_t growbar_step_constant(void) {
    return (fixed_t)(0.2 * (double)FX_ONE);
}

/* Chunk 11: same independent-re-derivation pattern for the height cap --
 * `min(self.gz,3.2)` (`pj:kj`, pico8/lua.lua:855). 3.2*FX_ONE (209715.2) is
 * NOT exactly representable, truncating to 209715. */
static fixed_t growbar_height_cap_constant(void) {
    return (fixed_t)(3.2 * (double)FX_ONE);
}

/* Chunk 11: `g->gz` advances by exactly EFFECTS_GROWBAR_STEP every call,
 * UNCAPPED -- checked well past the point where the HEIGHT formula's own
 * clamp engages (call 17, per the hand-derivation in
 * test_growbar_cap_engages_at_17th_call_not_16th below), confirming gz
 * itself keeps growing without bound, matching source's own `self.gz+=0.2`
 * (no ceiling on gz, only on its use in the height formula) -- see
 * effects.h's doc comment on effects_growbar_step for why. Checked as EXACT
 * equality (fixed_add is exact integer addition, and the step constant is
 * itself an exact integer once truncated), across 30 calls. */
static void test_growbar_gz_advances_by_exact_step_uncapped(void) {
    effects_growbar_t g = {0};
    fixed_t step = growbar_step_constant();

    fixed_t expected_gz = 0;
    for (int call = 1; call <= 30; call++) {
        effects_growbar_step(&g);
        expected_gz = fixed_add(expected_gz, step);
        T_CHECK(g.gz == expected_gz);
    }

    /* By call 30, gz is comfortably past the 3.2*FX_ONE height cap -- gz
     * itself must still equal the plain uncapped sum, not the clamped
     * value. */
    T_CHECK(g.gz > growbar_height_cap_constant());
}

/* Chunk 11: the exact call at which the HEIGHT formula's clamp first
 * engages, hand-derived here independently of both the PRD text (which
 * claims "the 16th" call) and effects.c's own EFFECTS_GROWBAR_STEP/
 * EFFECTS_GROWBAR_HEIGHT_CAP macros (which would make this test
 * tautological against a wrong constant):
 *
 *   EFFECTS_GROWBAR_STEP = (fixed_t)(0.2*65536) = (fixed_t)(13107.2) = 13107
 *   (truncated towards zero, exact double-to-fixed_t translation).
 *   EFFECTS_GROWBAR_HEIGHT_CAP = (fixed_t)(3.2*65536) =
 *   (fixed_t)(209715.2) = 209715 (truncated towards zero).
 *
 *   After N calls (starting from gz=0), gz = N*13107 exactly (each call
 *   adds the same already-truncated integer step, so this is exact integer
 *   arithmetic, no further rounding per call -- the same "exact accumulated
 *   sum of an exact per-step integer" reasoning
 *   test_popup_dies_at_exactly_51st_step above already establishes for this
 *   file).
 *
 *   N=16: gz = 16*13107 = 209712 -- STILL BELOW the cap (209712 < 209715),
 *   so `min(gz,cap)` returns gz itself (209712) unclamped at call 16 --
 *   contrary to the PRD's stated "the 16th" call. This is exactly the kind
 *   of truncation-caused off-by-one this chunk's own task description
 *   warns to verify precisely, not assume (mirroring
 *   test_popup_dies_at_exactly_51st_step's own identical "50th" vs "51st"
 *   finding for a different constant in this same file).
 *
 *   N=17: gz = 17*13107 = 222819 -- now >= the cap (222819 >= 209715), so
 *   `min(gz,cap)` clamps to exactly 209715 here for the first time. The cap
 *   FIRST ENGAGES ON THE 17TH CALL, not the 16th.
 *
 *   Heights: h(16) = fixed_mul(209712,209712) = (209712*209712)>>16 =
 *   671068 (still the UNCLAMPED gz squared). h(17) =
 *   fixed_mul(209715,209715) = (209715*209715)>>16 = 671087 (the CLAMPED
 *   cap squared, for the first time) -- both values computed here via
 *   fixed_mul directly (the same primitive effects_growbar_step itself
 *   uses), not via floating-point, so this is an exact integer check with
 *   no tolerance needed.
 */
static void test_growbar_cap_engages_at_17th_call_not_16th(void) {
    effects_growbar_t g = {0};

    fixed_t h = 0;
    for (int call = 1; call <= 17; call++) {
        h = effects_growbar_step(&g);
    }

    /* Re-derive both boundary heights independently via fixed_mul on the
     * hand-computed gz values above, not by copying effects_growbar_step's
     * own return value back at itself. */
    fixed_t h16 = fixed_mul(209712, 209712);
    fixed_t h17 = fixed_mul(209715, 209715);
    T_CHECK(h16 == 671068);
    T_CHECK(h17 == 671087);

    /* Rewind and check call 16 in isolation: still unclamped. */
    effects_growbar_t g16 = {0};
    fixed_t h_at_16 = 0;
    for (int call = 1; call <= 16; call++) {
        h_at_16 = effects_growbar_step(&g16);
    }
    T_CHECK(g16.gz == 16 * 13107);
    T_CHECK(g16.gz < growbar_height_cap_constant());
    T_CHECK(h_at_16 == h16);

    /* Call 17: gz has now crossed the cap, so the height is the CLAMPED cap
     * squared, not gz itself squared. */
    T_CHECK(g.gz == 17 * 13107);
    T_CHECK(g.gz >= growbar_height_cap_constant());
    T_CHECK(h == h17);
}

/* Chunk 11: effects_growbar_step matches `min(gz,3.2)^2` at several
 * hand-picked call counts well before the cap engages (calls 1, 5, 10),
 * confirming the plain unclamped-squaring path, not just the boundary
 * calls test_growbar_cap_engages_at_17th_call_not_16th already covers.
 * Expected heights are computed here via fixed_mul directly on the
 * independently hand-derived gz value at each call count (N*13107), not
 * copied from the PRD text and not recomputed through effects.c's own
 * macros. */
static void test_growbar_height_matches_formula_before_cap_engages(void) {
    struct {
        int calls;
        fixed_t expected_gz;
        fixed_t expected_height;
    } cases[] = {
        {1, 1 * 13107, fixed_mul(1 * 13107, 1 * 13107)},
        {5, 5 * 13107, fixed_mul(5 * 13107, 5 * 13107)},
        {10, 10 * 13107, fixed_mul(10 * 13107, 10 * 13107)},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        effects_growbar_t g = {0};
        fixed_t h = 0;
        for (int call = 0; call < cases[i].calls; call++) {
            h = effects_growbar_step(&g);
        }

        T_CHECK(g.gz == cases[i].expected_gz);
        T_CHECK(h == cases[i].expected_height);
    }
}

/* Chunk 11: once the cap has engaged (call 17 onward, per
 * test_growbar_cap_engages_at_17th_call_not_16th), the returned height
 * stays EXACTLY at the capped value (EFFECTS_GROWBAR_HEIGHT_CAP squared)
 * forever after, even though gz itself (per
 * test_growbar_gz_advances_by_exact_step_uncapped) keeps growing without
 * bound -- checked well past the cap-engaging call (up to call 50), not
 * just immediately after it. */
static void test_growbar_height_stays_capped_well_past_cap_engaging_call(void) {
    effects_growbar_t g = {0};
    fixed_t capped_height = fixed_mul(209715, 209715);

    fixed_t h = 0;
    for (int call = 1; call <= 50; call++) {
        h = effects_growbar_step(&g);
        if (call >= 17) {
            T_CHECK(h == capped_height);
        }
    }

    T_CHECK(h == capped_height);
    /* gz itself is still far past the cap, confirming the height clamp is
     * genuinely re-derived from a still-growing gz each call, not a
     * one-time latch. */
    T_CHECK(g.gz == 50 * 13107);
    T_CHECK(g.gz > growbar_height_cap_constant());
}

/* Chunk 12: `js:ph()`'s own `d=qf-ra` easing formula, `self.ra+=(#d<=1 and
 * d or d*0.4)` (pico8/lua.lua:264). A target more than 1 unit (squared)
 * away eases by exactly 40% of the remaining vector per call. d=(6,8)*FX_ONE
 * (a 6-8-10-scaled 3-4-5 right triangle, squared magnitude 100 -- nowhere
 * near the 1-unit-squared snap threshold), from ra0=(2,3)*FX_ONE so qf ends
 * up at (8,11)*FX_ONE (ra0+d), distinct from every from/to pair already
 * used elsewhere in this file. Expected new position is computed here via
 * fixed_add(ra0, fixed_mul(d, EASE)) directly, with EASE recomputed
 * independently as (fixed_t)(0.4*(double)FX_ONE) -- effects.c's own
 * EFFECTS_SETTLE_EASE_FACTOR is file-local, not visible here anyway -- both
 * expressions are the same literal double-to-fixed_t translation of
 * source's own `0.4` constant, so this is a real independent check (proving
 * effects_settle_step's own formula/argument order), not a tautology,
 * matching test_kick_offset_matches_fixed_polar_directly's own established
 * "recompute the constant independently, then call the same underlying
 * primitive directly" style. Exact equality is possible (no tolerance
 * needed): both paths run the identical fixed_mul computation on the
 * identical resolved arguments. qf itself must be left untouched by a call
 * that only eases (never snaps). */
static void test_settle_step_eases_by_exactly_40_percent_when_far(void) {
    fixed_t ease = (fixed_t)(0.4 * (double)FX_ONE);

    effects_settle_t s;
    s.ra_x = 2 * FX_ONE;
    s.ra_y = 3 * FX_ONE;
    s.qf_x = 8 * FX_ONE;  /* ra_x + 6*FX_ONE */
    s.qf_y = 11 * FX_ONE; /* ra_y + 8*FX_ONE */

    fixed_t expected_ra_x = fixed_add(2 * FX_ONE, fixed_mul(6 * FX_ONE, ease));
    fixed_t expected_ra_y = fixed_add(3 * FX_ONE, fixed_mul(8 * FX_ONE, ease));

    int still_moving = effects_settle_step(&s);

    T_CHECK(still_moving == 1);
    T_CHECK(s.ra_x == expected_ra_x);
    T_CHECK(s.ra_y == expected_ra_y);
    T_CHECK(s.qf_x == 8 * FX_ONE);
    T_CHECK(s.qf_y == 11 * FX_ONE);
}

/* Chunk 12: a target within the squared-1 threshold snaps exactly to it in
 * one call, never overshoots, never oscillates. d=(0.5,0)*FX_ONE -- linear
 * magnitude 0.5, squared magnitude 0.25*FX_ONE, comfortably under the
 * 1*FX_ONE threshold -- and, critically, NOT already zero distance (ra !=
 * qf going in), so this genuinely exercises the snap branch rather than the
 * already-settled trivial case test_settle_step_idempotent_after_snap below
 * covers separately. After one call, ra must equal qf EXACTLY (not merely
 * "close" -- checked as exact equality, matching this file's established
 * "checked as EXACT equality" precedent wherever the underlying arithmetic
 * guarantees it, e.g. test_step_decrements_lifetime_by_exactly_fx_one). */
static void test_settle_step_snaps_within_threshold_not_already_at_target(void) {
    effects_settle_t s;
    s.ra_x = 5 * FX_ONE;
    s.ra_y = -2 * FX_ONE;
    s.qf_x = 5 * FX_ONE + FX_ONE / 2; /* ra_x + 0.5*FX_ONE */
    s.qf_y = -2 * FX_ONE;

    int still_moving = effects_settle_step(&s);

    T_CHECK(still_moving == 0);
    T_CHECK(s.ra_x == s.qf_x);
    T_CHECK(s.ra_y == s.qf_y);
    T_CHECK(s.ra_x == 5 * FX_ONE + FX_ONE / 2);
}

/* Chunk 12: the squared-distance-1 threshold is INCLUSIVE (`#d<=1`, not
 * `#d<1`), matching source's own `<=` exactly -- checked at the exact
 * boundary, d=(1,0)*FX_ONE: linear magnitude exactly 1, so squared
 * magnitude fixed_mul(FX_ONE,FX_ONE) is also exactly FX_ONE (1*1==1, no
 * fixed-point rounding at this particular value -- (FX_ONE*FX_ONE)>>16 ==
 * FX_ONE exactly), landing precisely ON the threshold rather than merely
 * near it. Must still snap (mag_sq <= FX_ONE holds with equality), not
 * ease -- a `<` instead of `<=` bug would wrongly ease here instead. */
static void test_settle_step_snaps_exactly_at_squared_distance_one_boundary(void) {
    effects_settle_t s;
    s.ra_x = 0;
    s.ra_y = 0;
    s.qf_x = FX_ONE;
    s.qf_y = 0;

    fixed_t mag_sq_at_boundary = fixed_mul(FX_ONE, FX_ONE);
    T_CHECK(mag_sq_at_boundary == FX_ONE);

    int still_moving = effects_settle_step(&s);

    T_CHECK(still_moving == 0);
    T_CHECK(s.ra_x == FX_ONE);
    T_CHECK(s.ra_y == 0);
}

/* Chunk 12: repeated calls after snapping leave the position unchanged
 * (idempotent once settled) and continue to report "no longer moving" --
 * since d is exactly (0,0) once ra==qf, squared magnitude 0<=1*FX_ONE
 * trivially holds every subsequent call. Snaps first (reusing this same
 * near-but-not-at-target setup, distinct from the exactly-on-target-from-
 * the-start case below), then calls again twice more to confirm the
 * steady state genuinely holds, not just immediately after the snap. */
static void test_settle_step_idempotent_after_snap(void) {
    effects_settle_t s;
    s.ra_x = 10 * FX_ONE;
    s.ra_y = 4 * FX_ONE;
    s.qf_x = 10 * FX_ONE + FX_ONE / 4; /* squared magnitude 0.0625*FX_ONE */
    s.qf_y = 4 * FX_ONE;

    int first = effects_settle_step(&s);
    T_CHECK(first == 0);
    T_CHECK(s.ra_x == s.qf_x);
    T_CHECK(s.ra_y == s.qf_y);

    for (int call = 0; call < 2; call++) {
        fixed_t ra_x_before = s.ra_x, ra_y_before = s.ra_y;
        int still_moving = effects_settle_step(&s);

        T_CHECK(still_moving == 0);
        T_CHECK(s.ra_x == ra_x_before);
        T_CHECK(s.ra_y == ra_y_before);
        T_CHECK(s.ra_x == s.qf_x);
        T_CHECK(s.ra_y == s.qf_y);
    }
}

/* Chunk 12: proves `#d<=1` is a genuine COMBINED squared-magnitude test
 * (dx*dx+dy*dy), not a per-axis-independent check and not a linear sum
 * (dx+dy) -- the concrete boundary case the PRD's own testing mandate asks
 * for ("verify the squared, not linear, distance claim... work out a
 * concrete numeric case where an accidentally wrong threshold formula would
 * give a different, wrong answer").
 *
 * A single (dx,dy) case can NOT distinguish the correct combined-squared
 * formula (dx*dx+dy*dy<=1*FX_ONE) from an incorrectly-linear one
 * (sqrt(dx*dx+dy*dy)<=1*FX_ONE) -- both are mathematically equivalent at
 * threshold 1 (sqrt is monotonic and sqrt(1)==1, i.e. 1 is sqrt's own fixed
 * point: M<=1 iff sqrt(M)<=1 for any M>=0), matching this PRD's own
 * explicit "this threshold happens to be the same either way" caveat (see
 * effects.h's doc comment on effects_settle_step) -- so no such case is
 * constructed here (none exists to construct).
 *
 * What DOES genuinely distinguish the correct formula from a plausible
 * wrong one is dx=dy=0.9*FX_ONE: the TRUE combined squared magnitude is
 * 0.9^2+0.9^2 = 1.62 (real terms), comfortably OVER the 1-unit threshold, so
 * a correct implementation must EASE, not snap. But two plausible wrong
 * formulas would both wrongly snap here instead:
 *   - a per-axis-independent check (`dx<=1*FX_ONE && dy<=1*FX_ONE`, testing
 *     each component against the threshold separately instead of summing
 *     their squares first) -- 0.9<=1 is true for BOTH axes independently,
 *     so this bug would wrongly report "within threshold" and snap.
 *   - a raw (non-squared) magnitude sum (`dx+dy<=1*FX_ONE`, mistaking `#d`
 *     for a Manhattan-style sum rather than squared Euclidean magnitude) --
 *     0.9+0.9=1.8, which is itself over the 1*FX_ONE threshold too... but
 *     that one coincidentally does NOT snap here either (see below), so
 *     this particular case only catches the per-axis bug, not that one; no
 *     single case catches every conceivable wrong formula at once, and the
 *     per-axis mixup is the more plausible bug shape for this specific
 *     "#d<=1" one-liner (accidentally treating a vector comparison as two
 *     scalar comparisons) -- this test's job is that one concrete,
 *     traceable distinction, not an exhaustive catalog of every possible
 *     wrong formula.
 * Position after the call is also checked against the exact 40% ease
 * formula (same independent-recomputation style as
 * test_settle_step_eases_by_exactly_40_percent_when_far above), confirming
 * the ease branch itself still computes correctly here, not just that it
 * was taken. */
static void test_settle_step_combines_both_axes_not_per_axis_threshold(void) {
    fixed_t ease = (fixed_t)(0.4 * (double)FX_ONE);
    fixed_t component = (fixed_t)(0.9 * (double)FX_ONE);

    effects_settle_t s;
    s.ra_x = 0;
    s.ra_y = 0;
    s.qf_x = component;
    s.qf_y = component;

    fixed_t mag_sq = fixed_add(fixed_mul(component, component), fixed_mul(component, component));
    T_CHECK(mag_sq > FX_ONE); /* 0.81+0.81 == 1.62, over the 1-unit threshold */

    int still_moving = effects_settle_step(&s);

    T_CHECK(still_moving == 1); /* eases -- must NOT snap */
    T_CHECK(s.ra_x == fixed_mul(component, ease));
    T_CHECK(s.ra_y == fixed_mul(component, ease));
    T_CHECK(s.ra_x != s.qf_x);
    T_CHECK(s.ra_y != s.qf_y);
}

/* Chunk 13 (psx-juice-effects, Implementation Chunks item 13):
 * effects_tech_menu_just_opened is the pure, stateless half of the
 * live-wiring layer -- host-testable in isolation, mirroring
 * rules_tech_step's own pure-function tests. Only the false->true
 * transition reports "just opened"; every other combination (staying
 * closed, staying open, closing) does not. */
static void test_tech_menu_just_opened_only_on_false_to_true_edge(void) {
    T_CHECK(effects_tech_menu_just_opened(0, 0) == 0);
    T_CHECK(effects_tech_menu_just_opened(0, 1) == 1);
    T_CHECK(effects_tech_menu_just_opened(1, 1) == 0);
    T_CHECK(effects_tech_menu_just_opened(1, 0) == 0);
}

/* The stateful wrapper's open edge: every tile's ra is reset to the SAME
 * shared origin before this frame's own effects_settle_step() call, so a
 * tile whose target happens to equal the origin snaps immediately (mag_sq
 * 0 <= FX_ONE) while a tile with a genuinely distant target is still mid-
 * ease after that same call -- proving both "shared origin" and "distinct
 * per-tile target" in one round trip. */
static void test_tech_menu_update_resets_every_tile_to_shared_origin_on_open_edge(void) {
    effects_tech_menu_init();

    int target_x[RULES_TECH_ERA1_TILE_COUNT];
    int target_y[RULES_TECH_ERA1_TILE_COUNT];
    target_x[0] = 0;
    target_y[0] = 0; /* == origin: settles immediately */
    target_x[1] = 0;
    target_y[1] = 11; /* far from origin: still easing after one call */
    int i;
    for (i = 2; i < RULES_TECH_ERA1_TILE_COUNT; i++) {
        target_x[i] = 0;
        target_y[i] = 0;
    }

    effects_tech_menu_update(1, 0, 0, target_x, target_y);

    int x0, y0, x1, y1;
    effects_tech_menu_tile_pos(0, &x0, &y0);
    effects_tech_menu_tile_pos(1, &x1, &y1);

    T_CHECK(x0 == 0);
    T_CHECK(y0 == 0); /* tile 0: origin == target, snapped exactly */

    T_CHECK(x1 == 0);
    T_CHECK(y1 != 0);
    T_CHECK(y1 != 11); /* tile 1: started at the same origin, but still
                         * mid-ease toward its own distant target -- not
                         * yet at either endpoint. */
}

/* Repeated updates (menu held open, target unchanged) converge exactly to
 * the target and stay there -- effects_settle_step's own idempotent-after-
 * snap guarantee, exercised through the stateful wrapper rather than the
 * pure primitive directly. */
static void test_tech_menu_update_converges_and_stays_at_target_while_open(void) {
    effects_tech_menu_init();

    int target_x[RULES_TECH_ERA1_TILE_COUNT];
    int target_y[RULES_TECH_ERA1_TILE_COUNT];
    int i;
    for (i = 0; i < RULES_TECH_ERA1_TILE_COUNT; i++) {
        target_x[i] = 8 + i * 12;
        target_y[i] = 8;
    }

    int call;
    for (call = 0; call < 30; call++) {
        effects_tech_menu_update(1, 0, 0, target_x, target_y);
    }

    int t;
    for (t = 0; t < RULES_TECH_ERA1_TILE_COUNT; t++) {
        int x, y;
        effects_tech_menu_tile_pos(t, &x, &y);
        T_CHECK(x == target_x[t]);
        T_CHECK(y == target_y[t]);
    }

    /* One more call after settling must not move it off target. */
    effects_tech_menu_update(1, 0, 0, target_x, target_y);
    int x, y;
    effects_tech_menu_tile_pos(0, &x, &y);
    T_CHECK(x == target_x[0]);
    T_CHECK(y == target_y[0]);
}

/* Closing the menu (open=0) freezes every tile's position (no step, no
 * reset) -- and reopening it afterward re-fires the edge, resetting back
 * to the shared origin even though the tile had already settled at its
 * previous target. */
static void test_tech_menu_close_then_reopen_resets_again(void) {
    effects_tech_menu_init();

    int target_x[RULES_TECH_ERA1_TILE_COUNT];
    int target_y[RULES_TECH_ERA1_TILE_COUNT];
    int i;
    for (i = 0; i < RULES_TECH_ERA1_TILE_COUNT; i++) {
        target_x[i] = 8 + i * 12;
        target_y[i] = 8;
    }

    int call;
    for (call = 0; call < 30; call++) {
        effects_tech_menu_update(1, 0, 0, target_x, target_y);
    }
    int settled_x, settled_y;
    effects_tech_menu_tile_pos(0, &settled_x, &settled_y);
    T_CHECK(settled_x == target_x[0]);

    /* Closed: position must not change, even across several closed
     * frames. */
    effects_tech_menu_update(0, 0, 0, target_x, target_y);
    effects_tech_menu_update(0, 0, 0, target_x, target_y);
    int x, y;
    effects_tech_menu_tile_pos(0, &x, &y);
    T_CHECK(x == settled_x);
    T_CHECK(y == settled_y);

    /* Reopen with the same shared origin (0,0) -- away from tile 0's own
     * already-settled target -- the edge must fire again and reset tile 0
     * back toward that origin (away from where it was left settled). */
    effects_tech_menu_update(1, 0, 0, target_x, target_y);
    effects_tech_menu_tile_pos(0, &x, &y);
    T_CHECK(x != settled_x || y != settled_y);
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
    t_run("spiral_spawn_particle_same_seed_is_deterministic",
          test_spiral_spawn_particle_same_seed_is_deterministic);
    t_run("spiral_spawn_particle_position_within_radius_band",
          test_spiral_spawn_particle_position_within_radius_band);
    t_run("spiral_spawn_particle_tangential_velocity_is_perpendicular",
          test_spiral_spawn_particle_tangential_velocity_is_perpendicular);
    t_run("spiral_spawn_particle_radial_velocity_is_parallel",
          test_spiral_spawn_particle_radial_velocity_is_parallel);
    t_run("spiral_spawn_particle_lifetime_is_always_exact",
          test_spiral_spawn_particle_lifetime_is_always_exact);
    t_run("spiral_spawn_particle_never_sets_acceleration",
          test_spiral_spawn_particle_never_sets_acceleration);
    t_run("spiral_spawn_particle_nonpositive_sectors_never_spawns",
          test_spiral_spawn_particle_nonpositive_sectors_never_spawns);
    t_run("step_position_advances_by_pre_update_velocity",
          test_step_position_advances_by_pre_update_velocity);
    t_run("step_velocity_matches_2x2_matrix_transform",
          test_step_velocity_matches_2x2_matrix_transform);
    t_run("step_decrements_life_by_exactly_014_fx_one",
          test_step_decrements_life_by_exactly_014_fx_one);
    t_run("particle_survives_until_life_crosses_one_fx_one",
          test_particle_survives_until_life_crosses_one_fx_one);
    t_run("particle_with_life_between_zero_and_one_fx_one_is_dead",
          test_particle_with_life_between_zero_and_one_fx_one_is_dead);
    t_run("spiral_full_spawn_to_death_lifecycle", test_spiral_full_spawn_to_death_lifecycle);
    t_run("step_applies_acceleration_when_has_accel_set",
          test_step_applies_acceleration_when_has_accel_set);
    t_run("step_ignores_acceleration_garbage_when_has_accel_clear",
          test_step_ignores_acceleration_garbage_when_has_accel_clear);
    t_run("popup_start_sets_position_velocity_and_zero_age",
          test_popup_start_sets_position_velocity_and_zero_age);
    t_run("popup_step_advances_y_by_exact_velocity_each_call",
          test_popup_step_advances_y_by_exact_velocity_each_call);
    t_run("popup_dies_at_exactly_51st_step", test_popup_dies_at_exactly_51st_step);
    t_run("popup_fade_index_zero_at_spawn", test_popup_fade_index_zero_at_spawn);
    t_run("popup_fade_index_is_4_just_before_death",
          test_popup_fade_index_is_4_just_before_death);
    t_run("popup_fade_index_monotonic_and_bounded_through_and_past_death",
          test_popup_fade_index_monotonic_and_bounded_through_and_past_death);
    t_run("growbar_gz_advances_by_exact_step_uncapped",
          test_growbar_gz_advances_by_exact_step_uncapped);
    t_run("growbar_cap_engages_at_17th_call_not_16th",
          test_growbar_cap_engages_at_17th_call_not_16th);
    t_run("growbar_height_matches_formula_before_cap_engages",
          test_growbar_height_matches_formula_before_cap_engages);
    t_run("growbar_height_stays_capped_well_past_cap_engaging_call",
          test_growbar_height_stays_capped_well_past_cap_engaging_call);
    t_run("settle_step_eases_by_exactly_40_percent_when_far",
          test_settle_step_eases_by_exactly_40_percent_when_far);
    t_run("settle_step_snaps_within_threshold_not_already_at_target",
          test_settle_step_snaps_within_threshold_not_already_at_target);
    t_run("settle_step_snaps_exactly_at_squared_distance_one_boundary",
          test_settle_step_snaps_exactly_at_squared_distance_one_boundary);
    t_run("settle_step_idempotent_after_snap", test_settle_step_idempotent_after_snap);
    t_run("settle_step_combines_both_axes_not_per_axis_threshold",
          test_settle_step_combines_both_axes_not_per_axis_threshold);
    t_run("tech_menu_just_opened_only_on_false_to_true_edge",
          test_tech_menu_just_opened_only_on_false_to_true_edge);
    t_run("tech_menu_update_resets_every_tile_to_shared_origin_on_open_edge",
          test_tech_menu_update_resets_every_tile_to_shared_origin_on_open_edge);
    t_run("tech_menu_update_converges_and_stays_at_target_while_open",
          test_tech_menu_update_converges_and_stays_at_target_while_open);
    t_run("tech_menu_close_then_reopen_resets_again", test_tech_menu_close_then_reopen_resets_again);
    return t_report();
}
