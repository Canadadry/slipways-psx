#include "effects.h"

/*
 * effects_length -- integer square root of a raw squared-distance sum,
 * returning the result directly as a fixed_t (Q16.16) LINEAR distance.
 *
 * fixed_sqrt (fixed.h) takes a fixed_t input x (representing a real value
 * x/FX_ONE) and internally widens it by FX_ONE before taking an integer
 * square root -- correct for a modest, already-fixed_mul'd squared value,
 * but this module's dx/dy can be screen-space magnitudes up to a few
 * hundred pixels (SCREEN_XRES=320, SCREEN_YRES=240, screen.h), so
 * fixed_mul(dx,dx) (whose result must itself fit in a fixed_t, i.e.
 * represent a real value below ~32767) overflows for realistic on-screen
 * segment lengths well before fixed_sqrt is ever reached -- e.g. a
 * 320-unit dx alone squares to 102400, already past that ceiling.
 *
 * Avoids this entirely by working in raw, doubly-scaled int64_t squared
 * units instead, the same overflow-safe idiom liaison.c's
 * liaison_point_blocks_segment/liaison_check and rules_discovery.c's
 * rules_discovery_probe_in_range already establish for squared-distance
 * math: for fixed_t values dx_raw = real_dx*FX_ONE, dy_raw = real_dy*FX_ONE,
 * the raw sum dist_sq_raw = dx_raw*dx_raw + dy_raw*dy_raw equals exactly
 * (real_dx^2 + real_dy^2) * FX_ONE^2 = (real_length * FX_ONE)^2 -- i.e.
 * dist_sq_raw is already the SQUARE of the fixed_t we want (length_raw =
 * real_length*FX_ONE), with no extra FX_ONE scaling needed. So an ordinary
 * integer square root of dist_sq_raw itself (NOT dist_sq_raw pre-shifted
 * left by 16 again, unlike fixed_sqrt's own contract) gives length_raw
 * directly. dx/dy are bounded well within int32_t, so dist_sq_raw
 * comfortably fits uint64_t (worst case around a 320x240 screen-space
 * span, nowhere near uint64_t's range).
 *
 * The bit-by-bit integer square root loop below is the same standard
 * "digit by digit" method fixed_sqrt uses internally (see fixed.h),
 * applied directly to dist_sq_raw instead of dist_sq_raw<<16 -- kept as a
 * local static helper here, not a fixed.h addition, per this chunk's
 * "don't touch fixed.h" constraint.
 */
