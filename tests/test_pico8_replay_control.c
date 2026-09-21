/* Chunk 4 of docs/prd/triage/raylib-replay-then-resume.md ("Confort de
 * rejeu : reglage de vitesse, abregement, indication de progression").
 * Everything about replay comfort that can be decided without a window
 * lives in src/pico8_replay_control.{c,h}, and this is where it is
 * pinned to exact values.
 *
 * WHAT THIS FILE CAN AND CANNOT CLAIM. It cannot claim that pressing
 * Page Up makes the replay faster, nor that the overlay is legible, nor
 * that it appears where it should: all three need a real window and a
 * real keyboard, which this environment does not have and is not allowed
 * to open (CLAUDE.md). The PRD is explicit about what to do with that
 * gap -- "ce qui ne peut pas etre verifie ici doit etre enonce comme tel
 * plutot que contourne" -- so this file does not simulate key presses or
 * pretend to inspect pixels. It tests the two decisions those key
 * presses and that overlay reduce to: which frame-rate cap a given speed
 * level means, and what the progress line says. The wiring from a key to
 * pico8_replay_control_faster(), and from the returned string to
 * DrawText, is three lines in src/main_raylib.c and
 * src/pico8_raylib_backend.c, and is verified by a human session, not
 * here.
 *
 * WHY THE EXACT NUMBERS ARE ASSERTED AND NOT A RANGE. "Faster than
 * before" would pass on a ladder that went 60, 61, 62: the point of the
 * default is that a thirty-minute recording comes back in single-digit
 * minutes, which is a statement about a specific multiplier and nothing
 * weaker. The same goes for the uncapped level: 0 is raylib's own
 * "no wait at all" argument to SetTargetFPS, so asserting it is
 * asserting the actual behavior, not a sentinel this port invented. */

#include "../src/pico8_replay_control.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* The arbitration this chunk was given: replay runs faster than real
 * time by default, because a thirty-minute recording replayed at game
 * speed takes thirty minutes and destroys the whole benefit. */
static void test_default_speed_is_four_times_game_speed(void) {
    pico8_replay_control_t control;
    pico8_replay_control_init(&control);

    assert(pico8_replay_control_speed_multiplier(&control) == 4);
    assert(pico8_replay_control_target_fps(&control, 1) == 240);
}

/* The ladder itself, end to end. Doubling was chosen over a linear
 * ramp so that every level is a multiple the game's own 60 FPS target
 * divides into exactly -- a replay at 2x plays two recorded frames per
 * real 1/60th of a second, which is a sentence a player can check
 * against a clock. The values are asserted one by one because the
 * ladder is the feature: a mapping that quietly collapsed two levels
 * would still "go faster" and would still pass a range test. */
static void test_faster_and_slower_walk_the_whole_ladder(void) {
    pico8_replay_control_t control;
    pico8_replay_control_init(&control);

    pico8_replay_control_slower(&control);
    assert(pico8_replay_control_target_fps(&control, 1) == 120);
    pico8_replay_control_slower(&control);
    assert(pico8_replay_control_target_fps(&control, 1) == 60);

    pico8_replay_control_faster(&control);
    assert(pico8_replay_control_target_fps(&control, 1) == 120);
    pico8_replay_control_faster(&control);
    assert(pico8_replay_control_target_fps(&control, 1) == 240);
    pico8_replay_control_faster(&control);
    assert(pico8_replay_control_target_fps(&control, 1) == 480);
}

/* Both ends clamp rather than wrapping. Wrapping would make one extra
 * tap on the "faster" key drop a replay from full speed back to 1x,
 * which is the single most annoying thing a speed control can do to
 * someone who is holding the key down to get back to their game. The
 * slow end stops at game speed on purpose too: this chunk's mechanism
 * is shortening the wait between frames, never lengthening it -- a
 * slow-motion replay is a different feature nobody asked for. */
static void test_the_ladder_clamps_at_both_ends(void) {
    pico8_replay_control_t control;
    pico8_replay_control_init(&control);

    for (int i = 0; i < 10; i++) {
        pico8_replay_control_slower(&control);
    }
    assert(pico8_replay_control_speed_multiplier(&control) == 1);
    assert(pico8_replay_control_target_fps(&control, 1) == PICO8_REPLAY_CONTROL_LIVE_FPS);

    for (int i = 0; i < 10; i++) {
        pico8_replay_control_faster(&control);
    }
    assert(pico8_replay_control_speed_multiplier(&control) == 0);
    assert(pico8_replay_control_target_fps(&control, 1) == 0);
}

/* The "get me back to my game now" case, which is user story 7 read
 * literally ("abreger un rejeu long, afin de ne pas attendre une
 * demi-heure pour revenir a mon etat de jeu"). It is one key rather
 * than "hold faster until it stops moving" because the player who wants
 * it wants it at once, from whatever level they were on. 0 is not a
 * sentinel this port invented: it is what raylib's own SetTargetFPS
 * treats as "do not wait between frames at all", so the value handed to
 * it is the behavior, with nothing in between to translate it.
 *
 * ABREGER N'EST PAS SAUTER. Note what this test does NOT assert and
 * what the module therefore cannot do: there is no frame count here, no
 * stride, no "skip to frame N". The PRD's hard constraint is that
 * abbreviating never skips a frame, only shortens the wait between
 * frames -- every recorded frame is still fed to pico8__update60() and
 * still drawn -- because a replay that skipped frames would not arrive
 * at the state the recording describes, which is the only thing the
 * replay is for. A frame-rate cap is the only thing this module can
 * express, which is exactly why the abbreviation is expressed as one. */
