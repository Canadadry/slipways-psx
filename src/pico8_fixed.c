#include "pico8_fixed.h"

#include <math.h>

/* 16.16 fixed-point: bits 31-16 two's-complement integer part, bits 15-0
 * fractional part in sixteenths of 65536.
 * Source: PICO-8 Math wiki, https://pico-8.fandom.com/wiki/Math */
pico8_fixed_t pico8_fixed_from_int(int32_t i) {
    return (pico8_fixed_t)((uint32_t)i << PICO8_FIXED_SHIFT);
}

/* Test/debugging convenience only -- PICO-8 itself has no floating point
 * and never goes through this constructor. */
pico8_fixed_t pico8_fixed_from_double(double d) {
    double scaled = d * 65536.0;
    int64_t raw = (int64_t)(scaled >= 0 ? scaled + 0.5 : scaled - 0.5);
    return (pico8_fixed_t)(uint32_t)raw;
}

double pico8_fixed_to_double(pico8_fixed_t x) {
    return (double)x / 65536.0;
}

/* PICO-8 documents that +/- overflow wraps (two's-complement), confirmed
 * for the classic 128*128+128*128 overflow-to--32768 case.
 * Source: PICO-8 Known Bugs wiki, https://pico-8.fandom.com/wiki/Known_Bugs
 * Signed-integer overflow is undefined behavior in C, so the wraparound is
 * produced by computing in unsigned 32-bit arithmetic (whose overflow
 * behavior C does define) and reinterpreting the bit pattern back as
 * signed -- never by relying on int32_t overflow directly or on a
 * compiler flag like -fwrapv. */
pico8_fixed_t pico8_fixed_add(pico8_fixed_t a, pico8_fixed_t b) {
    return (pico8_fixed_t)((uint32_t)a + (uint32_t)b);
}

pico8_fixed_t pico8_fixed_sub(pico8_fixed_t a, pico8_fixed_t b) {
    return (pico8_fixed_t)((uint32_t)a - (uint32_t)b);
}

/* Computes the product in a 64-bit signed intermediate (numerator scaled
 * by 65536^2), shifts right by 16 to rescale back to 16.16, and truncates
 * to 32 bits -- the truncation is itself where PICO-8's documented
 * multiply overflow/wraparound occurs. The 64-bit-intermediate
 * shift-then-truncate direction is standard 16.16 practice, not itself
 * separately confirmed against an authoritative PICO-8 source beyond the
 * two documented endpoint behaviors (overflow wraps; divide-by-zero
 * saturates, see pico8_fixed_div below) -- flagged as an assumption. */
pico8_fixed_t pico8_fixed_mul(pico8_fixed_t a, pico8_fixed_t b) {
    int64_t product = (int64_t)a * (int64_t)b;
    int64_t rescaled = product >> PICO8_FIXED_SHIFT;
    return (pico8_fixed_t)(uint32_t)rescaled;
}

/* Computes ((int64_t)a << 16) / b and truncates to 32 bits the same way
 * pico8_fixed_mul does, except when b == 0: the PICO-8 manual documents
 * this special case explicitly as saturating to 0x7fff.ffff if the
 * dividend is non-negative, -0x7fff.ffff if negative.
 * Source: PICO-8 manual, https://www.lexaloffle.com/pico8_manual.txt
 * 0 divided by 0 is treated as non-negative, saturating to +0x7fff.ffff,
 * since it isn't separately documented and 0 is not negative. */
pico8_fixed_t pico8_fixed_div(pico8_fixed_t a, pico8_fixed_t b) {
    if (b == 0) {
        return a < 0 ? PICO8_FIXED_DIV0_NEG : PICO8_FIXED_DIV0_POS;
    }
    /* Left-shifting a negative signed value is undefined behavior in C
     * (C11 6.5.7p4), unlike right-shift of a negative value, which is only
     * implementation-defined (6.5.7p5). As with pico8_fixed_add/sub/neg
     * above, the numerator is computed via the corresponding unsigned type
     * (where left-shift is always well-defined, reduced modulo 2^64) and
     * reinterpreted back as signed only afterward, for the same bit
     * pattern the previous signed-shift codegen already produced. */
    int64_t numerator = (int64_t)((uint64_t)(int64_t)a << PICO8_FIXED_SHIFT);
    int64_t quotient = numerator / b;
    return (pico8_fixed_t)(uint32_t)quotient;
}

/* Unary - is 0 - x under the same wraparound rule as +/-, so negating the
 * most-negative representable value wraps rather than overflowing. */
