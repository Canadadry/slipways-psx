#include "p8/fixed.h"
#include "p8/quarter_wave.h"

p8_fixed_t p8_fixed_from_int(int32_t i) {
    return (p8_fixed_t)((uint32_t)i << P8_FIXED_SHIFT);
}

/* Overflow wraps, two's-complement. Signed overflow is undefined behavior
 * in C, so the wrap is produced in the unsigned type and reinterpreted --
 * never by relying on int32_t overflow or on -fwrapv. */
p8_fixed_t p8_fixed_add(p8_fixed_t a, p8_fixed_t b) {
    return (p8_fixed_t)((uint32_t)a + (uint32_t)b);
}

p8_fixed_t p8_fixed_sub(p8_fixed_t a, p8_fixed_t b) {
    return (p8_fixed_t)((uint32_t)a - (uint32_t)b);
}

/* The truncation to 32 bits is where PICO-8's multiply overflow wraps. */
p8_fixed_t p8_fixed_mul(p8_fixed_t a, p8_fixed_t b) {
    int64_t product = (int64_t)a * (int64_t)b;
    int64_t rescaled = product >> P8_FIXED_SHIFT;
    return (p8_fixed_t)(uint32_t)rescaled;
}

/* A zero divisor saturates by the sign of the dividend; see p8/fixed.h. */
p8_fixed_t p8_fixed_div(p8_fixed_t a, p8_fixed_t b) {
    if (b == 0) {
        return a < 0 ? P8_FIXED_DIV0_NEG : P8_FIXED_DIV0_POS;
    }
    /* Left-shifting a negative signed value is undefined behavior in C
     * (C11 6.5.7p4), unlike right-shifting one, which is merely
     * implementation-defined -- hence the detour through uint64_t. */
    int64_t numerator = (int64_t)((uint64_t)(int64_t)a << P8_FIXED_SHIFT);
    int64_t quotient = numerator / b;
    return (p8_fixed_t)(uint32_t)quotient;
}

p8_fixed_t p8_fixed_neg(p8_fixed_t x) {
    return (p8_fixed_t)((uint32_t)0 - (uint32_t)x);
}

/* Toward negative infinity, not toward zero: raw -1 (0xFFFFFFFF, a tiny
 * negative fraction) masks to 0xFFFF0000, i.e. -1.0, not 0. */
p8_fixed_t p8_fixed_flr(p8_fixed_t x) {
    return (p8_fixed_t)((uint32_t)x & 0xFFFF0000u);
}

/* Divides instead of shifting: >> on a negative is implementation-defined
 * (C99 6.5.7p5), and the masked value is an exact multiple of 2^16, so the
 * division's rounding direction never comes into play. */
int32_t p8_fixed_flr_int(p8_fixed_t x) {
    return p8_fixed_flr(x) / ((int32_t)1 << P8_FIXED_SHIFT);
}

p8_fixed_t p8_fixed_abs(p8_fixed_t x) {
    return x < 0 ? p8_fixed_neg(x) : x;
}

/* sgn(0) == 1, not 0. */
p8_fixed_t p8_fixed_sgn(p8_fixed_t x) {
    return x < 0 ? p8_fixed_from_int(-1) : p8_fixed_from_int(1);
}

/* Signed comparison of the raw representation is already the right total
 * order over fixed-point values, so the median needs no conversion. */
p8_fixed_t p8_fixed_mid(p8_fixed_t a, p8_fixed_t b, p8_fixed_t c) {
    if ((a <= b && b <= c) || (c <= b && b <= a)) {
        return b;
    }
    if ((b <= a && a <= c) || (c <= a && a <= b)) {
        return a;
    }
    return c;
}

/* Bitwise AND/OR are well defined on signed integers in C whatever the
 * sign, unlike shifts, so no unsigned detour here. */
p8_fixed_t p8_fixed_band(p8_fixed_t a, p8_fixed_t b) {
    return a & b;
}

p8_fixed_t p8_fixed_bor(p8_fixed_t a, p8_fixed_t b) {
    return a | b;
}

p8_fixed_t p8_fixed_mod(p8_fixed_t a, p8_fixed_t b) {
    return p8_fixed_sub(a, p8_fixed_mul(p8_fixed_flr(p8_fixed_div(a, b)), b));
}

