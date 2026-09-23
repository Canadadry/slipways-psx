/* A marker is one 0-based frame index, appended to a markers file when the
 * player taps space, and meant to be read as an index into the paired
 * recording's "frames" array. Coherent means exactly that the number
 * written and the frame's position in the recording on disk are the same.
 *
 * A resumed session is where that can break: it replays N frames the player
 * did not play before recording any they did. The marker index comes from
 * `pico8_session_recorder_frame_count()`, so everything turns on whether
 * that counter counts the replayed frames too -- it does, because every
 * frame about to be played is recorded, replayed or live alike.
 *
 * Every iteration is checked, not one chosen marker: an off-by-N can cancel
 * out at a single frame.
 *
 * Not covered here: the space bar itself, and that the marker file gets one
 * line per tap. Both need a window. What is covered is the number those
 * lines would carry. */

#include "../src/pico8_session_recorder.h"
#include "../src/pico8_replay_source.h"

#include <assert.h>
#include <stdio.h>

/* The session being resumed. Every coordinate stays below 50, and the live
 * frames below stay above it, so a marker pointing into the wrong half
 * names a visibly different frame rather than just an unequal number. */
#define REPLAYED_FRAME_COUNT 6
static const pico8_input_frame_t k_replayed_frames[REPLAYED_FRAME_COUNT] = {
    { 20, 21, 0, { 0, 0, 0, 0 } },
    { 22, 23, 1, { 1, 0, 0, 0 } },
    { 24, 25, 2, { 0, 1, 0, 0 } },
    { 26, 27, 4, { 0, 0, 1, 0 } },
    { 28, 29, 3, { 0, 0, 0, 1 } },
    { 30, 31, 5, { 1, 1, 0, 0 } },
};

/* What the player does once the controls come back. */
#define LIVE_FRAME_COUNT 5
static const pico8_input_frame_t k_live_frames[LIVE_FRAME_COUNT] = {
    { 80, 81, 1, { 1, 0, 1, 0 } },
    { 82, 83, 2, { 0, 1, 0, 1 } },
    { 84, 85, 4, { 1, 1, 1, 0 } },
    { 86, 87, 7, { 0, 0, 1, 1 } },
    { 88, 89, 6, { 1, 0, 0, 1 } },
};

#define TOTAL_FRAME_COUNT (REPLAYED_FRAME_COUNT + LIVE_FRAME_COUNT)

static void assert_frame_equals(const pico8_input_frame_t *got, const pico8_input_frame_t *want) {
    assert(got->mouse_x == want->mouse_x);
    assert(got->mouse_y == want->mouse_y);
    assert(got->mouse_buttons == want->mouse_buttons);
    assert(got->dpad[0] == want->dpad[0]);
    assert(got->dpad[1] == want->dpad[1]);
    assert(got->dpad[2] == want->dpad[2]);
    assert(got->dpad[3] == want->dpad[3]);
}

/* `main_raylib.c`'s loop, with the hardware calls scripted and the game
 * step dropped. `marker_for_iteration[i]` is what a space-bar release on
 * iteration i would write, read exactly where the backend reads it: just
 * BEFORE the frame is recorded. That placement is the whole arithmetic --
 * the count is the index the next recorded frame will take, so one line
 * later it would name the following frame. */
static void run_resumed_session(const char *source_path, const char *resumed_path, int *marker_for_iteration) {
    pico8_replay_source_t replay;
    assert(pico8_replay_source_open(&replay, source_path) == PICO8_REPLAY_SOURCE_LOADED);

    pico8_session_recorder_t recorder;
    pico8_session_recorder_open(&recorder, resumed_path);

    int live_index = 0;
    for (int i = 0; i < TOTAL_FRAME_COUNT; i++) {
        pico8_input_frame_t frame;
        if (!pico8_replay_source_next_frame(&replay, &frame)) {
            frame = k_live_frames[live_index];
            live_index++;
        }
        marker_for_iteration[i] = pico8_session_recorder_frame_count(&recorder);
        assert(pico8_session_recorder_record_frame(&recorder, &frame) == 1);
    }

    assert(live_index == LIVE_FRAME_COUNT);
    pico8_session_recorder_close(&recorder);
    pico8_replay_source_close(&replay);
}