pico8_fixed_t pico8_fixed_neg(pico8_fixed_t x) {
    return (pico8_fixed_t)((uint32_t)0 - (uint32_t)x);
}

/* flr truncates toward negative infinity on the fixed value. Clearing the
 * low 16 (fractional) bits of the two's-complement raw representation
 * does exactly this for both signs -- e.g. raw -1 (0xFFFFFFFF, a tiny
 * negative fraction just below 0) masks to 0xFFFF0000 (-1.0), not 0
 * (which truncation-toward-zero would give).
 * Source: PICO-8 Math wiki, https://pico-8.fandom.com/wiki/Math */
pico8_fixed_t pico8_fixed_flr(pico8_fixed_t x) {
    return (pico8_fixed_t)((uint32_t)x & 0xFFFF0000u);
}

/* abs is the fixed-point absolute value. Built on pico8_fixed_neg so it
 * inherits the same two's-complement wraparound: negating the
 * most-negative representable value wraps back to itself rather than
 * growing, matching +/-'s own overflow rule.
 * Source: PICO-8 Math wiki, https://pico-8.fandom.com/wiki/Math */
pico8_fixed_t pico8_fixed_abs(pico8_fixed_t x) {
    return x < 0 ? pico8_fixed_neg(x) : x;
}

/* sgn(x) returns 1 for x >= 0 and -1 for x < 0. sgn(0) == 1, not 0.
 * Source: PICO-8 Sgn wiki, https://pico-8.fandom.com/wiki/Sgn */
pico8_fixed_t pico8_fixed_sgn(pico8_fixed_t x) {
    return x < 0 ? pico8_fixed_from_int(-1) : pico8_fixed_from_int(1);
}

/* mid(a,b,c) returns the median of the three arguments, so mid(lo,hi,v)
 * clamps v into [lo,hi] regardless of which of the three call-site
 * conventions is used. Ordinary signed comparison of the raw
 * representation is a correct total order over fixed-point values, so no
 * conversion is needed.
 * Source: PICO-8 manual, https://www.lexaloffle.com/pico8_manual.txt */
pico8_fixed_t pico8_fixed_mid(pico8_fixed_t a, pico8_fixed_t b, pico8_fixed_t c) {
    if ((a <= b && b <= c) || (c <= b && b <= a)) {
        return b;
    }
    if ((b <= a && a <= c) || (c <= a && a <= b)) {
        return a;
    }
    return c;
}

/* band/bor are bitwise AND/OR on the raw 32-bit fixed representation, not
 * on the "human" integer value. Bitwise AND/OR are well-defined on signed
 * integers in C regardless of sign (unlike shift), so no unsigned
 * reinterpretation trick is needed here.
 * Source: PICO-8 Band wiki, https://pico-8.fandom.com/wiki/Band
 * Source: PICO-8 Bor wiki, https://pico-8.fandom.com/wiki/Bor */
pico8_fixed_t pico8_fixed_band(pico8_fixed_t a, pico8_fixed_t b) {
    return a & b;
}

pico8_fixed_t pico8_fixed_bor(pico8_fixed_t a, pico8_fixed_t b) {
    return a | b;
}

/* a-flr(a/b)*b -- see pico8_fixed.h's own comment on pico8_fixed_mod for
 * why this is a new Chunk 15 primitive, not a Phase 2 oversight, and why
 * b==0 is left to whatever pico8_fixed_div's own documented saturation
 * produces rather than a second special case. */
pico8_fixed_t pico8_fixed_mod(pico8_fixed_t a, pico8_fixed_t b) {
    return pico8_fixed_sub(a, pico8_fixed_mul(pico8_fixed_flr(pico8_fixed_div(a, b)), b));
}

/* shl(x,n) shifts the raw 32-bit representation left by n bits.
 * Left-shifting a negative signed value directly is undefined behavior in
 * C (C11 6.5.7p4), so -- as with pico8_fixed_add/sub/neg/div above -- x is
 * reinterpreted as unsigned first (where left-shift is always
 * well-defined, reduced modulo 2^32) and the result reinterpreted back as
 * signed. n outside [0,31] is also UB to shift by in C (regardless of the
 * shifted value's signedness) and is not documented for this function, so
 * it's defensively clamped to 0 rather than risking UB -- an unconfirmed
 * defensive choice, not sourced PICO-8 behavior.
 * Source: PICO-8 Shl wiki, https://pico-8.fandom.com/wiki/Shl */
