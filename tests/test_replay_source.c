/* The switchable input source: N frames from the file, then exhaustion.
 * That a file was read proves nothing, so every served frame is asserted
 * value by value and in order -- counting them, or checking them for being
 * non-zero, would pass on a source serving them backwards.
 *
 * The three open outcomes are pinned separately because an unreadable
 * recording must stay an explicit launch error and never become a silent
 * new game, which would look to the player like a lost save.
 *
 * The failure fixtures are the ones `tests/test_input.c` uses: this
 * module delegates "valid recording" to `p8_input_load`, so it must be
 * tested against that function's idea of a bad file, not a second one. */

#include "devtools/replay_source.h"
#include "game/save_state.h"
#include "game/save_storage.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_no_path_requests_no_recording(void) {
    replay_source_t source;

    assert(replay_source_open(&source, NULL) == REPLAY_SOURCE_NONE);
    assert(replay_source_frame_count(&source) == 0);

    replay_source_close(&source);
}

static void test_valid_recording_loads_with_its_frame_count(void) {
    replay_source_t source;

    assert(replay_source_open(&source, "tests/fixtures/events_valid.json") ==
           REPLAY_SOURCE_LOADED);
    assert(replay_source_frame_count(&source) == 3);

    replay_source_close(&source);
}

static void test_missing_path_reports_load_failure(void) {
    replay_source_t source;

    assert(replay_source_open(&source, "tests/fixtures/no_such_recording.json") ==
           REPLAY_SOURCE_LOAD_FAILED);
    assert(replay_source_frame_count(&source) == 0);

    replay_source_close(&source);
}

static void test_malformed_recording_reports_load_failure(void) {
    replay_source_t source;

    assert(replay_source_open(&source, "tests/fixtures/events_pad_missing_buttons.json") ==
           REPLAY_SOURCE_LOAD_FAILED);
    assert(replay_source_frame_count(&source) == 0);

    replay_source_close(&source);
}

/* The three frames of tests/fixtures/events_valid.json, transcribed: the
 * exact values are what catch an off-by-one, a reversed order or a frame
 * served twice. */
static const p8_input_frame_t events_valid_frames[] = {
    { 0, 0, 0, P8_PAD_RIGHT },
    { 0, 0, 0, P8_PAD_UP | P8_PAD_LEFT },
    { 0, 0, 0, P8_PAD_DOWN }
};
static const int events_valid_count = 3;

static void assert_frame_equals(const p8_input_frame_t *got, const p8_input_frame_t *want) {
    assert(got->mouse_x == want->mouse_x);
    assert(got->mouse_y == want->mouse_y);
    assert(got->mouse_buttons == want->mouse_buttons);
    assert(got->pad_buttons == want->pad_buttons);
}

static void test_loaded_recording_serves_its_first_frame(void) {
    replay_source_t source;
    p8_input_frame_t frame;

    assert(replay_source_open(&source, "tests/fixtures/events_valid.json") ==
           REPLAY_SOURCE_LOADED);

    assert(replay_source_next_frame(&source, &frame) == 1);
    assert_frame_equals(&frame, &events_valid_frames[0]);

    replay_source_close(&source);
}

/* A recording of N frames must feed the game those N frames, in the order
 * they were played, and nothing else. */
static void test_loaded_recording_serves_every_frame_in_order(void) {
    replay_source_t source;

    assert(replay_source_open(&source, "tests/fixtures/events_valid.json") ==
           REPLAY_SOURCE_LOADED);
    assert(replay_source_frame_count(&source) == events_valid_count);

    for (int i = 0; i < events_valid_count; i++) {
        p8_input_frame_t frame;
        assert(replay_source_next_frame(&source, &frame) == 1);
        assert_frame_equals(&frame, &events_valid_frames[i]);
    }

    replay_source_close(&source);
}

/* The end of the recording is where the player gets the controls back, so
 * it is reported once per frame, for as long as the session runs. A source
 * that aborted, looped back to frame 0 or re-served the last frame would
 * crash or keep driving the game from a file that ran out. The loop is far
 * longer than the recording on purpose: exhaustion is a terminal state. */
