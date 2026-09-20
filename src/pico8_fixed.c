#include "pico8_fixed.h"

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
