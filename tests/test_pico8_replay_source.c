/* The switchable input source: N frames from the file, then exhaustion.
 * That a file was read proves nothing, so every served frame is asserted
 * value by value and in order -- counting them, or checking them for being
 * non-zero, would pass on a source serving them backwards.
 *
 * The three open outcomes are pinned separately because an unreadable
 * recording must stay an explicit launch error and never become a silent
 * new game, which would look to the player like a lost save.
 *
 * The failure fixtures are the ones `tests/test_pico8_input.c` uses: this
 * module delegates "valid recording" to `pico8_input_load`, so it must be
 * tested against that function's idea of a bad file, not a second one. */

#include "../src/pico8_replay_source.h"

#include <assert.h>
#include <stdio.h>

static void test_no_path_requests_no_recording(void) {
    pico8_replay_source_t source;

    assert(pico8_replay_source_open(&source, NULL) == PICO8_REPLAY_SOURCE_NONE);
    assert(pico8_replay_source_frame_count(&source) == 0);

    pico8_replay_source_close(&source);
}

static void test_valid_recording_loads_with_its_frame_count(void) {
    pico8_replay_source_t source;

    assert(pico8_replay_source_open(&source, "tests/fixtures/events_valid.json") ==
           PICO8_REPLAY_SOURCE_LOADED);
    assert(pico8_replay_source_frame_count(&source) == 3);

    pico8_replay_source_close(&source);
}

static void test_missing_path_reports_load_failure(void) {
    pico8_replay_source_t source;

    assert(pico8_replay_source_open(&source, "tests/fixtures/no_such_recording.json") ==
           PICO8_REPLAY_SOURCE_LOAD_FAILED);
    assert(pico8_replay_source_frame_count(&source) == 0);

    pico8_replay_source_close(&source);
}

static void test_malformed_recording_reports_load_failure(void) {
    pico8_replay_source_t source;

    assert(pico8_replay_source_open(&source, "tests/fixtures/events_malformed_missing_field.json") ==
           PICO8_REPLAY_SOURCE_LOAD_FAILED);
    assert(pico8_replay_source_frame_count(&source) == 0);

    pico8_replay_source_close(&source);
}

/* The three frames of tests/fixtures/events_valid.json, transcribed: the
 * exact values are what catch an off-by-one, a reversed order or a frame
 * served twice. */
static const pico8_input_frame_t k_events_valid[] = {
    { 64, 64, 1, { 0, 1, 0, 0 } },
    { 10, 20, 3, { 1, 0, 1, 0 } },
    { 0, 127, 4, { 0, 0, 0, 1 } }
};
static const int k_events_valid_count = 3;

static void assert_frame_equals(const pico8_input_frame_t *got, const pico8_input_frame_t *want) {
    assert(got->mouse_x == want->mouse_x);
    assert(got->mouse_y == want->mouse_y);
    assert(got->mouse_buttons == want->mouse_buttons);
    assert(got->dpad[0] == want->dpad[0]);
    assert(got->dpad[1] == want->dpad[1]);
    assert(got->dpad[2] == want->dpad[2]);
    assert(got->dpad[3] == want->dpad[3]);
}

static void test_loaded_recording_serves_its_first_frame(void) {
    pico8_replay_source_t source;
    pico8_input_frame_t frame;

    assert(pico8_replay_source_open(&source, "tests/fixtures/events_valid.json") ==
           PICO8_REPLAY_SOURCE_LOADED);

    assert(pico8_replay_source_next_frame(&source, &frame) == 1);
    assert_frame_equals(&frame, &k_events_valid[0]);

    pico8_replay_source_close(&source);
}

/* A recording of N frames must feed the game those N frames, in the order
 * they were played, and nothing else. */
static void test_loaded_recording_serves_every_frame_in_order(void) {
    pico8_replay_source_t source;

    assert(pico8_replay_source_open(&source, "tests/fixtures/events_valid.json") ==
           PICO8_REPLAY_SOURCE_LOADED);
    assert(pico8_replay_source_frame_count(&source) == k_events_valid_count);

    for (int i = 0; i < k_events_valid_count; i++) {
        pico8_input_frame_t frame;
        assert(pico8_replay_source_next_frame(&source, &frame) == 1);
        assert_frame_equals(&frame, &k_events_valid[i]);
    }

    pico8_replay_source_close(&source);
}

/* The end of the recording is where the player gets the controls back, so
 * it is reported once per frame, for as long as the session runs. A source
 * that aborted, looped back to frame 0 or re-served the last frame would
 * crash or keep driving the game from a file that ran out. The loop is far
 * longer than the recording on purpose: exhaustion is a terminal state. */
static void test_exhausted_recording_reports_exhaustion_forever(void) {
    pico8_replay_source_t source;
    pico8_input_frame_t frame;

    assert(pico8_replay_source_open(&source, "tests/fixtures/events_valid.json") ==
           PICO8_REPLAY_SOURCE_LOADED);

    for (int i = 0; i < k_events_valid_count; i++) {
        assert(pico8_replay_source_next_frame(&source, &frame) == 1);
    }

    for (int i = 0; i < 100; i++) {
        assert(pico8_replay_source_next_frame(&source, &frame) == 0);
    }

    pico8_replay_source_close(&source);
}

/* Why the driver needs no mode flag: launching without an argument is a
 * replay that is over before it starts, so the first call already says
 * "take your frames from the hardware". */
static void test_no_recording_is_exhausted_from_the_first_frame(void) {
    pico8_replay_source_t source;
    pico8_input_frame_t frame;

    assert(pico8_replay_source_open(&source, NULL) == PICO8_REPLAY_SOURCE_NONE);

    assert(pico8_replay_source_next_frame(&source, &frame) == 0);
    assert(pico8_replay_source_next_frame(&source, &frame) == 0);

    pico8_replay_source_close(&source);
}

/* An accessor on the cursor this module already keeps, not a second
 * counter the driver maintains: two counters drift, and a progress line
 * that disagrees with the replay is worse than none. Counted in frames
 * SERVED, so 1 on the first frame and N on the last -- one ahead of the
 * 0-based index the marker file writes. */
static void test_frames_served_tracks_the_replay_position(void) {
    pico8_replay_source_t source;
    pico8_input_frame_t frame;

    assert(pico8_replay_source_open(&source, "tests/fixtures/events_valid.json") ==
           PICO8_REPLAY_SOURCE_LOADED);
    assert(pico8_replay_source_frames_served(&source) == 0);

    assert(pico8_replay_source_next_frame(&source, &frame) == 1);
    assert(pico8_replay_source_frames_served(&source) == 1);
    assert(pico8_replay_source_next_frame(&source, &frame) == 1);
    assert(pico8_replay_source_next_frame(&source, &frame) == 1);
    assert(pico8_replay_source_frames_served(&source) == 3);
    assert(pico8_replay_source_frames_served(&source) == pico8_replay_source_frame_count(&source));

    /* Exhaustion does not advance it: the replay stays at N of N instead
     * of counting the live frames the player now plays. */
    assert(pico8_replay_source_next_frame(&source, &frame) == 0);
    assert(pico8_replay_source_frames_served(&source) == 3);

    pico8_replay_source_close(&source);
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

    printf("test_pico8_replay_source: all tests passed\n");
    return 0;
}
