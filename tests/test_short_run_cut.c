#include "devtools/short_run_cut.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

enum { FULL = 2000 };

static p8_input_frame_t full[FULL];
static short_run_cut_plan_t plan;
static char error[SHORT_RUN_CUT_ERROR_CAP];

static void fill_full_with_pressed_frames(void) {
    for (int i = 0; i < FULL; i++) {
        full[i] = (p8_input_frame_t){ 0, 0, 0, 1 + i % 7 };
    }
}

static short_run_cut_entry_t entry_of(const char *line) {
    short_run_cut_entry_t entry;
    assert(short_run_cut_parse_entry(line, &entry, error));
    return entry;
}

static void test_a_prefix_run_is_the_first_frames_without_a_save(void) {
    fill_full_with_pressed_frames();
    short_run_cut_entry_t entry = entry_of("start prefix 12 12 99");
    assert(short_run_cut_plan(full, FULL, NULL, 0, &entry, &plan, error));
    assert(plan.frame_count == 100);
    assert(memcmp(plan.frames, full, sizeof full[0] * 100) == 0);
    assert(plan.save_index == -1);
    assert(plan.first_game_frame == 0 && plan.source_first == 0);
    assert(plan.reference_last == 99);
}

static const uint8_t RECORD_A[] = { 1, 2, 3 };
static const uint8_t RECORD_B[] = { 4, 5 };
static const uint8_t RECORD_C[] = { 6 };

static void test_a_resume_run_cuts_at_the_last_save_far_enough_before_the_window(void) {
    fill_full_with_pressed_frames();
    full[301].pad_buttons = 0;
    full[491].pad_buttons = 0;
    full[492].pad_buttons = 0;
    const short_run_cut_save_t saves[] = { { 300, RECORD_A, 3 }, { 491, RECORD_B, 2 }, { 492, RECORD_C, 1 } };
    short_run_cut_entry_t entry = entry_of("draw-500 resume 500 505");
    assert(short_run_cut_plan(full, FULL, saves, 3, &entry, &plan, error));
    assert(plan.save_index == 1 && plan.cut == 491);
    assert(plan.first_game_frame == SHORT_RUN_CUT_TITLE_FRAMES && plan.source_first == 492);
    assert(plan.frame_count == SHORT_RUN_CUT_TITLE_FRAMES + 505 - 491);
    assert(memcmp(&plan.frames[13], &full[492], sizeof full[0] * (505 - 491)) == 0);
    assert(plan.reference_last == 505);
}

static void test_the_title_frames_are_those_of_the_resume_fixture(void) {
    p8_input_t fixture;
    p8_input_init(&fixture);
    assert(p8_input_load(&fixture, "tests/e2e/pad_resume_at_start.json"));
    fill_full_with_pressed_frames();
    full[201].pad_buttons = 0;
    const short_run_cut_save_t saves[] = { { 200, RECORD_A, 3 } };
    short_run_cut_entry_t entry = entry_of("update-209 resume 209 209");
    assert(short_run_cut_plan(full, FULL, saves, 1, &entry, &plan, error));
    for (int i = 0; i < SHORT_RUN_CUT_TITLE_FRAMES; i++) {
        assert(plan.frames[i].pad_buttons == fixture.frames[i].pad_buttons);
    }
    p8_input_free(&fixture);
}

static void assert_refused(const char *needle) {
    if (strstr(error, needle) == NULL) {
        fprintf(stderr, "error '%s' lacks '%s'\n", error, needle);
        assert(0);
    }
}

static void test_a_save_whose_next_frame_is_pressed_is_skipped(void) {
    fill_full_with_pressed_frames();
    full[301].pad_buttons = 0;
    const short_run_cut_save_t saves[] = { { 300, RECORD_A, 3 }, { 491, RECORD_B, 2 } };
    short_run_cut_entry_t entry = entry_of("draw-500 resume 500 505");
    assert(short_run_cut_plan(full, FULL, saves, 2, &entry, &plan, error));
    assert(plan.save_index == 0 && plan.cut == 300);
}

static void test_no_eligible_save_is_refused_naming_the_run(void) {
    fill_full_with_pressed_frames();
    full[493].pad_buttons = 0;
    const short_run_cut_save_t saves[] = { { 300, RECORD_A, 3 }, { 492, RECORD_B, 2 } };
    short_run_cut_entry_t entry = entry_of("draw-500 resume 500 505");
    assert(!short_run_cut_plan(full, FULL, saves, 2, &entry, &plan, error));
    assert_refused("draw-500");
    assert_refused("no save");
}

