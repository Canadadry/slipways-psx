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

#include <assert.h>
#include <stdio.h>

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

    printf("test_replay_source: all tests passed\n");
    return 0;
}
