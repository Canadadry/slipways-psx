#include "game/bindings.h"

#include "p8/input.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_reader_accepts_every_button_number(void) {
    char text[4];
    for (int number = 1; number <= 12; number++) {
        snprintf(text, sizeof text, "%d", number);
        const char *end = NULL;
        assert(bindings_read_button_number(text, &end) == number);
        assert(end == text + strlen(text));
    }
}

static void assert_reads(const char *text, int number, int digits) {
    const char *end = NULL;
    assert(bindings_read_button_number(text, &end) == number);
    assert(end == text + digits);
}

static void test_reader_leaves_what_follows_the_digits_to_the_caller(void) {
    assert_reads("5,6", 5, 1);
    assert_reads("12]", 12, 2);
    assert_reads("5+6", 5, 1);
    assert_reads("1x", 1, 1);
}

static void assert_refused(const char *text) {
    const char *end = NULL;
    assert(bindings_read_button_number(text, &end) == 0);
    assert(end == text);
}

static void test_reader_refuses_anything_but_a_number_from_1_to_12(void) {
    assert_refused("13");
    assert_refused("0");
    assert_refused("05");
    assert_refused("012");
    assert_refused("");
    assert_refused("+5");
    assert_refused("-1");
    assert_refused(" 5");
    assert_refused("cross");
    assert_refused("123");
    assert_refused("12345678901234567890");
}

typedef char button_count_is_12[P8_PAD_BUTTON_COUNT == 12 ? 1 : -1];
typedef char button_count_ends_at_button_12[(1 << (P8_PAD_BUTTON_COUNT - 1)) == P8_PAD_BUTTON_12 ? 1 : -1];

int main(void) {
    test_reader_accepts_every_button_number();
    test_reader_leaves_what_follows_the_digits_to_the_caller();
    test_reader_refuses_anything_but_a_number_from_1_to_12();
    puts("test_bindings: ok");
    return 0;
}
