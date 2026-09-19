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

/*
 * EFFECTS_IDLE_BOB_ANGULAR_TURNS_PER_TICK -- ports `na()`'s literal
 * `t*0.005` (pico8/lua.lua:578): 0.005 turns swept per tick of age, fed to
 * fixed_sin as its turns argument (see effects.h's doc comment on
 * effects_idle_bob_delta for why fixed_sin, not fixed_cos). 0.005*FX_ONE
 * (327.68) is NOT exactly representable, so this truncates to 327 -- the
 * same double-to-fixed_t truncation convention EFFECTS_FLOW_SPAWN_CHANCE
 * (effects.h) and EFFECTS_KICK_DECAY_PER_STEP above already establish for
 * this codebase's literal-PICO8-constant macros.
 */
#define EFFECTS_IDLE_BOB_ANGULAR_TURNS_PER_TICK ((fixed_t)(0.005 * (double)FX_ONE))

/*
 * EFFECTS_IDLE_BOB_AMPLITUDE -- ports `na()`'s literal `*0.06`
 * (pico8/lua.lua:578), kept in PICO-8's native 128-unit space, unscaled --
 * see effects.h's doc comment on effects_idle_bob_delta for why this
 * function doesn't rescale it. 0.06*FX_ONE (3932.16) is NOT exactly
 * representable, so this truncates to 3932, same convention as the two
 * constants above.
 */
#define EFFECTS_IDLE_BOB_AMPLITUDE ((fixed_t)(0.06 * (double)FX_ONE))

fixed_t effects_idle_bob_delta(fixed_t age_ticks) {
    /* sin(t*0.005)*0.06 -- fixed_sin (not fixed_cos: a plain sine wave, see
     * effects.h's doc comment on this function), scaled by the traced,
     * unrescaled amplitude. */
    fixed_t turns = fixed_mul(age_ticks, EFFECTS_IDLE_BOB_ANGULAR_TURNS_PER_TICK);
    return fixed_mul(fixed_sin(turns), EFFECTS_IDLE_BOB_AMPLITUDE);
}

/*
 * EFFECTS_PROBE_PULSE_RAMP_RECIP_15 -- `mz:kz(t)`'s own `t/15` divisor
 * inside its ramp-up term `min(t/15,1)` (pico8/lua.lua:772-773), as a
 * precomputed reciprocal constant -- the same reciprocal-multiply idiom
 * RULES_ECONOMY_SLIPWAY_COST_DIVISOR_RECIP (rules_economy.c) already
 * establishes for dividing a fixed_t by a compile-time constant (`x/n`
 * becomes `fixed_mul(x, 1/n)`), since fixed.h has no general fixed_div.
 * 1/15 (0.0666...) is NOT exactly representable in Q16.16, so this
 * truncates -- the same double-to-fixed_t truncation convention every other
 * literal-constant macro in this file already follows.
 */
#define EFFECTS_PROBE_PULSE_RAMP_RECIP_15 ((fixed_t)((1.0 / 15.0) * (double)FX_ONE))

/*
 * EFFECTS_PROBE_PULSE_BREATH_RECIP_40 -- same reciprocal-multiply idiom as
 * above, for `mz:kz(t)`'s own `t/40` divisor inside its breathing term
 * `sin(t/40)*0.1` (pico8/lua.lua:772-773 again -- `kz`'s single `self.oi=...`
 * statement uses both divisors).
 */
#define EFFECTS_PROBE_PULSE_BREATH_RECIP_40 ((fixed_t)((1.0 / 40.0) * (double)FX_ONE))

/*
 * EFFECTS_PROBE_PULSE_BREATH_AMPLITUDE -- `mz:kz(t)`'s own literal `*0.1`
 * scaling the breathing sine term (pico8/lua.lua:773). 0.1*FX_ONE (6553.6)
 * is NOT exactly representable, so this truncates to 6553 -- the same
 * convention EFFECTS_KICK_DECAY_PER_STEP's identical `0.1` literal already
 * follows above.
 */
#define EFFECTS_PROBE_PULSE_BREATH_AMPLITUDE ((fixed_t)(0.1 * (double)FX_ONE))