/* A count outside [0,31] would be undefined behavior to shift by in C; 0
 * is this port's own defensive answer, not sourced PICO-8 behavior. */
p8_fixed_t p8_fixed_shl(p8_fixed_t x, int32_t n) {
    if (n < 0 || n >= 32) {
        return 0;
    }
    return (p8_fixed_t)((uint32_t)x << (uint32_t)n);
}

/* Largest root whose square is at most the argument, by binary search;
 * it fits in 32 bits for every argument p8_fixed_sqrt passes in. */
static uint32_t isqrt_u64(uint64_t n) {
    uint64_t lo = 0;
    uint64_t hi = 0xFFFFFFFFu;
    while (lo < hi) {
        uint64_t mid = lo + (hi - lo + 1) / 2;
        if (mid * mid <= n) {
            lo = mid;
        } else {
            hi = mid - 1;
        }
    }
    return (uint32_t)lo;
}

/* x's raw bits are x/65536, so the raw bits of sqrt(x) are
 * sqrt(x_raw * 65536): one integer square root, and the result truncates
 * rather than rounding to the nearest value. */
p8_fixed_t p8_fixed_sqrt(p8_fixed_t x) {
    if (x < 0) {
        return 0;
    }
    uint64_t scaled = (uint64_t)(uint32_t)x << P8_FIXED_SHIFT;
    return (p8_fixed_t)isqrt_u64(scaled);
}

p8_fixed_t p8_fixed_pow075(p8_fixed_t x) {
    p8_fixed_t s = p8_fixed_sqrt(x);
    return p8_fixed_mul(s, p8_fixed_sqrt(s));
}

/* Placeholder generator: a linear-congruential one with Numerical Recipes'
 * constants and a fixed seed. Not PICO-8's own, so no sequence matches a
 * real PICO-8 -- only rnd()'s documented range does. */
#define P8_RND_LCG_MULTIPLIER 1664525u
#define P8_RND_LCG_INCREMENT 1013904223u
#define P8_RND_DEFAULT_SEED 0x2545F491u

static uint32_t rnd_state = P8_RND_DEFAULT_SEED;

static uint32_t rnd_next_u32(void) {
    rnd_state = rnd_state * P8_RND_LCG_MULTIPLIER + P8_RND_LCG_INCREMENT;
    return rnd_state;
}

/* The raw 32-bit draw is read as a fraction of 2^32 and scaled by |x| in a
 * 64-bit intermediate, landing in [0, |x|) before x's sign is restored.
 * Negative x is not part of the documented contract; no call site uses it. */
p8_fixed_t p8_fixed_rnd(p8_fixed_t x) {
    if (x == 0) {
        return 0;
    }
    uint32_t magnitude = (x < 0) ? (uint32_t)(-(int64_t)x) : (uint32_t)x;
    uint32_t draw = rnd_next_u32();
    uint64_t scaled = ((uint64_t)magnitude * (uint64_t)draw) >> 32;
    p8_fixed_t result = (p8_fixed_t)(uint32_t)scaled;
    return (x < 0) ? p8_fixed_neg(result) : result;
}

p8_fixed_t p8_fixed_rnd_default(void) {
    return p8_fixed_rnd(p8_fixed_from_int(1));
}

/* Standard-sign sin of the turn fraction (the low 16 bits of the argument):
 * the quadrant folds it onto the quarter wave and sets the sign. */
static p8_fixed_t quarter_wave_at(uint32_t f) {
    uint32_t quadrant = (f >> 14) & 3u;
    uint32_t r = f & 0x3FFFu;
    int32_t v = (quadrant & 1u) ? p8_quarter_wave[0x4000u - r] : p8_quarter_wave[r];
    return (quadrant & 2u) ? -v : v;
}

/* GAP-1 */
p8_fixed_t p8_fixed_cos(p8_fixed_t x) {
    return quarter_wave_at(((uint32_t)x + 0x4000u) & 0xFFFFu);
}

/* Sign inverted relative to standard math: PICO-8's screen y-axis points
 * down. */
/* GAP-1 */
p8_fixed_t p8_fixed_sin(p8_fixed_t x) {
    return -quarter_wave_at((uint32_t)x & 0xFFFFu);
}
