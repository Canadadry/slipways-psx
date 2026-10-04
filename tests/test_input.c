#include "p8/input.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_load_and_step_multi_frame_matches_fields(void) {
    p8_input_t input;
    p8_input_init(&input);

    int ok = p8_input_load(&input, "tests/fixtures/events_valid.json");
    assert(ok);

    assert(p8_btn(&input, 0) == 0);
    assert(p8_btn(&input, 1) == 1);
    assert(p8_btn(&input, 2) == 0);
    assert(p8_btn(&input, 3) == 0);
    assert(p8_input_read_mouse(&input).x == 64);
    assert(p8_input_read_mouse(&input).y == 64);
    assert(p8_input_read_mouse(&input).buttons == 1);

    p8_input_step(&input);
    assert(p8_btn(&input, 0) == 1);
    assert(p8_btn(&input, 1) == 0);
    assert(p8_btn(&input, 2) == 1);
    assert(p8_btn(&input, 3) == 0);
    assert(p8_input_read_mouse(&input).x == 10);
    assert(p8_input_read_mouse(&input).y == 20);
    assert(p8_input_read_mouse(&input).buttons == 3);

    p8_input_step(&input);
    assert(p8_btn(&input, 0) == 0);
    assert(p8_btn(&input, 1) == 0);
    assert(p8_btn(&input, 2) == 0);
    assert(p8_btn(&input, 3) == 1);
    assert(p8_input_read_mouse(&input).x == 0);
    assert(p8_input_read_mouse(&input).y == 127);
    assert(p8_input_read_mouse(&input).buttons == 4);

    p8_input_free(&input);
}

static void test_four_boolean_dpad_loads_with_o_and_x_released(void) {
    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, "tests/fixtures/events_valid.json"));

    assert(p8_btn(&input, 4) == 0);
    assert(p8_btn(&input, 5) == 0);

    p8_input_free(&input);
}

static void test_six_boolean_dpad_loads_o_and_x(void) {
    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, "tests/fixtures/events_valid_ox.json"));

    assert(p8_btn(&input, 1) == 1);
    assert(p8_btn(&input, 4) == 1);
    assert(p8_btn(&input, 5) == 0);
    p8_input_step(&input);
    assert(p8_btn(&input, 0) == 1);
    assert(p8_btn(&input, 2) == 1);
    assert(p8_btn(&input, 4) == 0);
    assert(p8_btn(&input, 5) == 1);

    p8_input_free(&input);
}

static void test_load_malformed_five_boolean_dpad_is_hard_error(void) {
    p8_input_t input;
    p8_input_init(&input);

    assert(!p8_input_load(&input, "tests/fixtures/events_malformed_five_dpad.json"));

    p8_input_free(&input);
}

static void test_btn_out_of_range_returns_zero(void) {
    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, "tests/fixtures/events_valid.json"));

    assert(p8_btn(&input, -1) == 0);
    assert(p8_btn(&input, 6) == 0);

    p8_input_free(&input);
}

static void test_read_mouse_without_a_current_frame_is_all_zero(void) {
    p8_input_t input;
    p8_input_init(&input);

    p8_mouse_t mouse = p8_input_read_mouse(&input);
    assert(mouse.x == 0 && mouse.y == 0 && mouse.buttons == 0);

    assert(p8_input_load(&input, "tests/fixtures/events_valid.json"));
    input.current_frame = input.frame_count;
    mouse = p8_input_read_mouse(&input);
    assert(mouse.x == 0 && mouse.y == 0 && mouse.buttons == 0);

    p8_input_free(&input);
}

static void test_load_missing_file_is_hard_error(void) {
    p8_input_t input;
    p8_input_init(&input);

    int ok = p8_input_load(&input, "tests/fixtures/does_not_exist.json");
    assert(!ok);

    p8_input_free(&input);
}

static void test_load_malformed_missing_field_is_hard_error(void) {
    p8_input_t input;
    p8_input_init(&input);

    int ok = p8_input_load(&input, "tests/fixtures/events_malformed_missing_field.json");
    assert(!ok);

    p8_input_free(&input);
}

static void test_load_malformed_short_dpad_is_hard_error(void) {
    p8_input_t input;
    p8_input_init(&input);

    int ok = p8_input_load(&input, "tests/fixtures/events_malformed_short_dpad.json");
    assert(!ok);

    p8_input_free(&input);
}

static void test_load_malformed_no_frames_key_is_hard_error(void) {
    p8_input_t input;
    p8_input_init(&input);

    int ok = p8_input_load(&input, "tests/fixtures/events_malformed_no_frames_key.json");
    assert(!ok);

    p8_input_free(&input);
}

static void test_declared_keyboard_cursor_loads_on(void) {
    p8_input_t input;
    p8_input_init(&input);

    assert(p8_input_load(&input, "tests/fixtures/events_keyboard_cursor_true.json"));
    assert(input.keyboard_cursor == 1);

    p8_input_free(&input);
}