fixed_t effects_probe_pulse_radius(fixed_t base_radius, fixed_t t) {
    /* min(t/15,1) -- the linear ramp-up, clamped the same plain-ternary way
     * effects_kick_step's own max(0,...) floor is above (no fixed_min/
     * fixed_max helper -- see effects.h's doc comment on this function). */
    fixed_t ramp = fixed_mul(t, EFFECTS_PROBE_PULSE_RAMP_RECIP_15);
    if (ramp > FX_ONE) {
        ramp = FX_ONE;
    }

    /* sin(t/40)*0.1 -- t/40 is a plain numeric ratio in source's own
     * min()/sqrt() sense, but PICO-8's sin() takes its argument in turns
     * (fixed.h's fixed_sin block comment), so this ratio is already in the
     * right units to feed fixed_sin directly -- the same pattern
     * effects_idle_bob_delta's own sin(t*0.005) already establishes (see
     * effects.h's doc comment on this function). */
    fixed_t breath_turns = fixed_mul(t, EFFECTS_PROBE_PULSE_BREATH_RECIP_40);
    fixed_t breath = fixed_mul(fixed_sin(breath_turns), EFFECTS_PROBE_PULSE_BREATH_AMPLITUDE);

    return fixed_mul(base_radius, fixed_add(ramp, breath));
}

/*
 * EFFECTS_PROBE_SWEEP_RECIP_15 -- `mz:ej(t)`'s own `t/15` divisor inside
 * `sqrt(t/15)` (pico8/lua.lua:795-796), same reciprocal-multiply idiom as
 * EFFECTS_PROBE_PULSE_RAMP_RECIP_15 above -- kept as its own separately-
 * named constant (not shared with that macro) even though both equal 1/15,
 * since `kz` and `ej` are two independently-traced source formulas, matching
 * this file's "one macro per traced literal, tied to its own source
 * citation" style elsewhere (e.g. EFFECTS_KICK_START_AMPLITUDE vs
 * EFFECTS_IDLE_BOB_AMPLITUDE, each its own named constant regardless of
 * numeric relationship to any other).
 */
#define EFFECTS_PROBE_SWEEP_RECIP_15 ((fixed_t)((1.0 / 15.0) * (double)FX_ONE))

fixed_t effects_probe_sweep_radius(fixed_t frozen_radius, fixed_t t) {
    /* sqrt(t/15), then min(...,1) -- fixed_sqrt (fixed.h, chunk 1) takes a
     * fixed_t x representing the real value x/FX_ONE and returns the fixed_t
     * representing sqrt(x/FX_ONE), which is exactly the shape `ratio`
     * (representing the real value t/15) needs here. */
    fixed_t ratio = fixed_mul(t, EFFECTS_PROBE_SWEEP_RECIP_15);
    fixed_t root = fixed_sqrt(ratio);
    if (root > FX_ONE) {
        root = FX_ONE;
    }

    return fixed_mul(root, frozen_radius);
}

int effects_spiral_spawn_count(fixed_t rate, rng_t *rng) {
    int count = 0;

    /* while rnd()<n do ... n-=1 end (pico8/lua.lua:687-691). Draw a fresh
     * uniform [0,1) value each iteration -- the same
     * rng_next_range(rng, 0, FX_ONE) idiom effects_flow_try_spawn (above)
     * already establishes -- and stop the moment a draw fails to beat the
     * remaining rate, or the remaining rate has already reached/passed
     * zero (checked up front each iteration, so a rate<=0 call never draws
     * at all -- see effects.h's doc comment on this function for why that
     * still matches source's own observable spawn count exactly). */
    while (rate > 0) {
        fixed_t draw = (fixed_t)rng_next_range(rng, 0u, (uint32_t)FX_ONE);
        if (!(draw < rate)) {
            break;
        }

        count++;
        rate = fixed_sub(rate, FX_ONE);
    }

    return count;
}

/*
 * EFFECTS_SPIRAL_ANGLE_DRIFT_PER_TICK -- ports `g_:mm()`'s own literal
 * `t*0.001` (pico8/lua.lua:727): 0.001 turns of angle drift per tick of
 * age. 0.001*FX_ONE (65.536) is NOT exactly representable, so this
 * truncates to 65 -- the same double-to-fixed_t truncation convention
 * every other literal-constant macro in this file already follows.
 */
