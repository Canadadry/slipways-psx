/* The raylib-facing half of the interactive PC driver: the window, the
 * framebuffer upload, the live input poll, and this session's always-on
 * recording. */

#ifndef PLATFORM_RAYLIB_BACKEND_H
#define PLATFORM_RAYLIB_BACKEND_H

#include "p8/vram.h"
#include "p8/input.h"
#include "devtools/session_recorder.h"
#include "devtools/replay_control.h"

#include "raylib.h"

/* Window pixels per screen pixel; the window is not resizable. */
#define RAYLIB_BACKEND_SCALE 6

/* No field is meant to be read or written directly. */
typedef struct {
    Texture2D screen_texture;
    Color pixel_buffer[P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT];
    session_recorder_t recorder;
    char markers_path[64];
} raylib_backend_t;

/* Opens the window, hides the host cursor, and starts this session's
 * (still empty) recording at tmp/events_recorded_<timestamp>.json, with
 * its marker file at tmp/events_recorded_<timestamp>.markers.txt.
 * `keyboard_cursor` is written in the recording's head. Call exactly
 * once, before any other call taking the backend. */
void raylib_backend_init(raylib_backend_t *backend, int keyboard_cursor);

/* 0 once the window has been closed or the quit key pressed. */
int raylib_backend_should_continue(const raylib_backend_t *backend);

/* Fills *out_frame from the mouse (in screen space), the arrow keys
 * (d-pad 0 to 3), Z and X (4 and 5). Not a pure read: releasing the space
 * bar appends the frame's index in the recording to the marker file. That
 * index is session_recorder_frame_count: stop recording replayed frames
 * and every marker of a resumed session shifts by the replay's length. */
void raylib_backend_poll_input(raylib_backend_t *backend, p8_input_frame_t *out_frame);

/* Draws mem->screen scaled to the window. `frame` is unused.
 * `replay_status`, when non-NULL, is drawn over it as raylib text, never
 * into the PICO-8 framebuffer; NULL draws nothing. */
void raylib_backend_present(raylib_backend_t *backend, const p8_vram_t *mem, const p8_input_frame_t *frame, const char *replay_status);

/* Page Up faster, Page Down slower, End uncapped; one tap is one level.
 * Kept apart from poll_input, which writes markers: a space bar tapped
 * during a replay would bookmark a frame the player is only watching. */
void raylib_backend_poll_replay_controls(replay_control_t *control);

/* 0 means "do not wait between frames". Call every frame, not on change,
 * so no path leaves a live frame running at replay speed. */
void raylib_backend_set_frame_rate_cap(int target_fps);

/* Call BEFORE cart_update60(), so a frame whose update crashes is
 * already on disk. Give it replayed and live frames alike, or a resumed
 * session's recording is incomplete. A failed append drops the frame
 * silently. */
void raylib_backend_record_frame(raylib_backend_t *backend, const p8_input_frame_t *frame);

/* Call once, after init. The recording on disk is left in place. */
void raylib_backend_shutdown(raylib_backend_t *backend);

#endif
