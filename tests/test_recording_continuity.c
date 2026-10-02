/* The seam between the replay source and the session recorder, which each
 * have their own tests and neither of which says what happens where they
 * meet -- the only place a resume can go wrong. The loop below drives both
 * real modules in `main_raylib.c`'s order, replacing only the hardware
 * poll.
 *
 * Frames are compared field by field and in order: a count of N+M would
 * pass on a recording that replayed the wrong frames, replayed them
 * backwards, or wrote the live ones first, and each of those reloads fine
 * and replays to a different game.
 *
 * The last test serves the resumed recording back as a replay source
 * rather than merely reloading it: parsing says nothing about what the
 * next session does with the file. */

#include "devtools/session_recorder.h"
#include "devtools/replay_source.h"

#include <assert.h>
#include <stdio.h>

/* The session that is resumed: every field distinct, all coordinates
 * below 50, so a live frame can never be mistaken for a replayed one. */
#define SOURCE_FRAME_COUNT 5
static const p8_input_frame_t k_source_frames[SOURCE_FRAME_COUNT] = {
    { 10, 11, 0, { 0, 0, 0, 0 } },
    { 12, 13, 1, { 1, 0, 0, 0 } },
    { 14, 15, 2, { 0, 1, 0, 0 } },
    { 16, 17, 4, { 0, 0, 1, 0 } },
    { 18, 19, 3, { 0, 0, 0, 1 } },
};

/* What the player does after getting the controls back, in a value range
 * the replayed frames never reach. */
#define LIVE_FRAME_COUNT 4
static const p8_input_frame_t k_live_frames[LIVE_FRAME_COUNT] = {
    { 90, 91, 1, { 1, 1, 0, 0 } },
    { 92, 93, 5, { 0, 0, 1, 1 } },
    { 94, 95, 7, { 1, 0, 1, 0 } },
    { 96, 97, 6, { 0, 1, 0, 1 } },
};

static void assert_frame_equals(const p8_input_frame_t *got, const p8_input_frame_t *want) {
    assert(got->mouse_x == want->mouse_x);
    assert(got->mouse_y == want->mouse_y);
    assert(got->mouse_buttons == want->mouse_buttons);
    assert(got->dpad[0] == want->dpad[0]);
    assert(got->dpad[1] == want->dpad[1]);
    assert(got->dpad[2] == want->dpad[2]);
    assert(got->dpad[3] == want->dpad[3]);
}

/* Stands in for `pico8_raylib_backend_poll_input`, the one call below that
 * needs a window. */
static void poll_simulated_live_frame(int live_index, p8_input_frame_t *out_frame) {
    assert(live_index >= 0 && live_index < LIVE_FRAME_COUNT);
    *out_frame = k_live_frames[live_index];
}

/* `main_raylib.c`'s loop, with that call replaced and the game step
 * dropped. The frame is recorded before anything else happens to it, an
 * ordering a playtest segfault forced and this test must not reverse. */
static void run_resumed_session(const char *source_path, const char *resumed_path, int iterations) {
    replay_source_t replay;
    assert(replay_source_open(&replay, source_path) == REPLAY_SOURCE_LOADED);

    session_recorder_t recorder;
    session_recorder_open(&recorder, resumed_path);

    int live_index = 0;
    for (int i = 0; i < iterations; i++) {
        p8_input_frame_t frame;
        if (!replay_source_next_frame(&replay, &frame)) {
            poll_simulated_live_frame(live_index, &frame);
            live_index++;
        }
        assert(session_recorder_record_frame(&recorder, &frame) == 1);
    }

    assert(session_recorder_frame_count(&recorder) == iterations);
    session_recorder_close(&recorder);
    replay_source_close(&replay);
}

static void test_recorder_writes_a_loadable_recording(void) {
    const char *path = "tmp/test_recording_continuity_basic.json";
    p8_input_frame_t frames[2] = {
        { 11, 22, 1, { 1, 0, 0, 0 } },
        { 33, 44, 2, { 0, 1, 0, 0 } },
    };

    session_recorder_t recorder;
    session_recorder_open(&recorder, path);
    assert(session_recorder_frame_count(&recorder) == 0);

    for (int i = 0; i < 2; i++) {
        assert(session_recorder_record_frame(&recorder, &frames[i]) == 1);
        assert(session_recorder_frame_count(&recorder) == i + 1);
    }
    session_recorder_close(&recorder);

    p8_input_t loaded;
    p8_input_init(&loaded);
    assert(p8_input_load(&loaded, path));
    assert(loaded.frame_count == 2);
    for (int i = 0; i < 2; i++) {
        assert_frame_equals(&loaded.frames[i], &frames[i]);
    }
    p8_input_free(&loaded);

    remove(path);
}

/* The file a resumed session leaves behind is the whole game: replayed
 * part first, played part after, and it reloads without error. */
static void test_resumed_session_records_replayed_frames_then_new_ones(void) {
    const char *source_path = "tmp/test_recording_continuity_source.json";
    const char *resumed_path = "tmp/test_recording_continuity_resumed.json";

    /* Written by `pico8_input_save`, not by the recorder under test. */
    assert(p8_input_save(k_source_frames, SOURCE_FRAME_COUNT, source_path));

    run_resumed_session(source_path, resumed_path, SOURCE_FRAME_COUNT + LIVE_FRAME_COUNT);

    p8_input_t resumed;
    p8_input_init(&resumed);
    assert(p8_input_load(&resumed, resumed_path));
    assert(resumed.frame_count == SOURCE_FRAME_COUNT + LIVE_FRAME_COUNT);

    for (int i = 0; i < SOURCE_FRAME_COUNT; i++) {
        assert_frame_equals(&resumed.frames[i], &k_source_frames[i]);
    }
    for (int i = 0; i < LIVE_FRAME_COUNT; i++) {
        assert_frame_equals(&resumed.frames[SOURCE_FRAME_COUNT + i], &k_live_frames[i]);
    }

    p8_input_free(&resumed);
    remove(source_path);
    remove(resumed_path);
}

/* A resumed session's recording must itself be resumable: all N+M frames
 * in order, then exhaustion as the original reported it. Serving fewer
 * would hand the controls back mid-game on the second resume. */
static void test_resumed_recording_is_itself_resumable(void) {
    const char *source_path = "tmp/test_recording_continuity_chain_source.json";
    const char *resumed_path = "tmp/test_recording_continuity_chain_resumed.json";
    const int total = SOURCE_FRAME_COUNT + LIVE_FRAME_COUNT;

    assert(p8_input_save(k_source_frames, SOURCE_FRAME_COUNT, source_path));
    run_resumed_session(source_path, resumed_path, total);

    replay_source_t second;
    assert(replay_source_open(&second, resumed_path) == REPLAY_SOURCE_LOADED);
    assert(replay_source_frame_count(&second) == total);

    for (int i = 0; i < total; i++) {
        p8_input_frame_t frame;
        assert(replay_source_next_frame(&second, &frame) == 1);
        if (i < SOURCE_FRAME_COUNT) {
            assert_frame_equals(&frame, &k_source_frames[i]);
        } else {
            assert_frame_equals(&frame, &k_live_frames[i - SOURCE_FRAME_COUNT]);
        }
    }

    p8_input_frame_t after;
    assert(replay_source_next_frame(&second, &after) == 0);

    replay_source_close(&second);
    remove(source_path);
    remove(resumed_path);
}

int main(void) {
    test_recorder_writes_a_loadable_recording();
    test_resumed_session_records_replayed_frames_then_new_ones();
    test_resumed_recording_is_itself_resumable();

    printf("test_recording_continuity: all tests passed\n");
    return 0;
}
