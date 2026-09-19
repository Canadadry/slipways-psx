#ifndef FIXED_H
#define FIXED_H

#include <stdint.h>

typedef int32_t fixed_t;

#define FX_ONE ((fixed_t)(1 << 16))

static inline fixed_t fixed_add(fixed_t a, fixed_t b) {
    return a + b;
}

static inline fixed_t fixed_sub(fixed_t a, fixed_t b) {
    return a - b;
}

static inline fixed_t fixed_mul(fixed_t a, fixed_t b) {
    return (fixed_t)(((int64_t)a * (int64_t)b) >> 16);
}

static inline fixed_t fixed_from_double(double d) {
    return (fixed_t)(d * (double)FX_ONE);
}

static inline double fixed_to_double(fixed_t f) {
    return (double)f / (double)FX_ONE;
}

/*
 * fixed_sqrt: Q16.16 fixed-point square root.
 *
 * For x < 0, returns 0. This is a defensive convention for a general
 * arithmetic primitive (like fixed_mul), not a traced game-design source
 * value -- callers that can legitimately hit negative inputs are
 * responsible for deciding what that means for them.
 *
 * For x >= 0: a fixed_t value x represents the real number x / FX_ONE, so
 * the fixed_t representing sqrt(x / FX_ONE) satisfies the identity
 * sqrt(x / FX_ONE) * FX_ONE == isqrt(x * FX_ONE), where isqrt is an
 * integer square root. x is widened to int64_t before shifting left by 16
 * (i.e. multiplying by FX_ONE) since x can be as large as INT32_MAX and
 * x << 16 would overflow a 32-bit type. The integer square root itself is
 * computed with the standard bit-by-bit ("digit by digit") method, using
 * only integer arithmetic (no float/double), matching this codebase's
 * no-FPU convention. The result is truncated back down to fixed_t.
 */
