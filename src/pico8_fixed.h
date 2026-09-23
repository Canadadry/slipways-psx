#ifndef PICO8_FIXED_H
#define PICO8_FIXED_H

#include <stdint.h>

/* PICO-8's 16.16 fixed-point number: bits 31-16 are the two's-complement
 * integer part, bits 15-0 the fraction, so the range is -32768.0 to
 * 32767.9999847412109375. */
typedef int32_t pico8_fixed_t;

#define PICO8_FIXED_SHIFT 16

/* What a division by zero saturates to: +0x7fff.ffff / -0x7fff.ffff. See
 * pico8_fixed_div. */
#define PICO8_FIXED_DIV0_POS ((pico8_fixed_t)0x7FFFFFFF)
#define PICO8_FIXED_DIV0_NEG ((pico8_fixed_t)(-0x7FFFFFFF))

/* raw: the 8 hex digits of tools/probe_out.txt block A, point removed; lua:
 * the cart's literal, for grep and tools/constants_compare.sh -- never
 * evaluated. */
#define PICO8_FIXED_LIT(raw, lua) ((pico8_fixed_t)(uint32_t)(raw##u))

pico8_fixed_t pico8_fixed_from_int(int32_t i);

/* Overflow wraps, two's-complement, as PICO-8 documents. Computed through
 * the unsigned 32-bit type, since signed overflow is undefined behavior in
 * C -- never by relying on int32_t overflow or on -fwrapv. */
pico8_fixed_t pico8_fixed_add(pico8_fixed_t a, pico8_fixed_t b);
pico8_fixed_t pico8_fixed_sub(pico8_fixed_t a, pico8_fixed_t b);

/* Product in a 64-bit intermediate, shifted back down by 16 and truncated
 * to 32 bits; that truncation is where PICO-8's multiply overflow wraps. */
pico8_fixed_t pico8_fixed_mul(pico8_fixed_t a, pico8_fixed_t b);

/* (a << 16) / b, truncated the same way, except for b == 0: the PICO-8
 * manual documents that as saturating to +0x7fff.ffff for a non-negative
 * dividend, -0x7fff.ffff for a negative one. 0/0 counts as non-negative
 * and saturates positive. */
pico8_fixed_t pico8_fixed_div(pico8_fixed_t a, pico8_fixed_t b);

/* 0 - x, under the same wraparound as + and -, so negating the
 * most-negative value wraps back to itself. */
pico8_fixed_t pico8_fixed_neg(pico8_fixed_t x);

/* flr truncates toward negative infinity, not toward zero: raw -1 (a tiny
 * negative fraction) floors to -1.0, not to 0. */
pico8_fixed_t pico8_fixed_flr(pico8_fixed_t x);

/* flr(x) as a plain integer, rounded toward negative infinity, not toward
 * zero: raw -1 gives -1, not 0. */
int32_t pico8_fixed_flr_int(pico8_fixed_t x);

/* Inherits pico8_fixed_neg's wraparound: abs of the most-negative value is
 * itself. */
pico8_fixed_t pico8_fixed_abs(pico8_fixed_t x);

/* sgn(0) == 1, not 0. */
pico8_fixed_t pico8_fixed_sgn(pico8_fixed_t x);

/* Median of the three arguments, so mid(lo,hi,v) clamps v into [lo,hi]
 * whatever order the call site uses. mid(lo,hi,v) == v is this port's
 * membership test -- it says v was already inside (see pico8_qz_call). */
pico8_fixed_t pico8_fixed_mid(pico8_fixed_t a, pico8_fixed_t b, pico8_fixed_t c);

/* Bitwise on the raw 32-bit representation, not on the "human" value. */
pico8_fixed_t pico8_fixed_band(pico8_fixed_t a, pico8_fixed_t b);
pico8_fixed_t pico8_fixed_bor(pico8_fixed_t a, pico8_fixed_t b);

/* Lua's floor-based modulo, a-flr(a/b)*b. b == 0 is not special-cased: it
 * inherits pico8_fixed_div's saturation. */
pico8_fixed_t pico8_fixed_mod(pico8_fixed_t a, pico8_fixed_t b);

/* Shifts the raw representation. n is a plain shift count, not a fixed
 * value. n outside [0,31] returns 0 -- this port's own defensive answer to
 * what C leaves undefined, not sourced PICO-8 behavior. */
pico8_fixed_t pico8_fixed_shl(pico8_fixed_t x, int32_t n);

/* sqrt(x < 0) is 0. Otherwise the largest fixed value whose square does
 * not exceed x: it truncates, it never rounds. Computed by an integer
 * square root on the raw bits, with no host floating point. */
pico8_fixed_t pico8_fixed_sqrt(pico8_fixed_t x);

/* x^0.75 as PICO-8 computes it (tools/probe_out.txt block D): s*sqrt(s)
 * with s = sqrt(x), each step truncated. 0 and negative x give 0. */
pico8_fixed_t pico8_fixed_pow075(pico8_fixed_t x);

/* Deterministic placeholder generator, NOT PICO-8's: it satisfies the
 * documented range but reproduces no real sequence, so a galaxy generated
 * here differs from the same seed in PICO-8. rnd(0) is 0; otherwise the
 * result lies in [0, x). */
pico8_fixed_t pico8_fixed_rnd(pico8_fixed_t x);

/* rnd() with no argument, i.e. rnd(1). */
pico8_fixed_t pico8_fixed_rnd_default(void);

/* x is in turns: 0.0-1.0 is a full rotation. */
/* TROU-1 */
pico8_fixed_t pico8_fixed_cos(pico8_fixed_t x);

/* Same turns convention, but the sign is inverted relative to standard
 * math, matching PICO-8's screen y-axis pointing down. */
/* TROU-1 */
pico8_fixed_t pico8_fixed_sin(pico8_fixed_t x);

#ifdef PICO8_FLOAT_AUDIT
#include "pico8_float_audit.h"
#endif

#endif
