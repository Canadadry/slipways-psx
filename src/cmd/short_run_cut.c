#define _POSIX_C_SOURCE 200809L

#include "devtools/replay_source.h"
#include "devtools/save_storage_memory.h"
#include "devtools/short_run_cut.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "game/input_step.h"
#include "game/save_state.h"
#include "game/save_storage.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/sprite_sheet.h"
#include "p8/vram.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum { PATH_CAP = 1024, LINE_CAP = 256 };

typedef struct {
    short_run_cut_save_t *items;
    int count;
    int cap;
} saves_t;

static saves_t saves;
static short_run_cut_plan_t plan;

static void usage(void) {
    fprintf(stderr, "usage: short_run_cut <full.json> <manifest> <out-dir> [<name>...]\n");
}

static int keep_save(int frame) {
    static uint8_t bytes[SAVE_STATE_MAX_BYTES];
    int count = save_storage_read(bytes, (int)sizeof bytes);
    if (saves.count == saves.cap) {
        int cap = saves.cap == 0 ? 256 : 2 * saves.cap;
        short_run_cut_save_t *grown = realloc(saves.items, sizeof saves.items[0] * (size_t)cap);
        if (grown == NULL) return 0;
        saves.items = grown;
        saves.cap = cap;
    }
    uint8_t *copy = malloc((size_t)count);
    if (copy == NULL) return 0;
    memcpy(copy, bytes, (size_t)count);
    saves.items[saves.count++] = (short_run_cut_save_t){ frame, copy, count };
    return 1;
}

/* Replays the recording as tmp/replay does and keeps the record stored by each frame that wrote one. */
static int collect_saves(replay_source_t *source) {
    static p8_vram_t memory;
    static p8_graphics_t graphics;
    static bindings_t bindings;
    static replay_source_bindings_report_t report;
    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    p8_sprite_sheet_load(&memory);
    if (!replay_source_seed_storage(source)
        || replay_source_read_bindings(source, &bindings_compiled, &bindings, &report) != REPLAY_SOURCE_BINDINGS_DEFAULT) {
        fprintf(stderr, "short_run_cut: %s: needs the default bindings and no initial_save\n", source->path);
        return 0;
    }
    p8_input_t *input = &source->recording;
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(input);
    cart_load();
    cart_init();
    input_step_t step;
    input_step_init(&step, &bindings);
    input_step_set_attract_after(&step, replay_source_attract_after(source));
    for (int frame = 0; frame < input->frame_count; frame++) {
        int writes = save_storage_memory_write_count();
        input_step_run(&step, &input->frames[input->current_frame]);
        cart_update60();
        cart_draw();
        if (save_storage_memory_write_count() != writes && !keep_save(frame)) {
            fprintf(stderr, "short_run_cut: out of memory at f%d\n", frame);
            return 0;
        }
        if (frame < input->frame_count - 1) p8_input_step(input);
    }
    return 1;
}

static int selected(const char *name, char **names, int name_count) {
    if (name_count == 0) return 1;
    for (int i = 0; i < name_count; i++) {
        if (strcmp(names[i], name) == 0) return 1;
    }
    return 0;
}

static int write_map(const char *path, const char *line) {
    FILE *out = fopen(path, "w");
    if (out == NULL) return 0;
    int ok = fputs(line, out) >= 0;
    return fclose(out) == 0 && ok;
}

static int write_run(const char *out_dir, const short_run_cut_entry_t *entry, const p8_input_t *full) {
    static char header[sizeof "\"initial_save\": \"\"" + 2 * SAVE_STATE_MAX_BYTES];
    char dir[PATH_CAP];
    char path[PATH_CAP + 32];
    char map[SHORT_RUN_CUT_MAP_CAP];
    snprintf(dir, sizeof dir, "%s/%s", out_dir, entry->name);
    if (mkdir(dir, 0777) != 0 && errno != EEXIST) {
        fprintf(stderr, "short_run_cut: %s: cannot create %s\n", entry->name, dir);
        return 0;
    }
    const char *members = NULL;
    if (plan.save_index >= 0) {
        const short_run_cut_save_t *save = &saves.items[plan.save_index];
        char *at = header + sprintf(header, "\"initial_save\": \"");
        for (int i = 0; i < save->count; i++) at += sprintf(at, "%02x", save->bytes[i]);
        sprintf(at, "\"");
        members = header;
    }
    short_run_cut_format_map(entry, &plan, map);
    snprintf(path, sizeof path, "%s/run.json", dir);
    int ok = p8_input_save(plan.frames, plan.frame_count, path, members);
    snprintf(path, sizeof path, "%s/reference.json", dir);
    ok = ok && p8_input_save(full->frames, plan.reference_last + 1, path, NULL);
    snprintf(path, sizeof path, "%s/map.txt", dir);
    ok = ok && write_map(path, map);
    if (!ok) {
        fprintf(stderr, "short_run_cut: %s: cannot write the run into %s\n", entry->name, dir);
        return 0;
    }
    printf("short-run-cut: %s %s length %d -> %s\n", entry->name, plan.cut >= 0 ? "resume" : "prefix",
           plan.frame_count, dir);
    printf("short-run-cut: %s %s", entry->name, map);
    return 1;
}

/* Returns the number of runs refused or not written; -1 when the manifest cannot be read. */
static int cut_runs(const char *manifest_path, const char *out_dir, const p8_input_t *full, char **names,
                    int name_count) {
    FILE *in = fopen(manifest_path, "r");
    if (in == NULL) return -1;
    char line[LINE_CAP];
    char error[SHORT_RUN_CUT_ERROR_CAP];
    int failures = 0;
    int matched = 0;
    while (fgets(line, sizeof line, in) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        const char *text = line + strspn(line, " \t");
        if (*text == '\0' || *text == '#') continue;
        short_run_cut_entry_t entry;
        if (!short_run_cut_parse_entry(text, &entry, error)) {
            fprintf(stderr, "short_run_cut: %s: %s\n", manifest_path, error);
            failures++;
            continue;
        }
        if (!selected(entry.name, names, name_count)) continue;
        matched++;
        if (!short_run_cut_plan(full->frames, full->frame_count, saves.items, saves.count, &entry, &plan, error)) {
            fprintf(stderr, "short_run_cut: refused: %s\n", error);
            failures++;
            continue;
        }
        if (!write_run(out_dir, &entry, full)) failures++;
    }
    fclose(in);
    if (matched < (name_count == 0 ? 1 : name_count)) {
        fprintf(stderr, "short_run_cut: %s: a selected run is not in the manifest, or the manifest is empty\n",
                manifest_path);
        failures++;
    }
    return failures;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        usage();
        return 2;
    }
    static replay_source_t source;
    if (replay_source_open(&source, argv[1]) != REPLAY_SOURCE_LOADED
        || replay_source_start(&source) != REPLAY_SOURCE_START_TITLE) {
        fprintf(stderr, "short_run_cut: %s: not a pad recording that starts on the title\n", argv[1]);
        return 1;
    }
    if (mkdir(argv[3], 0777) != 0 && errno != EEXIST) {
        fprintf(stderr, "short_run_cut: cannot create %s\n", argv[3]);
        return 1;
    }
    if (!collect_saves(&source)) return 1;
    printf("short-run-cut: %s: %d frames, %d save frames\n", argv[1], source.recording.frame_count, saves.count);
    int failures = cut_runs(argv[2], argv[3], &source.recording, argv + 4, argc - 4);
    if (failures < 0) {
        fprintf(stderr, "short_run_cut: cannot read the manifest %s\n", argv[2]);
        return 1;
    }
    return failures != 0;
}
