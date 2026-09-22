/* Chunks 1 and 2 of docs/prd/triage/raylib-replay-then-resume.md: the
 * opening decision of a replayed session -- no recording requested, a
 * recording loaded with N frames, or a recording requested and
 * unloadable -- and then the switchable input source itself, which the
 * PRD names as the one genuinely testable unit of this feature: "etant
 * donne un enregistrement de N frames, elle fournit les frames du
 * fichier pour les N premieres puis signale l'epuisement. Elle se teste
 * sans fenetre et sans materiel, et c'est elle qui porte le comportement
 * decrit par les user stories 1, 3 et 9."
 *
 * That PRD's Testing Decisions open with a warning aimed straight at a
 * file like this one: "Verifier que le fichier a ete lu ne prouve
 * rien", the real proof being the game state reached after a replay.
 * That proof is not available to a test that cannot open a window, so
 * this file does the next honest thing rather than a weaker version of
 * the same claim: it asserts the exact recorded values of every served
 * frame, in order, which is the only property the game state after a
 * replay actually depends on. A test that merely counted frames, or
 * checked them for being non-zero, would pass on a source that served
 * them backwards.
 *
 * The three-way open outcome stays asserted here in its own right,
 * because the PRD's Implementation Decisions make one of those three a
 * hard requirement: an unreadable recording must be an explicit launch
 * error and never a silent new game ("ce repli ferait croire a une perte
 * de sauvegarde", user story 12). A test that pins the missing and the
 * malformed case to an outcome distinct from "no recording requested" is
 * what keeps a later refactor from quietly reintroducing that fallback.
 *
 * Both failure fixtures are reused from tests/test_pico8_input.c rather
 * than duplicated: this module deliberately delegates the whole notion
 * of "valid recording" to pico8_input_load, so it must be tested
 * against that function's own idea of a bad file, not a second one.
 * Links against pico8_replay_source.c and pico8_input.c only -- no
 * raylib, no window -- which is the reason the module exists at all
 * (see src/pico8_replay_source.h), and what lets this run in the plain
 * `make unit` suite. */

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

/* The three frames of tests/fixtures/events_valid.json, transcribed.
 * Asserting the exact recorded values -- not merely "a frame came back"
 * -- is what makes this test able to catch an off-by-one, a reversed
 * order, or a frame served twice, which are the three ways a switchable
 * input source silently desynchronises a replay from the session that
 * produced it. */
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

/* User stories 1 and 9: replaying a recording of N frames must feed the
 * game those N frames, in the order they were played, and nothing else.
 * This is the whole of what the PRD calls the testable unit -- "elle
 * fournit les frames du fichier pour les N premieres puis signale
 * l'epuisement" -- exercised without a window and without raylib. */
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

/* User story 3: the end of the recording is where the player gets the
 * controls back, so it must be reported -- as a value, once per frame --
 * and it must keep being reported for as long as the session runs. A
 * source that aborted here (pico8_input_step's own contract), looped
 * back to frame 0, or re-served the last frame would each make the
 * hand-back either a crash or a game still being driven by a file that
 * ran out. The loop below is deliberately far longer than the recording:
 * exhaustion is a terminal state, not a one-shot event. */
static void test_exhausted_recording_reports_exhaustion_forever(void) {
    pico8_replay_source_t source;
    pico8_input_frame_t frame;

    assert(pico8_replay_source_open(&source, "tests/fixtures/events_valid.json") ==
           PICO8_REPLAY_SOURCE_LOADED);

    for (int i = 0; i < k_events_valid_count; i++) {
        assert(pico8_replay_source_next_frame(&source, &frame) == 1);
    }

    /* The (N+1)-th call, and then a hundred more live frames' worth. */
    for (int i = 0; i < 100; i++) {
        assert(pico8_replay_source_next_frame(&source, &frame) == 0);
    }

    pico8_replay_source_close(&source);
}

/* User story 5 and the reason the driver needs no mode flag: launching
 * without an argument is just a replay that is over before it starts,
 * so the very first call already says "take your frames from the
 * hardware". */
static void test_no_recording_is_exhausted_from_the_first_frame(void) {
    pico8_replay_source_t source;
    pico8_input_frame_t frame;

    assert(pico8_replay_source_open(&source, NULL) == PICO8_REPLAY_SOURCE_NONE);

    assert(pico8_replay_source_next_frame(&source, &frame) == 0);
    assert(pico8_replay_source_next_frame(&source, &frame) == 0);

    pico8_replay_source_close(&source);
}

/* Chunk 4 adds the one number a progress indicator needs and nothing
 * else: how far into the recording the replay has got (user story 6,
 * "savoir ou en est le rejeu... afin de ne pas croire que le jeu est
 * fige"). It is an accessor on the cursor this module already keeps,
 * rather than a counter the driver maintains alongside it, precisely
 * because a second counter can drift from the first -- and a progress
 * line that disagrees with the replay is worse than none, since it is
 * the thing the player consults to decide whether to keep waiting.
 * Counted in frames SERVED, so it is 1 on the frame being played first
 * and N on the last: the numbering the overlay shows a human, one step
 * ahead of the 0-based index the marker file writes. */
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

    /* Exhaustion does not advance it: once the player has the controls
     * back, the replay stays finished at N of N rather than counting
     * the live frames they now play. */
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
