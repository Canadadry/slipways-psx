#include "../src/pico8_fixed.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void test_add_representative_values(void) {
    pico8_fixed_t a = pico8_fixed_from_double(2.5);
    pico8_fixed_t b = pico8_fixed_from_double(1.25);
    pico8_fixed_t result = pico8_fixed_add(a, b);
    assert(result == pico8_fixed_from_double(3.75));
}

static void test_sub_representative_values(void) {
    pico8_fixed_t a = pico8_fixed_from_double(5.5);
    pico8_fixed_t b = pico8_fixed_from_double(2.25);
    pico8_fixed_t result = pico8_fixed_sub(a, b);
    assert(result == pico8_fixed_from_double(3.25));
}

static void test_mul_representative_values(void) {
    pico8_fixed_t a = pico8_fixed_from_double(2.5);
    pico8_fixed_t b = pico8_fixed_from_double(4.0);
    pico8_fixed_t result = pico8_fixed_mul(a, b);
    assert(result == pico8_fixed_from_double(10.0));
}

static void test_mul_overflow_wraps(void) {
    /* 200*200 = 40000, which exceeds the 16-bit signed integer part's
     * range (max 32767), so the truncation wraps it to 40000 - 65536 =
     * -25536, per pico8_fixed_mul's documented truncation behavior. */
    pico8_fixed_t a = pico8_fixed_from_int(200);
    pico8_fixed_t b = pico8_fixed_from_int(200);
    pico8_fixed_t result = pico8_fixed_mul(a, b);
    assert(result == pico8_fixed_from_int(-25536));
}

static void test_add_overflow_128_squared_twice_wraps_to_min(void) {
    /* PICO-8's documented classic overflow bug: 128*128+128*128 overflows
     * the representable range and wraps to -32768.
     * Source: PICO-8 Known Bugs wiki,
     * https://pico-8.fandom.com/wiki/Known_Bugs */
    pico8_fixed_t hundred_twenty_eight = pico8_fixed_from_int(128);
    pico8_fixed_t squared = pico8_fixed_mul(hundred_twenty_eight, hundred_twenty_eight);
    assert(squared == pico8_fixed_from_int(16384));

    pico8_fixed_t result = pico8_fixed_add(squared, squared);
    assert(result == pico8_fixed_from_int(-32768));
}

static void test_div_representative_value(void) {
    pico8_fixed_t a = pico8_fixed_from_double(10.0);
    pico8_fixed_t b = pico8_fixed_from_double(4.0);
    pico8_fixed_t result = pico8_fixed_div(a, b);
    assert(result == pico8_fixed_from_double(2.5));
}

static void test_div_negative_dividend_nonzero_divisor(void) {
    /* -10 / 4 = -2.5. Regression test for a UB bug where the numerator
     * (a << PICO8_FIXED_SHIFT) was computed via a signed left-shift, which
     * is undefined behavior in C when a is negative (C11 6.5.7p4) -- caught
     * by UBSan even though the numeric result is correct under today's x86
     * codegen. See tmp/test_pico8_fixed_ubsan in the Makefile. */
    pico8_fixed_t a = pico8_fixed_from_int(-10);
    pico8_fixed_t b = pico8_fixed_from_int(4);
    pico8_fixed_t result = pico8_fixed_div(a, b);
    assert(result == pico8_fixed_from_double(-2.5));
}

static void test_div_by_zero_positive_dividend_saturates_positive(void) {
    /* Per the PICO-8 manual, division by zero saturates to 0x7fff.ffff
     * when the dividend is non-negative.
     * Source: https://www.lexaloffle.com/pico8_manual.txt */
    pico8_fixed_t a = pico8_fixed_from_int(5);
    pico8_fixed_t result = pico8_fixed_div(a, pico8_fixed_from_int(0));
    assert(result == PICO8_FIXED_DIV0_POS);
}

static void test_div_by_zero_negative_dividend_saturates_negative(void) {
    /* Per the PICO-8 manual, division by zero saturates to -0x7fff.ffff
     * when the dividend is negative.
     * Source: https://www.lexaloffle.com/pico8_manual.txt */
    pico8_fixed_t a = pico8_fixed_from_int(-5);
    pico8_fixed_t result = pico8_fixed_div(a, pico8_fixed_from_int(0));
    assert(result == PICO8_FIXED_DIV0_NEG);
}

static void test_div_zero_by_zero_saturates_positive(void) {
    /* 0 divided by 0 is treated as non-negative (0 is not negative),
     * so it saturates to +0x7fff.ffff, since this exact sub-case isn't
     * separately documented. */
    pico8_fixed_t result = pico8_fixed_div(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    assert(result == PICO8_FIXED_DIV0_POS);
}

static void test_neg_representative_value(void) {
    pico8_fixed_t a = pico8_fixed_from_double(3.5);
    pico8_fixed_t result = pico8_fixed_neg(a);
    assert(result == pico8_fixed_from_double(-3.5));
}

static void test_neg_most_negative_value_wraps_to_itself(void) {
    /* Unary - is 0 - x under the same two's-complement wraparound rule as
     * +/-, so negating the most-negative representable value (-32768.0,
     * raw INT32_MIN) wraps rather than overflowing: 0 - INT32_MIN, computed
     * in unsigned 32-bit arithmetic, wraps back around to INT32_MIN
     * itself. */
    pico8_fixed_t most_negative = (pico8_fixed_t)INT32_MIN;
    assert(most_negative == pico8_fixed_from_int(-32768));

    pico8_fixed_t result = pico8_fixed_neg(most_negative);
    assert(result == most_negative);
}

int main(void) {
    test_add_representative_values();
    test_sub_representative_values();
    test_mul_representative_values();
    test_mul_overflow_wraps();
    test_add_overflow_128_squared_twice_wraps_to_min();
    test_div_representative_value();
    test_div_negative_dividend_nonzero_divisor();
    test_div_by_zero_positive_dividend_saturates_positive();
    test_div_by_zero_negative_dividend_saturates_negative();
    test_div_zero_by_zero_saturates_positive();
    test_neg_representative_value();
    test_neg_most_negative_value_wraps_to_itself();

    printf("test_pico8_fixed: all tests passed\n");
    return 0;
}