/* A marker on the k-th live frame of a resumed session must be N+k, not k:
 * the recording it indexes into opens with the N replayed frames. */
static void test_marker_index_counts_the_replayed_frames_too(void) {
    const char *source_path = "tmp/test_marker_coherence_source.json";
    const char *resumed_path = "tmp/test_marker_coherence_resumed.json";
    int marker_for_iteration[TOTAL_FRAME_COUNT];

    /* Written by `pico8_input_save`, not by the recorder under test. */
    assert(pico8_input_save(k_replayed_frames, REPLAYED_FRAME_COUNT, source_path));

    run_resumed_session(source_path, resumed_path, marker_for_iteration);

    for (int i = 0; i < TOTAL_FRAME_COUNT; i++) {
        assert(marker_for_iteration[i] == i);
    }
    /* The same thing in the user's terms: the first frame the player
     * actually plays is numbered N, not 0. */
    assert(marker_for_iteration[REPLAYED_FRAME_COUNT] == REPLAYED_FRAME_COUNT);
    assert(marker_for_iteration[TOTAL_FRAME_COUNT - 1] == TOTAL_FRAME_COUNT - 1);

    remove(source_path);
    remove(resumed_path);
}

/* The half arithmetic cannot reach: N+k is only right if the file on disk
 * really does hold the replayed frames first. */
static void test_marker_index_names_the_frame_that_was_on_screen(void) {
    const char *source_path = "tmp/test_marker_coherence_resolve_source.json";
    const char *resumed_path = "tmp/test_marker_coherence_resolve_resumed.json";
    int marker_for_iteration[TOTAL_FRAME_COUNT];

    assert(pico8_input_save(k_replayed_frames, REPLAYED_FRAME_COUNT, source_path));
    run_resumed_session(source_path, resumed_path, marker_for_iteration);

    pico8_input_t resumed;
    pico8_input_init(&resumed);
    assert(pico8_input_load(&resumed, resumed_path));
    assert(resumed.frame_count == TOTAL_FRAME_COUNT);

    for (int i = 0; i < TOTAL_FRAME_COUNT; i++) {
        int marked = marker_for_iteration[i];
        assert(marked >= 0 && marked < resumed.frame_count);
        const pico8_input_frame_t *want = (i < REPLAYED_FRAME_COUNT)
                                              ? &k_replayed_frames[i]
                                              : &k_live_frames[i - REPLAYED_FRAME_COUNT];
        assert_frame_equals(&resumed.frames[marked], want);
    }

    pico8_input_free(&resumed);
    remove(source_path);
    remove(resumed_path);
}

/* The same path with N = 0: with no recording replayed, markers stay plain
 * frame indices. "Unchanged when the feature is unused" is the cheapest
 * regression to lose. */
static void test_markers_are_unchanged_when_no_recording_is_replayed(void) {
    const char *path = "tmp/test_marker_coherence_fresh.json";

    pico8_replay_source_t replay;
    assert(pico8_replay_source_open(&replay, NULL) == PICO8_REPLAY_SOURCE_NONE);

    pico8_session_recorder_t recorder;
    pico8_session_recorder_open(&recorder, path);

    for (int i = 0; i < LIVE_FRAME_COUNT; i++) {
        pico8_input_frame_t frame;
        assert(pico8_replay_source_next_frame(&replay, &frame) == 0);
        frame = k_live_frames[i];
        assert(pico8_session_recorder_frame_count(&recorder) == i);
        assert(pico8_session_recorder_record_frame(&recorder, &frame) == 1);
    }

    pico8_session_recorder_close(&recorder);
    pico8_replay_source_close(&replay);
    remove(path);
}

int main(void) {
    test_marker_index_counts_the_replayed_frames_too();
    test_marker_index_names_the_frame_that_was_on_screen();
    test_markers_are_unchanged_when_no_recording_is_replayed();

    printf("test_pico8_marker_coherence: all tests passed\n");
    return 0;
}
