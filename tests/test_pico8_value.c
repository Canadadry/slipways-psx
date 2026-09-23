#include "../src/pico8_value.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_fixed_constructs_with_fixed_kind(void) {
    pico8_value_t v = pico8_value_fixed(pico8_fixed_from_int(42));
    assert(v.kind == PICO8_VALUE_FIXED);
    assert(v.as.fixed == pico8_fixed_from_int(42));
}

static void test_string_constructs_with_string_kind(void) {
    const char *s = "hello";
    pico8_value_t v = pico8_value_string(s);
    assert(v.kind == PICO8_VALUE_STRING);
    assert(v.as.string == s);
}

static void test_bool_constructs_with_bool_kind(void) {
    pico8_value_t t = pico8_value_bool(1);
    pico8_value_t f = pico8_value_bool(0);
    assert(t.kind == PICO8_VALUE_BOOL);
    assert(t.as.boolean == 1);
    assert(f.as.boolean == 0);
}

static void test_ref_constructs_with_ref_kind(void) {
    int payload = 7;
    pico8_value_t v = pico8_value_ref(&payload);
    assert(v.kind == PICO8_VALUE_REF);
    assert(v.as.ref == &payload);
}

static void test_equal_fixed_by_numeric_value(void) {
    pico8_value_t a = pico8_value_fixed(pico8_fixed_from_int(3));
    pico8_value_t b = pico8_value_fixed(pico8_fixed_from_int(3));
    pico8_value_t c = pico8_value_fixed(pico8_fixed_from_int(4));
    assert(pico8_value_equal(a, b));
    assert(!pico8_value_equal(a, c));
}

static void test_equal_bool_by_value(void) {
    assert(pico8_value_equal(pico8_value_bool(1), pico8_value_bool(1)));
    assert(!pico8_value_equal(pico8_value_bool(1), pico8_value_bool(0)));
}

static void test_equal_ref_by_pointer_identity(void) {
    int a = 1, b = 1;
    assert(pico8_value_equal(pico8_value_ref(&a), pico8_value_ref(&a)));
    assert(!pico8_value_equal(pico8_value_ref(&a), pico8_value_ref(&b)));
}

static void test_equal_string_by_pointer_identity_not_content(void) {
    char buf1[] = "same";
    char buf2[] = "same";
    assert(pico8_value_equal(pico8_value_string(buf1), pico8_value_string(buf1)));
    assert(!pico8_value_equal(pico8_value_string(buf1), pico8_value_string(buf2)));
}

static void test_equal_different_kinds_never_equal(void) {
    pico8_value_t fixed_zero = pico8_value_fixed(pico8_fixed_from_int(0));
    pico8_value_t bool_false = pico8_value_bool(0);
    assert(!pico8_value_equal(fixed_zero, bool_false));
}

static void test_tostr_small_positive_integer(void) {
    char *s = pico8_value_tostr(pico8_value_fixed(pico8_fixed_from_int(3)));
    assert(strcmp(s, "3") == 0);
    free(s);
}

static void test_tostr_zero(void) {
    char *s = pico8_value_tostr(pico8_value_fixed(pico8_fixed_from_int(0)));
    assert(strcmp(s, "0") == 0);
    free(s);
}

static void test_tostr_negative_integer(void) {
    char *s = pico8_value_tostr(pico8_value_fixed(pico8_fixed_from_int(-5)));
    assert(strcmp(s, "-5") == 0);
    free(s);
}

static void test_tostr_string_is_content_passthrough(void) {
    char *s = pico8_value_tostr(pico8_value_string("hi"));
    assert(strcmp(s, "hi") == 0);
    free(s);
}

static void test_tostr_bool(void) {
    char *t = pico8_value_tostr(pico8_value_bool(1));
    char *f = pico8_value_tostr(pico8_value_bool(0));
    assert(strcmp(t, "true") == 0);
    assert(strcmp(f, "false") == 0);
    free(t);
    free(f);
}

static pico8_fixed_t raw_fixed(uint32_t raw) {
    return (pico8_fixed_t)raw;
}

static void assert_tostr_raw(uint32_t raw, const char *expected) {
    char *s = pico8_value_tostr(pico8_value_fixed(raw_fixed(raw)));
    if (strcmp(s, expected) != 0) {
        fprintf(stderr, "tostr 0x%08x: got \"%s\", expected \"%s\"\n",
                (unsigned)raw, s, expected);
    }
    assert(strcmp(s, expected) == 0);
    free(s);
}