static fixed_t effects_length(fixed_t dx, fixed_t dy) {
    int64_t dx64 = (int64_t)dx;
    int64_t dy64 = (int64_t)dy;
    uint64_t n = (uint64_t)(dx64 * dx64 + dy64 * dy64);
    uint64_t result = 0;
    uint64_t bit = (uint64_t)1 << 62;

    while (bit > n) {
        bit >>= 2;
    }

    while (bit != 0) {
        if (n >= result + bit) {
            n -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }

    return (fixed_t)result;
}

/*
 * effects_fixed_div -- local Q16.16 division helper. fixed.h (chunk 1)
 * only added fixed_sqrt alongside the pre-existing fixed_add/fixed_sub/
 * fixed_mul; none of those is a division, and this chunk must not modify
 * fixed.h. Needed here to build a unit vector (dx/length, dy/length).
 * Standard fixed-point division: widen the numerator by FX_ONE (shift left
 * 16) in int64_t before dividing, mirroring fixed_mul's own "widen to
 * int64_t, then shift/divide" shape. `a` is a screen-space coordinate delta
 * (well within int32_t) and `b` is always effects_length's result, which is
 * nonzero whenever this is called (the from == to case is already excluded
 * by effects_flow_try_spawn before reaching here), so no divide-by-zero.
 */
static fixed_t effects_fixed_div(fixed_t a, fixed_t b) {
    return (fixed_t)(((int64_t)a << 16) / (int64_t)b);
}

int effects_flow_try_spawn(fixed_t from_x, fixed_t from_y, fixed_t to_x, fixed_t to_y, int active, rng_t *rng, effects_flow_particle_t *out) {
    /* Always draw first, unconditionally -- mirrors Lua's left-to-right
     * "and" short-circuit in "rnd()<0.08 and self.hg[t.id]": rnd() (the
     * left operand) is always evaluated, even when the right operand
     * (self.hg[t.id], here `active`) would end up false. See effects.h's
     * doc comment on this function for the full trail. */
    fixed_t draw = (fixed_t)rng_next_range(rng, 0u, (uint32_t)FX_ONE);

    /* Defensive zero-length-segment guard -- source never exercises this
     * (see effects.h), so its placement relative to the RNG draw above is
     * an arbitrary implementation choice, not a source-fidelity concern;
     * checked after the draw purely to keep the "always consumes a draw"
     * rule simple and exceptionless. */
    if (from_x == to_x && from_y == to_y) {
        return 0;
    }

    if (!(draw < EFFECTS_FLOW_SPAWN_CHANCE) || !active) {
        return 0;
    }

    fixed_t dx = fixed_sub(to_x, from_x);
    fixed_t dy = fixed_sub(to_y, from_y);
    fixed_t length = effects_length(dx, dy);

    fixed_t unit_x = effects_fixed_div(dx, length);
    fixed_t unit_y = effects_fixed_div(dy, length);

    /* km:pa()'s 90-degree rotation (x,y)->(-y,x), applied to the unit
     * vector: spawn = from + (-unit_y, unit_x). */
    out->x = fixed_add(from_x, -unit_y);
    out->y = fixed_add(from_y, unit_x);
    out->vx = unit_x;
    out->vy = unit_y;
    out->lifetime = length;

    return 1;
}

int effects_flow_step_particle(effects_flow_particle_t *p) {
    /* t.ra+=t.v (pico8/lua.lua:674): position advances by the stored
     * velocity, once per call. */
    p->x = fixed_add(p->x, p->vx);
    p->y = fixed_add(p->y, p->vy);

    /* t.l-=1 (pico8/lua.lua:675): PICO-8's own numbers are natively
     * Q16.16 fixed-point, so this literal "1" is already one full
     * number-unit in that same representation -- exactly FX_ONE here, not
     * a plain integer needing rescale. See effects.h's doc comment on
     * this function for the full trail. */
    p->lifetime = fixed_sub(p->lifetime, FX_ONE);

    /* if (t.l<=0)del(...) (pico8/lua.lua:676), inverted to "still alive". */
    return p->lifetime > 0;
}

/*
 * EFFECTS_KICK_START_AMPLITUDE -- ports `jo:kd()`'s literal `self.ma=1.25`
 * (pico8/lua.lua:469). Exact: 1.25*FX_ONE == 1.25*65536 == 81920.0 has no
 * fractional part, so this double-to-fixed_t truncation is lossless.
 */
#define EFFECTS_KICK_START_AMPLITUDE ((fixed_t)(1.25 * (double)FX_ONE))

/*
 * EFFECTS_KICK_DECAY_PER_STEP -- ports `ma(p,e)`'s literal `e.ma-0.1`
 * (pico8/lua.lua:349). Unlike EFFECTS_KICK_START_AMPLITUDE above,
 * 0.1*FX_ONE (6553.6) is NOT exactly representable, so this truncates to
 * 6553 -- the same double-to-fixed_t truncation convention
 * EFFECTS_FLOW_SPAWN_CHANCE (effects.h) already establishes for this
 * codebase's literal-PICO8-constant macros.
 */
#define EFFECTS_KICK_DECAY_PER_STEP ((fixed_t)(0.1 * (double)FX_ONE))

/*
 * EFFECTS_KICK_ANGULAR_TURNS_PER_TICK -- ports `ma(p,e)`'s literal
 * `e.t*0.2` (pico8/lua.lua:350): 0.2 turns swept per tick of age. Kept
 * local to this file (not effects.h) since, unlike
 * EFFECTS_KICK_START_AMPLITUDE/EFFECTS_KICK_DECAY_PER_STEP, no other
 * function in this codebase has a reason to reference this particular
 * constant.
 */
#define EFFECTS_KICK_ANGULAR_TURNS_PER_TICK ((fixed_t)(0.2 * (double)FX_ONE))

void effects_kick_start(effects_kick_t *k) {
    k->amplitude = EFFECTS_KICK_START_AMPLITUDE;
}

void effects_kick_step(effects_kick_t *k) {
    fixed_t decayed = fixed_sub(k->amplitude, EFFECTS_KICK_DECAY_PER_STEP);
    k->amplitude = decayed > 0 ? decayed : 0;
}

void effects_kick_offset(const effects_kick_t *k, fixed_t age_ticks, fixed_t *out_dx, fixed_t *out_dy) {
    /* qm(e.ma, e.t*0.2) -- magnitude is the amplitude, turns is the AGE
     * scaled by 0.2, not the other way around (see effects.h's doc comment
     * on this function for why that argument order matters). */
    fixed_t turns = fixed_mul(age_ticks, EFFECTS_KICK_ANGULAR_TURNS_PER_TICK);
    fixed_polar(k->amplitude, turns, out_dx, out_dy);
}
