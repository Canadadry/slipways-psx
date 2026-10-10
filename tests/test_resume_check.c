#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "devtools/save_check.h"

#include "devtools/replay_source.h"
#include "devtools/save_storage_memory.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "game/economy_score.h"
#include "game/input_step.h"
#include "game/main_economy.h"
#include "game/save_state.h"
#include "game/save_storage.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum { REPORT_CAP = 3072 };

typedef enum { SIDE_ORIGINAL, SIDE_RESUMED, SIDE_COUNT } side_t;

static const char *const SIDE_NAMES[SIDE_COUNT] = {"original", "resumed"};

/* Lives in memory shared with the parent: a snapshot is too big for one atomic pipe write. */
typedef struct {
    save_check_snapshot_t snapshot;
    int done;
    char report[REPORT_CAP];
} side_result_t;

static p8_vram_t memory;
static p8_graphics_t graphics;
static replay_source_t source;
static input_step_t step;
static uint8_t record[SAVE_STATE_MAX_BYTES];
static uint8_t stored[SAVE_STATE_MAX_BYTES];

static int game_is_running(void) {
    return main_economy_typed() != NULL && economy_score_difficulty() != NULL;
}

static void boot(const char *path) {
    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    assert(replay_source_open(&source, path) == REPLAY_SOURCE_LOADED);
    assert(replay_source_seed_storage(&source));
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&source.recording);
    cart_load();
    cart_init();
    input_step_init(&step);
}

/* snapshot: when not NULL, captured between the update and the draw. */
static void play_recorded_frame(int frame, save_check_snapshot_t *snapshot) {
    p8_input_t *input = &source.recording;
    input_step_run(&step, &input->frames[input->current_frame]);
    cart_update60();
    if (snapshot != NULL && game_is_running()) {
        save_check_capture(snapshot);
    }
    cart_draw();
    if (frame < input->frame_count - 1) {
        p8_input_step(input);
    }
}

/* Returns 1 when the screen was taken, 0 when a box hides the map. */
static int play_compared_frame(save_check_snapshot_t *snapshot) {
    static const p8_input_frame_t no_button;
    int taken = 0;
    /* Through the input step, not a bare empty input: the pad step sets the crosshair each frame,
       and a game resumed inside a frame's update has a fresh UI without it. */
    input_step_run(&step, &no_button);
    save_check_hide_particles(1);
    cart_update60();
    cart_draw();
    if (save_check_screen_comparable()) {
        save_check_capture_screen(snapshot);
        taken = 1;
    }
    save_check_hide_particles(0);
    return taken;
}

static int storage_holds_record(int count) {
    return save_storage_read(stored, (int)sizeof stored) == count && memcmp(stored, record, (size_t)count) == 0;
}

/* Returns 0 when the side's snapshot is complete, else fills result->report. */
static int run_side(side_t side, const char *path, int before, int after, side_result_t *result) {
    int last = side == SIDE_ORIGINAL ? before : after;
    int count = 0;
    int writes = 0;
    int title_seen = 0;
    boot(path);
    if (last >= source.recording.frame_count) {
        snprintf(result->report, sizeof result->report, "f%d is past the recording's %d frames", last,
                 source.recording.frame_count);
        return 1;
    }
    for (int frame = 0; frame <= last; frame++) {
        if (frame == after && !(storage_holds_record(count) && save_storage_memory_write_count() == writes)) {
            snprintf(result->report, sizeof result->report, "the storage changed before the resume");
            return 1;
        }
        play_recorded_frame(frame, side == SIDE_RESUMED && frame == after ? &result->snapshot : NULL);
        if (frame > before && frame < after && !game_is_running()) {
            title_seen = 1;
        }
        if (frame == before) {
            if (!game_is_running()) {
                snprintf(result->report, sizeof result->report, "no game after frame %d", before);
                return 1;
            }
            count = save_storage_read(record, (int)sizeof record);
            writes = save_storage_memory_write_count();
            if (count == 0) {
                snprintf(result->report, sizeof result->report, "no stored record after frame %d", before);
                return 1;
            }
        }
    }
    if (!game_is_running()) {
        snprintf(result->report, sizeof result->report, "no game after frame %d", last);
        return 1;
    }
    if (side == SIDE_RESUMED && !title_seen) {
        snprintf(result->report, sizeof result->report, "no title between f%d and f%d: nothing was resumed", before, after);
        return 1;
    }
    if (side == SIDE_ORIGINAL) {
        save_check_capture(&result->snapshot);
        /* The record is what resume loads. */
        memcpy(result->snapshot.bytes, record, (size_t)count);
        result->snapshot.byte_count = count;
    }
    if (!play_compared_frame(&result->snapshot)) {
        snprintf(result->report, sizeof result->report, "a box is open on the compared frame after f%d", last);
        return 1;
    }
    if (side == SIDE_RESUMED && !(storage_holds_record(count) && save_storage_memory_write_count() == writes)) {
        snprintf(result->report, sizeof result->report, "the resume or the resumed game's compared frame wrote a save");
        return 1;
    }
    return 0;
}

