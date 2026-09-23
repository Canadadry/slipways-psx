#include "pico8_fixed.h"

#include <math.h>

pico8_fixed_t pico8_fixed_from_int(int32_t i) {
    return (pico8_fixed_t)((uint32_t)i << PICO8_FIXED_SHIFT);
}

/* TROU-1 */
pico8_fixed_t pico8_fixed_from_double(double d) {
    double scaled = d * 65536.0;
    int64_t raw = (int64_t)(scaled >= 0 ? scaled + 0.5 : scaled - 0.5);
    return (pico8_fixed_t)(uint32_t)raw;
}

double pico8_fixed_to_double(pico8_fixed_t x) {
    return (double)x / 65536.0;
}

/* Overflow wraps, two's-complement. Signed overflow is undefined behavior
 * in C, so the wrap is produced in the unsigned type and reinterpreted --
 * never by relying on int32_t overflow or on -fwrapv. */
pico8_fixed_t pico8_fixed_add(pico8_fixed_t a, pico8_fixed_t b) {
    return (pico8_fixed_t)((uint32_t)a + (uint32_t)b);
}

pico8_fixed_t pico8_fixed_sub(pico8_fixed_t a, pico8_fixed_t b) {
    return (pico8_fixed_t)((uint32_t)a - (uint32_t)b);
}

/* The truncation to 32 bits is where PICO-8's multiply overflow wraps. */
pico8_fixed_t pico8_fixed_mul(pico8_fixed_t a, pico8_fixed_t b) {
    int64_t product = (int64_t)a * (int64_t)b;
    int64_t rescaled = product >> PICO8_FIXED_SHIFT;
    return (pico8_fixed_t)(uint32_t)rescaled;
}

/* b == 0 saturates by the sign of the dividend; see pico8_fixed.h. */
pico8_fixed_t pico8_fixed_div(pico8_fixed_t a, pico8_fixed_t b) {
    if (b == 0) {
        return a < 0 ? PICO8_FIXED_DIV0_NEG : PICO8_FIXED_DIV0_POS;
    }
    /* Left-shifting a negative signed value is undefined behavior in C
     * (C11 6.5.7p4), unlike right-shifting one, which is merely
     * implementation-defined -- hence the detour through uint64_t. */
    int64_t numerator = (int64_t)((uint64_t)(int64_t)a << PICO8_FIXED_SHIFT);
    int64_t quotient = numerator / b;
    return (pico8_fixed_t)(uint32_t)quotient;
}

pico8_fixed_t pico8_fixed_neg(pico8_fixed_t x) {
    return (pico8_fixed_t)((uint32_t)0 - (uint32_t)x);
}

/* Toward negative infinity, not toward zero: raw -1 (0xFFFFFFFF, a tiny
 * negative fraction) masks to 0xFFFF0000, i.e. -1.0, not 0. */
pico8_fixed_t pico8_fixed_flr(pico8_fixed_t x) {
    return (pico8_fixed_t)((uint32_t)x & 0xFFFF0000u);
}

/* Divides instead of shifting: >> on a negative is implementation-defined
 * (C99 6.5.7p5), and the masked value is an exact multiple of 2^16, so the
 * division's rounding direction never comes into play. */
int32_t pico8_fixed_flr_int(pico8_fixed_t x) {
    return pico8_fixed_flr(x) / ((int32_t)1 << PICO8_FIXED_SHIFT);
}

pico8_fixed_t pico8_fixed_abs(pico8_fixed_t x) {
    return x < 0 ? pico8_fixed_neg(x) : x;
}

/* sgn(0) == 1, not 0. */
pico8_fixed_t pico8_fixed_sgn(pico8_fixed_t x) {
    return x < 0 ? pico8_fixed_from_int(-1) : pico8_fixed_from_int(1);
}

/* Signed comparison of the raw representation is already the right total
 * order over fixed-point values, so the median needs no conversion. */