pico8_fixed_t pico8_fixed_shl(pico8_fixed_t x, int32_t n) {
    if (n < 0 || n >= 32) {
        return 0;
    }
    return (pico8_fixed_t)((uint32_t)x << (uint32_t)n);
}

/* Integer square root (floor) of a 64-bit value via binary search: the
 * largest y such that y*y <= n. y fits in 32 bits for every n this module
 * calls it with (see pico8_fixed_sqrt), and doing this in integer
 * arithmetic avoids a host floating-point dependency. */
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

/* sqrt(x) for x < 0 returns 0.
 * Source: PICO-8 Sqrt wiki, https://pico-8.fandom.com/wiki/Sqrt
 * For x >= 0: x's raw representation is x_raw/65536 as a real number, so
 * sqrt(x) = sqrt(x_raw/65536), and the *raw* representation of that
 * result is sqrt(x_raw/65536)*65536 = sqrt(x_raw*65536). x_raw*65536 is
 * computed in a 64-bit unsigned intermediate (x is known non-negative
 * here, so reinterpreting it as uint32_t first and widening is safe and
 * avoids ever left-shifting a signed value), and its integer square root
 * is the answer's raw bits directly. This truncates (floors) rather than
 * rounds to the nearest representable fixed-point value -- a reasonable
 * but not separately sourced choice, consistent with this module's other
 * truncating operations. */
pico8_fixed_t pico8_fixed_sqrt(pico8_fixed_t x) {
    if (x < 0) {
        return 0;
    }
    uint64_t scaled = (uint64_t)(uint32_t)x << PICO8_FIXED_SHIFT;
    return (pico8_fixed_t)pico8_fixed_isqrt_u64(scaled);
}

/* rnd/sin/cos -- documented CONTRACT ONLY, bit-exact algorithm NOT
 * reproduced. See the header's comment above these declarations for the
 * full explanation of this gap; it is deliberately not repeated here in
 * full to avoid drifting out of sync with two copies of the same caveat. */

/* Placeholder PRNG: a small linear-congruential generator (Numerical
 * Recipes' constants), seeded at a fixed default. This is NOT PICO-8's
 * real internal PRNG -- just any reasonable, simple, deterministic
 * generator satisfying rnd()'s documented range contract. */
#define PICO8_RND_LCG_MULTIPLIER 1664525u
#define PICO8_RND_LCG_INCREMENT 1013904223u
#define PICO8_RND_DEFAULT_SEED 0x2545F491u

static uint32_t pico8_rnd_state = PICO8_RND_DEFAULT_SEED;

static uint32_t pico8_rnd_next_u32(void) {
    pico8_rnd_state = pico8_rnd_state * PICO8_RND_LCG_MULTIPLIER + PICO8_RND_LCG_INCREMENT;
    return pico8_rnd_state;
}

/* rnd(x): 0 for x == 0 (documented special case); otherwise a value in
 * [0, x) for x > 0 (the only case PICO-8's documented contract and this
 * project's own call sites need). The generator's raw 32-bit output is
 * treated as a fraction in [0, 1) (numerator / 2^32) and scaled by x's raw
 * magnitude in a 64-bit intermediate, landing the result's raw fixed-point
 * bits in [0, |x|) before restoring x's sign -- this sign handling
 * generalizes to negative x but is not itself part of the documented
 * contract this phase tests against. */
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

/* Two*pi, spelled out rather than relying on <math.h>'s M_PI: M_PI is a
 * common extension, not standard C99, and is not defined by glibc's
 * math.h under strict -std=c99 (confirmed while implementing this
 * function -- without a GNU/POSIX feature-test macro, M_PI is simply
 * absent, not merely a warning). */
#define PICO8_TWO_PI 6.28318530717958647692528676655900576839433879875021

/* cos(x): x in "turns" (0.0-1.0 == a full 0-2*pi rotation), standard
 * mathematical convention -- computed via the host's own cos() over the
 * turns-to-radians conversion. */
pico8_fixed_t pico8_fixed_cos(pico8_fixed_t x) {
    double turns = pico8_fixed_to_double(x);
    double radians = turns * PICO8_TWO_PI;
    return pico8_fixed_from_double(cos(radians));
}

/* sin(x): same turns convention as cos above, but the result's sign is
 * inverted relative to standard math, per PICO-8's documented convention
 * (screen y-axis increases downward). */
pico8_fixed_t pico8_fixed_sin(pico8_fixed_t x) {
    double turns = pico8_fixed_to_double(x);
    double radians = turns * PICO8_TWO_PI;
    return pico8_fixed_from_double(-sin(radians));
}
