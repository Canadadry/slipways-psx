/* REPLAY_FILL=<file>, read by the tools/ scripts: one "<sort> <count>" line of live
 * places per interface sort, then per particle sort; "max_live <sort> <count>"
 * per particle sort; "max_bytes <sort> <bytes>", then "max_elements <sort>
 * <count>", for the list sorts only, burst without max_elements; then
 * "game <bytes>", "max_bytes game <bytes>", "load <bytes>",
 * "max_bytes load <bytes>".
 *
 * STATE=<file>: the devtools/state text of the last frame, then "cheat
 * activations <n>"; after a p8_fatal, "stopped p8_fatal frame <n>" first and
 * no cheat line. Also installs the draw meter, whose per-frame GPU cost peaks
 * the state reports.
 *
 * DIGEST=<file>: the devtools/vram_digest lines, "sprite-sheet" once loaded,
 * "screen" after cart_draw on each sampled frame, "map" after the last frame;
 * nothing is hashed when unset.
 *
 * CAPTURE_LIST=<list> with CAPTURE_DIR=<dir>: parses the capture list (devtools/capture_list.h)
 * against this recording, installs the draw filter under the meter, draws each listed frame with its
 * mask, and writes <dir>/pc_frames.bin (those frames, screencast format, list order),
 * <dir>/counts.txt ("<frame> <family> <calls>", ten lines a capture) and <dir>/capture_script.txt
 * ("<frame> <mask hex>" a capture); exits 1 naming the line on a bad list. Nothing when unset.
 *
 * SAVE_HEX=<file>: the stored record as one line of lowercase hex (the
 * initial_save format), then "save: " lines on stdout; exits 1 when the
 * record does not decode, or when there is none (the file is then removed).
 *
 * PAD_METRICS=<file>: the devtools/pad_metrics report of the run (time per
 * mode, dead presses by group, hold repeats, the section 4 verdict); exits 1
 * when the file cannot be written. The verdict never changes the exit status. */

#include "p8/vram.h"
#include "p8/sprite_sheet.h"
#include "p8/gfx.h"
#include "p8/draw_meter.h"
#include "p8/input.h"
#include "game/input_step.h"
#include "devtools/capture_list.h"
#include "devtools/draw_filter.h"
#include "devtools/dump.h"
#include "devtools/pad_metrics.h"
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
#include <string.h>

static const char *const SLOT_KIND_NAMES[SLOT_KIND_COUNT] = {"button", "modal_backdrop", "message_box", "toast", "row", "ephemerals"};
static const char *const PARTICLE_NAMES[PARTICLE_SLOT_COUNT] = {"particle", "particles", "particles_help_screen", "burst", "transit_dots"};

#define PAD_METRICS_SCRATCH_BYTES (1 << 16)
/* Candidate lists of the pad metrics: never frame_zone(), or make state would see the hook in its peaks. */
BUMP_ZONE_STORAGE(pad_metrics_cells, PAD_METRICS_SCRATCH_BYTES);
static bump_zone_t pad_metrics_scratch;
static pad_metrics_t pad_metrics;

/* Returns 1 when the report was written. */
static int write_pad_metrics(const char *path) {
    pad_metrics_finish(&pad_metrics);
    FILE *out = fopen(path, "w");
    if (out == NULL) {
        fprintf(stderr, "replay: cannot open the pad metrics file '%s'\n", path);
        return 0;
    }
    pad_metrics_write_report(&pad_metrics, out);
    if (fclose(out) != 0) {
        fprintf(stderr, "replay: cannot write the pad metrics file '%s'\n", path);
        return 0;
    }
    return 1;
}

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

typedef struct {
    capture_list_t list;
    int next;
    screencast_writer_t frames;
    FILE *counts;
} captures_t;

/* Returns the length read, or -1. */
static long read_text(const char *path, char *text, size_t cap) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) return -1;
    size_t length = fread(text, 1, cap - 1, file);
    int whole = feof(file);
    fclose(file);
    if (!whole) return -1;
    text[length] = '\0';
    return (long)length;
}

static void recording_name(const char *path, char name[CAPTURE_LIST_REPLAY_CAP]) {
    const char *base = strrchr(path, '/');
    base = base != NULL ? base + 1 : path;
    const char *dot = strrchr(base, '.');
    size_t length = dot != NULL && strcmp(dot, ".json") == 0 ? (size_t)(dot - base) : strlen(base);
    snprintf(name, CAPTURE_LIST_REPLAY_CAP, "%.*s", (int)length, base);
}

static FILE *open_in(const char *dir, const char *file_name, const char *mode) {
    char path[1024];
    snprintf(path, sizeof path, "%s/%s", dir, file_name);
    FILE *file = fopen(path, mode);
    if (file == NULL) fprintf(stderr, "replay: cannot open '%s'\n", path);
    return file;
}

