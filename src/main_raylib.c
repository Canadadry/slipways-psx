/* The interactive PC driver: same boot sequence as src/main.c up to
 * pico8_cart_load()/pico8__init(), but its per-frame input comes from a
 * live raylib window, or from a recording given as the one optional
 * positional argument, and the framebuffer is presented as a texture
 * instead of dumped as text. src/main.c is never modified by this file.
 *
 * Unlike main.c, this driver must NOT pre-click a difficulty button with
 * pico8_eu(): a recording made here starts on the title screen and holds
 * its own click, so a pre-clicked driver desynchronises every recording
 * it replays. */

#include "p8/mem.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "pico8_cheat.h"
#include "pico8_dispatch_context.h"
#include "pico8_cart.h"
#include "pico8_raylib_backend.h"
#include "pico8_replay_source.h"
#include "pico8_replay_control.h"

#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv) {
    /* One optional positional argument, nothing else: no flags, no
     * recording-picker UI. */
    const char *replay_path = (argc > 1) ? argv[1] : NULL;

    pico8_replay_source_t replay;
    if (pico8_replay_source_open(&replay, replay_path) == PICO8_REPLAY_SOURCE_LOAD_FAILED) {
        assert(!IsWindowReady() && "a bad recording must fail before the window opens: a window on the title screen reads as a lost save");
        fprintf(stderr,
                "main_raylib: '%s' could not be loaded as a recording "
                "(missing, unreadable, or not a valid events.json). "
                "Refusing to start a new game instead.\n",
                replay_path);
        pico8_replay_source_close(&replay);
        return 1;
    }

    if (pico8_replay_source_frame_count(&replay) > 0) {
        fprintf(stderr,
                "main_raylib: replaying '%s' (%d frames), then handing control back.\n"
                "main_raylib: during the replay -- PAGE UP faster, PAGE DOWN slower, END uncapped.\n",
                replay_path, pico8_replay_source_frame_count(&replay));
    }

    /* Initialised before the window opens so the first replayed frame
     * already runs at the replay's default speed. */
    pico8_replay_control_t replay_control;
    pico8_replay_control_init(&replay_control);

    pico8_mem_t mem;
    pico8_mem_init(&mem);

    pico8_gfx_t gfx;
    pico8_gfx_init(&gfx);

    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);

    pico8_raylib_backend_t backend;
    pico8_raylib_backend_init(&backend);

    pico8_cart_load();
    pico8__init();

    /* One loop for replay and live play, with one call site for
     * update60(), draw() and present(): they differ only in where the
     * frame came from. Every recorded frame is stepped AND drawn --
     * drawing has measured side effects on game state, so skipping either
     * misses the recorded state. */
    pico8_cheat_t cheat;
    pico8_cheat_init(&cheat);
    while (pico8_raylib_backend_should_continue(&backend)) {
        pico8_input_frame_t frame;
        int from_recording = pico8_replay_source_next_frame(&replay, &frame);
        if (!from_recording) {
            pico8_raylib_backend_poll_input(&backend, &frame);
        } else {
            /* poll_input is not called while the file drives: live
             * hardware has no say in a replayed frame, and a space bar
             * tapped here would bookmark a frame the player is only
             * watching. raylib still pumps events in present(), so the
             * window stays closable. */
            pico8_raylib_backend_poll_replay_controls(&replay_control);
        }
        pico8_raylib_backend_set_frame_rate_cap(pico8_replay_control_target_fps(&replay_control, from_recording));
        /* Recorded before the game steps, and unconditionally: see
         * pico8_raylib_backend_record_frame. */
        pico8_raylib_backend_record_frame(&backend, &frame);
        pico8_input_t input = { &frame, 1, 0 };
        pico8_dispatch_set_input(&input);

        pico8_cheat_step(&cheat, &input);
        pico8__update60();
        pico8__draw();

        const char *replay_status = NULL;
        char replay_status_text[PICO8_REPLAY_CONTROL_STATUS_MAX];
        if (from_recording) {
            pico8_replay_control_format_status(&replay_control,
                                               pico8_replay_source_frames_served(&replay),
                                               pico8_replay_source_frame_count(&replay),
                                               replay_status_text, sizeof replay_status_text);
            replay_status = replay_status_text;
        }
        pico8_raylib_backend_present(&backend, &mem, &frame, replay_status);
    }

    pico8_raylib_backend_shutdown(&backend);
    pico8_replay_source_close(&replay);

    return 0;
}
