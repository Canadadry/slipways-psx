/* The interactive PC driver: the same boot as src/cmd/replay.c up to
 * cart_load() and cart_init(), but the input of each frame comes from a
 * live raylib window, or from a recording given as the one optional
 * positional argument, and the framebuffer is presented as a texture
 * instead of dumped as text.
 *
 * The driver must NOT click a difficulty button itself: a recording made
 * here starts on the title screen and holds its own click, so a driver
 * that clicked first would desynchronise every recording it replays. */

#include "p8/vram.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/cheat.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "platform/raylib_backend.h"
#include "devtools/replay_source.h"
#include "devtools/replay_control.h"

#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv) {
    /* One optional positional argument, nothing else: no flags, no
     * recording-picker UI. */
    const char *replay_path = (argc > 1) ? argv[1] : NULL;

    replay_source_t replay;
    if (replay_source_open(&replay, replay_path) == REPLAY_SOURCE_LOAD_FAILED) {
        assert(!IsWindowReady() && "a bad recording must fail before the window opens: a window on the title screen reads as a lost save");
        fprintf(stderr,
                "game: '%s' could not be loaded as a recording "
                "(missing, unreadable, or not a valid events.json). "
                "Refusing to start a new game instead.\n",
                replay_path);
        replay_source_close(&replay);
        return 1;
    }

    if (replay_source_frame_count(&replay) > 0) {
        fprintf(stderr,
                "game: replaying '%s' (%d frames), then handing control back.\n"
                "game: during the replay -- PAGE UP faster, PAGE DOWN slower, END uncapped.\n",
                replay_path, replay_source_frame_count(&replay));
    }

    /* Initialised before the window opens so the first replayed frame
     * already runs at the replay's default speed. */
    replay_control_t replay_control;
    replay_control_init(&replay_control);

    p8_vram_t memory;
    p8_vram_init(&memory);

    p8_graphics_t graphics;
    p8_gfx_init(&graphics);

    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));

    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);

    raylib_backend_t backend;
    raylib_backend_init(&backend);

    cart_load();
    cart_init();

    /* One loop for replay and live play, with one call site for
     * cart_update60(), cart_draw() and raylib_backend_present(): they
     * differ only in where the frame came from. Every recorded frame is
     * stepped AND drawn -- drawing has measured side effects on game
     * state, so skipping either misses the recorded state. */
    cheat_t cheat;
    cheat_init(&cheat);
    while (raylib_backend_should_continue(&backend)) {
        p8_input_frame_t frame;
        int from_recording = replay_source_next_frame(&replay, &frame);
        if (!from_recording) {
            raylib_backend_poll_input(&backend, &frame);
        } else {
            /* poll_input is not called while the file drives: live
             * hardware has no say in a replayed frame, and a space bar
             * tapped here would bookmark a frame the player is only
             * watching. raylib still pumps events in present(), so the
             * window stays closable. */
            raylib_backend_poll_replay_controls(&replay_control);
        }
        raylib_backend_set_frame_rate_cap(replay_control_target_fps(&replay_control, from_recording));
        /* Recorded before the game steps, and unconditionally: see
         * raylib_backend_record_frame. */
        raylib_backend_record_frame(&backend, &frame);
        p8_input_t input = { .frames = &frame, .frame_count = 1, .current_frame = 0 };
        dispatch_context_set_input(&input);

        cheat_step(&cheat, &input);
        cart_update60();
        cart_draw();

        const char *replay_status = NULL;
        char replay_status_text[REPLAY_CONTROL_STATUS_MAX];
        if (from_recording) {
            replay_control_format_status(&replay_control,
                                               replay_source_frames_served(&replay),
                                               replay_source_frame_count(&replay),
                                               replay_status_text, sizeof replay_status_text);
            replay_status = replay_status_text;
        }
        raylib_backend_present(&backend, &memory, &frame, replay_status);
    }

    raylib_backend_shutdown(&backend);
    replay_source_close(&replay);

    return 0;
}
