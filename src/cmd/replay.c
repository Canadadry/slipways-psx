/* The headless replay driver: loads pico8/gfx.json, calls cart_load()
 * and cart_init(), steps the game one frame per events.json entry, and
 * dumps the final framebuffer as text. It asserts nothing about the game
 * itself -- that is what tests/ is for.
 *
 * argv[1], required, names the script to replay; without it the driver
 * prints its usage and exits 2.
 *
 * argv[2], optional, is where each frame's pixels are written. Without it,
 * nothing is captured and the final dump is the same as with it --
 * tests/test_headless_capture.c asserts that.
 *
 * argv[3], optional, is where the text dump goes; absent means
 * tmp/replay_dump.txt. Giving it requires giving argv[2].
 *
 * Every replay starts on the title screen: the file itself clicks the
 * difficulty. The driver takes no option; an argument starting with '-'
 * gets the usage and exit 2.
 *
 * A file declaring "keyboard_cursor": true plays each frame through
 * button_cursor; the cheat still reads the raw frame. An undeclared file
 * plays the script as recorded.
 *
 * REPLAY_FILL=<file>, optional, gets after the dump, once the exit queue
 * is flushed: one "<sort> <count>" line of live places per interface sort
 * ("button" to "ephemerals"), then per particle sort; "max_live <sort>
 * <count>" per particle sort; "max_bytes <sort> <bytes>", then
 * "max_elements <sort> <count>", for the list sorts only, burst without
 * max_elements; then "game <bytes>", "max_bytes game <bytes>", "load
 * <bytes>", "max_bytes load <bytes>": the game zone, its loading sub-zone
 * and their session maxima. Capture, dump and exit code stay the same; a
 * fill file that cannot be opened gets a message on stderr, still exit 0.
 *
 * STATE=<file>, optional, gets the devtools/state text of the last frame,
 * before the fill file; a p8_fatal during the replay still writes it there,
 * "stopped p8_fatal frame <n>" first. */

#include "p8/vram.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/cheat.h"
#include "engine/button_cursor.h"
#include "devtools/dump.h"
#include "devtools/screencast.h"
#include "devtools/state.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "game/ui_zone.h"
#include "game/particle_zone.h"
#include "engine/game_zone.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const char *const SLOT_KIND_NAMES[SLOT_KIND_COUNT] = {"button", "modal_backdrop", "message_box", "toast", "row", "ephemerals"};
static const char *const PARTICLE_NAMES[PARTICLE_SLOT_COUNT] = {"particle", "particles", "particles_help_screen", "burst", "transit_dots"};

static void write_fill(const char *path) {
    FILE *out = fopen(path, "w");
    if (out == NULL) {
        fprintf(stderr, "replay: cannot open the fill file '%s'\n", path);
        return;
    }
    cart_flush_exit_queue();
    for (int s = 0; s < SLOT_KIND_COUNT; s++) {
        fprintf(out, "%s %d\n", SLOT_KIND_NAMES[s], ui_zone_live((slot_kind_t)s));
    }
    for (int s = 0; s < PARTICLE_SLOT_COUNT; s++) {
        fprintf(out, "%s %d\n", PARTICLE_NAMES[s], particle_zone_live((particle_slot_kind_t)s));
    }
    for (int s = 0; s < PARTICLE_SLOT_COUNT; s++) {
        fprintf(out, "max_live %s %d\n", PARTICLE_NAMES[s],
                particle_zone_max_live((particle_slot_kind_t)s));
    }
    for (int s = PARTICLE_SLOT_LIST; s < PARTICLE_SLOT_COUNT; s++) {
        fprintf(out, "max_bytes %s %lu\n", PARTICLE_NAMES[s],
                (unsigned long)particle_zone_max_bytes((particle_slot_kind_t)s));
    }
    for (int s = PARTICLE_SLOT_LIST; s < PARTICLE_SLOT_COUNT; s++) {
        if (s == PARTICLE_SLOT_BURST) continue;
        fprintf(out, "max_elements %s %d\n", PARTICLE_NAMES[s],
                particle_zone_max_elements((particle_slot_kind_t)s));
    }
    fprintf(out, "game %lu\n", (unsigned long)game_zone_used());
    fprintf(out, "max_bytes game %lu\n", (unsigned long)game_zone_max());
    fprintf(out, "load %lu\n", (unsigned long)load_zone_used());
    fprintf(out, "max_bytes load %lu\n", (unsigned long)game_zone_load_zone_max());
    fclose(out);
}