#define EFFECTS_SPIRAL_ANGLE_DRIFT_PER_TICK ((fixed_t)(0.001 * (double)FX_ONE))

int effects_spiral_try_spawn_particle(fixed_t base_radius, int sectors, int tangential, fixed_t age_ticks, fixed_t velocity_scale, fixed_t lifetime, rng_t *rng, effects_spiral_particle_t *out) {
    /* a=...(flr(rnd(pn))+rnd(0.5))/pn... divides by sectors -- defensive
     * guard against a division by zero no traced source call site ever
     * exercises (see this function's doc comment in effects.h). */
    if (sectors <= 0) {
        return 0;
    }

    /* Three RNG draws, in source's own exact left-to-right order -- see
     * effects.h's doc comment on this function for the full trail. */
    int sector_index = (int)rng_next_range(rng, 0u, (uint32_t)sectors);         /* flr(rnd(pn)) */
    fixed_t jitter = (fixed_t)rng_next_range(rng, 0u, (uint32_t)(FX_ONE / 2));  /* rnd(0.5) */
    fixed_t radius_jitter = (fixed_t)rng_next_range(rng, 0u, (uint32_t)FX_ONE); /* rnd() */

    /* a = (flr(rnd(pn))+rnd(0.5))/pn + t*0.001 */
    fixed_t sector_and_jitter = fixed_add((fixed_t)sector_index * FX_ONE, jitter);
    fixed_t sector_fraction = effects_fixed_div(sector_and_jitter, (fixed_t)sectors * FX_ONE);
    fixed_t drift = fixed_mul(age_ticks, EFFECTS_SPIRAL_ANGLE_DRIFT_PER_TICK);
    fixed_t angle = fixed_add(sector_fraction, drift);

    /* p = qm(du+rnd(), a) */
    fixed_t radius = fixed_add(base_radius, radius_jitter);
    fixed_t px, py_pos;
    fixed_polar(radius, angle, &px, &py_pos);
    out->x = px;
    out->y = py_pos;

    /* v = p*py, then self.pk and v:pa() or v -- km:pa()'s 90-degree
     * rotation (x,y)->(-y,x), the same helper/direction effects_flow_try_
     * spawn's own doc comment above already cites, applied here to the
     * radial velocity instead of a unit vector. */
    fixed_t radial_vx = fixed_mul(px, velocity_scale);
    fixed_t radial_vy = fixed_mul(py_pos, velocity_scale);
    if (tangential) {
        out->vx = -radial_vy;
        out->vy = radial_vx;
    } else {
        out->vx = radial_vx;
        out->vy = radial_vy;
    }

    /* l = ki -- flat, already-scaled, no randomness. */
    out->life = lifetime;

    /* ga:mm's own optional acceleration never applies to this reference
     * (g_:mm) variant -- see effects_spiral_particle_t's doc comment. */
    out->ax = 0;
    out->ay = 0;
    out->has_accel = 0;

    return 1;
}

/*
 * EFFECTS_SPIRAL_LIFE_DECAY_PER_STEP -- ports `ei(self)`'s own literal
 * `p.l-=0.14` (pico8/lua.lua:702), a flat per-tick lifetime decay independent
 * of any per-type constant. 0.14*FX_ONE (9175.04) is NOT exactly
 * representable, so this truncates to 9175 -- the same double-to-fixed_t
 * truncation convention every other literal-constant macro in this file
 * already follows.
 */
#define EFFECTS_SPIRAL_LIFE_DECAY_PER_STEP ((fixed_t)(0.14 * (double)FX_ONE))