static inline fixed_t fixed_sqrt(fixed_t x) {
    if (x < 0) {
        return 0;
    }

    uint64_t n = (uint64_t)((int64_t)x << 16);
    uint64_t result = 0;
    /* Highest power of 4 that fits in a 64-bit value, halved each step. */
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
 * fixed_sin / fixed_cos: Q16.16 fixed-point sine/cosine, PICO-8 convention.
 *
 * PICO-8's built-in sin()/cos() do not take radians: their argument is a
 * fraction of one full turn (0.0 to 1.0 == one full revolution), matching
 * this header's own "a fixed_t is naturally a 0..1-normalized quantity"
 * convention elsewhere (FX_ONE means "one whole unit" here exactly as it
 * does for every other fixed_t in this codebase -- for these two functions
 * specifically, "one whole unit" means "one whole turn").
 *
 * PICO-8's sin() is ALSO deliberately sign-flipped relative to the ordinary
 * mathematical convention: pico8_sin(x) == -sin(2*pi*x) in standard-math
 * terms, because PICO-8's screen y-axis points down and this flip keeps
 * "increasing angle" rotate the same visual direction a normal screen-space
 * caller expects. cos() has no such flip: pico8_cos(x) == cos(2*pi*x). This
 * is documented, stable PICO-8 API behavior (not something reverse-derived
 * from decompiled bytecode), and every angle-based formula this port traces
 * from source assumes it -- get it backwards here and every future caller
 * (kick wobble, idle bob, spiral particle engine, probe pulse, ...) silently
 * rotates/bobs the wrong way.
 *
 * Implementation: no runtime float/double (this codebase's hard no-FPU rule
 * for shipped C, matching fixed_sqrt's own integer-only posture above).
 * Instead of an iterative numeric method, this uses a precomputed lookup
 * table plus linear interpolation -- the standard "no hardware trig" answer
 * for a target (PS1 R3000A) that has none, and the posture docs/plan.md's
 * own "la trigo en virgule fixe reste necessaire" note calls for.
 * FIXED_QUARTER_SIN_TABLE holds 257 standard-math sin() samples evenly
 * spaced across exactly one quarter turn (turns 0.0 through 0.25 inclusive,
 * 256 equal steps of 1/1024 turn each), generated once, offline, by a
 * throwaway host-side C program (not shipped, not a runtime dependency --
 * see docs/prd/triage/psx-juice-effects.md, chunk 1). Every other quarter of
 * the full turn is derived from this one table by the standard sin/cos
 * quadrant-symmetry identities in fixed_std_sincos below, so only a quarter
 * of the wave needs to be stored.
 */
#define FIXED_QUARTER_TABLE_SIZE 256
#define FIXED_QUARTER_TURNS (FX_ONE / 4) /* 0.25 turn, in Q16.16 */

static const fixed_t FIXED_QUARTER_SIN_TABLE[FIXED_QUARTER_TABLE_SIZE + 1] = {
    0,     402,   804,   1206,  1608,  2010,  2412,  2814,
    3216,  3617,  4019,  4420,  4821,  5222,  5623,  6023,
    6424,  6824,  7224,  7623,  8022,  8421,  8820,  9218,
    9616,  10014, 10411, 10808, 11204, 11600, 11996, 12391,
    12785, 13180, 13573, 13966, 14359, 14751, 15143, 15534,
    15924, 16314, 16703, 17091, 17479, 17867, 18253, 18639,
    19024, 19409, 19792, 20175, 20557, 20939, 21320, 21699,
    22078, 22457, 22834, 23210, 23586, 23961, 24335, 24708,
    25080, 25451, 25821, 26190, 26558, 26925, 27291, 27656,
    28020, 28383, 28745, 29106, 29466, 29824, 30182, 30538,
    30893, 31248, 31600, 31952, 32303, 32652, 33000, 33347,
    33692, 34037, 34380, 34721, 35062, 35401, 35738, 36075,
    36410, 36744, 37076, 37407, 37736, 38064, 38391, 38716,
    39040, 39362, 39683, 40002, 40320, 40636, 40951, 41264,
    41576, 41886, 42194, 42501, 42806, 43110, 43412, 43713,
    44011, 44308, 44604, 44898, 45190, 45480, 45769, 46056,
    46341, 46624, 46906, 47186, 47464, 47741, 48015, 48288,
    48559, 48828, 49095, 49361, 49624, 49886, 50146, 50404,
    50660, 50914, 51166, 51417, 51665, 51911, 52156, 52398,
    52639, 52878, 53114, 53349, 53581, 53812, 54040, 54267,
    54491, 54714, 54934, 55152, 55368, 55582, 55794, 56004,
    56212, 56418, 56621, 56823, 57022, 57219, 57414, 57607,
    57798, 57986, 58172, 58356, 58538, 58718, 58896, 59071,
    59244, 59415, 59583, 59750, 59914, 60075, 60235, 60392,
    60547, 60700, 60851, 60999, 61145, 61288, 61429, 61568,
    61705, 61839, 61971, 62101, 62228, 62353, 62476, 62596,
    62714, 62830, 62943, 63054, 63162, 63268, 63372, 63473,
    63572, 63668, 63763, 63854, 63944, 64031, 64115, 64197,
    64277, 64354, 64429, 64501, 64571, 64639, 64704, 64766,
    64827, 64884, 64940, 64993, 65043, 65091, 65137, 65180,
    65220, 65259, 65294, 65328, 65358, 65387, 65413, 65436,
    65457, 65476, 65492, 65505, 65516, 65525, 65531, 65535,
    65536,
};

/*
 * fixed_std_sin_quarter: linear-interpolated lookup into
 * FIXED_QUARTER_SIN_TABLE. t must be in [0, FIXED_QUARTER_TURNS] (0 to a
 * quarter turn, inclusive); returns standard-math sin(2*pi*(t/FX_ONE)) in
 * Q16.16. Internal helper, not part of this header's public primitive set.
 */
static inline fixed_t fixed_std_sin_quarter(fixed_t t) {
    /* Each table step covers FIXED_QUARTER_TURNS / FIXED_QUARTER_TABLE_SIZE
     * == 16384/256 == 64 turns-units (a power of two), so both the index
     * and the between-samples fraction fall out of t via shift/mask instead
     * of a division. */
    int index = t >> 6;
    fixed_t frac_of_step = t & 63; /* 0..63: how far past table[index] */

    if (index >= FIXED_QUARTER_TABLE_SIZE) {
        /* t == FIXED_QUARTER_TURNS exactly: no table[index+1] to interpolate
         * toward (and none is needed -- this is the table's last sample). */
        return FIXED_QUARTER_SIN_TABLE[FIXED_QUARTER_TABLE_SIZE];
    }

    fixed_t lo = FIXED_QUARTER_SIN_TABLE[index];
    fixed_t hi = FIXED_QUARTER_SIN_TABLE[index + 1];
    /* frac_of_step is "out of 64 steps"; <<10 rescales it to a Q16.16
     * fraction in [0, FX_ONE) (64 * 1024 == FX_ONE == 65536), then the
     * usual fixed-point lerp blends the two neighboring samples. */
    fixed_t frac_q16 = frac_of_step << 10;
    return lo + fixed_mul(hi - lo, frac_q16);
}

/*
 * fixed_std_sincos: reduces an arbitrary turns value to [0, FX_ONE) (turns
 * wrap every full revolution, exactly like PICO-8's own sin()/cos(), which
 * accept any real argument, not just [0,1)) and folds it into whichever
 * quadrant FIXED_QUARTER_SIN_TABLE covers, via the standard sin/cos
 * quarter-turn symmetries (t always in [0, 0.25 turn)):
 *   quadrant 0, x = 0.00+t: sin(x) =  sin(t),      cos(x) =  sin(0.25-t)
 *   quadrant 1, x = 0.25+t: sin(x) =  sin(0.25-t), cos(x) = -sin(t)
 *   quadrant 2, x = 0.50+t: sin(x) = -sin(t),      cos(x) = -sin(0.25-t)
 *   quadrant 3, x = 0.75+t: sin(x) = -sin(0.25-t), cos(x) =  sin(t)
 * (cos(t) == sin(0.25-t) is just the ordinary complementary-angle identity,
 * so both come from fixed_std_sin_quarter and no separate cosine table is
 * needed). Writes STANDARD-math (unflipped) sin/cos to *out_sin and *out_cos;
 * fixed_sin/fixed_cos below apply PICO-8's sign convention on top of this
 * shared helper.
 */
static inline void fixed_std_sincos(fixed_t turns, fixed_t *out_sin, fixed_t *out_cos) {
    fixed_t wrapped = turns % FX_ONE;
    if (wrapped < 0) {
        wrapped += FX_ONE;
    }

    int quadrant = wrapped / FIXED_QUARTER_TURNS;
    fixed_t t = wrapped - quadrant * FIXED_QUARTER_TURNS; /* in [0, quarter) */

    fixed_t sin_t = fixed_std_sin_quarter(t);
    fixed_t cos_t = fixed_std_sin_quarter(FIXED_QUARTER_TURNS - t);

    switch (quadrant) {
        case 0:
            *out_sin = sin_t;
            *out_cos = cos_t;
            break;
        case 1:
            *out_sin = cos_t;
            *out_cos = -sin_t;
            break;
        case 2:
            *out_sin = -sin_t;
            *out_cos = -cos_t;
            break;
        default: /* quadrant 3 */
            *out_sin = -cos_t;
            *out_cos = sin_t;
            break;
    }
}

static inline fixed_t fixed_sin(fixed_t turns) {
    fixed_t s, c;
    fixed_std_sincos(turns, &s, &c);
    /* PICO-8's sin() sign flip -- see the block comment above. */
    return -s;
}

static inline fixed_t fixed_cos(fixed_t turns) {
    fixed_t s, c;
    fixed_std_sincos(turns, &s, &c);
    return c;
}

/*
 * fixed_polar: Q16.16 fixed-point polar-to-Cartesian conversion, ported
 * directly from PICO-8's qm(magnitude, turns) helper (pico8/lua.lua:124-126):
 *
 *   function qm(fy,lx)
 *   return v(cos(lx),sin(lx))*fy
 *   end
 *
 * i.e. qm(fy, lx) returns the vector (cos(lx), sin(lx)) scaled by fy, where
 * cos()/sin() are PICO-8's own turns-based, sign-flipped-sine trig functions
 * (see the fixed_sin/fixed_cos block comment above). fixed_polar is built
 * directly on fixed_sin/fixed_cos (not on standard-math sin/cos), so PICO-8's
 * sign convention carries through here automatically, exactly matching qm()'s
 * own behavior. magnitude and turns both follow this header's usual Q16.16
 * convention (turns: 0..FX_ONE == one full turn, same argument convention as
 * fixed_sin/fixed_cos above).
 *
 * Writes the resulting Cartesian offset to out_x/out_y -- an output-
 * parameter pair, matching fixed_std_sincos's own established
 * out_sin/out_cos convention above, rather than returning a struct this
 * header has no existing precedent for.
 *
 * Every angle-based juice effect this primitive is meant to support (kick
 * wobble, probe pulse/sweep, spiral particle engine spawn position/velocity,
 * slipway line wobble -- see docs/prd/triage/psx-juice-effects.md) is a
 * direct caller of this one function, exactly mirroring qm()'s own
 * single-shared-helper role in source.
 */
static inline void fixed_polar(fixed_t magnitude, fixed_t turns, fixed_t *out_x, fixed_t *out_y) {
    *out_x = fixed_mul(magnitude, fixed_cos(turns));
    *out_y = fixed_mul(magnitude, fixed_sin(turns));
}

#endif
