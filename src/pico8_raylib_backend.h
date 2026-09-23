/* The raylib-facing half of the interactive PC driver: the window, the
 * upload of the 128x128 PICO-8 framebuffer, the live input poll, and this
 * session's always-on recording. This file includes raylib.h, so nothing
 * in it can be built or exercised by `make unit` -- every piece of the
 * driver that can be decided without a window lives in a raylib-free
 * module instead (pico8_raylib_backend_coords, pico8_replay_source,
 * pico8_session_recorder, pico8_replay_control). */

#ifndef PICO8_RAYLIB_BACKEND_H
#define PICO8_RAYLIB_BACKEND_H

#include "pico8_mem.h"
#include "pico8_input.h"
#include "pico8_session_recorder.h"
#include "pico8_replay_control.h"

#include "raylib.h"

/* Fixed integer scale of the 128x128 framebuffer -> window pixels. The
 * window is not resizable. */
#define PICO8_RAYLIB_BACKEND_SCALE 6

/* Owns the raylib window and texture, this session's recording and the
 * paired marker file. Played frames are never kept in memory. No field is
 * meant to be read or written directly. */
typedef struct {
    Texture2D screen_texture;
    Color pixel_buffer[128 * 128];
    pico8_session_recorder_t recorder;
    char markers_path[64];
} pico8_raylib_backend_t;

/* Opens the window, starts this session's (still empty) recording at
 * tmp/events_recorded_<timestamp>.json and mints the paired marker path
 * tmp/events_recorded_<timestamp>.markers.txt from the same timestamp.
 * Also hides the host cursor: the cart pokes PICO-8's "enable mouse" flag
 * once at boot (nf:pq, pico8_mouse_reader.c) and draws its own cursor, so
 * both would otherwise be visible; shutdown restores it. Must be called
 * exactly once, before any other call on this backend. */
void pico8_raylib_backend_init(pico8_raylib_backend_t *backend);

/* Wraps WindowShouldClose(): 0 once the window has been closed or the
 * platform quit key pressed, 1 otherwise. The main loop's condition. */
int pico8_raylib_backend_should_continue(const pico8_raylib_backend_t *backend);

/* Fills *out_frame from live hardware: the mouse position converted to
 * the 128x128 space (pico8_raylib_backend_window_to_pico8), the left/
 * right/middle buttons as bits 0/1/2 of stat(34)'s layout, and the four
 * arrow keys as btn(0..3) for player 0. Records and presents nothing.
 *
 * Not a pure read: releasing the space bar appends the current frame's
 * 0-based index in the recording to the marker file, as a playtest
 * bookmark with no effect on *out_frame. The index comes from
 * pico8_session_recorder_frame_count: stop recording replayed frames and
 * every marker of a resumed session silently shifts by the replay's
 * length (tests/test_pico8_marker_coherence.c). */
void pico8_raylib_backend_poll_input(pico8_raylib_backend_t *backend, pico8_input_frame_t *out_frame);

/* Uploads the current framebuffer (pico8_peek from PICO8_ADDR_SCREEN, one
 * palette lookup per pixel) and draws it scaled to fill the window.
 * Called once per loop iteration, after pico8__update60()/pico8__draw().
 * Records nothing. `frame` is unused and kept only for the call shape.
 *
 * `replay_status`, when non-NULL, is drawn over the presented texture as
 * plain raylib text -- the only pixels in this port that do not come from
 * the cart. It is drawn after the blit, so it cannot alter the PICO-8
 * framebuffer or the recorded frames, and NULL (live play, and every
 * frame once the recording runs out) draws nothing at all. */
void pico8_raylib_backend_present(pico8_raylib_backend_t *backend, const pico8_mem_t *mem, const pico8_input_frame_t *frame, const char *replay_status);

/* Reads the replay's three keys -- Page Up faster, Page Down slower, End
 * uncapped -- and applies them to *control. Pressed, not held: one tap is
 * one level. Deliberately not folded into poll_input, which is not a pure
 * read: a space bar tapped during a replay would bookmark a frame the
 * player is only watching. Not given the backend, so it cannot reach the
 * recording or the markers. */
void pico8_raylib_backend_poll_replay_controls(pico8_replay_control_t *control);

/* Applies the cap for the frame about to be presented, straight from
 * pico8_replay_control_target_fps; 0 is raylib's own "do not wait between
 * frames". Shortening the wait is the whole abbreviation mechanism -- no
 * recorded frame is ever skipped, or the replayed state is no longer the
 * recorded one. Called every frame rather than on change, so no path can
 * leave a live frame running at replay speed. */
void pico8_raylib_backend_set_frame_rate_cap(int target_fps);

/* Adds *frame to the session's recording. Called once per loop iteration
 * on the frame about to be played, BEFORE pico8__update60()/pico8__draw(),
 * so a frame whose update60() crashes is already on disk -- that frame is
 * the one a crash report needs. A failed append drops the frame rather
 * than killing a live session. Takes replayed and live frames alike, which
 * is what keeps a resumed session's recording complete and replayable in
 * its turn (tests/test_pico8_recording_continuity.c). */
void pico8_raylib_backend_record_frame(pico8_raylib_backend_t *backend, const pico8_input_frame_t *frame);

/* Restores the host cursor, unloads the texture, closes the window and
 * ends the recording; the file on disk is left in place, already complete.
 * Call once, after init, on an initialized backend. */
void pico8_raylib_backend_shutdown(pico8_raylib_backend_t *backend);

#endif