int effects_spiral_step_particle(effects_spiral_particle_t *p, fixed_t m00, fixed_t m01, fixed_t m10, fixed_t m11) {
    /* local pv=p.v (pico8/lua.lua:697): capture the OLD velocity before the
     * matrix update below overwrites it -- the position advance just below
     * uses these captured locals, not p->vx/p->vy re-read after the update,
     * preserving source's own exact evaluation order. */
    fixed_t old_vx = p->vx;
    fixed_t old_vy = p->vy;

    /* p.p+=pv (pico8/lua.lua:698): position advances by the OLD velocity. */
    p->x = fixed_add(p->x, old_vx);
    p->y = fixed_add(p->y, old_vy);

    /* p.v.x=qo*pv.x+ro*pv.y, p.v.y=qg*pv.x+q_*pv.y (pico8/lua.lua:699-700): a
     * general 2x2 linear transform of the OLD velocity -- see effects.h's
     * doc comment on this function for the m00/m01/m10/m11 mapping. */
    p->vx = fixed_add(fixed_mul(m00, old_vx), fixed_mul(m01, old_vy));
    p->vy = fixed_add(fixed_mul(m10, old_vx), fixed_mul(m11, old_vy));

    /* if (p.a)p.v+=p.a (pico8/lua.lua:701): optional acceleration, applied
     * AFTER the matrix transform above, only when has_accel is set -- a real
     * conditional, not dead code, per effects_spiral_particle_t's own
     * general-purpose struct design (see effects.h's doc comment). */
    if (p->has_accel) {
        p->vx = fixed_add(p->vx, p->ax);
        p->vy = fixed_add(p->vy, p->ay);
    }

    /* p.l-=0.14 (pico8/lua.lua:702): flat per-tick decay, independent of any
     * per-type constant. */
    p->life = fixed_sub(p->life, EFFECTS_SPIRAL_LIFE_DECAY_PER_STEP);

    /* if p.l<=1 then del(...) (pico8/lua.lua:703-704), inverted to "still
     * alive" -- NOT the <=0 threshold effects_flow_step_particle uses; see
     * effects.h's doc comment on this function for why. */
    return p->life > FX_ONE;
}

int effects_reveal_ramp_step(int *nh) {
    /* local jv=self.k_[self.nh] (pico8/lua.lua:669): read the CURRENT index
     * before it advances -- this is the value THIS call returns. */
    int index = *nh;

    /* if (self.nh<20)self.nh+=1 (pico8/lua.lua:670): advance, capped at 20
     * -- the increment is skipped once *nh already equals the cap, so it
     * never overshoots. */
    if (*nh < EFFECTS_SLIPWAY_REVEAL_RAMP_LENGTH) {
        *nh += 1;
    }

    return index;
}

/*
 * EFFECTS_POPUP_VELOCITY_Y -- ports `iv`'s own literal `v=(0,-0.34)`
 * (pico8/lua.lua:941-951, `iv`'s prototype default; see effects.h's doc
 * comment on effects_popup_t for the full excerpt): a fixed, never-changing
 * upward drift (PICO-8's y-axis points down, so a negative y velocity is
 * "up" on screen). 0.34*FX_ONE (22282.24) is NOT exactly representable, so
 * -0.34*(double)FX_ONE truncates to -22282 -- the same double-to-fixed_t
 * truncation convention every other literal-constant macro in this file
 * already follows, applied here to a negative literal: C's (fixed_t) cast
 * from a negative double truncates TOWARDS ZERO (drops the fractional part,
 * the same direction as this file's own positive-literal truncations
 * elsewhere), so this is exactly -22282, not -22283 (which a
 * floor-towards-negative-infinity truncation would instead produce).
 */
#define EFFECTS_POPUP_VELOCITY_Y ((fixed_t)(-0.34 * (double)FX_ONE))

/*
 * EFFECTS_POPUP_AGE_PER_STEP -- ports `iv:ph()`'s own literal `self.l+=0.1`
 * (pico8/lua.lua:941-951). 0.1*FX_ONE (6553.6) is NOT exactly
 * representable, so this truncates to 6553 -- the same convention
 * EFFECTS_KICK_DECAY_PER_STEP's identical `0.1` literal already follows
 * above, kept as its OWN separately-named macro (not shared with that one)
 * since `iv:ph`'s age timer and `ma()`'s kick-amplitude decay are two
 * independently-traced source formulas that merely happen to share the
 * same numeric literal -- matching this file's "one macro per traced
 * literal, tied to its own source citation" style elsewhere
 * (EFFECTS_PROBE_PULSE_RAMP_RECIP_15 vs EFFECTS_PROBE_SWEEP_RECIP_15's
 * identical precedent).
 */