static pid_t start_side(side_t side, const char *path, int before, int after, side_result_t *result) {
    pid_t pid;
    fflush(stdout);
    fflush(stderr);
    pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        result->done = run_side(side, path, before, after, result) == 0;
        _exit(0);
    }
    return pid;
}

/* Returns 0 when the side finished with a complete snapshot; otherwise prints why. */
static int finish_side(side_t side, pid_t pid, const side_result_t *result, const char *prefix) {
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        printf("%s: %s side stopped (%s %d)\n", prefix, SIDE_NAMES[side], WIFSIGNALED(status) ? "signal" : "exit",
               WIFSIGNALED(status) ? WTERMSIG(status) : WEXITSTATUS(status));
        return 1;
    }
    if (!result->done) {
        printf("%s: %s side: %s\n", prefix, SIDE_NAMES[side], result->report);
        return 1;
    }
    return 0;
}

static int parse_frame(const char *text, int *out) {
    char *end;
    long value = strtol(text, &end, 10);
    if (*text == '\0' || *end != '\0' || value < 0 || value > 1000000) {
        return 0;
    }
    *out = (int)value;
    return 1;
}

int main(int argc, char **argv) {
    static char report[REPORT_CAP];
    static char prefix[512];
    side_result_t *results;
    pid_t pids[SIDE_COUNT];
    int before;
    int after;
    int failed = 0;

    if (argc != 4 || !parse_frame(argv[2], &before) || !parse_frame(argv[3], &after) || before >= after) {
        fprintf(stderr, "usage: test_resume_check <events.json> <before> <after>, with before < after\n");
        return 2;
    }
    snprintf(prefix, sizeof prefix, "resume-check: %s f%d vs f%d", argv[1], before, after);
    results = mmap(NULL, sizeof(side_result_t) * SIDE_COUNT, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(results != MAP_FAILED);
    memset(results, 0, sizeof(side_result_t) * SIDE_COUNT);
    for (int side = 0; side < SIDE_COUNT; side++) {
        pids[side] = start_side((side_t)side, argv[1], before, after, &results[side]);
    }
    for (int side = 0; side < SIDE_COUNT; side++) {
        failed |= finish_side((side_t)side, pids[side], &results[side], prefix);
    }
    if (failed) {
        return 1;
    }
    if (save_check_compare(&results[SIDE_ORIGINAL].snapshot, &results[SIDE_RESUMED].snapshot, report, sizeof report) != 0) {
        printf("%s (%d bytes): %s\n", prefix, results[SIDE_ORIGINAL].snapshot.byte_count, report);
        return 1;
    }
    printf("%s (%d bytes): ok\n", prefix, results[SIDE_ORIGINAL].snapshot.byte_count);
    return 0;
}