/* Returns 0 after naming the failure; installs the draw filter on success. */
static int captures_open(captures_t *captures, const char *list_path, const char *dir, const char *replay_path,
                         int frame_count) {
    static char text[1 << 14];
    if (dir == NULL) {
        fprintf(stderr, "replay: CAPTURE_LIST needs CAPTURE_DIR\n");
        return 0;
    }
    if (read_text(list_path, text, sizeof text) < 0) {
        fprintf(stderr, "replay: cannot read the capture list '%s'\n", list_path);
        return 0;
    }
    char name[CAPTURE_LIST_REPLAY_CAP];
    recording_name(replay_path, name);
    char error[CAPTURE_LIST_ERROR_CAP];
    if (!capture_list_parse(text, name, frame_count, &captures->list, error)) {
        fprintf(stderr, "replay: %s: %s\n", list_path, error);
        return 0;
    }
    FILE *script = open_in(dir, "capture_script.txt", "w");
    if (script == NULL) return 0;
    for (int c = 0; c < captures->list.count; c++) {
        char line[CAPTURE_LIST_SCRIPT_LINE_CAP];
        capture_list_script_line(&captures->list.entries[c], line);
        fputs(line, script);
    }
    if (fclose(script) != 0) return 0;
    captures->counts = open_in(dir, "counts.txt", "w");
    if (captures->counts == NULL) return 0;
    char frames_path[1024];
    snprintf(frames_path, sizeof frames_path, "%s/pc_frames.bin", dir);
    if (!screencast_writer_open(&captures->frames, frames_path)) {
        fprintf(stderr, "replay: cannot open '%s'\n", frames_path);
        return 0;
    }
    captures->next = 0;
    draw_filter_install();
    return 1;
}

static const capture_list_entry_t *captures_at(const captures_t *captures, int frame) {
    if (captures->next >= captures->list.count) return NULL;
    const capture_list_entry_t *entry = &captures->list.entries[captures->next];
    return entry->frame == frame ? entry : NULL;
}

/* Returns 0 on a write failure. */
static int captures_frame_drawn(captures_t *captures, int frame, const p8_vram_t *mem) {
    const capture_list_entry_t *entry = captures_at(captures, frame);
    int ok = 1;
    if (entry != NULL) {
        ok = screencast_write_frame(&captures->frames, mem);
        for (int family = 0; family < DRAW_FILTER_FAMILY_COUNT; family++) {
            fprintf(captures->counts, "%d %s %d\n", frame, capture_list_family_name((draw_filter_family_t)family),
                    draw_filter_count((draw_filter_family_t)family));
        }
        captures->next++;
    }
    draw_filter_frame_end();
    return ok;
}

static int captures_close(captures_t *captures) {
    int counts_ok = fclose(captures->counts) == 0;
    return screencast_writer_close(&captures->frames) && counts_ok;
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

    const char *capture_list_path = getenv("CAPTURE_LIST");
    static captures_t captures;
    if (capture_list_path != NULL
        && !captures_open(&captures, capture_list_path, getenv("CAPTURE_DIR"), replay_path, input->frame_count)) {
        return 1;
    }

    const char *state_path = getenv("STATE");
    if (state_path != NULL) p8_draw_meter_install();

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
    static int i;
    if (state_path != NULL) state_write_on_exit(state_path, &i);
    input_step_t step;
    input_step_init(&step);
    const char *pad_metrics_path = getenv("PAD_METRICS");
    if (pad_metrics_path != NULL) {
        bump_zone_init(&pad_metrics_scratch, "pad metrics scratch", pad_metrics_cells, PAD_METRICS_SCRATCH_BYTES);
        pad_metrics_init_judging(&pad_metrics, &pad_metrics_scratch);
        pad_metrics_observe(&pad_metrics, &step);
    }
    int cheat_activations = 0;
    for (i = 0; i < frame_count; i++) {
        const p8_input_frame_t *frame = &input->frames[input->current_frame];
        int cheat = input_step_run(&step, frame);
        cheat_activations += cheat;
        cart_update60();
        if (capture_list_path != NULL) {
            const capture_list_entry_t *entry = captures_at(&captures, i);
            draw_filter_set_mask(entry != NULL ? entry->mask : DRAW_FILTER_ALL);
        }
        cart_draw();
        if (state_path != NULL) p8_draw_meter_frame_end();
        if (capture_list_path != NULL && !captures_frame_drawn(&captures, i, &memory)) {
            fprintf(stderr, "replay: cannot write the capture outputs of frame %d\n", i);
            return 1;
        }
        if (digest_path != NULL && vram_digest_samples_frame(i, frame_count)) {
            vram_digest_write_screen(&digest, &memory, i);
        }
        if (pad_metrics_path != NULL) pad_metrics_record(&pad_metrics, &step, frame->pad_buttons, cheat);
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

    if (capture_list_path != NULL && !captures_close(&captures)) {
        fprintf(stderr, "replay: cannot close the capture outputs in '%s'\n", getenv("CAPTURE_DIR"));
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
        state_write_cheat_activations(state_out, cheat_activations);
        fclose(state_out);
    }

    if (pad_metrics_path != NULL && !write_pad_metrics(pad_metrics_path)) return 1;

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