static void test_a_run_longer_than_the_breakdown_table_is_refused(void) {
    fill_full_with_pressed_frames();
    full[101].pad_buttons = 0;
    const short_run_cut_save_t saves[] = { { 100, RECORD_A, 3 } };
    short_run_cut_entry_t fits = entry_of("long resume 1000 1111");
    assert(short_run_cut_plan(full, FULL, saves, 1, &fits, &plan, error));
    assert(plan.frame_count == SHORT_RUN_CUT_FRAMES_CAP);
    short_run_cut_entry_t resume = entry_of("long resume 1000 1112");
    assert(!short_run_cut_plan(full, FULL, saves, 1, &resume, &plan, error));
    assert_refused("long: 1025 frames");
    short_run_cut_entry_t prefix = entry_of("start prefix 12 12 1024");
    assert(!short_run_cut_plan(full, FULL, NULL, 0, &prefix, &plan, error));
    assert_refused("start: 1025 frames");
}

static void test_a_run_past_the_recording_is_refused(void) {
    fill_full_with_pressed_frames();
    short_run_cut_entry_t prefix = entry_of("start prefix 12 12 99");
    assert(!short_run_cut_plan(full, 50, NULL, 0, &prefix, &plan, error));
    assert_refused("start: ends at f99, past the recording's 50 frames");
    full[101].pad_buttons = 0;
    const short_run_cut_save_t saves[] = { { 100, RECORD_A, 3 } };
    short_run_cut_entry_t resume = entry_of("late resume 150 160");
    assert(!short_run_cut_plan(full, 160, saves, 1, &resume, &plan, error));
    assert_refused("late: ends at f160, past the recording's 160 frames");
}

static void test_the_map_line_names_the_cut_and_the_frame_mapping(void) {
    char line[SHORT_RUN_CUT_MAP_CAP];
    fill_full_with_pressed_frames();
    full[492].pad_buttons = 0;
    const short_run_cut_save_t saves[] = { { 491, RECORD_B, 2 } };
    short_run_cut_entry_t resume = entry_of("draw-500 resume 500 505");
    assert(short_run_cut_plan(full, FULL, saves, 1, &resume, &plan, error));
    short_run_cut_format_map(&resume, &plan, line);
    assert(strcmp(line, "cut 491 first-game-frame 13 source-first 492 window 500 505\n") == 0);
    short_run_cut_entry_t prefix = entry_of("start prefix 12 12 99");
    assert(short_run_cut_plan(full, FULL, NULL, 0, &prefix, &plan, error));
    short_run_cut_format_map(&prefix, &plan, line);
    assert(strcmp(line, "cut none first-game-frame 0 source-first 0 window 12 12\n") == 0);
}

static void assert_line_refused(const char *line, const char *needle) {
    short_run_cut_entry_t entry;
    assert(!short_run_cut_parse_entry(line, &entry, error));
    assert_refused(needle);
}

static void test_a_malformed_manifest_line_is_refused(void) {
    assert_line_refused("start prefix 12", "not <name> <kind> <first> <last>");
    assert_line_refused("start middle 12 12", "unknown kind 'middle'");
    assert_line_refused("start prefix 12 12", "a prefix run needs its end");
    assert_line_refused("start prefix 12 12 11", "end before the window's last frame");
    assert_line_refused("spike resume 4737 4737 5000", "extra text");
    assert_line_refused("spike resume 4737 4736", "last frame before the first");
    assert_line_refused("spike resume -1 4", "negative frame");
}

int main(void) {
    test_the_map_line_names_the_cut_and_the_frame_mapping();
    test_a_malformed_manifest_line_is_refused();
    test_a_run_longer_than_the_breakdown_table_is_refused();
    test_a_run_past_the_recording_is_refused();
    test_a_save_whose_next_frame_is_pressed_is_skipped();
    test_no_eligible_save_is_refused_naming_the_run();
    test_a_prefix_run_is_the_first_frames_without_a_save();
    test_a_resume_run_cuts_at_the_last_save_far_enough_before_the_window();
    test_the_title_frames_are_those_of_the_resume_fixture();
    printf("test_short_run_cut: ok\n");
    return 0;
}