static void test_exhausted_recording_reports_exhaustion_forever(void) {
    replay_source_t source;
    p8_input_frame_t frame;

    assert(replay_source_open(&source, "tests/fixtures/events_valid.json") ==
           REPLAY_SOURCE_LOADED);

    for (int i = 0; i < events_valid_count; i++) {
        assert(replay_source_next_frame(&source, &frame) == 1);
    }

    for (int i = 0; i < 100; i++) {
        assert(replay_source_next_frame(&source, &frame) == 0);
    }

    replay_source_close(&source);
}

/* Why the driver needs no mode flag: launching without an argument is a
 * replay that is over before it starts, so the first call already says
 * "take your frames from the hardware". */
static void test_no_recording_is_exhausted_from_the_first_frame(void) {
    replay_source_t source;
    p8_input_frame_t frame;

    assert(replay_source_open(&source, NULL) == REPLAY_SOURCE_NONE);

    assert(replay_source_next_frame(&source, &frame) == 0);
    assert(replay_source_next_frame(&source, &frame) == 0);

    replay_source_close(&source);
}

/* An accessor on the cursor this module already keeps, not a second
 * counter the driver maintains: two counters drift, and a progress line
 * that disagrees with the replay is worse than none. Counted in frames
 * served, so 1 on the first frame and N on the last -- one ahead of the
 * 0-based index the marker file writes. */
static void test_frames_served_tracks_the_replay_position(void) {
    replay_source_t source;
    p8_input_frame_t frame;

    assert(replay_source_open(&source, "tests/fixtures/events_valid.json") ==
           REPLAY_SOURCE_LOADED);
    assert(replay_source_frames_served(&source) == 0);

    assert(replay_source_next_frame(&source, &frame) == 1);
    assert(replay_source_frames_served(&source) == 1);
    assert(replay_source_next_frame(&source, &frame) == 1);
    assert(replay_source_next_frame(&source, &frame) == 1);
    assert(replay_source_frames_served(&source) == 3);
    assert(replay_source_frames_served(&source) == replay_source_frame_count(&source));

    /* Exhaustion does not advance it: the replay stays at N of N instead
     * of counting the live frames the player now plays. */
    assert(replay_source_next_frame(&source, &frame) == 0);
    assert(replay_source_frames_served(&source) == 3);

    replay_source_close(&source);
}

#define PAD_FRAME_COUNT 3
static const p8_input_frame_t pad_frames[PAD_FRAME_COUNT] = {
    { 0, 0, 0, P8_PAD_CROSS },
    { 0, 0, 0, 0 },
    { 0, 0, 0, P8_PAD_UP | P8_PAD_CIRCLE | P8_PAD_START },
};

static void test_pad_recording_serves_its_masks_in_order(void) {
    const char *path = "tmp/test_replay_source_pad.json";
    assert(p8_input_save(pad_frames, PAD_FRAME_COUNT, path));

    replay_source_t source;
    assert(replay_source_open(&source, path) == REPLAY_SOURCE_LOADED);
    assert(replay_source_frame_count(&source) == PAD_FRAME_COUNT);
    for (int i = 0; i < PAD_FRAME_COUNT; i++) {
        p8_input_frame_t frame;
        assert(replay_source_next_frame(&source, &frame) == 1);
        assert_frame_equals(&frame, &pad_frames[i]);
    }

    replay_source_close(&source);
    remove(path);
}

static void test_old_mouse_recording_reports_load_failure(void) {
    replay_source_t source;

    assert(replay_source_open(&source, "tests/fixtures/events_mouse_recording_refused.json") ==
           REPLAY_SOURCE_LOAD_FAILED);
    assert(replay_source_frame_count(&source) == 0);

    replay_source_close(&source);
}

#define SAVE_FRAMES_TEXT "  \"frames\": [\n    { \"pad\": [\"up\"] },\n    { \"pad\": [] }\n  ]\n"

