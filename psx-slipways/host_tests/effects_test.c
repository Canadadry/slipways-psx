#include "t.h"
#include "../effects.h"
#include "../fixed.h"
#include "../rng.h"

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
    return t_report();
}
