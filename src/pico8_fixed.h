#ifndef PICO8_FIXED_H
#define PICO8_FIXED_H

#include <stdint.h>

/* PICO-8's 16.16 fixed-point number: a signed 32-bit integer whose bits
 * 31-16 are the two's-complement integer part and bits 15-0 are the
 * fractional part as sixteenths of 65536, matching the documented range
 * -32768.0 to 32767.9999847412109375.
 * Source: PICO-8 Math wiki, https://pico-8.fandom.com/wiki/Math */
typedef int32_t pico8_fixed_t;

#define PICO8_FIXED_SHIFT 16

/* Saturation results for division by zero: +0x7fff.ffff / -0x7fff.ffff,
 * per the PICO-8 manual (see pico8_fixed_div below). */
#define PICO8_FIXED_DIV0_POS ((pico8_fixed_t)0x7FFFFFFF)
#define PICO8_FIXED_DIV0_NEG ((pico8_fixed_t)(-0x7FFFFFFF))

/* Constructs a fixed-point value from a plain integer (i << 16). */
pico8_fixed_t pico8_fixed_from_int(int32_t i);

/* Constructs a fixed-point value from a double, rounding to the nearest
 * representable 16.16 value. Test/debugging convenience for writing
 * expected values with a fractional part; PICO-8 itself has no floating
 * point and never goes through this constructor. */
pico8_fixed_t pico8_fixed_from_double(double d);

/* Converts a fixed-point value back to a double, for assertions/printing. */
double pico8_fixed_to_double(pico8_fixed_t x);

/* + and - are plain 32-bit addition/subtraction. PICO-8 documents that
 * overflow wraps (two's-complement), confirmed for the classic
 * 128*128+128*128 overflow-to--32768 case.
 * Source: PICO-8 Known Bugs wiki, https://pico-8.fandom.com/wiki/Known_Bugs
 * Because signed-integer overflow is undefined behavior in C, the
 * wraparound is produced by computing in the corresponding unsigned 32-bit
 * type (whose overflow behavior C does define) and reinterpreting the bit
 * pattern back as signed -- never by relying on int32_t overflow directly
 * or on a compiler flag like -fwrapv. */
pico8_fixed_t pico8_fixed_add(pico8_fixed_t a, pico8_fixed_t b);
pico8_fixed_t pico8_fixed_sub(pico8_fixed_t a, pico8_fixed_t b);

/* Computes the product in a 64-bit signed intermediate (a*b as raw 32-bit
 * integers, i.e. the numerator scaled by 65536^2), shifts right by 16 to
 * rescale back to the 16.16 format, and truncates to 32 bits -- the
 * truncation is itself where PICO-8's documented multiply overflow/
 * wraparound occurs. */
pico8_fixed_t pico8_fixed_mul(pico8_fixed_t a, pico8_fixed_t b);

/* Computes ((int64_t)a << 16) / b and truncates to 32 bits the same way,
 * except when b == 0: PICO-8's manual documents this special case
 * explicitly as saturating to 0x7fff.ffff if the dividend is non-negative,
 * -0x7fff.ffff if negative.
 * Source: PICO-8 manual, https://www.lexaloffle.com/pico8_manual.txt
 * 0 divided by 0 is treated as non-negative, saturating to +0x7fff.ffff,
 * since it isn't separately documented and 0 is not negative. */
pico8_fixed_t pico8_fixed_div(pico8_fixed_t a, pico8_fixed_t b);

/* Unary - is 0 - x under the same wraparound rule as + and -, so negating
 * the most-negative representable value wraps rather than overflowing. */
pico8_fixed_t pico8_fixed_neg(pico8_fixed_t x);

/* flr truncates toward negative infinity on the fixed value (not toward
 * zero): clearing the low 16 (fractional) bits of the two's-complement raw
 * representation does exactly this for both signs, with no shift needed.
 * Source: PICO-8 Math wiki, https://pico-8.fandom.com/wiki/Math */
pico8_fixed_t pico8_fixed_flr(pico8_fixed_t x);

/* abs is the fixed-point absolute value, with the same two's-complement
 * caveat as pico8_fixed_neg: the most-negative representable value's abs
 * wraps rather than growing, matching +/-'s own overflow rule.
 * Source: PICO-8 Math wiki, https://pico-8.fandom.com/wiki/Math */
pico8_fixed_t pico8_fixed_abs(pico8_fixed_t x);

