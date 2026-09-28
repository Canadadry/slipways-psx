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

static void test_four_boolean_dpad_loads_with_o_and_x_released(void) {
    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, "tests/fixtures/events_valid.json"));

    assert(pico8_btn(&input, 4) == 0);
    assert(pico8_btn(&input, 5) == 0);

    pico8_input_free(&input);
}

static void test_six_boolean_dpad_loads_o_and_x(void) {
    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, "tests/fixtures/events_valid_ox.json"));

    assert(pico8_btn(&input, 1) == 1);
    assert(pico8_btn(&input, 4) == 1);
    assert(pico8_btn(&input, 5) == 0);
    pico8_input_step(&input);
    assert(pico8_btn(&input, 0) == 1);
    assert(pico8_btn(&input, 2) == 1);
    assert(pico8_btn(&input, 4) == 0);
    assert(pico8_btn(&input, 5) == 1);

    pico8_input_free(&input);
}

static void test_load_malformed_five_boolean_dpad_is_hard_error(void) {
    pico8_input_t input;
    pico8_input_init(&input);

    assert(!pico8_input_load(&input, "tests/fixtures/events_malformed_five_dpad.json"));

    pico8_input_free(&input);
}

static void test_btn_out_of_range_returns_zero(void) {
    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, "tests/fixtures/events_valid.json"));

    assert(pico8_btn(&input, -1) == 0);
    assert(pico8_btn(&input, 6) == 0);

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
        {64, 32, 1, {0, 1, 0, 0, 1, 0}},
        {0, 127, 3, {1, 0, 1, 0, 0, 1}},
        {127, 0, 0, {0, 0, 0, 1, 0, 0}},
        {10, 20, 7, {1, 1, 1, 1, 1, 1}},
        {5, 99, 2, {0, 0, 0, 0, 0, 0}},
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
        assert(pico8_btn(&input, 4) == written[i].dpad[4]);
        assert(pico8_btn(&input, 5) == written[i].dpad[5]);
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
    assert(strstr(written_json, "\"dpad\": [false, true, false, false, false, false]") != NULL);
    assert(strstr(written_json, "[") != NULL);
    assert(strstr(written_json, "]") != NULL);

    /* Cross-check against a committed recording this test did not write --
     * same key names in the same shape, not a self-consistent-but-different
     * schema invented by writer and reader agreeing with each other alone. */
    char *real_events_json = read_file_raw("tests/fixtures/demo_events.json");
    assert(strstr(real_events_json, "\"frames\":") != NULL);
    assert(strstr(real_events_json, "\"mouse_x\":") != NULL);
    assert(strstr(real_events_json, "\"mouse_y\":") != NULL);
    assert(strstr(real_events_json, "\"mouse_buttons\":") != NULL);
    assert(strstr(real_events_json, "\"dpad\":") != NULL);
    assert(strstr(real_events_json, "false") != NULL);
    assert(strstr(real_events_json, "true") != NULL);

    free(written_json);
    free(real_events_json);

    /* That recording must itself load through pico8_input_load (sanity: the
     * shape assumptions above are grounded in a file the loader actually
     * accepts, not just string content). */
    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, "tests/fixtures/demo_events.json"));
    pico8_input_free(&input);
}

/* Offset of the last frame object's line in a recording -- i.e. the length
 * of the "history" region holding every frame recorded before the last
 * one. Used to assert that recording one more frame leaves that region
 * alone; deliberately derived from the file's own content rather than from
 * any constant of the writer's layout. */
static long history_region_length(const char *json) {
    const char *last = NULL;
    const char *p = json;
    while ((p = strstr(p, "    {")) != NULL) {
        last = p;
        p += 5;
    }
    assert(last != NULL);
    return (long)(last - json);
}

/* The regression test for the recording slowdown: recording frame N must
 * not re-emit the N-1 frames already on disk. Proven by marking the
 * already-recorded region with a byte no correct re-serialisation of those
 * frames could produce, then recording one more frame and checking the
 * mark survives -- a writer that rewrites history necessarily overwrites
 * it with the correct value. */
static void test_append_does_not_rewrite_already_recorded_frames(void) {
    const char *path = "tmp/test_pico8_input_append_history.json";
    long tail_offset = 0;

    for (int i = 0; i < 40; i++) {
        pico8_input_frame_t fr = {i % 128, (i * 3) % 128, i % 8, {i % 2, 0, 1, 0}};
        assert(pico8_input_append(&fr, path, &tail_offset));
    }

    char *before = read_file_raw(path);
    long history_len = history_region_length(before);
    char *victim = strstr(before, "\"mouse_x\": 0,");
    assert(victim != NULL);
    long victim_offset = (long)(victim - before) + (long)strlen("\"mouse_x\": ");
    assert(victim_offset < history_len);
    free(before);

    FILE *f = fopen(path, "r+b");
    assert(f);
    assert(fseek(f, victim_offset, SEEK_SET) == 0);
    assert(fputc('9', f) == '9');
    assert(fclose(f) == 0);

    char *marked = read_file_raw(path);

    pico8_input_frame_t extra = {77, 88, 1, {0, 0, 0, 1}};
    assert(pico8_input_append(&extra, path, &tail_offset));

    char *after = read_file_raw(path);
    assert(after[victim_offset] == '9');
    assert(memcmp(marked, after, (size_t)history_len) == 0);

    free(marked);
    free(after);
}