#define EFFECTS_POPUP_AGE_PER_STEP ((fixed_t)(0.1 * (double)FX_ONE))

/*
 * EFFECTS_POPUP_DEATH_AGE -- ports `iv:ph()`'s own death threshold
 * `self.l>=5`: 5 full PICO-8 number-units, i.e. exactly `5*FX_ONE` in this
 * port's own "PICO-8's numbers are natively Q16.16" convention (matching
 * effects_flow_particle_t's own lifetime-literal precedent). Exact -- 5 has
 * no fractional part, so no truncation is involved in this one.
 */
#define EFFECTS_POPUP_DEATH_AGE (5 * FX_ONE)

void effects_popup_start(effects_popup_t *p, fixed_t x, fixed_t y) {
    p->x = x;
    p->y = y;
    p->vy = EFFECTS_POPUP_VELOCITY_Y;
    p->age = 0;
}

int effects_popup_step(effects_popup_t *p) {
    /* self.ra+=self.v (pico8/lua.lua:941-951): y only -- v has no x
     * component, matching source's own "no horizontal motion" literal. */
    p->y = fixed_add(p->y, p->vy);

    /* self.l+=0.1 */
    p->age = fixed_add(p->age, EFFECTS_POPUP_AGE_PER_STEP);

    /* self.p_=self.l>=5, inverted to "still alive". */
    return p->age < EFFECTS_POPUP_DEATH_AGE;
}

int effects_popup_fade_index(const effects_popup_t *p) {
    /* (int)(age>>16) -- the same truncate-to-integer idiom rules_planets_
     * job_cost's own (int)(cost>>16) already establishes (rules_planets.c).
     * age is always >=0 in this struct's own normal lifecycle (see
     * effects.h's doc comment on this function), so this plain arithmetic
     * right shift is an exact integer-part extraction here, not an
     * approximation. */
    int index = (int)(p->age >> 16);

    /* Lower clamp: ordinary defensive robustness, not a traced source path
     * (see effects.h's doc comment). Upper clamp: a real, exercised case --
     * age can legitimately reach or exceed 5*FX_ONE once this popup is
     * already dead. */
    if (index < 0) {
        index = 0;
    }
    if (index > 4) {
        index = 4;
    }

    return index;
}

/*
 * EFFECTS_GROWBAR_STEP -- ports `pj:kj`'s own literal `self.gz+=0.2`
 * (pico8/lua.lua:854). 0.2*FX_ONE (13107.2) is NOT exactly representable,
 * so this truncates to 13107 -- the same double-to-fixed_t truncation
 * convention every other literal-constant macro in this file already
 * follows (e.g. EFFECTS_KICK_DECAY_PER_STEP's identical `0.1` literal
 * above).
 */
#define EFFECTS_GROWBAR_STEP ((fixed_t)(0.2 * (double)FX_ONE))

/*
 * EFFECTS_GROWBAR_HEIGHT_CAP -- ports `pj:kj`'s own literal `min(self.gz,
 * 3.2)` (pico8/lua.lua:855). 3.2*FX_ONE (209715.2) is NOT exactly
 * representable, so this truncates to 209715 -- same convention as
 * EFFECTS_GROWBAR_STEP above.
 */
#define EFFECTS_GROWBAR_HEIGHT_CAP ((fixed_t)(3.2 * (double)FX_ONE))

fixed_t effects_growbar_step(effects_growbar_t *g) {
    /* self.gz+=0.2 (pico8/lua.lua:854): uncapped -- only the height formula
     * below clamps its USE of gz, not gz itself. */
    g->gz = fixed_add(g->gz, EFFECTS_GROWBAR_STEP);

    /* h=min(self.gz,3.2)^2 (pico8/lua.lua:855): clamp FIRST, then square via
     * fixed_mul -- matching source's own exact order. */
    fixed_t clamped = g->gz > EFFECTS_GROWBAR_HEIGHT_CAP ? EFFECTS_GROWBAR_HEIGHT_CAP : g->gz;
    return fixed_mul(clamped, clamped);
}
