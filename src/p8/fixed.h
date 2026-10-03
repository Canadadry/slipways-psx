#ifndef P8_FIXED_H
#define P8_FIXED_H

#include <stdint.h>

/* PICO-8's 16.16 fixed-point number: bits 31-16 are the two's-complement
 * integer part, bits 15-0 the fraction, so the range is -32768.0 to
 * 32767.9999847412109375. */
typedef int32_t p8_fixed_t;

#define P8_FIXED_SHIFT 16

/* What a division by zero saturates to: +0x7fff.ffff / -0x7fff.ffff. See
 * p8_fixed_div. */
#define P8_FIXED_DIV0_POS ((p8_fixed_t)0x7FFFFFFF)
#define P8_FIXED_DIV0_NEG ((p8_fixed_t)(-0x7FFFFFFF))

/* First argument: the 8 hex digits of block A of
 * tools/pico8_reference_probe_out.txt, point removed. Second: the decimal
 * literal as the source writes it, for grep and tools/constants_compare.sh
 * -- never evaluated. */
#define P8_FIXED_LIT(raw, decimal) ((p8_fixed_t)(uint32_t)(raw##u))

p8_fixed_t p8_fixed_from_int(int32_t i);

/* Overflow wraps, two's-complement, as PICO-8 documents. Computed through
 * the unsigned 32-bit type, since signed overflow is undefined behavior in
 * C -- never by relying on int32_t overflow or on -fwrapv. */
p8_fixed_t p8_fixed_add(p8_fixed_t a, p8_fixed_t b);
p8_fixed_t p8_fixed_sub(p8_fixed_t a, p8_fixed_t b);

/* Product in a 64-bit intermediate, shifted back down by 16 and truncated
 * to 32 bits; that truncation is where PICO-8's multiply overflow wraps. */
p8_fixed_t p8_fixed_mul(p8_fixed_t a, p8_fixed_t b);

/* The dividend shifted left by 16, divided by the divisor, truncated the
 * same way, except for a zero divisor: the PICO-8 manual documents that as
 * saturating to +0x7fff.ffff for a non-negative dividend, -0x7fff.ffff for
 * a negative one. 0/0 counts as non-negative and saturates positive. */
p8_fixed_t p8_fixed_div(p8_fixed_t a, p8_fixed_t b);

/* 0 - x, under the same wraparound as + and -, so negating the
 * most-negative value wraps back to itself. */
p8_fixed_t p8_fixed_neg(p8_fixed_t x);

/* flr truncates toward negative infinity, not toward zero: raw -1 (a tiny
 * negative fraction) floors to -1.0, not to 0. */
p8_fixed_t p8_fixed_flr(p8_fixed_t x);

/* flr(x) as a plain integer, rounded toward negative infinity, not toward
 * zero: raw -1 gives -1, not 0. */
int32_t p8_fixed_flr_int(p8_fixed_t x);

/* Inherits p8_fixed_neg's wraparound: abs of the most-negative value is
 * itself. */
p8_fixed_t p8_fixed_abs(p8_fixed_t x);

/* sgn(0) == 1, not 0. */
p8_fixed_t p8_fixed_sgn(p8_fixed_t x);

/* Median of the three arguments, so it clamps a value between two bounds
 * whatever order the call site passes them in. A median equal to the value
 * is this port's membership test -- it says the value was already inside
 * (see box_contains). */
p8_fixed_t p8_fixed_mid(p8_fixed_t a, p8_fixed_t b, p8_fixed_t c);

/* Bitwise on the raw 32-bit representation, not on the "human" value. */
p8_fixed_t p8_fixed_band(p8_fixed_t a, p8_fixed_t b);
p8_fixed_t p8_fixed_bor(p8_fixed_t a, p8_fixed_t b);

/* PICO-8's floor-based modulo: the dividend minus flr(dividend / divisor)
 * times the divisor. A zero divisor is not special-cased: it inherits
 * p8_fixed_div's saturation. */
p8_fixed_t p8_fixed_mod(p8_fixed_t a, p8_fixed_t b);

/* Shifts the raw representation. The count is a plain integer, not a fixed
 * value. A count outside [0,31] returns 0 -- this port's own defensive
 * answer to what C leaves undefined, not sourced PICO-8 behavior. */
p8_fixed_t p8_fixed_shl(p8_fixed_t x, int32_t n);

/* sqrt(x < 0) is 0. Otherwise the largest fixed value whose square does
 * not exceed x: it truncates, it never rounds. Computed by an integer
 * square root on the raw bits, with no host floating point. */
p8_fixed_t p8_fixed_sqrt(p8_fixed_t x);

/* x^0.75 as PICO-8 computes it (tools/pico8_reference_probe_out.txt block
 * D): sqrt(x) times the square root of sqrt(x), each step truncated. 0 and
 * negative x give 0. */
p8_fixed_t p8_fixed_pow075(p8_fixed_t x);

/* Deterministic placeholder generator, NOT PICO-8's: it satisfies the
 * documented range but reproduces no real sequence, so a galaxy generated
 * here differs from the same seed in PICO-8. rnd(0) is 0; otherwise the
 * result lies in [0, x). */
p8_fixed_t p8_fixed_rnd(p8_fixed_t x);

/* rnd() with no argument, i.e. rnd(1). */
p8_fixed_t p8_fixed_rnd_default(void);

/* x is in turns: 0.0-1.0 is a full rotation. */
/* GAP-1 */
p8_fixed_t p8_fixed_cos(p8_fixed_t x);

/* Same turns convention, but the sign is inverted relative to standard
 * math, matching PICO-8's screen y-axis pointing down. */
/* GAP-1 */
p8_fixed_t p8_fixed_sin(p8_fixed_t x);

#endif