pico8_fixed_t pico8_fixed_mid(pico8_fixed_t a, pico8_fixed_t b, pico8_fixed_t c) {
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
pico8_fixed_t pico8_fixed_band(pico8_fixed_t a, pico8_fixed_t b) {
    return a & b;
}

pico8_fixed_t pico8_fixed_bor(pico8_fixed_t a, pico8_fixed_t b) {
    return a | b;
}

pico8_fixed_t pico8_fixed_mod(pico8_fixed_t a, pico8_fixed_t b) {
    return pico8_fixed_sub(a, pico8_fixed_mul(pico8_fixed_flr(pico8_fixed_div(a, b)), b));
}

/* n outside [0,31] would be undefined behavior to shift by in C; 0 is this
 * port's own defensive answer, not sourced PICO-8 behavior. */
pico8_fixed_t pico8_fixed_shl(pico8_fixed_t x, int32_t n) {
    if (n < 0 || n >= 32) {
        return 0;
    }
    return (pico8_fixed_t)((uint32_t)x << (uint32_t)n);
}

/* Largest y such that y*y <= n, by binary search. y fits in 32 bits for
 * every n pico8_fixed_sqrt passes in. */
static uint32_t pico8_fixed_isqrt_u64(uint64_t n) {
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
 * sqrt(x_raw * 65536): one integer square root, no host float, and the
 * result truncates rather than rounding to the nearest value. */
pico8_fixed_t pico8_fixed_sqrt(pico8_fixed_t x) {
    if (x < 0) {
        return 0;
    }
    uint64_t scaled = (uint64_t)(uint32_t)x << PICO8_FIXED_SHIFT;
    return (pico8_fixed_t)pico8_fixed_isqrt_u64(scaled);
}

pico8_fixed_t pico8_fixed_pow075(pico8_fixed_t x) {
    pico8_fixed_t s = pico8_fixed_sqrt(x);
    return pico8_fixed_mul(s, pico8_fixed_sqrt(s));
}

/* Placeholder generator: a linear-congruential one with Numerical Recipes'
 * constants and a fixed seed. Not PICO-8's own, so no sequence matches the
 * real cart -- only rnd()'s documented range does. */
#define PICO8_RND_LCG_MULTIPLIER 1664525u
#define PICO8_RND_LCG_INCREMENT 1013904223u
#define PICO8_RND_DEFAULT_SEED 0x2545F491u

static uint32_t pico8_rnd_state = PICO8_RND_DEFAULT_SEED;

static uint32_t pico8_rnd_next_u32(void) {
    pico8_rnd_state = pico8_rnd_state * PICO8_RND_LCG_MULTIPLIER + PICO8_RND_LCG_INCREMENT;
    return pico8_rnd_state;
}

/* The raw 32-bit draw is read as a fraction of 2^32 and scaled by |x| in a
 * 64-bit intermediate, landing in [0, |x|) before x's sign is restored.
 * Negative x is not part of the documented contract; no call site uses it. */
pico8_fixed_t pico8_fixed_rnd(pico8_fixed_t x) {
    if (x == 0) {
        return 0;
    }
    uint32_t magnitude = (x < 0) ? (uint32_t)(-(int64_t)x) : (uint32_t)x;
    uint32_t draw = pico8_rnd_next_u32();
    uint64_t scaled = ((uint64_t)magnitude * (uint64_t)draw) >> 32;
    pico8_fixed_t result = (pico8_fixed_t)(uint32_t)scaled;
    return (x < 0) ? pico8_fixed_neg(result) : result;
}

pico8_fixed_t pico8_fixed_rnd_default(void) {
    return pico8_fixed_rnd(pico8_fixed_from_int(1));
}

/* Spelled out rather than <math.h>'s M_PI, which is an extension glibc
 * does not define under strict -std=c99. */
#define PICO8_TWO_PI 6.28318530717958647692528676655900576839433879875021

/* TROU-1 */
pico8_fixed_t pico8_fixed_cos(pico8_fixed_t x) {
    double turns = pico8_fixed_to_double(x);
    double radians = turns * PICO8_TWO_PI;
    return pico8_fixed_from_double(cos(radians));
}

/* Sign inverted relative to standard math: PICO-8's screen y-axis points
 * down. */
/* TROU-1 */
pico8_fixed_t pico8_fixed_sin(pico8_fixed_t x) {
    double turns = pico8_fixed_to_double(x);
    double radians = turns * PICO8_TWO_PI;
    return pico8_fixed_from_double(-sin(radians));
}
