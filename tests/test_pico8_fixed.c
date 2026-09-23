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
    /* -10 / 4 = -2.5. Guards against the numerator going back to a signed
     * left shift, which is undefined for a negative value however right the
     * result looks on x86. */
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

static void test_flr_positive_truncates_fraction(void) {
    /* flr truncates toward negative infinity on the fixed value: 3.75
     * floors to 3.0. */
    pico8_fixed_t x = pico8_fixed_from_double(3.75);
    assert(pico8_fixed_flr(x) == pico8_fixed_from_int(3));
}

static void test_flr_negative_rounds_toward_negative_infinity(void) {
    /* -3.25 floors to -4.0, not -3.0 -- toward negative infinity, not
     * toward zero. */
    pico8_fixed_t x = pico8_fixed_from_double(-3.25);
    assert(pico8_fixed_flr(x) == pico8_fixed_from_int(-4));
}

static void test_flr_exact_integer_is_unchanged(void) {
    pico8_fixed_t x = pico8_fixed_from_int(5);
    assert(pico8_fixed_flr(x) == pico8_fixed_from_int(5));
}

static void test_flr_int_matches_probe_block_b(void) {
    /* tools/probe_out.txt, "## b rounding": copied from the probe, never
     * derived. */
    static const struct { uint32_t raw; int32_t expected; } probe[] = {
        {0xFFFE8000u, -2}, {0xFFFF8000u, -1}, {0xFFFF199Au, -1},
        {0xFFFF0000u, -1}, {0xFFFD8000u, -3}, {0x00018000u, 1},
        {0x00008000u, 0},  {0x0000E666u, 0},  {0x00010000u, 1},
        {0x00000000u, 0},
    };
    for (size_t i = 0; i < sizeof probe / sizeof probe[0]; i++) {
        assert(pico8_fixed_flr_int((pico8_fixed_t)probe[i].raw) == probe[i].expected);
    }
}

static void test_lit_is_the_raw_probe_value(void) {
    assert(PICO8_FIXED_LIT(0x0000451e, 0.27) == 17694);
    assert(PICO8_FIXED_LIT(0x000a8000, 10.5) == (pico8_fixed_t)0x000A8000u);
    assert(PICO8_FIXED_LIT(0x0000fd70, 0.99) == 0xFD70);
    assert(PICO8_FIXED_LIT(0x00008000, 0.5) == pico8_fixed_from_double(0.5));
}

/* Compiles only if the macro drops its second argument unevaluated. */
static void test_lit_never_evaluates_its_literal(void) {
    assert(PICO8_FIXED_LIT(0x00018000, never_declared_anywhere) == 0x18000);
}

static void test_flr_int_extremes(void) {
    assert(pico8_fixed_flr_int((pico8_fixed_t)0x7FFFFFFFu) == 32767);
    assert(pico8_fixed_flr_int((pico8_fixed_t)0x80000000u) == -32768);
}

static void test_flr_int_raw_minus_one_is_minus_one_not_zero(void) {
    assert(pico8_fixed_flr_int((pico8_fixed_t)0xFFFFFFFFu) == -1);
}

static void test_mod_positive_operands(void) {
    /* 45 % 42 == 3: the shape `nl:ly`'s `t%42/6` needs. */
    pico8_fixed_t a = pico8_fixed_from_int(45);
    pico8_fixed_t b = pico8_fixed_from_int(42);
    assert(pico8_fixed_mod(a, b) == pico8_fixed_from_int(3));
}

static void test_mod_fractional_operand(void) {
    /* a-flr(a/b)*b: 5.5 % 2 == 1.5. */
    pico8_fixed_t a = pico8_fixed_from_double(5.5);
    pico8_fixed_t b = pico8_fixed_from_int(2);
    assert(pico8_fixed_mod(a, b) == pico8_fixed_from_double(1.5));
}

static void test_mod_negative_dividend_matches_lua_floor_semantics(void) {
    /* Lua's floor-based modulo (not C's truncating "%"): -1 % 3 == 2, not
     * -1, since flr(-1/3)==-1 and -1-(-1*3)==2. */
    pico8_fixed_t a = pico8_fixed_from_int(-1);
    pico8_fixed_t b = pico8_fixed_from_int(3);
    assert(pico8_fixed_mod(a, b) == pico8_fixed_from_int(2));
}

