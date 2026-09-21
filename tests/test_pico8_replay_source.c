/* Chunk 1 of docs/prd/triage/raylib-replay-then-resume.md: the opening
 * decision of a replayed session -- no recording requested, a recording
 * loaded with N frames, or a recording requested and unloadable -- is
 * the whole of what that chunk adds, so it is the whole of what this
 * file asserts.
 *
 * That PRD's Testing Decisions open with a warning aimed straight at a
 * file like this one: "Verifier que le fichier a ete lu ne prouve
 * rien", the real proof being the game state reached after a replay.
 * That proof belongs to Chunk 2, which is where frames actually reach
 * the game. What IS honestly testable here is the three-way outcome
 * itself, because the PRD's Implementation Decisions make one of those
 * three a hard requirement in its own right: an unreadable recording
 * must be an explicit launch error and never a silent new game ("ce
 * repli ferait croire a une perte de sauvegarde", user story 12). A
 * test that pins the missing and the malformed case to an outcome
 * distinct from "no recording requested" is what keeps a later
 * refactor from quietly reintroducing that fallback.
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

int main(void) {
    test_no_path_requests_no_recording();
    test_valid_recording_loads_with_its_frame_count();
    test_missing_path_reports_load_failure();
    test_malformed_recording_reports_load_failure();

    printf("test_pico8_replay_source: all tests passed\n");
    return 0;
}
