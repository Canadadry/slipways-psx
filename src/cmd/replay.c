/* REPLAY_FILL=<file>, read by the tools/ scripts: one "<sort> <count>" line of live
 * places per interface sort, then per particle sort; "max_live <sort> <count>"
 * per particle sort; "max_bytes <sort> <bytes>", then "max_elements <sort>
 * <count>", for the list sorts only, burst without max_elements; then
 * "game <bytes>", "max_bytes game <bytes>", "load <bytes>",
 * "max_bytes load <bytes>".
 *
 * STATE=<file>: the devtools/state text of the last frame; after a p8_fatal,
 * "stopped p8_fatal frame <n>" first.
 *
 * DIGEST=<file>: the devtools/vram_digest lines, "sprite-sheet" once loaded,
 * "screen" after cart_draw on each sampled frame, "map" after the last frame;
 * nothing is hashed when unset.
 *
 * SAVE_HEX=<file>: the stored record as one line of lowercase hex (the
 * initial_save format), then "save: " lines on stdout; exits 1 when the
 * record does not decode, or when there is none (the file is then removed). */

#include "p8/vram.h"
#include "p8/sprite_sheet.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/input_step.h"
#include "devtools/dump.h"
#include "devtools/replay_source.h"
#include "devtools/screencast.h"
#include "devtools/state.h"
#include "devtools/vram_digest.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "game/save_state.h"
#include "game/save_storage.h"
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

static void write_digest_line(void *context, const char *line) {
    fputs(line, (FILE *)context);
}

/* Returns the replay's exit status. */
static int write_save_hex(const char *path) {
    static uint8_t bytes[SAVE_STATE_MAX_BYTES];
    static save_state_t decoded;
    int count = save_storage_read(bytes, (int)sizeof bytes);
    if (count == 0) {
        remove(path);
        fprintf(stderr, "replay: no stored record at the end of the run, '%s' not written\n", path);
        return 1;
    }
    FILE *out = fopen(path, "w");
    if (out == NULL) {
        fprintf(stderr, "replay: cannot open the save file '%s'\n", path);
        return 1;
    }
    static const char hex_digits[] = "0123456789abcdef";
    for (int i = 0; i < count; i++) {
        fputc(hex_digits[bytes[i] >> 4], out);
        fputc(hex_digits[bytes[i] & 0x0f], out);
    }
    fputc('\n', out);
    if (fclose(out) != 0) {
        fprintf(stderr, "replay: cannot write the save file '%s'\n", path);
        return 1;
    }
    int decodes = save_state_decode(bytes, count, &decoded);
    printf("save: bytes %d\n", count);
    printf("save: version %d\n", count >= 6 ? bytes[4] | bytes[5] << 8 : -1);
    printf("save: decodes %d\n", decodes);
    if (!decodes) return 1;
    state_write_save_summary(stdout, &decoded, "save: ");
    return 0;
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

    p8_sprite_sheet_load(&memory);

    const char *digest_path = getenv("DIGEST");
    vram_digest_sink_t digest = { write_digest_line, NULL };
    if (digest_path != NULL) {
        digest.context = fopen(digest_path, "w");
        if (digest.context == NULL) {
            fprintf(stderr, "replay: cannot open the digest file '%s'\n", digest_path);
            return 1;
        }
        vram_digest_write_sprite_sheet(&digest, &memory);
    }

    static replay_source_t source;
    replay_source_status_t source_status = replay_source_open(&source, replay_path);
    assert(source_status == REPLAY_SOURCE_LOADED);
    int seeded = replay_source_seed_storage(&source);
    assert(seeded);
    p8_input_t *input = &source.recording;

    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(input);

    cart_load();
    cart_init();

    /* Every capture failure exits 1: a truncated capture yields a wrong hash that reads as a regression. */
    screencast_writer_t capture = { NULL };
    if (capture_path && !screencast_writer_open(&capture, capture_path)) {
        fprintf(stderr, "replay: cannot open the capture file '%s'\n",
                capture_path);
        return 1;
    }

    int frame_count = input->frame_count;
    const char *state_path = getenv("STATE");
    static int i;
    if (state_path != NULL) state_write_on_exit(state_path, &i);
    input_step_t step;
    input_step_init(&step);
    for (i = 0; i < frame_count; i++) {
        input_step_run(&step, &input->frames[input->current_frame]);
        cart_update60();
        cart_draw();
        if (digest_path != NULL && vram_digest_samples_frame(i, frame_count)) {
            vram_digest_write_screen(&digest, &memory, i);
        }
        /* After cart_draw(): at the top of the loop the capture would be shifted by one frame. */
        if (capture_path && !screencast_write_frame(&capture, &memory)) {
            fprintf(stderr, "replay: cannot write frame %d to '%s'\n",
                    i, capture_path);
            screencast_writer_close(&capture);
            return 1;
        }
        if (i < frame_count - 1) {
            p8_input_step(input);
        }
    }

    /* Before the text dump: a fresh dump would make a run whose capture is lost look normal. */
    if (capture_path && !screencast_writer_close(&capture)) {
        fprintf(stderr, "replay: cannot close the capture file '%s'\n",
                capture_path);
        return 1;
    }

    if (digest_path != NULL) {
        vram_digest_write_map(&digest, &memory);
        if (fclose(digest.context) != 0) {
            fprintf(stderr, "replay: cannot write the digest file '%s'\n", digest_path);
            return 1;
        }
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

    const char *save_hex_path = getenv("SAVE_HEX");
    int status = save_hex_path != NULL ? write_save_hex(save_hex_path) : 0;

    replay_source_close(&source);

    printf("replay: %d frame(s) run against %s; framebuffer dumped to %s\n",
           frame_count, replay_path, dump_path);
    if (capture_path) {
        printf("replay: %d frame(s) captured to %s\n", frame_count, capture_path);
    }
    return status;
}