static void test_mod_result_less_than_divisor_when_exact(void) {
    /* An exact multiple leaves a zero remainder. */
    pico8_fixed_t a = pico8_fixed_from_int(84);
    pico8_fixed_t b = pico8_fixed_from_int(42);
    assert(pico8_fixed_mod(a, b) == pico8_fixed_from_int(0));
}

static void test_abs_negative_value(void) {
    pico8_fixed_t x = pico8_fixed_from_double(-3.5);
    assert(pico8_fixed_abs(x) == pico8_fixed_from_double(3.5));
}

static void test_abs_positive_value_is_unchanged(void) {
    pico8_fixed_t x = pico8_fixed_from_double(3.5);
    assert(pico8_fixed_abs(x) == pico8_fixed_from_double(3.5));
}

static void test_abs_zero_is_zero(void) {
    assert(pico8_fixed_abs(pico8_fixed_from_int(0)) == pico8_fixed_from_int(0));
}

static void test_abs_most_negative_value_wraps_to_itself(void) {
    /* Same two's-complement caveat as neg: the most-negative representable
     * value's abs wraps rather than growing. */
    pico8_fixed_t most_negative = (pico8_fixed_t)INT32_MIN;
    assert(pico8_fixed_abs(most_negative) == most_negative);
}

static void test_sgn_positive_value_is_one(void) {
    pico8_fixed_t x = pico8_fixed_from_double(3.5);
    assert(pico8_fixed_sgn(x) == pico8_fixed_from_int(1));
}

static void test_sgn_negative_value_is_negative_one(void) {
    pico8_fixed_t x = pico8_fixed_from_double(-3.5);
    assert(pico8_fixed_sgn(x) == pico8_fixed_from_int(-1));
}

static void test_sgn_zero_is_one_not_zero(void) {
    /* PICO-8's documented sgn(0) == 1, not 0.
     * Source: PICO-8 Sgn wiki, https://pico-8.fandom.com/wiki/Sgn */
    pico8_fixed_t result = pico8_fixed_sgn(pico8_fixed_from_int(0));
    assert(result == pico8_fixed_from_int(1));
}

static void test_mid_middle_argument_already_sorted(void) {
    pico8_fixed_t a = pico8_fixed_from_int(1);
    pico8_fixed_t b = pico8_fixed_from_int(5);
    pico8_fixed_t c = pico8_fixed_from_int(10);
    assert(pico8_fixed_mid(a, b, c) == b);
}

static void test_mid_is_order_independent(void) {
    /* The median of three values does not depend on the order they are
     * passed in, so every call-site convention clamps the same. */
    pico8_fixed_t lo = pico8_fixed_from_int(0);
    pico8_fixed_t hi = pico8_fixed_from_int(10);
    pico8_fixed_t v = pico8_fixed_from_int(4);
    pico8_fixed_t expected = v; /* v already within [lo,hi] */

    assert(pico8_fixed_mid(lo, hi, v) == expected);
    assert(pico8_fixed_mid(hi, lo, v) == expected);
    assert(pico8_fixed_mid(v, lo, hi) == expected);
    assert(pico8_fixed_mid(lo, v, hi) == expected);
    assert(pico8_fixed_mid(hi, v, lo) == expected);
    assert(pico8_fixed_mid(v, hi, lo) == expected);
}

static void test_mid_clamps_value_below_range(void) {
    pico8_fixed_t lo = pico8_fixed_from_int(0);
    pico8_fixed_t hi = pico8_fixed_from_int(10);
    pico8_fixed_t v = pico8_fixed_from_int(-5);
    assert(pico8_fixed_mid(lo, hi, v) == lo);
}

static void test_mid_clamps_value_above_range(void) {
    pico8_fixed_t lo = pico8_fixed_from_int(0);
    pico8_fixed_t hi = pico8_fixed_from_int(10);
    pico8_fixed_t v = pico8_fixed_from_int(50);
    assert(pico8_fixed_mid(lo, hi, v) == hi);
}

static void test_band_positive_raw_operands(void) {
    /* `band` combines the raw 32-bit representations, not the human
     * values: 0x00060000 AND 0x00030000 is 0x00020000. It matches integer
     * AND here only because the fractional bits are all zero. */
    pico8_fixed_t a = pico8_fixed_from_int(6);
    pico8_fixed_t b = pico8_fixed_from_int(3);
    assert(pico8_fixed_band(a, b) == pico8_fixed_from_int(2));
}

