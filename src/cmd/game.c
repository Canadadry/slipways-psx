#include "p8/vram.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/input_step.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "platform/raylib_backend.h"
#include "platform/keymap.h"
#include "devtools/replay_source.h"
#include "devtools/replay_control.h"

#include <assert.h>
#include <stdio.h>

static const char KEYMAP_PATH[] = "keymap.cfg";
static const char USAGE[] = "usage: game [<recording.json>]\n";

int main(int argc, char **argv) {
    const char *replay_path = NULL;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' || replay_path != NULL) {
            fputs(USAGE, stderr);
            return 2;
        } else {
            replay_path = argv[i];
        }
    }

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

    keymap_t keymap;
    keymap_error_t keymap_error;
    keymap_load_status_t keymap_status = keymap_load(KEYMAP_PATH, &keymap, &keymap_error);
    if (keymap_status == KEYMAP_INVALID) {
        assert(!IsWindowReady() && "an invalid keymap.cfg must fail before the window opens");
        if (keymap_error.line > 0) {
            fprintf(stderr, "game: %s:%d: %s\n", KEYMAP_PATH, keymap_error.line, keymap_error.message);
        } else {
            fprintf(stderr, "game: %s: %s\n", KEYMAP_PATH, keymap_error.message);
        }
        replay_source_close(&replay);
        return 1;
    }
    if (keymap_status == KEYMAP_MISSING) {
        fprintf(stderr, "game: %s not found, using the built-in defaults.\n", KEYMAP_PATH);
    }

    if (replay_source_frame_count(&replay) > 0) {
        fprintf(stderr,
                "game: replaying '%s' (%d frames), then handing control back.\n"
                "game: during the replay -- PAGE UP faster, PAGE DOWN slower, END uncapped.\n",
                replay_path, replay_source_frame_count(&replay));
    }

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
    raylib_backend_init(&backend, &keymap);

    cart_load();
    /* Never click a difficulty here: a recording starts on the title screen and holds its own click. */
    cart_init();

    /* Step AND draw every recorded frame: drawing has side effects on game state. */
    input_step_t step;
    input_step_init(&step);
    while (raylib_backend_should_continue(&backend)) {
        p8_input_frame_t frame;
        int from_recording = replay_source_next_frame(&replay, &frame);
        if (!from_recording) {
            raylib_backend_poll_input(&backend, &frame);
        } else {
            raylib_backend_poll_replay_controls(&replay_control);
        }
        raylib_backend_set_frame_rate_cap(replay_control_target_fps(&replay_control, from_recording));
        /* The raw frame, never the played one: a replay runs pad navigation again on what it reads. */
        raylib_backend_record_frame(&backend, &frame);
        input_step_run(&step, &frame);
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
        raylib_backend_present(&backend, &memory, input_step_played_frame(&step), replay_status);
    }

    raylib_backend_shutdown(&backend);
    replay_source_close(&replay);

    return 0;
}
