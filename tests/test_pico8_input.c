#include "../src/pico8_input.h"

#include <assert.h>
#include <stdio.h>

static void test_load_and_step_multi_frame_matches_fields(void) {
    pico8_input_t input;
    pico8_input_init(&input);

    int ok = pico8_input_load(&input, "tests/fixtures/events_valid.json");
    assert(ok);

    assert(pico8_btn(&input, 0) == 0);
    assert(pico8_btn(&input, 1) == 1);
    assert(pico8_btn(&input, 2) == 0);
    assert(pico8_btn(&input, 3) == 0);
    assert(pico8_stat(&input, 32) == 64);
    assert(pico8_stat(&input, 33) == 64);
    assert(pico8_stat(&input, 34) == 1);

    pico8_input_step(&input);
    assert(pico8_btn(&input, 0) == 1);
    assert(pico8_btn(&input, 1) == 0);
    assert(pico8_btn(&input, 2) == 1);
    assert(pico8_btn(&input, 3) == 0);
    assert(pico8_stat(&input, 32) == 10);
    assert(pico8_stat(&input, 33) == 20);
    assert(pico8_stat(&input, 34) == 3);

    pico8_input_step(&input);
    assert(pico8_btn(&input, 0) == 0);
    assert(pico8_btn(&input, 1) == 0);
    assert(pico8_btn(&input, 2) == 0);
    assert(pico8_btn(&input, 3) == 1);
    assert(pico8_stat(&input, 32) == 0);
    assert(pico8_stat(&input, 33) == 127);
    assert(pico8_stat(&input, 34) == 4);

    pico8_input_free(&input);
}

static void test_btn_out_of_range_returns_zero(void) {
    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, "tests/fixtures/events_valid.json"));

    assert(pico8_btn(&input, -1) == 0);
    assert(pico8_btn(&input, 4) == 0);

    pico8_input_free(&input);
}

static void test_stat_unsupported_index_returns_zero(void) {
    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, "tests/fixtures/events_valid.json"));

    assert(pico8_stat(&input, 0) == 0);
    assert(pico8_stat(&input, 35) == 0);

    pico8_input_free(&input);
}

static void test_load_missing_file_is_hard_error(void) {
    pico8_input_t input;
    pico8_input_init(&input);

    int ok = pico8_input_load(&input, "tests/fixtures/does_not_exist.json");
    assert(!ok);

    pico8_input_free(&input);
}

static void test_load_malformed_missing_field_is_hard_error(void) {
    pico8_input_t input;
    pico8_input_init(&input);

    int ok = pico8_input_load(&input, "tests/fixtures/events_malformed_missing_field.json");
    assert(!ok);

    pico8_input_free(&input);
}

static void test_load_malformed_short_dpad_is_hard_error(void) {
    pico8_input_t input;
    pico8_input_init(&input);

    int ok = pico8_input_load(&input, "tests/fixtures/events_malformed_short_dpad.json");
    assert(!ok);

    pico8_input_free(&input);
}

static void test_load_malformed_no_frames_key_is_hard_error(void) {
    pico8_input_t input;
    pico8_input_init(&input);

    int ok = pico8_input_load(&input, "tests/fixtures/events_malformed_no_frames_key.json");
    assert(!ok);

    pico8_input_free(&input);
}

int main(void) {
    test_load_and_step_multi_frame_matches_fields();
    test_btn_out_of_range_returns_zero();
    test_stat_unsupported_index_returns_zero();
    test_load_missing_file_is_hard_error();
    test_load_malformed_missing_field_is_hard_error();
    test_load_malformed_short_dpad_is_hard_error();
    test_load_malformed_no_frames_key_is_hard_error();

    printf("test_pico8_input: all tests passed\n");
    return 0;
}