static void test_band_negative_raw_operand(void) {
    /* -1 is raw 0xFFFFFFFF (all bits set). AND with any value returns
     * that value unchanged. */
    pico8_fixed_t all_ones = (pico8_fixed_t)0xFFFFFFFFu;
    pico8_fixed_t x = pico8_fixed_from_double(2.5);
    assert(pico8_fixed_band(all_ones, x) == x);
}

static void test_bor_positive_raw_operands(void) {
    pico8_fixed_t a = pico8_fixed_from_int(4);
    pico8_fixed_t b = pico8_fixed_from_int(1);
    assert(pico8_fixed_bor(a, b) == pico8_fixed_from_int(5));
}

static void test_bor_negative_raw_operand(void) {
    /* OR with all-bits-set (-1) always yields all-bits-set, regardless of
     * the other operand. */
    pico8_fixed_t all_ones = (pico8_fixed_t)0xFFFFFFFFu;
    pico8_fixed_t x = pico8_fixed_from_double(2.5);
    assert(pico8_fixed_bor(all_ones, x) == all_ones);
}

static void test_shl_positive_raw_operand(void) {
    /* `shl` shifts the raw representation; the cart's own use is
     * `shl(flr(no),4)`, times sixteen on an integer value. */
    pico8_fixed_t x = pico8_fixed_from_int(5);
    assert(pico8_fixed_shl(x, 4) == pico8_fixed_from_int(80));
}

static void test_shl_negative_raw_operand(void) {
    /* Same shift UB as in `pico8_fixed_div`: shifting a negative signed
     * value must go through the unsigned reinterpretation. -1, raw
     * 0xFFFFFFFF, shifted by 1 is 0xFFFFFFFE. */
    pico8_fixed_t x = (pico8_fixed_t)0xFFFFFFFFu;
    pico8_fixed_t result = pico8_fixed_shl(x, 1);
    assert(result == (pico8_fixed_t)0xFFFFFFFEu);
}

static void test_shl_by_zero_is_unchanged(void) {
    pico8_fixed_t x = pico8_fixed_from_double(2.5);
    assert(pico8_fixed_shl(x, 0) == x);
}

static void test_sqrt_perfect_square(void) {
    pico8_fixed_t x = pico8_fixed_from_int(4);
    assert(pico8_fixed_sqrt(x) == pico8_fixed_from_int(2));
}

static void test_sqrt_perfect_square_fractional(void) {
    pico8_fixed_t x = pico8_fixed_from_double(0.25);
    assert(pico8_fixed_sqrt(x) == pico8_fixed_from_double(0.5));
}

static void test_sqrt_zero_is_zero(void) {
    assert(pico8_fixed_sqrt(pico8_fixed_from_int(0)) == pico8_fixed_from_int(0));
}

static void test_sqrt_negative_value_is_zero(void) {
    /* As of PICO-8 0.2.6b, sqrt of a negative value returns 0.
     * Source: PICO-8 Sqrt wiki, https://pico-8.fandom.com/wiki/Sqrt */
    pico8_fixed_t x = pico8_fixed_from_int(-4);
    assert(pico8_fixed_sqrt(x) == pico8_fixed_from_int(0));
}

/* rnd/sin/cos are asserted on their documented contract only -- range,
 * documented identities, sign convention, periodicity. No bit-exact value
 * is asserted: PICO-8's own PRNG and sin/cos approximation are not
 * published, so there is no reference to compare against. */
/* TROU-1 */

static void test_rnd_zero_is_zero(void) {
    /* rnd(0) returns 0. */
    assert(pico8_fixed_rnd(pico8_fixed_from_int(0)) == pico8_fixed_from_int(0));
}

static void test_rnd_result_in_documented_range(void) {
    /* rnd(x) returns a value in [0, x). Sampled repeatedly (the generator
     * is stateful) to exercise more than one draw, asserting only the
     * documented range property, never a specific drawn value. */
    pico8_fixed_t x = pico8_fixed_from_int(10);
    for (int i = 0; i < 200; i++) {
        pico8_fixed_t result = pico8_fixed_rnd(x);
        assert(result >= pico8_fixed_from_int(0));
        assert(result < x);
    }
}

static void test_rnd_default_behaves_as_rnd_one(void) {
    /* rnd() (no argument) behaves as rnd(1): its result is in the same
     * documented range [0, 1) that rnd(1) itself is bound by. */
    pico8_fixed_t one = pico8_fixed_from_int(1);
    for (int i = 0; i < 200; i++) {
        pico8_fixed_t result = pico8_fixed_rnd_default();
        assert(result >= pico8_fixed_from_int(0));
        assert(result < one);
    }
}

