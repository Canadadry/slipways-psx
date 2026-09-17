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

int main(void) {
    t_run("active_zero_never_spawns_even_when_draw_would_succeed",
          test_active_zero_never_spawns_even_when_draw_would_succeed);
    t_run("same_seed_is_deterministic", test_same_seed_is_deterministic);
    t_run("spawned_velocity_is_unit_length", test_spawned_velocity_is_unit_length);
    t_run("spawn_position_is_perpendicular_offset_from_source",
          test_spawn_position_is_perpendicular_offset_from_source);
    t_run("from_equals_to_never_spawns", test_from_equals_to_never_spawns);
    return t_report();
}
