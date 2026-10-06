/* REPLAY_FILL=<file>, read by the tools/ scripts: one "<sort> <count>" line of live
 * places per interface sort, then per particle sort; "max_live <sort> <count>"
 * per particle sort; "max_bytes <sort> <bytes>", then "max_elements <sort>
 * <count>", for the list sorts only, burst without max_elements; then
 * "game <bytes>", "max_bytes game <bytes>", "load <bytes>",
 * "max_bytes load <bytes>".
 *
 * STATE=<file>: the devtools/state text of the last frame; after a p8_fatal,
 * "stopped p8_fatal frame <n>" first. */

#include "p8/vram.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/input_step.h"
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

    /* Every capture failure exits 1: a truncated capture yields a wrong hash that reads as a regression. */
    screencast_writer_t capture = { NULL };
    if (capture_path && !screencast_writer_open(&capture, capture_path)) {
        fprintf(stderr, "replay: cannot open the capture file '%s'\n",
                capture_path);
        return 1;
    }

    int frame_count = input.frame_count;
    const char *state_path = getenv("STATE");
    static int i;
    if (state_path != NULL) state_write_on_exit(state_path, &i);
    input_step_t step;
    input_step_init(&step);
    for (i = 0; i < frame_count; i++) {
        input_step_run(&step, &input.frames[input.current_frame]);
        cart_update60();
        cart_draw();
        /* After cart_draw(): at the top of the loop the capture would be shifted by one frame. */
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

    /* Before the text dump: a fresh dump would make a run whose capture is lost look normal. */
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