int main(int argc, char **argv) {
    if (argc < 2 || argv[1][0] == '-') {
        fprintf(stderr, "usage: replay <events.json> [<capture.bin> [<dump.txt>]]\n");
        return 2;
    }
    const char *replay_path = argv[1];
    const char *capture_path = (argc > 2) ? argv[2] : NULL;
    const char *dump_path = (argc > 3) ? argv[3] : "tmp/replay_dump.txt";

    p8_vram_t memory;
    p8_vram_init(&memory);

    p8_graphics_t graphics;
    p8_gfx_init(&graphics);

    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));

    p8_input_t input;
    p8_input_init(&input);
    assert(p8_input_load(&input, replay_path));

    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);

    cart_load();
    cart_init();

    /* A failure prints its message and returns 1: a missing or truncated
     * capture would yield a wrong hash, indistinguishable from a
     * regression. fprintf rather than assert, so the message names the path. */
    screencast_writer_t capture = { NULL };
    if (capture_path && !screencast_writer_open(&capture, capture_path)) {
        fprintf(stderr, "replay: cannot open the capture file '%s'\n",
                capture_path);
        return 1;
    }

    int frame_count = input.frame_count;
    cheat_t cheat;
    cheat_init(&cheat);
    const char *state_path = getenv("STATE");
    static int i;
    if (state_path != NULL) state_write_on_exit(state_path, &i);
    button_cursor_t cursor;
    button_cursor_init(&cursor);
    p8_input_frame_t played_frame;
    p8_input_t played = { .frames = &played_frame, .frame_count = 1, .current_frame = 0 };
    for (i = 0; i < frame_count; i++) {
        if (input.keyboard_cursor) {
            button_cursor_step(&cursor, &input.frames[input.current_frame], &played_frame);
            dispatch_context_set_input(&played);
        } else {
            dispatch_context_set_input(&input);
        }
        cheat_step(&cheat, &input);
        cart_update60();
        cart_draw();
        /* AFTER cart_draw() and before the next frame: the only moment
         * the screen memory holds the whole frame just drawn. At the top
         * of the loop, the whole capture would be shifted by one frame --
         * tests/test_headless_capture.c catches that by comparing the
         * last captured frame with the text dump. */
        if (capture_path && !screencast_write_frame(&capture, &memory)) {
            fprintf(stderr, "replay: cannot write frame %d to '%s'\n",
                    i, capture_path);
            screencast_writer_close(&capture);
            return 1;
        }
        if (i < frame_count - 1) {
            p8_input_step(&input);
        }
    }

    /* The close is checked like the writes: fclose is the last place
     * where a buffered write can fail. It comes BEFORE the text dump, and
     * a failure therefore skips the dump: a fresh dump would make a run
     * whose capture is lost look normal to whoever reads only the
     * dump. */
    if (capture_path && !screencast_writer_close(&capture)) {
        fprintf(stderr, "replay: cannot close the capture file '%s'\n",
                capture_path);
        return 1;
    }

    FILE *dump_out = fopen(dump_path, "w");
    assert(dump_out != NULL);
    int dump_ok = dump_screen(&memory, dump_out);
    fclose(dump_out);
    assert(dump_ok);

    if (state_path != NULL) {
        state_release();
        FILE *state_out = fopen(state_path, "w");
        if (state_out == NULL) {
            fprintf(stderr, "replay: cannot open the state file '%s'\n", state_path);
            return 1;
        }
        state_write(state_out);
        fclose(state_out);
    }

    const char *fill_path = getenv("REPLAY_FILL");
    if (fill_path != NULL) {
        write_fill(fill_path);
    }

    p8_input_free(&input);

    printf("replay: %d frame(s) run against %s; framebuffer dumped to %s\n",
           frame_count, replay_path, dump_path);
    if (capture_path) {
        printf("replay: %d frame(s) captured to %s\n", frame_count, capture_path);
    }
    return 0;
}
