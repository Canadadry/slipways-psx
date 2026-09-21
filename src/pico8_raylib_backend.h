/* Phase 5, Etape 1, Chunk 3 (docs/prd/triage/phase5-etape1-raylib-
 * interactive-backend.md): everything genuinely raylib-specific for the
 * interactive PC driver -- opening/closing a fixed-scale window,
 * uploading the 128x128 PICO-8 framebuffer (read via pico8_peek/
 * PICO8_ADDR_SCREEN, exactly like pico8_dump_screen already does) as a
 * texture and presenting it scaled, polling live mouse/keyboard input
 * into a pico8_input_frame_t, and recording every played frame to a
 * timestamped events.json-shaped file via pico8_input_save after every
 * single frame. None of the ported PICO-8 mimicry API (pico8_gfx_t/
 * pico8_mem_t/pico8_input_t/pico8_btn/pico8_stat) is touched or
 * duplicated here -- this module only feeds it a live-polled
 * pico8_input_frame_t and reads pico8_mem_t for presentation, same as
 * main.c's headless driver already does from a parsed events.json.
 *
 * Intended call flow from src/main_raylib.c (Chunk 4), a thin loop:
 *
 *   pico8_raylib_backend_t backend;
 *   pico8_raylib_backend_init(&backend);
 *   while (pico8_raylib_backend_should_continue(&backend)) {
 *       pico8_input_frame_t frame;
 *       pico8_raylib_backend_poll_input(&backend, &frame);
 *       pico8_input_t input = { &frame, 1, 0 };
 *       pico8_dispatch_set_input(&input);   // same call main.c's headless
 *                                           // driver already makes every
 *                                           // frame, just fed live input
 *       pico8__update60();
 *       pico8__draw();
 *       pico8_raylib_backend_present(&backend, &mem, &frame);
 *   }
 *   pico8_raylib_backend_shutdown(&backend);
 *
 * poll_input and present are two calls rather than one "poll + present +
 * record" call because the real game's own update60()/draw() step must
 * run between polling this frame's input and presenting the framebuffer
 * they produced -- this module never calls those ported functions
 * itself (they are main_raylib.c's job, mirroring main.c), so it cannot
 * offer a single call spanning both sides of them. Recording is folded
 * into present() rather than given its own call: by the time present()
 * runs, this frame's input is already known and the frame it produced
 * is already on screen, so that is the one point in the loop where
 * "this frame is done" is true and there is nothing to gain from a
 * fourth call the caller must remember to make every iteration. */

#ifndef PICO8_RAYLIB_BACKEND_H
#define PICO8_RAYLIB_BACKEND_H

#include "pico8_mem.h"
#include "pico8_input.h"

#include "raylib.h"

/* Fixed integer scale of the 128x128 framebuffer -> window pixels
 * (768x768 total). No resizable-window support -- out of scope per the
 * PRD. Chosen as a reasonable, legible default; not load-bearing for
 * anything else in this port, so free to change later without touching
 * any other module (see the PRD's Further Notes). */
#define PICO8_RAYLIB_BACKEND_SCALE 6

/* Owns the raylib window/texture and the growing in-memory list of every
 * pico8_input_frame_t played this session (for the always-on events.json
 * recording). Callers only ever pass a pointer to one of these to the
 * functions below -- no field is meant to be read or written directly. */
typedef struct {
    Texture2D screen_texture;
    Color pixel_buffer[128 * 128];
    pico8_input_frame_t *session_frames;
    int session_frame_count;
    int session_frame_capacity;
    char record_path[64];
    char markers_path[64];
} pico8_raylib_backend_t;

/* Opens the raylib window at PICO8_RAYLIB_BACKEND_SCALE, allocates the
 * (initially empty) session frame list, and computes this session's
 * timestamped recording path (tmp/events_recorded_<timestamp>.json) and
 * its paired marker-file path (tmp/events_recorded_<timestamp>.markers.
 * txt, see pico8_raylib_backend_poll_input()) -- both taken once here,
 * not per frame. Also hides the host OS cursor: this cart's own nf:pq()
 * (pico8_mouse_reader.c) unconditionally pokes PICO-8's real "enable
 * mouse" flag once during boot and never clears it, and real hardware
 * ties that flag to hiding the host cursor since the cart draws its own
 * (ui:ev(), already ported) -- pico8_raylib_backend_shutdown() restores
 * it. Must be called exactly once, before any other
 * pico8_raylib_backend_* call on this backend. */
void pico8_raylib_backend_init(pico8_raylib_backend_t *backend);

/* Wraps raylib's WindowShouldClose(): 0 once the window has been closed
 * (or the platform's usual quit key pressed), 1 otherwise. The caller's
 * main loop condition. */
int pico8_raylib_backend_should_continue(const pico8_raylib_backend_t *backend);

/* Polls the live mouse position (converted to the 128x128 PICO-8
 * coordinate space via pico8_raylib_backend_window_to_pico8, declared in
 * pico8_raylib_backend_coords.h -- see that header for why it is a
 * separate, raylib-free file), the live
 * left/right/middle mouse button state (bit 0/1/2, matching
 * pico8_input.h's documented stat(34) layout), and the four arrow keys
 * (PICO-8's own default btn(0..3) binding for player 0: left/right/up/
 * down -- no alternate binding) into *out_frame. Does not record or
 * present anything.
 *
 * Also writes a small debug-only marker file alongside the recording:
 * releasing the space bar during play appends the current frame's
 * 0-based index (matching the paired recording's own "frames" array
 * index) as a line of plain text to
 * tmp/events_recorded_<timestamp>.markers.txt -- the same <timestamp>
 * as record_path, so the two files pair up by name. Purely a playtest-
 * feedback aid (bookmark "the frame I want to talk about" without
 * describing it) -- no Lua-source equivalent, no effect on *out_frame
 * or the recorded frame data itself. */
void pico8_raylib_backend_poll_input(pico8_raylib_backend_t *backend, pico8_input_frame_t *out_frame);

/* Uploads the current 128x128 framebuffer (read from *mem via
 * pico8_peek/PICO8_ADDR_SCREEN, one PICO8_PALETTE lookup per pixel) as a
 * texture and draws it scaled to fill the window -- the only raylib
 * drawing this module (or this whole backend) ever does; no raylib-
 * native UI/HUD. Then appends *frame (the same frame poll_input filled
 * this iteration) to the session's growing frame list and rewrites the
 * full recording file via pico8_input_save. Called once per loop
 * iteration, after this frame's pico8__update60()/pico8__draw(). */
void pico8_raylib_backend_present(pico8_raylib_backend_t *backend, const pico8_mem_t *mem, const pico8_input_frame_t *frame);

/* Restores the host OS cursor init() hid, unloads the texture, closes
 * the window, and frees the session frame list. The recording file on
 * disk is left in place (it is already complete and valid after every
 * present() call, per pico8_input_save's own full-rewrite-every-time
 * contract). Safe to call only once, after init; not safe to call twice
 * or on a backend that was never initialized. */
void pico8_raylib_backend_shutdown(pico8_raylib_backend_t *backend);

#endif