static void test_sin_zero_is_zero(void) {
    assert(pico8_fixed_sin(pico8_fixed_from_int(0)) == pico8_fixed_from_int(0));
}

static void test_cos_zero_is_one(void) {
    assert(pico8_fixed_cos(pico8_fixed_from_int(0)) == pico8_fixed_from_int(1));
}

static void test_sin_quarter_turn_sign_is_inverted(void) {
    /* A quarter turn is a documented identity: standard math has
     * sin(pi/2) == 1, but PICO-8's sin() inverts the sign for
     * screen-space y, so sin(0.25 turns) == -1 exactly -- this pins down
     * the sign-inversion convention itself, not an arbitrary computed
     * decimal. */
    pico8_fixed_t quarter_turn = pico8_fixed_from_double(0.25);
    assert(pico8_fixed_sin(quarter_turn) == pico8_fixed_from_int(-1));
}

static void test_cos_quarter_turn_is_zero(void) {
    /* Another documented identity: cos(pi/2) == 0, unaffected by sin's
     * sign inversion since cos follows the standard convention. */
    pico8_fixed_t quarter_turn = pico8_fixed_from_double(0.25);
    assert(pico8_fixed_cos(quarter_turn) == pico8_fixed_from_int(0));
}

static void test_cos_half_turn_is_negative_one(void) {
    pico8_fixed_t half_turn = pico8_fixed_from_double(0.5);
    assert(pico8_fixed_cos(half_turn) == pico8_fixed_from_int(-1));
}

static void test_sin_and_cos_periodic_at_whole_turn_boundary(void) {
    /* A full turn (1.0) is the same rotation as no turn (0.0): sin/cos
     * must agree at that whole-turn boundary. */
    pico8_fixed_t zero_turns = pico8_fixed_from_int(0);
    pico8_fixed_t one_turn = pico8_fixed_from_int(1);
    pico8_fixed_t two_turns = pico8_fixed_from_int(2);

    assert(pico8_fixed_sin(one_turn) == pico8_fixed_sin(zero_turns));
    assert(pico8_fixed_cos(one_turn) == pico8_fixed_cos(zero_turns));
    assert(pico8_fixed_sin(two_turns) == pico8_fixed_sin(zero_turns));
    assert(pico8_fixed_cos(two_turns) == pico8_fixed_cos(zero_turns));
}

int main(void) {
    test_sqrt_perfect_square();
    test_sqrt_perfect_square_fractional();
    test_sqrt_zero_is_zero();
    test_sqrt_negative_value_is_zero();

    test_shl_positive_raw_operand();
    test_shl_negative_raw_operand();
    test_shl_by_zero_is_unchanged();

    test_band_positive_raw_operands();
    test_band_negative_raw_operand();
    test_bor_positive_raw_operands();
    test_bor_negative_raw_operand();

    test_mid_middle_argument_already_sorted();
    test_mid_is_order_independent();
    test_mid_clamps_value_below_range();
    test_mid_clamps_value_above_range();

    test_flr_positive_truncates_fraction();
    test_flr_negative_rounds_toward_negative_infinity();
    test_flr_exact_integer_is_unchanged();
    test_flr_int_matches_probe_block_b();
    test_lit_is_the_raw_probe_value();
    test_lit_never_evaluates_its_literal();
    test_flr_int_extremes();
    test_flr_int_raw_minus_one_is_minus_one_not_zero();

    test_mod_positive_operands();
    test_mod_fractional_operand();
    test_mod_negative_dividend_matches_lua_floor_semantics();
    test_mod_result_less_than_divisor_when_exact();

    test_abs_negative_value();
    test_abs_positive_value_is_unchanged();
    test_abs_zero_is_zero();
    test_abs_most_negative_value_wraps_to_itself();

    test_sgn_positive_value_is_one();
    test_sgn_negative_value_is_negative_one();
    test_sgn_zero_is_one_not_zero();

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

    test_rnd_zero_is_zero();
    test_rnd_result_in_documented_range();
    test_rnd_default_behaves_as_rnd_one();
    test_sin_zero_is_zero();
    test_cos_zero_is_one();
    test_sin_quarter_turn_sign_is_inverted();
    test_cos_quarter_turn_is_zero();
    test_cos_half_turn_is_negative_one();
    test_sin_and_cos_periodic_at_whole_turn_boundary();

    printf("test_pico8_fixed: all tests passed\n");
    return 0;
}