/* sgn(x) returns 1 for x >= 0 and -1 for x < 0 -- sgn(0) == 1, not 0.
 * Source: PICO-8 Sgn wiki, https://pico-8.fandom.com/wiki/Sgn */
pico8_fixed_t pico8_fixed_sgn(pico8_fixed_t x);

/* mid(a,b,c) returns the median of the three arguments, so mid(lo,hi,v)
 * clamps v into [lo,hi] regardless of which of the three call-site
 * conventions (lo,hi,v)/(v,lo,hi)/etc. is used.
 * Source: PICO-8 manual, https://www.lexaloffle.com/pico8_manual.txt */
pico8_fixed_t pico8_fixed_mid(pico8_fixed_t a, pico8_fixed_t b, pico8_fixed_t c);

/* band/bor are bitwise AND/OR on the raw 32-bit fixed representation, not
 * on the "human" integer value.
 * Source: PICO-8 Band wiki, https://pico-8.fandom.com/wiki/Band
 * Source: PICO-8 Bor wiki, https://pico-8.fandom.com/wiki/Bor */
pico8_fixed_t pico8_fixed_band(pico8_fixed_t a, pico8_fixed_t b);
pico8_fixed_t pico8_fixed_bor(pico8_fixed_t a, pico8_fixed_t b);

/* shl(x,n) shifts the raw 32-bit representation left by n bits. n is a
 * plain shift count (not itself a fixed-point value), matching this
 * cart's own use at shl(flr(no),4). n outside [0,31] is not documented by
 * the source below; this port defensively returns 0 for such n rather
 * than invoking undefined behavior from an out-of-width/negative C shift
 * -- an unconfirmed defensive choice, not sourced PICO-8 behavior.
 * Source: PICO-8 Shl wiki, https://pico-8.fandom.com/wiki/Shl */
pico8_fixed_t pico8_fixed_shl(pico8_fixed_t x, int32_t n);

/* sqrt(x) for x < 0 returns 0 (as of PICO-8 0.2.6b). For x >= 0, returns
 * the largest fixed-point value whose square does not exceed x (i.e.
 * truncates rather than rounds), computed via an integer square root on
 * the raw representation to avoid a host floating-point dependency --
 * this truncation direction is a reasonable, but not separately sourced,
 * choice, consistent with this module's other truncating operations.
 * Source: PICO-8 Sqrt wiki, https://pico-8.fandom.com/wiki/Sqrt */
pico8_fixed_t pico8_fixed_sqrt(pico8_fixed_t x);

/* rnd/sin/cos -- documented CONTRACT ONLY, bit-exact algorithm NOT
 * reproduced. PICO-8 is closed-source; no authoritative source for the
 * exact internal PRNG state-update formula or the exact sin/cos
 * approximation was reachable from this sandbox (searched: the Lexaloffle
 * forums, the PICO-8 wiki, and the official manual -- all document
 * behavior/contract, none publish the internal algorithm). What follows
 * satisfies the documented contract (range/domain, sign convention,
 * documented zero/identity cases) with a placeholder deterministic PRNG
 * and the host's standard trig functions -- it is NOT claimed to match
 * real PICO-8 bit-for-bit, and docs/plan.md's "same seed -> same galaxy"
 * fidelity goal does not hold for these three functions yet. This is a
 * flagged open risk (see docs/plan.md's "Notes de portage"), to be
 * revisited if a reliable PICO-8 reference (a legally-obtained binary to
 * trace, or an independently-verified community reimplementation) ever
 * becomes available.
 * Source (contract only): PICO-8 manual, https://www.lexaloffle.com/pico8_manual.txt */

/* rnd(x) returns a pseudo-random value in [0, x) for x > 0, and 0 for
 * x == 0. Driven by a small internal linear-congruential generator seeded
 * at a fixed default -- a placeholder, not PICO-8's real PRNG. */
pico8_fixed_t pico8_fixed_rnd(pico8_fixed_t x);

/* rnd() with no argument behaves as rnd(1). */
pico8_fixed_t pico8_fixed_rnd_default(void);

/* cos(x): x is in "turns" (0.0-1.0 maps to a full 0-2*pi rotation).
 * Follows the standard mathematical convention (no sign inversion). */
pico8_fixed_t pico8_fixed_cos(pico8_fixed_t x);

/* sin(x): x is in "turns", same convention as cos above, but the result's
 * sign is inverted relative to standard math, to account for PICO-8's
 * y-axis increasing downward on screen. */
pico8_fixed_t pico8_fixed_sin(pico8_fixed_t x);

#endif