static void test_undeclared_keyboard_cursor_loads_off(void) {
    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, "tests/fixtures/events_keyboard_cursor_true.json"));

    assert(p8_input_load(&input, "tests/fixtures/events_valid.json"));
    assert(input.keyboard_cursor == 0);

    p8_input_free(&input);
}

static void test_keyboard_cursor_declared_false_loads_off(void) {
    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, "tests/fixtures/events_keyboard_cursor_true.json"));

    assert(p8_input_load(&input, "tests/fixtures/events_keyboard_cursor_false.json"));
    assert(input.keyboard_cursor == 0);

    p8_input_free(&input);
}

static void test_malformed_keyboard_cursor_fails_and_leaves_input_untouched(void) {
    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, "tests/fixtures/events_keyboard_cursor_true.json"));
    p8_input_frame_t *frames_before = input.frames;

    assert(!p8_input_load(&input, "tests/fixtures/events_keyboard_cursor_malformed.json"));
    assert(input.keyboard_cursor == 1);
    assert(input.frames == frames_before);
    assert(input.frame_count == 1);

    p8_input_free(&input);
}

static void test_keyboard_cursor_after_the_frames_is_not_read(void) {
    p8_input_t input;
    p8_input_init(&input);

    assert(p8_input_load(&input, "tests/fixtures/events_keyboard_cursor_after_frames.json"));
    assert(input.keyboard_cursor == 0);

    p8_input_free(&input);
}

static void test_init_and_free_leave_keyboard_cursor_off(void) {
    p8_input_t input;
    input.keyboard_cursor = 1;
    p8_input_init(&input);
    assert(input.keyboard_cursor == 0);

    assert(p8_input_load(&input, "tests/fixtures/events_keyboard_cursor_true.json"));
    p8_input_free(&input);
    assert(input.keyboard_cursor == 0);
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
    p8_input_frame_t written[5] = {
        {64, 32, 1, {0, 1, 0, 0, 1, 0}},
        {0, 127, 3, {1, 0, 1, 0, 0, 1}},
        {127, 0, 0, {0, 0, 0, 1, 0, 0}},
        {10, 20, 7, {1, 1, 1, 1, 1, 1}},
        {5, 99, 2, {0, 0, 0, 0, 0, 0}},
    };
    const char *path = "tmp/test_input_save_roundtrip.json";

    int saved = p8_input_save(written, 5, path);
    assert(saved);

    p8_input_t input;
    p8_input_init(&input);
    int loaded = p8_input_load(&input, path);
    assert(loaded);
    assert(input.frame_count == 5);

    for (int i = 0; i < 5; i++) {
        assert(p8_input_read_mouse(&input).x == written[i].mouse_x);
        assert(p8_input_read_mouse(&input).y == written[i].mouse_y);
        assert(p8_input_read_mouse(&input).buttons == written[i].mouse_buttons);
        assert(p8_btn(&input, 0) == written[i].dpad[0]);
        assert(p8_btn(&input, 1) == written[i].dpad[1]);
        assert(p8_btn(&input, 2) == written[i].dpad[2]);
        assert(p8_btn(&input, 3) == written[i].dpad[3]);
        assert(p8_btn(&input, 4) == written[i].dpad[4]);
        assert(p8_btn(&input, 5) == written[i].dpad[5]);
        if (i + 1 < 5) {
            p8_input_step(&input);
        }
    }

    p8_input_free(&input);
}

static void test_save_writes_events_json_schema_shape(void) {
    p8_input_frame_t written[2] = {
        {64, 32, 1, {0, 1, 0, 0}},
        {10, 20, 3, {1, 0, 0, 1}},
    };
    const char *path = "tmp/test_input_save_shape.json";

    assert(p8_input_save(written, 2, path));

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

    /* That recording must itself load through p8_input_load (sanity: the
     * shape assumptions above are grounded in a file the loader actually
     * accepts, not just string content). */
    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, "tests/fixtures/demo_events.json"));
    p8_input_free(&input);
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
    const char *path = "tmp/test_input_append_history.json";
    long tail_offset = 0;

    for (int i = 0; i < 40; i++) {
        p8_input_frame_t fr = {i % 128, (i * 3) % 128, i % 8, {i % 2, 0, 1, 0}};
        assert(p8_input_append(&fr, path, &tail_offset));
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

    p8_input_frame_t extra = {77, 88, 1, {0, 0, 0, 1}};
    assert(p8_input_append(&extra, path, &tail_offset));

    char *after = read_file_raw(path);
    assert(after[victim_offset] == '9');
    assert(memcmp(marked, after, (size_t)history_len) == 0);

    free(marked);
    free(after);
}

