/* Chunk 3 of docs/prd/triage/raylib-replay-then-resume.md ("Continuite
 * de l'enregistrement : la session reprise ecrit un enregistrement
 * complet, contenant les frames rejouees puis les nouvelles, verifie
 * rejouable"). The PRD prescribes this test almost word for word: "La
 * continuite de l'enregistrement se teste de bout en bout en headless :
 * produire un enregistrement de reprise, le recharger, et verifier
 * qu'il contient les frames rejouees suivies des nouvelles, et qu'il se
 * recharge sans erreur."
 *
 * WHY THIS FILE EXISTS AT ALL, GIVEN THAT THE CODE ALREADY DID IT.
 * src/main_raylib.c's loop has recorded every frame it is about to
 * play, replayed or live, since Chunk 2 -- one unconditional call to
 * the recorder. So the behavior this chunk owes was plausibly already
 * there, which is exactly why it is worth proving instead of
 * declaring: nothing anywhere asserted that the file coming out of a
 * resumed session holds the replayed frames FOLLOWED BY the new ones,
 * and a single misplaced `if` would have turned a resumed game into a
 * recording that describes half a game -- "reprendre une partie casse
 * la chaine", the PRD's own words, and the failure the player would
 * only discover the next time they tried to resume.
 *
 * WHAT IS REALLY UNDER TEST IS THE PAIR, NOT EITHER PIECE. The input
 * source alone is already covered (tests/test_pico8_replay_source.c),
 * and so is the incremental writer (tests/test_pico8_input.c, whose
 * test_append_recording_is_byte_identical_to_save_every_frame pins the
 * file on disk to what a full rewrite would have produced after EVERY
 * appended frame -- the prior art the PRD's Testing Decisions cite, and
 * it does hold). Neither says anything about what happens at the seam
 * between them, which is the only place a resume can go wrong. So the
 * loop below is not a stand-in for main_raylib.c's loop: it drives the
 * same two real modules, in the same order, with the same "take the
 * frame from the file while there is one, from the hardware
 * afterwards" switch, and the only thing it replaces is the hardware
 * poll -- the one part that needs a window and is therefore the one
 * part this environment can never run (CLAUDE.md).
 *
 * WHY THE ASSERTIONS ARE FIELD-BY-FIELD AND ORDERED. A test that
 * counted N+M frames would pass on a recording that replayed the wrong
 * frames, replayed them backwards, or recorded the live frames first;
 * every one of those produces a file that reloads fine and replays to
 * a different game. The recorded values are chosen to make any such
 * mix-up visible: replayed and live frames occupy disjoint value ranges
 * and no two frames are alike.
 *
 * WHY IT DOES NOT STOP AT RELOADING THE FILE. The last test feeds the
 * resumed recording back into a fresh pico8_replay_source, which is
 * user story 9 ("je veux que l'enregistrement issu d'une reprise soit
 * rejouable a son tour, afin de pouvoir reprendre une partie deja
 * reprise") and the PRD's stated reason the chain does not break at the
 * first resume. Loading a file only says it parses; serving it as a
 * replay source is what the next session actually does with it. */

#include "../src/pico8_session_recorder.h"
#include "../src/pico8_replay_source.h"

#include <assert.h>
#include <stdio.h>

/* The session that is resumed: N frames, every field distinct, all
 * mouse coordinates below 50 so that a live frame can never be mistaken
 * for a replayed one. */
#define SOURCE_FRAME_COUNT 5
static const pico8_input_frame_t k_source_frames[SOURCE_FRAME_COUNT] = {
    { 10, 11, 0, { 0, 0, 0, 0 } },
    { 12, 13, 1, { 1, 0, 0, 0 } },
    { 14, 15, 2, { 0, 1, 0, 0 } },
    { 16, 17, 4, { 0, 0, 1, 0 } },
    { 18, 19, 3, { 0, 0, 0, 1 } },
};

/* What the player does after getting the controls back: M frames, in a
 * value range the replayed frames never reach. */
#define LIVE_FRAME_COUNT 4
static const pico8_input_frame_t k_live_frames[LIVE_FRAME_COUNT] = {
    { 90, 91, 1, { 1, 1, 0, 0 } },
    { 92, 93, 5, { 0, 0, 1, 1 } },
    { 94, 95, 7, { 1, 0, 1, 0 } },
    { 96, 97, 6, { 0, 1, 0, 1 } },
};

static void assert_frame_equals(const pico8_input_frame_t *got, const pico8_input_frame_t *want) {
    assert(got->mouse_x == want->mouse_x);
    assert(got->mouse_y == want->mouse_y);
    assert(got->mouse_buttons == want->mouse_buttons);
    assert(got->dpad[0] == want->dpad[0]);
    assert(got->dpad[1] == want->dpad[1]);
    assert(got->dpad[2] == want->dpad[2]);
    assert(got->dpad[3] == want->dpad[3]);
}

/* Stands in for pico8_raylib_backend_poll_input, and for nothing else:
 * it is the single call in the loop below that needs a mouse, a
 * keyboard and an open window. Handing out a scripted frame instead is
 * what makes the rest of the loop -- the switch, the recorder, the file
 * -- runnable headlessly. */
static void poll_simulated_live_frame(int live_index, pico8_input_frame_t *out_frame) {
    assert(live_index >= 0 && live_index < LIVE_FRAME_COUNT);
    *out_frame = k_live_frames[live_index];
}

