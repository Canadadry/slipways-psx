/* The only two decisions the screencast viewer makes that need no
 * window: which frame to show now, and what to tell a human whose file
 * cannot be played.
 *
 * What this file cannot assert: that a window opens, that the image is
 * at scale 6, that PAGE UP really speeds playback up, that the status
 * line is readable. All of that needs a display server and a human, and
 * CLAUDE.md forbids opening that window. What cannot be tested without a
 * window is in the wrong place; this file tests what was put in the
 * right place, and simulates neither keys nor pixels.
 *
 * Everywhere, the sequence of the indices served is asserted, one by one
 * and in order, never their count: a test that counted the advances
 * would pass a cursor that always served frame 0 again, or served them
 * backwards. */

#include "devtools/screencast_player.h"

#include "p8/palette.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Playback starts on the first frame of the file and moves one frame
 * per call, in order. The format and the indexed reading live in
 * devtools/screencast, the speed in devtools/replay_control. */
static void test_playback_starts_at_the_first_frame_and_walks_in_order(void) {
    screencast_player_t player;
    screencast_player_init(&player, 5);

    assert(screencast_player_current_frame(&player) == 0);
    screencast_player_advance(&player);
    assert(screencast_player_current_frame(&player) == 1);
    screencast_player_advance(&player);
    assert(screencast_player_current_frame(&player) == 2);
    screencast_player_advance(&player);
    assert(screencast_player_current_frame(&player) == 3);
    screencast_player_advance(&player);
    assert(screencast_player_current_frame(&player) == 4);
}

/* The end of the file, the real decision of this module. Playback
 * freezes on the last frame instead of looping: a capture has an end,
 * and playback that started over would take the last image away from
 * whoever is looking at it.
 *
 * The advance is called many times past the end, and the index is
 * asserted on every turn: a cursor that froze once then started over,
 * or that overflowed on the second extra advance, would pass a test
 * that stopped at the first. */
static void test_playback_freezes_on_the_last_frame(void) {
    screencast_player_t player;
    screencast_player_init(&player, 3);

    assert(screencast_player_current_frame(&player) == 0);
    screencast_player_advance(&player);
    assert(screencast_player_current_frame(&player) == 1);
    screencast_player_advance(&player);
    assert(screencast_player_current_frame(&player) == 2);

    /* The last frame is reached: from here on nothing moves, and above
     * all never back to 0. */
    for (int i = 0; i < 50; i++) {
        screencast_player_advance(&player);
        assert(screencast_player_current_frame(&player) == 2);
    }
}

/* A one-frame file is a valid capture (8192 bytes), and the case where
 * a badly written advance leaves the file soonest: the index stays 0,
 * whatever the number of advances. An out-of-range index must never be
 * asked of screencast_read_frame, which would refuse it. */
static void test_a_single_frame_capture_stays_on_its_only_frame(void) {
    screencast_player_t player;
    screencast_player_init(&player, 1);

    for (int i = 0; i < 100; i++) {
        assert(screencast_player_current_frame(&player) == 0);
        screencast_player_advance(&player);
    }
}

/* The refusal, the other half of the module. devtools/screencast
 * already tells apart the three ways a pixel file can be unplayable
 * (missing or unreadable, empty, truncated); what it does not do, and
 * must not do, is decide what to tell a human about it. A window that
 * opens on black cannot be told from a session that starts on a black
 * screen, so the refusal must be explicit and must come before the
 * window.
 *
 * The three messages are asserted one by one and in full. A test that
 * only checked that a non-empty string comes out would pass the most
 * likely mutation of this module -- a single generic message for all
 * three cases. Each names the path (the one typed is not always the one
 * meant), says the cause, and says that no window was opened. */
