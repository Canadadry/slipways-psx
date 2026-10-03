/* What a key press and an overlay reduce to without a window: which
 * frame-rate cap a speed level means, and what the progress line says.
 * That a key reaches `replay_control_faster()` (through
 * raylib_backend_poll_replay_controls) and that the string is drawn (by
 * raylib_backend_present) is not
 * checked here -- it needs a real window and a real keyboard.
 *
 * The numbers are asserted exactly rather than as ranges: "faster than
 * before" would pass on a ladder that went 60, 61, 62, and the point of
 * the default is a specific multiplier. */

#include "devtools/replay_control.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Replay is faster than real time by default: at game speed a
 * thirty-minute recording takes thirty minutes. */
static void test_default_speed_is_four_times_game_speed(void) {
    replay_control_t control;
    replay_control_init(&control);

    assert(replay_control_speed_multiplier(&control) == 4);
    assert(replay_control_target_fps(&control, 1) == 240);
}

/* Doubling, so every level is an exact multiple of the 60 FPS target.
 * Each rung is asserted on its own: a mapping that collapsed two levels
 * would still "go faster" and still pass a range test. */
static void test_faster_and_slower_walk_the_whole_ladder(void) {
    replay_control_t control;
    replay_control_init(&control);

    replay_control_slower(&control);
    assert(replay_control_target_fps(&control, 1) == 120);
    replay_control_slower(&control);
    assert(replay_control_target_fps(&control, 1) == 60);

    replay_control_faster(&control);
    assert(replay_control_target_fps(&control, 1) == 120);
    replay_control_faster(&control);
    assert(replay_control_target_fps(&control, 1) == 240);
    replay_control_faster(&control);
    assert(replay_control_target_fps(&control, 1) == 480);
}

/* Both ends clamp rather than wrap: one extra tap must not drop a replay
 * from full speed back to 1x. The slow end stops at game speed because the
 * mechanism only ever shortens the wait between frames. */
static void test_the_ladder_clamps_at_both_ends(void) {
    replay_control_t control;
    replay_control_init(&control);

    for (int i = 0; i < 10; i++) {
        replay_control_slower(&control);
    }
    assert(replay_control_speed_multiplier(&control) == 1);
    assert(replay_control_target_fps(&control, 1) == REPLAY_CONTROL_LIVE_FPS);

    for (int i = 0; i < 10; i++) {
        replay_control_faster(&control);
    }
    assert(replay_control_speed_multiplier(&control) == 0);
    assert(replay_control_target_fps(&control, 1) == 0);
}

/* 0 is what raylib's SetTargetFPS takes as "do not wait between frames at
 * all".
 *
 * Note what is absent: no frame count, no stride, no "skip to frame N".
 * Abbreviating shortens the wait between frames and never skips one, or the
 * replay would not arrive at the state the recording describes. */
static void test_uncapping_removes_the_wait_between_frames(void) {
    replay_control_t control;
    replay_control_init(&control);

    replay_control_uncap(&control);
    assert(replay_control_speed_multiplier(&control) == 0);
    assert(replay_control_target_fps(&control, 1) == 0);

    /* Not a one-way door: a player who overshot can come back down. */
    replay_control_slower(&control);
    assert(replay_control_target_fps(&control, 1) == 480);
}

/* The resumed game runs at 60 FPS whatever speed the replay was left on.
 * Enforced here rather than by the driver remembering to reset the cap: a
 * driver that forgets hands the player an unplayable game. */
static void test_live_play_is_always_game_speed_whatever_the_replay_was_on(void) {
    replay_control_t control;
    replay_control_init(&control);

    assert(replay_control_target_fps(&control, 0) == 60);

    replay_control_uncap(&control);
    assert(replay_control_target_fps(&control, 1) == 0);
    assert(replay_control_target_fps(&control, 0) == 60);

    replay_control_slower(&control);
    assert(replay_control_target_fps(&control, 0) == 60);
}

/* What the overlay says is the only part of it with a right answer; where
 * it is drawn and whether it is legible are a human's judgement.
 *
 * The count is 1-based, so the line reads 1/9589 on the first frame and
 * 9589/9589 on the last -- the numbering a player counts in, deliberately
 * not the 0-based index the marker file records. */
static void test_progress_line_names_the_frame_the_total_and_the_speed(void) {
    replay_control_t control;
    char status[REPLAY_CONTROL_STATUS_MAX];

    replay_control_init(&control);
    replay_control_format_status(&control, 1, 9589, status, sizeof status);
    assert(strcmp(status, "replay 1/9589 (0%) speed x4") == 0);

    replay_control_format_status(&control, 1234, 9589, status, sizeof status);
    assert(strcmp(status, "replay 1234/9589 (12%) speed x4") == 0);

    replay_control_format_status(&control, 9589, 9589, status, sizeof status);
    assert(strcmp(status, "replay 9589/9589 (100%) speed x4") == 0);
}

/* The speed shown is the speed in force, so a tap on a machine too slow to
 * reach the new cap is still visible. The uncapped level prints a word:
 * "x0" would read as "stopped". */
static void test_progress_line_reports_the_current_speed(void) {
    replay_control_t control;
    char status[REPLAY_CONTROL_STATUS_MAX];

    replay_control_init(&control);
    replay_control_slower(&control);
    replay_control_format_status(&control, 50, 200, status, sizeof status);
    assert(strcmp(status, "replay 50/200 (25%) speed x2") == 0);

    replay_control_uncap(&control);
    replay_control_format_status(&control, 50, 200, status, sizeof status);
    assert(strcmp(status, "replay 50/200 (25%) speed max") == 0);
}

/* One frame is the shortest valid replay -- `p8_input_load` rejects an
 * empty recording -- and the percentage must not divide by zero on the way
 * there. A total of zero is pinned too: a comfort overlay must never be
 * what kills the session. */
static void test_progress_line_survives_the_degenerate_totals(void) {
    replay_control_t control;
    char status[REPLAY_CONTROL_STATUS_MAX];

    replay_control_init(&control);
    replay_control_format_status(&control, 1, 1, status, sizeof status);
    assert(strcmp(status, "replay 1/1 (100%) speed x4") == 0);

    replay_control_format_status(&control, 0, 0, status, sizeof status);
    assert(strcmp(status, "replay 0/0 (0%) speed x4") == 0);
}

int main(void) {
    test_default_speed_is_four_times_game_speed();
    test_faster_and_slower_walk_the_whole_ladder();
    test_the_ladder_clamps_at_both_ends();
    test_uncapping_removes_the_wait_between_frames();
    test_live_play_is_always_game_speed_whatever_the_replay_was_on();
    test_progress_line_names_the_frame_the_total_and_the_speed();
    test_progress_line_reports_the_current_speed();
    test_progress_line_survives_the_degenerate_totals();

    printf("test_replay_control: all tests passed\n");
    return 0;
}