static void write_text(const char *path, const char *text) {
    FILE *file = fopen(path, "wb");
    assert(file);
    assert(fputs(text, file) >= 0);
    assert(fclose(file) == 0);
}

/* Writes a recording whose header carries `member` (NULL for none) after "input". */
static void write_recording_with_member(const char *path, const char *member) {
    char text[4 * SAVE_STATE_MAX_BYTES];
    int used = snprintf(text, sizeof(text), "{\n  \"input\": \"pad\",\n%s%s%s" SAVE_FRAMES_TEXT "}\n",
                        member ? "  " : "", member ? member : "", member ? ",\n" : "");
    assert(used > 0 && (size_t)used < sizeof(text));
    write_text(path, text);
}

static void store_a_record(void) {
    const uint8_t previous[3] = { 9, 9, 9 };
    assert(save_storage_write(previous, 3) == 1);
}

static void assert_storage_holds(const uint8_t *bytes, int count) {
    static uint8_t got[SAVE_STATE_MAX_BYTES];
    assert(save_storage_read(got, SAVE_STATE_MAX_BYTES) == count);
    assert(memcmp(got, bytes, (size_t)count) == 0);
}

static void test_initial_save_seeds_the_storage_with_its_bytes(void) {
    const char *path = "tmp/test_replay_source_initial_save.json";
    write_recording_with_member(path, "\"initial_save\": \"00a17fff\"");
    store_a_record();

    replay_source_t source;
    assert(replay_source_open(&source, path) == REPLAY_SOURCE_LOADED);
    assert(replay_source_frame_count(&source) == 2);
    assert(replay_source_seed_storage(&source) == 1);
    const uint8_t expected[4] = { 0x00, 0xa1, 0x7f, 0xff };
    assert_storage_holds(expected, 4);

    replay_source_close(&source);
    remove(path);
}

static void test_recording_without_initial_save_empties_the_storage(void) {
    const char *path = "tmp/test_replay_source_no_initial_save.json";
    write_recording_with_member(path, NULL);
    store_a_record();

    replay_source_t source;
    assert(replay_source_open(&source, path) == REPLAY_SOURCE_LOADED);
    assert(replay_source_seed_storage(&source) == 1);
    static uint8_t got[SAVE_STATE_MAX_BYTES];
    assert(save_storage_read(got, SAVE_STATE_MAX_BYTES) == 0);

    replay_source_close(&source);
    remove(path);
}

/* Live play reads whatever the storage holds: the player's own save. */
static void test_live_play_leaves_the_storage_untouched(void) {
    store_a_record();

    replay_source_t source;
    assert(replay_source_open(&source, NULL) == REPLAY_SOURCE_NONE);
    assert(replay_source_seed_storage(&source) == 1);
    const uint8_t previous[3] = { 9, 9, 9 };
    assert_storage_holds(previous, 3);

    replay_source_close(&source);
}

static void test_initial_save_accepts_uppercase_hex(void) {
    const char *path = "tmp/test_replay_source_uppercase.json";
    write_recording_with_member(path, "\"initial_save\": \"ABcdEF\"");

    replay_source_t source;
    assert(replay_source_open(&source, path) == REPLAY_SOURCE_LOADED);
    assert(replay_source_seed_storage(&source) == 1);
    const uint8_t expected[3] = { 0xab, 0xcd, 0xef };
    assert_storage_holds(expected, 3);

    replay_source_close(&source);
    remove(path);
}

/* A session that started from a corrupt save must replay as it happened:
 * the bytes are never decoded on load. One byte is shorter than any
 * record's header. */
static void test_initial_save_that_is_not_a_valid_record_loads(void) {
    const char *path = "tmp/test_replay_source_invalid_record.json";
    write_recording_with_member(path, "\"initial_save\": \"00\"");
    const uint8_t record[1] = { 0 };

    replay_source_t source;
    assert(replay_source_open(&source, path) == REPLAY_SOURCE_LOADED);
    assert(replay_source_seed_storage(&source) == 1);
    assert_storage_holds(record, 1);

    replay_source_close(&source);
    remove(path);
}