static void test_tostr_matches_probe_block_c(void) {
    assert_tostr_raw(0xfffe8000u, "-1.5");
    assert_tostr_raw(0x00010000u, "1");
    assert_tostr_raw(0x00000000u, "0");
    assert_tostr_raw(0x00002666u, "0.15");
    assert_tostr_raw(0x0000451eu, "0.27");
    assert_tostr_raw(0x00001999u, "0.1");
    assert_tostr_raw(0x00028000u, "2.5");
    assert_tostr_raw(0x00640000u, "100");
    assert_tostr_raw(0x03e80000u, "1000");
    assert_tostr_raw(0x80000000u, "-32768");
    assert_tostr_raw(0x7fff0000u, "32767");
    assert_tostr_raw(0x00005555u, "0.3333");
    assert_tostr_raw(0x0000aaaau, "0.6667");
}

static void test_tostr_below_0_0001_keeps_port_output(void) {
    char *s = pico8_value_tostr(pico8_value_fixed(raw_fixed(0xfffffffau)));
    assert(strcmp(s, "-0.0001") == 0
           && "TROU-22: PICO-8 prints \"-0\" for raw -6; the port keeps \"-0.0001\"");
    free(s);
}

static void test_tostr_exact_ties_round_half_to_even(void) {
    assert_tostr_raw(0x00000800u, "0.0312");
    assert_tostr_raw(0x00001800u, "0.0938");
}

/* The pre-D2 tostr, byte for byte, kept as the oracle of the exhaustive
 * test: host glibc "%.4f" rounds the exact binary value half to even. */
static void old_tostr_percent_4f_for_test(int32_t raw, char *buf, size_t size) {
    double d = raw / 65536.0;
    snprintf(buf, size, "%.4f", d);
    char *dot = strchr(buf, '.');
    if (dot) {
        char *end = buf + strlen(buf) - 1;
        while (end > dot && *end == '0') {
            *end-- = '\0';
        }
        if (end == dot) {
            *end = '\0';
        }
    }
}

static int check_against_old_tostr(int32_t raw) {
    char old[64];
    old_tostr_percent_4f_for_test(raw, old, sizeof old);
    char *s = pico8_value_tostr(pico8_value_fixed(raw));
    int same = strcmp(s, old) == 0;
    if (!same) {
        fprintf(stderr, "tostr 0x%08x: old \"%s\", new \"%s\"\n",
                (unsigned)(uint32_t)raw, old, s);
    }
    free(s);
    return same;
}

static int is_exact_4_decimal_tie(int32_t raw) {
    uint32_t mag = raw < 0 ? 0u - (uint32_t)raw : (uint32_t)raw;
    return (((mag & 0xFFFFu) * 625u) & 4095u) == 2048u;
}

static void test_tostr_matches_old_percent_4f_exhaustively(void) {
    static const int32_t int_parts[] = {
        0, 1, 2, 9, 10, 99, 100, 999, 1000, 9999, 10000, 32766, 32767,
        -1, -2, -10, -32767, -32768,
    };
    const size_t n_int_parts = sizeof int_parts / sizeof int_parts[0];
    long inputs = 0;
    long ties = 0;

    for (size_t i = 0; i < n_int_parts; i++) {
        for (uint32_t f = 0; f <= 0xFFFFu; f++) {
            int32_t raw = (int32_t)(((uint32_t)int_parts[i] << 16) | f);
            assert(check_against_old_tostr(raw));
            ties += is_exact_4_decimal_tie(raw);
            inputs++;
        }
    }
    for (int32_t raw = -1; raw >= -65535; raw--) {
        assert(check_against_old_tostr(raw));
        ties += is_exact_4_decimal_tie(raw);
        inputs++;
    }

    assert(inputs == 1245183);
    /* 16 ties per integer part, the negative pure fractions counting as one. */
    assert(ties == 16 * (long)(n_int_parts + 1));
}

int main(void) {
    test_fixed_constructs_with_fixed_kind();
    test_string_constructs_with_string_kind();
    test_bool_constructs_with_bool_kind();
    test_ref_constructs_with_ref_kind();
    test_equal_fixed_by_numeric_value();
    test_equal_bool_by_value();
    test_equal_ref_by_pointer_identity();
    test_equal_string_by_pointer_identity_not_content();
    test_equal_different_kinds_never_equal();
    test_tostr_small_positive_integer();
    test_tostr_zero();
    test_tostr_negative_integer();
    test_tostr_string_is_content_passthrough();
    test_tostr_bool();
    test_tostr_matches_probe_block_c();
    test_tostr_below_0_0001_keeps_port_output();
    test_tostr_exact_ties_round_half_to_even();
    test_tostr_matches_old_percent_4f_exhaustively();

    printf("test_pico8_value: all tests passed\n");
    return 0;
}
