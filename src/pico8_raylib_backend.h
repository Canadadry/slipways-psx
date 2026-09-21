/* Phase 5, Etape 1, Chunk 3 (docs/prd/triage/phase5-etape1-raylib-
 * interactive-backend.md): everything genuinely raylib-specific for the
 * interactive PC driver -- opening/closing a fixed-scale window,
 * uploading the 128x128 PICO-8 framebuffer (read via pico8_peek/
 * PICO8_ADDR_SCREEN, exactly like pico8_dump_screen already does) as a
 * texture and presenting it scaled, polling live mouse/keyboard input
 * into a pico8_input_frame_t, and recording every played frame to a
 * timestamped events.json-shaped file (through pico8_session_recorder,
 * which owns that bookkeeping since Chunk 3 of docs/prd/triage/raylib-
 * replay-then-resume.md -- see the struct below) after every single
 * frame. None of the ported PICO-8 mimicry API (pico8_gfx_t/
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
 *       pico8_raylib_backend_record_frame(&backend, &frame);
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
 * offer a single call spanning both sides of them.
 *
 * Recording is its own third call, made BEFORE update60()/draw() rather
 * than folded into present() after them. It used to be folded into
 * present(), on the reasoning that present() is the one point where
 * "this frame is done" is true; a real playtest segfault disproved that
 * reasoning. update60() is exactly where a crash lands, and it runs
 * BEFORE present(), so a crashing frame's input was polled, fed to the
 * game, and killed the process without ever reaching the append -- every
 * crash recording was therefore truncated one frame short of the input
 * that caused the crash, which is the single frame a bug report most
 * needs. Reproducing that segfault required reconstructing the missing
 * frame by hand (see docs/plan.md's own "Notes de portage" entry).
 * Recording the frame as soon as it is known costs the same
 * pico8_input_append call and makes a crash recording end WITH the
 * crashing frame, so replaying it headlessly reproduces the crash
 * directly. The cost of the new ordering is one frame of over-recording
 * on a clean quit (the final polled frame is recorded even though
 * WindowShouldClose ends the loop before it is presented), which is
 * harmless: it replays as one extra frame of real input. */

#ifndef PICO8_RAYLIB_BACKEND_H
#define PICO8_RAYLIB_BACKEND_H

#include "pico8_mem.h"
#include "pico8_input.h"
#include "pico8_session_recorder.h"

#include "raylib.h"

/* Fixed integer scale of the 128x128 framebuffer -> window pixels
 * (768x768 total). No resizable-window support -- out of scope per the
 * PRD. Chosen as a reasonable, legible default; not load-bearing for
 * anything else in this port, so free to change later without touching
 * any other module (see the PRD's Further Notes). */
#define PICO8_RAYLIB_BACKEND_SCALE 6

/* Owns the raylib window/texture, this session's always-on events.json
 * recording, and the paired marker file. The frames themselves are never
 * kept in memory -- each one is added to the recording as it is played
 * and never needed again. Callers only ever pass a pointer to one of
 * these to the functions below -- no field is meant to be read or
 * written directly.
 *
 * `recorder` used to be three plain fields here (a frame count, an
 * append tail offset and a path). Chunk 3 of docs/prd/triage/raylib-
 * replay-then-resume.md moved them into pico8_session_recorder.{c,h}
 * unchanged, for a testing reason rather than a design one: this file
 * includes raylib.h, so nothing in it can be exercised by `make unit`,
 * and the one property that chunk owes the player -- a resumed session
 * keeps recording, replayed frames then new ones -- could not be
 * asserted anywhere while the bookkeeping lived here. It is asserted
 * now, in tests/test_pico8_recording_continuity.c, against the real
 * recorder and the real pico8_replay_source. See that module's header
 * for the rest, including why the timestamped path is still minted
 * below and not there. */
typedef struct {
    Texture2D screen_texture;
    Color pixel_buffer[128 * 128];
    pico8_session_recorder_t recorder;
    char markers_path[64];
} pico8_raylib_backend_t;

/* Opens the raylib window at PICO8_RAYLIB_BACKEND_SCALE, starts this
 * session's (still empty) recording, and computes this session's
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
 * as the recording's own path, so the two files pair up by name. Purely a playtest-
 * feedback aid (bookmark "the frame I want to talk about" without
 * describing it) -- no Lua-source equivalent, no effect on *out_frame
 * or the recorded frame data itself. */
void pico8_raylib_backend_poll_input(pico8_raylib_backend_t *backend, pico8_input_frame_t *out_frame);

/* Uploads the current 128x128 framebuffer (read from *mem via
 * pico8_peek/PICO8_ADDR_SCREEN, one PICO8_PALETTE lookup per pixel) as a
 * texture and draws it scaled to fill the window -- the only raylib
 * drawing this module (or this whole backend) ever does; no raylib-
 * native UI/HUD. Called once per loop iteration, after this frame's
 * pico8__update60()/pico8__draw(). Does not record anything: that is
 * pico8_raylib_backend_record_frame()'s job, and it deliberately happens
 * before update60() rather than here (see this header's own top comment
 * on why). `frame` is no longer read by this function and is kept only
 * so the call site's shape stays stable. */
void pico8_raylib_backend_present(pico8_raylib_backend_t *backend, const pico8_mem_t *mem, const pico8_input_frame_t *frame);

/* Adds *frame to the session's recording (pico8_session_recorder, which
 * appends rather than rewriting, so recording a frame costs the same
 * whether it is the session's first or its thirty-thousandth). Called
 * once per loop iteration, on whatever frame is about to be played,
 * BEFORE pico8__update60()/pico8__draw(), so that a frame whose
 * update60() crashes is already on disk -- see this header's own top
 * comment for the playtest segfault that forced this ordering. A failed
 * append silently drops the frame rather than crashing a live playtest
 * session.
 *
 * `frame` is deliberately not asked where it came from. src/main_raylib.c
 * calls this unconditionally, so a frame replayed from a recording is
 * recorded exactly like a live one, which is what makes the recording of
 * a resumed session hold the replayed frames followed by the new ones
 * and stay replayable in its turn (PRD "continuite de l'enregistrement",
 * user stories 8 and 9, proven end to end in
 * tests/test_pico8_recording_continuity.c). */
void pico8_raylib_backend_record_frame(pico8_raylib_backend_t *backend, const pico8_input_frame_t *frame);

/* Restores the host OS cursor init() hid, unloads the texture and closes
 * the window. The recording file on disk is left in place (it is already
 * complete and valid after every present() call, per pico8_input_append's
 * own contract). Safe to call only once, after init; not safe to call
 * twice or on a backend that was never initialized. */
void pico8_raylib_backend_shutdown(pico8_raylib_backend_t *backend);

#endif
