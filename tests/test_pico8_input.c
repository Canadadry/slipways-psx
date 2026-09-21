#include "../src/pico8_input.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static char *read_file_raw(const char *path) {
    FILE *f = fopen(path, "rb");
    assert(f);
    assert(fseek(f, 0, SEEK_END) == 0);
    long size = ftell(f);
    assert(size >= 0);
    assert(fseek(f, 0, SEEK_SET) == 0);
    char *buf = malloc((size_t)size + 1);
    assert(buf);
    size_t got = fread(buf, 1, (size_t)size, f);
    fclose(f);
    buf[got] = '\0';
    return buf;
}

static void test_save_then_load_round_trips_every_field(void) {
    pico8_input_frame_t written[5] = {
        {64, 32, 1, {0, 1, 0, 0}},
        {0, 127, 3, {1, 0, 1, 0}},
        {127, 0, 0, {0, 0, 0, 1}},
        {10, 20, 7, {1, 1, 1, 1}},
        {5, 99, 2, {0, 0, 0, 0}},
    };
    const char *path = "tmp/test_pico8_input_save_roundtrip.json";

    int saved = pico8_input_save(written, 5, path);
    assert(saved);

    pico8_input_t input;
    pico8_input_init(&input);
    int loaded = pico8_input_load(&input, path);
    assert(loaded);
    assert(input.frame_count == 5);

    for (int i = 0; i < 5; i++) {
        assert(pico8_stat(&input, 32) == written[i].mouse_x);
        assert(pico8_stat(&input, 33) == written[i].mouse_y);
        assert(pico8_stat(&input, 34) == written[i].mouse_buttons);
        assert(pico8_btn(&input, 0) == written[i].dpad[0]);
        assert(pico8_btn(&input, 1) == written[i].dpad[1]);
        assert(pico8_btn(&input, 2) == written[i].dpad[2]);
        assert(pico8_btn(&input, 3) == written[i].dpad[3]);
        if (i + 1 < 5) {
            pico8_input_step(&input);
        }
    }

    pico8_input_free(&input);
}

static void test_save_writes_events_json_schema_shape(void) {
    pico8_input_frame_t written[2] = {
        {64, 32, 1, {0, 1, 0, 0}},
        {10, 20, 3, {1, 0, 0, 1}},
    };
    const char *path = "tmp/test_pico8_input_save_shape.json";

    assert(pico8_input_save(written, 2, path));

    char *written_json = read_file_raw(path);
    assert(strstr(written_json, "\"frames\":") != NULL);
    assert(strstr(written_json, "\"mouse_x\": 64") != NULL);
    assert(strstr(written_json, "\"mouse_y\": 32") != NULL);
    assert(strstr(written_json, "\"mouse_buttons\": 1") != NULL);
    assert(strstr(written_json, "\"dpad\": [false, true, false, false]") != NULL);
    assert(strstr(written_json, "[") != NULL);
    assert(strstr(written_json, "]") != NULL);

    /* Cross-check against the real, committed root events.json's own
     * structure -- same key names in the same shape, not a
     * self-consistent-but-different schema invented by writer+reader
     * agreeing with each other alone. */
    char *real_events_json = read_file_raw("events.json");
    assert(strstr(real_events_json, "\"frames\":") != NULL);
    assert(strstr(real_events_json, "\"mouse_x\":") != NULL);
    assert(strstr(real_events_json, "\"mouse_y\":") != NULL);
    assert(strstr(real_events_json, "\"mouse_buttons\":") != NULL);
    assert(strstr(real_events_json, "\"dpad\":") != NULL);
    assert(strstr(real_events_json, "false") != NULL);
    assert(strstr(real_events_json, "true") != NULL);

    free(written_json);
    free(real_events_json);

    /* The real events.json must itself load through pico8_input_load
     * (sanity: our shape assumptions above are grounded in a file the
     * loader actually accepts, not just string content). */
    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, "events.json"));
    pico8_input_free(&input);
}

static void test_save_rejects_empty_frame_list(void) {
    const char *path = "tmp/test_pico8_input_save_empty.json";
    int saved = pico8_input_save(NULL, 0, path);
    assert(!saved);
}

int main(void) {
    test_load_and_step_multi_frame_matches_fields();
    test_btn_out_of_range_returns_zero();
    test_stat_unsupported_index_returns_zero();
    test_load_missing_file_is_hard_error();
    test_load_malformed_missing_field_is_hard_error();
    test_load_malformed_short_dpad_is_hard_error();
    test_load_malformed_no_frames_key_is_hard_error();
    test_save_then_load_round_trips_every_field();
    test_save_writes_events_json_schema_shape();
    test_save_rejects_empty_frame_list();

    printf("test_pico8_input: all tests passed\n");
    return 0;
}