static void test_each_refusal_names_the_path_and_its_own_reason(void) {
    char out[SCREENCAST_PLAYER_REFUSAL_MAX];

    screencast_player_format_refusal(SCREENCAST_UNREADABLE, "tmp/absent.bin", out, sizeof out);
    assert(strcmp(out,
                  "screencast-viewer: 'tmp/absent.bin' is missing or unreadable -- no window was opened. "
                  "Make the pixels: make screencast EVENTS=<events.json>") == 0);

    screencast_player_format_refusal(SCREENCAST_EMPTY, "tmp/empty.bin", out, sizeof out);
    assert(strcmp(out,
                  "screencast-viewer: 'tmp/empty.bin' is empty -- no window was opened. "
                  "A capture interrupted before its first frame leaves this file; redo it: make screencast EVENTS=<events.json>") == 0);

    screencast_player_format_refusal(SCREENCAST_TRUNCATED, "tmp/truncated.bin", out, sizeof out);
    assert(strcmp(out,
                  "screencast-viewer: 'tmp/truncated.bin' is truncated: its size is not a multiple of 8192 bytes, "
                  "so it ends in the middle of a frame -- no window was opened. "
                  "Redo it: make screencast EVENTS=<events.json>") == 0);
}

/* The same contract as replay_control_format_status: an output always
 * terminated by a NUL, truncated rather than overflowed. An error message
 * must not itself be the fault -- and this one is written just before
 * screencast_viewer returns 1, where nothing catches an overflow any
 * more. The 16-byte buffer is shorter than the shortest of the three
 * messages, so truncation is certain. */
static void test_a_refusal_truncates_rather_than_overflowing(void) {
    char out[16];
    char guard[8];

    memset(guard, 0x7f, sizeof guard);
    screencast_player_format_refusal(SCREENCAST_UNREADABLE, "tmp/absent.bin", out, sizeof out);

    assert(out[sizeof out - 1] == '\0');
    assert(strlen(out) < sizeof out);
    for (size_t i = 0; i < sizeof guard; i++) {
        assert(guard[i] == 0x7f);
    }
}

/* The palette. The viewer has no palette of its own: like the game's
 * raylib backend and the GIF writer, it converts each color index itself
 * through `P8_PALETTE_RGB` (src/p8/palette.c, no raylib, three bytes per
 * color).
 *
 * Forty-eight hand-written hex bytes are exactly where a typo hides
 * without breaking anything visible: a slightly wrong blue fails no
 * build and only shows to the eye, on a window nobody here can open.
 * Hence these sixteen assertions, one per color. The values are the
 * published, widely documented ones of the PICO-8 default palette. */
static void test_the_shared_palette_is_the_published_pico8_one(void) {
    static const unsigned char expected[P8_PALETTE_COLORS][3] = {
        { 0x00, 0x00, 0x00 }, { 0x1D, 0x2B, 0x53 }, { 0x7E, 0x25, 0x53 }, { 0x00, 0x87, 0x51 },
        { 0xAB, 0x52, 0x36 }, { 0x5F, 0x57, 0x4F }, { 0xC2, 0xC3, 0xC7 }, { 0xFF, 0xF1, 0xE8 },
        { 0xFF, 0x00, 0x4D }, { 0xFF, 0xA3, 0x00 }, { 0xFF, 0xEC, 0x27 }, { 0x00, 0xE4, 0x36 },
        { 0x29, 0xAD, 0xFF }, { 0x83, 0x76, 0x9C }, { 0xFF, 0x77, 0xA8 }, { 0xFF, 0xCC, 0xAA },
    };

    for (int i = 0; i < P8_PALETTE_COLORS; i++) {
        assert(P8_PALETTE_RGB[i].red == expected[i][0]);
        assert(P8_PALETTE_RGB[i].green == expected[i][1]);
        assert(P8_PALETTE_RGB[i].blue == expected[i][2]);
    }
}

int main(void) {
    test_playback_starts_at_the_first_frame_and_walks_in_order();
    test_playback_freezes_on_the_last_frame();
    test_a_single_frame_capture_stays_on_its_only_frame();
    test_each_refusal_names_the_path_and_its_own_reason();
    test_a_refusal_truncates_rather_than_overflowing();
    test_the_shared_palette_is_the_published_pico8_one();

    printf("test_screencast_player: OK\n");
    return 0;
}
