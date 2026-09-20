#include "../src/pico8_value.h"

#include <assert.h>
#include <stdio.h>

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

    printf("test_pico8_value: all tests passed\n");
    return 0;
}