/* src/main_raylib.c's loop, with its one unrunnable call replaced and
 * its game step removed: the frame comes from the recording while there
 * is one and from the "hardware" afterwards, and either way it is
 * recorded before anything else happens to it -- the ordering
 * pico8_raylib_backend.h's own top comment explains a playtest segfault
 * forced, and which this test must not quietly reverse. Runs for
 * exactly `iterations` frames, the way a player quitting after M live
 * frames would end it. */
static void run_resumed_session(const char *source_path, const char *resumed_path, int iterations) {
    pico8_replay_source_t replay;
    assert(pico8_replay_source_open(&replay, source_path) == PICO8_REPLAY_SOURCE_LOADED);

    pico8_session_recorder_t recorder;
    pico8_session_recorder_open(&recorder, resumed_path);

    int live_index = 0;
    for (int i = 0; i < iterations; i++) {
        pico8_input_frame_t frame;
        if (!pico8_replay_source_next_frame(&replay, &frame)) {
            poll_simulated_live_frame(live_index, &frame);
            live_index++;
        }
        assert(pico8_session_recorder_record_frame(&recorder, &frame) == 1);
    }

    assert(pico8_session_recorder_frame_count(&recorder) == iterations);
    pico8_session_recorder_close(&recorder);
    pico8_replay_source_close(&replay);
}

static void test_recorder_writes_a_loadable_recording(void) {
    const char *path = "tmp/test_recording_continuity_basic.json";
    pico8_input_frame_t frames[2] = {
        { 11, 22, 1, { 1, 0, 0, 0 } },
        { 33, 44, 2, { 0, 1, 0, 0 } },
    };

    pico8_session_recorder_t recorder;
    pico8_session_recorder_open(&recorder, path);
    assert(pico8_session_recorder_frame_count(&recorder) == 0);

    for (int i = 0; i < 2; i++) {
        assert(pico8_session_recorder_record_frame(&recorder, &frames[i]) == 1);
        assert(pico8_session_recorder_frame_count(&recorder) == i + 1);
    }
    pico8_session_recorder_close(&recorder);

    pico8_input_t loaded;
    pico8_input_init(&loaded);
    assert(pico8_input_load(&loaded, path));
    assert(loaded.frame_count == 2);
    for (int i = 0; i < 2; i++) {
        assert_frame_equals(&loaded.frames[i], &frames[i]);
    }
    pico8_input_free(&loaded);

    remove(path);
}

/* The chunk itself: the file a resumed session leaves behind is the
 * whole game, replayed part first, played part after -- "un
 * enregistrement complet et autonome" -- and it reloads with no error.
 * User story 8. */
static void test_resumed_session_records_replayed_frames_then_new_ones(void) {
    const char *source_path = "tmp/test_recording_continuity_source.json";
    const char *resumed_path = "tmp/test_recording_continuity_resumed.json";

    /* The session being resumed is written by pico8_input_save, not by
     * the recorder under test: the fixture this chunk is measured
     * against must not be produced by the code it is measuring. */
    assert(pico8_input_save(k_source_frames, SOURCE_FRAME_COUNT, source_path));

    run_resumed_session(source_path, resumed_path, SOURCE_FRAME_COUNT + LIVE_FRAME_COUNT);

    pico8_input_t resumed;
    pico8_input_init(&resumed);
    assert(pico8_input_load(&resumed, resumed_path));
    assert(resumed.frame_count == SOURCE_FRAME_COUNT + LIVE_FRAME_COUNT);

    for (int i = 0; i < SOURCE_FRAME_COUNT; i++) {
        assert_frame_equals(&resumed.frames[i], &k_source_frames[i]);
    }
    for (int i = 0; i < LIVE_FRAME_COUNT; i++) {
        assert_frame_equals(&resumed.frames[SOURCE_FRAME_COUNT + i], &k_live_frames[i]);
    }

    pico8_input_free(&resumed);
    remove(source_path);
    remove(resumed_path);
}

/* User story 9, and the reason the chain does not break at the first
 * resume: the recording a resumed session produces is itself a
 * recording a session can be resumed from. Replaying it must serve all
 * N+M frames, in order, and then report exhaustion exactly as the
 * original did -- if it served fewer, the second resume would hand the
 * controls back in the middle of a game the player had already
 * played. */
static void test_resumed_recording_is_itself_resumable(void) {
    const char *source_path = "tmp/test_recording_continuity_chain_source.json";
    const char *resumed_path = "tmp/test_recording_continuity_chain_resumed.json";
    const int total = SOURCE_FRAME_COUNT + LIVE_FRAME_COUNT;

    assert(pico8_input_save(k_source_frames, SOURCE_FRAME_COUNT, source_path));
    run_resumed_session(source_path, resumed_path, total);

    pico8_replay_source_t second;
    assert(pico8_replay_source_open(&second, resumed_path) == PICO8_REPLAY_SOURCE_LOADED);
    assert(pico8_replay_source_frame_count(&second) == total);

    for (int i = 0; i < total; i++) {
        pico8_input_frame_t frame;
        assert(pico8_replay_source_next_frame(&second, &frame) == 1);
        if (i < SOURCE_FRAME_COUNT) {
            assert_frame_equals(&frame, &k_source_frames[i]);
        } else {
            assert_frame_equals(&frame, &k_live_frames[i - SOURCE_FRAME_COUNT]);
        }
    }

    pico8_input_frame_t after;
    assert(pico8_replay_source_next_frame(&second, &after) == 0);

    pico8_replay_source_close(&second);
    remove(source_path);
    remove(resumed_path);
}

int main(void) {
    test_recorder_writes_a_loadable_recording();
    test_resumed_session_records_replayed_frames_then_new_ones();
    test_resumed_recording_is_itself_resumable();

    printf("test_pico8_recording_continuity: all tests passed\n");
    return 0;
}