static void test_uncapping_removes_the_wait_between_frames(void) {
    pico8_replay_control_t control;
    pico8_replay_control_init(&control);

    pico8_replay_control_uncap(&control);
    assert(pico8_replay_control_speed_multiplier(&control) == 0);
    assert(pico8_replay_control_target_fps(&control, 1) == 0);

    /* And it is not a one-way door: the player who overshot can come
     * back down and watch the rest at a speed they can read. */
    pico8_replay_control_slower(&control);
    assert(pico8_replay_control_target_fps(&control, 1) == 480);
}

/* The property that matters most once the replay is over, and the one
 * this module exists to make untestable-by-hand code unnecessary: the
 * resumed game plays at 60 FPS, whatever speed the replay was left on.
 * Enforced here rather than by the driver remembering to reset the cap,
 * because a driver that forgets hands the player a game running at
 * eight times speed -- or, after the uncap key, at whatever rate the
 * machine can spin, which is unplayable and would look like the port
 * being broken rather than a leftover setting. */
static void test_live_play_is_always_game_speed_whatever_the_replay_was_on(void) {
    pico8_replay_control_t control;
    pico8_replay_control_init(&control);

    assert(pico8_replay_control_target_fps(&control, 0) == 60);

    pico8_replay_control_uncap(&control);
    assert(pico8_replay_control_target_fps(&control, 1) == 0);
    assert(pico8_replay_control_target_fps(&control, 0) == 60);

    pico8_replay_control_slower(&control);
    assert(pico8_replay_control_target_fps(&control, 0) == 60);
}

/* User story 6: "savoir ou en est le rejeu pendant qu'il se deroule,
 * afin de ne pas croire que le jeu est fige". The string is built here
 * rather than inside the raylib backend for the reason this whole
 * module exists -- the backend is unreachable by `make unit` -- but
 * also because "what the overlay says" is the only part of the overlay
 * that has a right answer. Where it is drawn and whether it is legible
 * are judgements a human makes in a real window; that a replay stopped
 * at frame 1234 of 9589 says 1234/9589 and not 1233/9589 is not.
 *
 * The count is 1-based and the total is the recording's own frame
 * count, so the line reads "frame 1 of 9589" on the first frame and
 * "9589 of 9589" on the last -- the numbering a player counts in, not
 * the 0-based index the marker file uses. The two are deliberately
 * different and that difference is why the marker index gets its own
 * test file rather than being inferred from this string. */
static void test_progress_line_names_the_frame_the_total_and_the_speed(void) {
    pico8_replay_control_t control;
    char status[PICO8_REPLAY_CONTROL_STATUS_MAX];

    pico8_replay_control_init(&control);
    pico8_replay_control_format_status(&control, 1, 9589, status, sizeof status);
    assert(strcmp(status, "replay 1/9589 (0%) speed x4") == 0);

    pico8_replay_control_format_status(&control, 1234, 9589, status, sizeof status);
    assert(strcmp(status, "replay 1234/9589 (12%) speed x4") == 0);

    pico8_replay_control_format_status(&control, 9589, 9589, status, sizeof status);
    assert(strcmp(status, "replay 9589/9589 (100%) speed x4") == 0);
}

/* The speed shown is the speed in force, so that a player who taps a
 * key and sees nothing change on a machine too slow to reach the new
 * cap can still tell the key was received. The uncapped level prints a
 * word rather than "x0", which would read as "stopped". */
static void test_progress_line_reports_the_current_speed(void) {
    pico8_replay_control_t control;
    char status[PICO8_REPLAY_CONTROL_STATUS_MAX];

    pico8_replay_control_init(&control);
    pico8_replay_control_slower(&control);
    pico8_replay_control_format_status(&control, 50, 200, status, sizeof status);
    assert(strcmp(status, "replay 50/200 (25%) speed x2") == 0);

    pico8_replay_control_uncap(&control);
    pico8_replay_control_format_status(&control, 50, 200, status, sizeof status);
    assert(strcmp(status, "replay 50/200 (25%) speed max") == 0);
}

/* A one-frame recording is a real thing (pico8_input_load rejects an
 * empty one, so one frame is the shortest valid replay) and a division
 * by its total must not be a division by zero on the way there. The
 * defensive total-of-zero case is pinned too: the overlay is a comfort
 * feature and must never be the thing that kills a session it exists to
 * reassure the player about. */
static void test_progress_line_survives_the_degenerate_totals(void) {
    pico8_replay_control_t control;
    char status[PICO8_REPLAY_CONTROL_STATUS_MAX];

    pico8_replay_control_init(&control);
    pico8_replay_control_format_status(&control, 1, 1, status, sizeof status);
    assert(strcmp(status, "replay 1/1 (100%) speed x4") == 0);

    pico8_replay_control_format_status(&control, 0, 0, status, sizeof status);
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

    printf("test_pico8_replay_control: all tests passed\n");
    return 0;
}
