/* Chunk 4 of docs/prd/triage/raylib-replay-then-resume.md, user story
 * 14: "que le marqueur de frame pose pendant le jeu reste coherent
 * apres une reprise, afin de pouvoir continuer a designer un instant
 * precis".
 *
 * WHAT A MARKER IS AND WHAT "COHERENT" MEANS HERE. Releasing the space
 * bar during play appends one line to tmp/events_recorded_<timestamp>.
 * markers.txt: a 0-based frame index whose whole purpose is to be read
 * as an index into the paired recording's own "frames" array, so that a
 * playtester can say "look at frame 4127" instead of describing what
 * they saw. Coherent therefore means one exact thing, and it is the
 * thing this file asserts: the number written in the marker file and
 * the position of that frame in the recording on disk are the same
 * number. Anything else -- a marker that counts loop iterations, or
 * live frames only, or frames since the replay ended -- names a
 * different frame than the one the player was looking at, which is
 * worse than no marker at all because it looks right.
 *
 * WHY THIS BECAME A QUESTION AT ALL. Before this PRD a session was all
 * live frames, and "how many frames have I recorded" and "how many
 * frames have I played" were the same number by construction. A resumed
 * session breaks that symmetry: it opens by replaying N frames the
 * player did not play, and only then starts recording frames they did.
 * The marker index is read from pico8_session_recorder_frame_count(),
 * so the whole question reduces to whether that counter counts the
 * replayed frames too.
 *
 * IT DOES, AND THAT IS NOT AN ACCIDENT, BUT IT WAS ALSO NOT PROVEN.
 * Chunk 3 made src/main_raylib.c record every frame it is about to
 * play, replayed or live alike, through one unconditional call --
 * deliberately, because the resumed recording has to contain the
 * replayed frames followed by the new ones to stay replayable in its
 * turn. The marker index inherits its coherence from that decision for
 * free. But "inherits for free" is exactly the kind of property that a
 * later, locally reasonable edit deletes without noticing: someone who
 * decided that replayed frames need not be re-recorded would break
 * user story 9 AND silently shift every marker in the session by N,
 * and only the first of those two has a test. Now both do.
 *
 * WHY THE ASSERTION IS MADE ON EVERY FRAME AND NOT ON ONE MARKER.
 * A test that marked a single frame and checked it would pass on an
 * off-by-N that happened to cancel out at the frame it chose. The loop
 * below instead records, for every single iteration, the index a marker
 * dropped on that iteration would have received, and then reloads the
 * finished recording and checks each one against the frame actually
 * stored there, field by field. The replayed and the live frames use
 * disjoint value ranges, so a marker pointing into the wrong half is
 * not merely an unequal number but a visibly different frame.
 *
 * WHAT THIS FILE DOES NOT AND CANNOT COVER. It does not press the space
 * bar, because that needs a window this environment may not open
 * (CLAUDE.md), and it does not check that the marker file itself gets
 * one line per tap -- that is four lines of fopen/fprintf in
 * src/pico8_raylib_backend.c. What it covers is the number those four
 * lines write. Two further facts about markers during a replay are
 * stated in src/main_raylib.c rather than tested here, for the same
 * reason: src/pico8_raylib_backend_poll_input() is not called at all
 * while the file drives the frames, so no marker can be dropped on a
 * frame the player is merely watching, and raylib recomputes its
 * "key released" edge from the previous and current frame's key state
 * every time it pumps events, so a space bar released mid-replay is
 * dropped rather than queued up to fire on the first live frame. */

#include "../src/pico8_session_recorder.h"
#include "../src/pico8_replay_source.h"

#include <assert.h>
#include <stdio.h>

/* The session being resumed. Every coordinate stays below 50 so that no
 * replayed frame can be confused with a live one. */
#define REPLAYED_FRAME_COUNT 6
static const pico8_input_frame_t k_replayed_frames[REPLAYED_FRAME_COUNT] = {
    { 20, 21, 0, { 0, 0, 0, 0 } },
    { 22, 23, 1, { 1, 0, 0, 0 } },
    { 24, 25, 2, { 0, 1, 0, 0 } },
    { 26, 27, 4, { 0, 0, 1, 0 } },
    { 28, 29, 3, { 0, 0, 0, 1 } },
    { 30, 31, 5, { 1, 1, 0, 0 } },
};

/* What the player does once the controls come back, in a value range
 * the replayed frames never reach. */
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

/* src/main_raylib.c's loop, with the two calls that need hardware
 * replaced by scripted values and the game step removed: the frame
 * comes from the recording while there is one and from the "hardware"
 * afterwards; whichever it was, it is recorded before anything else
 * happens to it. `marker_for_iteration[i]` is the number the marker
 * file would receive for a space-bar release on iteration i -- read
 * from the recorder at the exact point src/pico8_raylib_backend_poll_
 * input() reads it, which is immediately BEFORE that frame is recorded.
 * That "before" is the whole arithmetic: the count is defined as the
 * index the next recorded frame will occupy, so reading it there yields
 * this frame's own index, and reading it one line later would yield the
 * next frame's. */
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

/* The chunk's question, answered. A marker dropped on the k-th live
 * frame of a resumed session must be N+k, not k: the recording it
 * indexes into starts with the N replayed frames. */
static void test_marker_index_counts_the_replayed_frames_too(void) {
    const char *source_path = "tmp/test_marker_coherence_source.json";
    const char *resumed_path = "tmp/test_marker_coherence_resumed.json";
    int marker_for_iteration[TOTAL_FRAME_COUNT];

    /* Written by pico8_input_save, not by the recorder under test: the
     * fixture must not be produced by the code being measured. */
    assert(pico8_input_save(k_replayed_frames, REPLAYED_FRAME_COUNT, source_path));

    run_resumed_session(source_path, resumed_path, marker_for_iteration);

    for (int i = 0; i < TOTAL_FRAME_COUNT; i++) {
        assert(marker_for_iteration[i] == i);
    }
    /* Spelled out once in the terms of the user story, so that the
     * intent survives a reader who skips the loop above: the first
     * frame the player actually plays is numbered N, not 0. */
    assert(marker_for_iteration[REPLAYED_FRAME_COUNT] == REPLAYED_FRAME_COUNT);
    assert(marker_for_iteration[TOTAL_FRAME_COUNT - 1] == TOTAL_FRAME_COUNT - 1);

    remove(source_path);
    remove(resumed_path);
}

/* And the number means what it claims to mean: it indexes the "frames"
 * array of the recording this session actually left on disk. This is
 * the half a pure arithmetic assertion cannot reach -- N+k is only the
 * right answer if the file really does hold the replayed frames first. */
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

/* A session started with no recording at all is the same code path with
 * N = 0, and its markers must still be plain frame indices -- the
 * behavior that existed before this PRD and that a fix for the resumed
 * case could easily have broken. Kept as its own test because "unchanged
 * when the feature is not used" is the cheapest regression to lose. */
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