/* The recording must stay exactly the recording it was: after every single
 * appended frame the file on disk is byte-for-byte what p8_input_save
 * would have written for the same frames, so a session recorded through
 * the append path replays identically to one recorded through the full
 * rewrite it replaces. */
static void test_append_recording_is_byte_identical_to_save_every_frame(void) {
    p8_input_frame_t frames[7] = {
        {64, 32, 1, {0, 1, 0, 0, 1, 0}},
        {0, 127, 3, {1, 0, 1, 0, 0, 1}},
        {127, 0, 0, {0, 0, 0, 1, 0, 0}},
        {10, 20, 7, {1, 1, 1, 1, 1, 1}},
        {5, 99, 2, {0, 0, 0, 0, 0, 0}},
        {100, 100, 4, {0, 1, 1, 0, 1, 1}},
        {1, 2, 6, {1, 0, 0, 1, 0, 1}},
    };
    const char *appended_path = "tmp/test_input_append_identical.json";
    const char *reference_path = "tmp/test_input_append_reference.json";
    long tail_offset = 0;

    for (int i = 0; i < 7; i++) {
        assert(p8_input_append(&frames[i], appended_path, &tail_offset));
        assert(p8_input_save(frames, i + 1, reference_path));

        char *appended = read_file_raw(appended_path);
        char *reference = read_file_raw(reference_path);
        assert(strcmp(appended, reference) == 0);
        free(appended);
        free(reference);
    }

    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, appended_path));
    assert(input.frame_count == 7);
    for (int i = 0; i < 7; i++) {
        assert(p8_input_read_mouse(&input).x == frames[i].mouse_x);
        assert(p8_input_read_mouse(&input).y == frames[i].mouse_y);
        assert(p8_input_read_mouse(&input).buttons == frames[i].mouse_buttons);
        assert(p8_btn(&input, 0) == frames[i].dpad[0]);
        assert(p8_btn(&input, 1) == frames[i].dpad[1]);
        assert(p8_btn(&input, 2) == frames[i].dpad[2]);
        assert(p8_btn(&input, 3) == frames[i].dpad[3]);
        assert(p8_btn(&input, 4) == frames[i].dpad[4]);
        assert(p8_btn(&input, 5) == frames[i].dpad[5]);
        if (i + 1 < 7) {
            p8_input_step(&input);
        }
    }
    p8_input_free(&input);
}

/* A recording path is reused across sessions only by accident (the backend
 * timestamps it), but a fresh recording must never inherit a stale file's
 * frames: tail_offset 0 means "start over". */
static void test_append_with_zero_tail_offset_starts_a_fresh_recording(void) {
    const char *path = "tmp/test_input_append_fresh.json";
    p8_input_frame_t first[3] = {
        {1, 1, 1, {1, 0, 0, 0}},
        {2, 2, 2, {0, 1, 0, 0}},
        {3, 3, 3, {0, 0, 1, 0}},
    };
    long tail_offset = 0;
    for (int i = 0; i < 3; i++) {
        assert(p8_input_append(&first[i], path, &tail_offset));
    }

    p8_input_frame_t only = {9, 9, 0, {0, 0, 0, 1}};
    tail_offset = 0;
    assert(p8_input_append(&only, path, &tail_offset));

    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, path));
    assert(input.frame_count == 1);
    assert(p8_input_read_mouse(&input).x == 9);
    p8_input_free(&input);
}

static void test_save_rejects_empty_frame_list(void) {
    const char *path = "tmp/test_input_save_empty.json";
    int saved = p8_input_save(NULL, 0, path);
    assert(!saved);
}

int main(void) {
    test_load_and_step_multi_frame_matches_fields();
    test_four_boolean_dpad_loads_with_o_and_x_released();
    test_six_boolean_dpad_loads_o_and_x();
    test_load_malformed_five_boolean_dpad_is_hard_error();
    test_btn_out_of_range_returns_zero();
    test_read_mouse_without_a_current_frame_is_all_zero();
    test_load_missing_file_is_hard_error();
    test_load_malformed_missing_field_is_hard_error();
    test_load_malformed_short_dpad_is_hard_error();
    test_load_malformed_no_frames_key_is_hard_error();
    test_declared_keyboard_cursor_loads_on();
    test_undeclared_keyboard_cursor_loads_off();
    test_keyboard_cursor_declared_false_loads_off();
    test_malformed_keyboard_cursor_fails_and_leaves_input_untouched();
    test_keyboard_cursor_after_the_frames_is_not_read();
    test_init_and_free_leave_keyboard_cursor_off();
    test_save_then_load_round_trips_every_field();
    test_save_writes_events_json_schema_shape();
    test_save_rejects_empty_frame_list();
    test_append_does_not_rewrite_already_recorded_frames();
    test_append_recording_is_byte_identical_to_save_every_frame();
    test_append_with_zero_tail_offset_starts_a_fresh_recording();

    printf("test_input: all tests passed\n");
    return 0;
}