static void assert_member_fails_to_load(const char *member) {
    const char *path = "tmp/test_replay_source_malformed_initial_save.json";
    write_recording_with_member(path, member);

    replay_source_t source;
    assert(replay_source_open(&source, path) == REPLAY_SOURCE_LOAD_FAILED);
    assert(replay_source_frame_count(&source) == 0);

    replay_source_close(&source);
    remove(path);
}

/* `digit_count` hex digits "ab" repeated, as an initial_save member. */
static void format_long_member(char *member, size_t size, int digit_count) {
    int used = snprintf(member, size, "\"initial_save\": \"");
    assert(used > 0 && (size_t)(used + digit_count + 2) < size);
    for (int i = 0; i < digit_count; i++) {
        member[used + i] = (i % 2) ? 'b' : 'a';
    }
    member[used + digit_count] = '"';
    member[used + digit_count + 1] = '\0';
}

static void test_malformed_initial_save_fails_to_load(void) {
    assert_member_fails_to_load("\"initial_save\": \"abc\"");
    assert_member_fails_to_load("\"initial_save\": \"0g\"");
    assert_member_fails_to_load("\"initial_save\": \"\"");
    assert_member_fails_to_load("\"initial_save\": 12");

    static char member[2 * SAVE_STATE_MAX_BYTES + 64];
    format_long_member(member, sizeof(member), 2 * SAVE_STATE_MAX_BYTES + 2);
    assert_member_fails_to_load(member);
}

static void test_initial_save_of_the_full_size_loads(void) {
    const char *path = "tmp/test_replay_source_full_size.json";
    static char member[2 * SAVE_STATE_MAX_BYTES + 64];
    format_long_member(member, sizeof(member), 2 * SAVE_STATE_MAX_BYTES);
    write_recording_with_member(path, member);

    replay_source_t source;
    assert(replay_source_open(&source, path) == REPLAY_SOURCE_LOADED);
    assert(replay_source_seed_storage(&source) == 1);
    static uint8_t expected[SAVE_STATE_MAX_BYTES];
    memset(expected, 0xab, sizeof(expected));
    assert_storage_holds(expected, SAVE_STATE_MAX_BYTES);

    replay_source_close(&source);
    remove(path);
}

/* The rule "input" follows: only the header, before "frames", counts. */
static void test_initial_save_after_the_frames_is_ignored(void) {
    const char *path = "tmp/test_replay_source_after_frames.json";
    write_text(path, "{\n  \"input\": \"pad\",\n" SAVE_FRAMES_TEXT "  ,\"initial_save\": \"zz\"\n}\n");
    store_a_record();

    replay_source_t source;
    assert(replay_source_open(&source, path) == REPLAY_SOURCE_LOADED);
    assert(replay_source_seed_storage(&source) == 1);
    static uint8_t got[SAVE_STATE_MAX_BYTES];
    assert(save_storage_read(got, SAVE_STATE_MAX_BYTES) == 0);

    replay_source_close(&source);
    remove(path);
}

int main(void) {
    test_no_path_requests_no_recording();
    test_valid_recording_loads_with_its_frame_count();
    test_missing_path_reports_load_failure();
    test_malformed_recording_reports_load_failure();
    test_loaded_recording_serves_its_first_frame();
    test_loaded_recording_serves_every_frame_in_order();
    test_exhausted_recording_reports_exhaustion_forever();
    test_no_recording_is_exhausted_from_the_first_frame();
    test_frames_served_tracks_the_replay_position();
    test_pad_recording_serves_its_masks_in_order();
    test_old_mouse_recording_reports_load_failure();
    test_initial_save_seeds_the_storage_with_its_bytes();
    test_recording_without_initial_save_empties_the_storage();
    test_live_play_leaves_the_storage_untouched();
    test_initial_save_accepts_uppercase_hex();
    test_initial_save_that_is_not_a_valid_record_loads();
    test_malformed_initial_save_fails_to_load();
    test_initial_save_of_the_full_size_loads();
    test_initial_save_after_the_frames_is_ignored();

    printf("test_replay_source: all tests passed\n");
    return 0;
}