/* The recording must stay exactly the recording it was: after every single
 * appended frame the file on disk is byte-for-byte what pico8_input_save
 * would have written for the same frames, so a session recorded through
 * the append path replays identically to one recorded through the full
 * rewrite it replaces. */
static void test_append_recording_is_byte_identical_to_save_every_frame(void) {
    pico8_input_frame_t frames[7] = {
        {64, 32, 1, {0, 1, 0, 0, 1, 0}},
        {0, 127, 3, {1, 0, 1, 0, 0, 1}},
        {127, 0, 0, {0, 0, 0, 1, 0, 0}},
        {10, 20, 7, {1, 1, 1, 1, 1, 1}},
        {5, 99, 2, {0, 0, 0, 0, 0, 0}},
        {100, 100, 4, {0, 1, 1, 0, 1, 1}},
        {1, 2, 6, {1, 0, 0, 1, 0, 1}},
    };
    const char *appended_path = "tmp/test_pico8_input_append_identical.json";
    const char *reference_path = "tmp/test_pico8_input_append_reference.json";
    long tail_offset = 0;

    for (int i = 0; i < 7; i++) {
        assert(pico8_input_append(&frames[i], appended_path, &tail_offset));
        assert(pico8_input_save(frames, i + 1, reference_path));

        char *appended = read_file_raw(appended_path);
        char *reference = read_file_raw(reference_path);
        assert(strcmp(appended, reference) == 0);
        free(appended);
        free(reference);
    }

    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, appended_path));
    assert(input.frame_count == 7);
    for (int i = 0; i < 7; i++) {
        assert(pico8_stat(&input, 32) == frames[i].mouse_x);
        assert(pico8_stat(&input, 33) == frames[i].mouse_y);
        assert(pico8_stat(&input, 34) == frames[i].mouse_buttons);
        assert(pico8_btn(&input, 0) == frames[i].dpad[0]);
        assert(pico8_btn(&input, 1) == frames[i].dpad[1]);
        assert(pico8_btn(&input, 2) == frames[i].dpad[2]);
        assert(pico8_btn(&input, 3) == frames[i].dpad[3]);
        assert(pico8_btn(&input, 4) == frames[i].dpad[4]);
        assert(pico8_btn(&input, 5) == frames[i].dpad[5]);
        if (i + 1 < 7) {
            pico8_input_step(&input);
        }
    }
    pico8_input_free(&input);
}

/* A recording path is reused across sessions only by accident (the backend
 * timestamps it), but a fresh recording must never inherit a stale file's
 * frames: tail_offset 0 means "start over". */
static void test_append_with_zero_tail_offset_starts_a_fresh_recording(void) {
    const char *path = "tmp/test_pico8_input_append_fresh.json";
    pico8_input_frame_t first[3] = {
        {1, 1, 1, {1, 0, 0, 0}},
        {2, 2, 2, {0, 1, 0, 0}},
        {3, 3, 3, {0, 0, 1, 0}},
    };
    long tail_offset = 0;
    for (int i = 0; i < 3; i++) {
        assert(pico8_input_append(&first[i], path, &tail_offset));
    }

    pico8_input_frame_t only = {9, 9, 0, {0, 0, 0, 1}};
    tail_offset = 0;
    assert(pico8_input_append(&only, path, &tail_offset));

    pico8_input_t input;
    pico8_input_init(&input);
    assert(pico8_input_load(&input, path));
    assert(input.frame_count == 1);
    assert(pico8_stat(&input, 32) == 9);
    pico8_input_free(&input);
}

static void test_save_rejects_empty_frame_list(void) {
    const char *path = "tmp/test_pico8_input_save_empty.json";
    int saved = pico8_input_save(NULL, 0, path);
    assert(!saved);
}

int main(void) {
    test_load_and_step_multi_frame_matches_fields();
    test_four_boolean_dpad_loads_with_o_and_x_released();
    test_six_boolean_dpad_loads_o_and_x();
    test_load_malformed_five_boolean_dpad_is_hard_error();
    test_btn_out_of_range_returns_zero();
    test_stat_unsupported_index_returns_zero();
    test_load_missing_file_is_hard_error();
    test_load_malformed_missing_field_is_hard_error();
    test_load_malformed_short_dpad_is_hard_error();
    test_load_malformed_no_frames_key_is_hard_error();
    test_save_then_load_round_trips_every_field();
    test_save_writes_events_json_schema_shape();
    test_save_rejects_empty_frame_list();
    test_append_does_not_rewrite_already_recorded_frames();
    test_append_recording_is_byte_identical_to_save_every_frame();
    test_append_with_zero_tail_offset_starts_a_fresh_recording();

    printf("test_pico8_input: all tests passed\n");
    return 0;
}
