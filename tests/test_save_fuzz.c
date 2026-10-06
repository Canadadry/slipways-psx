#define _POSIX_C_SOURCE 200809L

#include "devtools/save_check.h"

#include "devtools/save_storage_memory.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "game/cart.h"
#include "game/input_step.h"
#include "game/save_state.h"
#include "game/save_storage.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/vram.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum { REPORT_CAP = 3072, PRINTED_CAP = 40, CHILDREN_CAP = 256 };

typedef enum { FUZZ_OK, FUZZ_MISMATCH } fuzz_status_t;

typedef struct {
    int frame;
    int size;
    int status;
    int screen_compared;
    char report[REPORT_CAP];
} fuzz_result_t;

typedef struct {
    pid_t pid;
    int fd;
    int frame;
    int size;
} fuzz_child_t;

typedef struct {
    int save_frames;
    int screen_frames;
    int dump_only_frames;
    int mismatches;
    int largest_size;
    int largest_frame;
} fuzz_totals_t;

static p8_vram_t memory;
static p8_graphics_t graphics;
static p8_input_t recording;
static p8_input_t idle_input;
static input_step_t step;
static save_state_t captured;
static uint8_t stored[SAVE_STATE_MAX_BYTES];
static uint8_t encoded[SAVE_STATE_MAX_BYTES];
static save_check_snapshot_t original;
static save_check_snapshot_t resumed;

/* Returns -1 when the capture encodes to the stored bytes, else the first differing byte offset. */
static int capture_differs_from(const uint8_t *bytes, int count) {
    int size;
    save_state_capture(&captured);
    size = save_state_encode(&captured, encoded);
    for (int i = 0; i < size && i < count; i++) {
        if (encoded[i] != bytes[i]) {
            return i;
        }
    }
    return size == count ? -1 : (size < count ? size : count);
}

static void play_idle_frame(void) {
    dispatch_context_set_input(&idle_input);
    cart_update60();
    cart_draw();
}

/* Returns 1 when the screen was taken. */
static int play_compared_frame(save_check_snapshot_t *snapshot) {
    int taken = 0;
    save_check_hide_particles(1);
    play_idle_frame();
    if (save_check_screen_comparable()) {
        save_check_capture_screen(snapshot);
        taken = 1;
    }
    save_check_hide_particles(0);
    return taken;
}

static void fail(fuzz_result_t *result, const char *text) {
    result->status = FUZZ_MISMATCH;
    snprintf(result->report, sizeof result->report, "%s", text);
}

static void compare_at_save_frame(fuzz_result_t *result) {
    int offset;
    int writes;
    int original_screen;
    result->size = save_storage_read(stored, (int)sizeof stored);

    offset = capture_differs_from(stored, result->size);
    if (offset >= 0) {
        result->status = FUZZ_MISMATCH;
        snprintf(result->report, sizeof result->report, "stored record != capture at end of update (byte %d)", offset);
        return;
    }
    if (entity_registry_count_tagged(TAG_WORMHOLE) > 0) {
        fail(result, "wormhole live at a save frame");
        return;
    }
    save_check_drop_unsaved_state();
    offset = capture_differs_from(stored, result->size);
    if (offset >= 0) {
        result->status = FUZZ_MISMATCH;
        snprintf(result->report, sizeof result->report, "drop changed saved state (byte %d)", offset);
        return;
    }

    save_check_capture(&original);
    writes = save_storage_memory_write_count();
    original_screen = play_compared_frame(&original);
    if (save_storage_memory_write_count() != writes) {
        fail(result, "the original's compared frame wrote a save");
        return;
    }

    cart_init();
    play_idle_frame();
    memcpy(resumed.bytes, stored, (size_t)result->size);
    resumed.byte_count = result->size;
    save_check_resume(&resumed);
    save_check_capture(&resumed);
    if (!play_compared_frame(&resumed) && original_screen) {
        fail(result, "the resumed game shows a box");
        return;
    }
    if (save_storage_memory_write_count() != writes) {
        fail(result, "the title frame, the resume or the resumed game's compared frame wrote a save");
        return;
    }
    result->screen_compared = original_screen;
    if (save_check_compare(&original, &resumed, result->report, sizeof result->report) != 0) {
        result->status = FUZZ_MISMATCH;
    }
}

static fuzz_child_t start_child(int frame) {
    fuzz_child_t child;
    int fds[2];
    assert(pipe(fds) == 0);
    fflush(stdout);
    fflush(stderr);
    child.frame = frame;
    child.size = 0;
    child.pid = fork();
    assert(child.pid >= 0);
    if (child.pid == 0) {
        fuzz_result_t result;
        memset(&result, 0, sizeof result);
        result.frame = frame;
        result.status = FUZZ_OK;
        close(fds[0]);
        compare_at_save_frame(&result);
        assert(write(fds[1], &result, sizeof result) == (ssize_t)sizeof result);
        _exit(0);
    }
    close(fds[1]);
    child.fd = fds[0];
    return child;
}

static void print_mismatch(const char *path, const fuzz_totals_t *totals, int frame, int size, const char *report) {
    if (totals->mismatches <= PRINTED_CAP) {
        printf("save-fuzz: %s f%d (%d bytes): %s\n", path, frame, size, report);
    }
}

static void finish_child(const char *path, fuzz_child_t child, fuzz_totals_t *totals) {
    static fuzz_result_t result;
    int status;
    ssize_t got = read(child.fd, &result, sizeof result);
    close(child.fd);
    assert(waitpid(child.pid, &status, 0) == child.pid);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 || got != (ssize_t)sizeof result) {
        char report[64];
        snprintf(report, sizeof report, "stopped (%s %d)", WIFSIGNALED(status) ? "signal" : "exit",
                 WIFSIGNALED(status) ? WTERMSIG(status) : WEXITSTATUS(status));
        totals->mismatches++;
        print_mismatch(path, totals, child.frame, child.size, report);
        return;
    }
    if (result.screen_compared) {
        totals->screen_frames++;
    } else {
        totals->dump_only_frames++;
    }
    if (result.status != FUZZ_OK) {
        totals->mismatches++;
        print_mismatch(path, totals, result.frame, result.size, result.report);
    }
}

static int fuzz_game(const char *path, int cores) {
    static fuzz_child_t children[CHILDREN_CAP];
    fuzz_totals_t totals = {0, 0, 0, 0, 0, -1};
    int oldest = 0;
    int live = 0;

    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    p8_input_init(&recording);
    p8_input_init(&idle_input);
    assert(p8_input_load(&recording, path));
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&recording);
    cart_load();
    cart_init();
    input_step_init(&step);
    for (int frame = 0; frame < recording.frame_count; frame++) {
        int writes = save_storage_memory_write_count();
        input_step_run(&step, &recording.frames[recording.current_frame]);
        cart_update60();
        if (save_storage_memory_write_count() != writes) {
            int size = save_storage_read(stored, (int)sizeof stored);
            totals.save_frames++;
            if (size > totals.largest_size) {
                totals.largest_size = size;
                totals.largest_frame = frame;
            }
            if (live == cores) {
                finish_child(path, children[oldest], &totals);
                oldest = (oldest + 1) % CHILDREN_CAP;
                live--;
            }
            /* Forked before cart_draw: the record holds the end-of-update RNG state and drawing consumes the RNG. */
            children[(oldest + live) % CHILDREN_CAP] = start_child(frame);
            children[(oldest + live) % CHILDREN_CAP].size = size;
            live++;
        }
        cart_draw();
        if (frame < recording.frame_count - 1) {
            p8_input_step(&recording);
        }
    }
    while (live > 0) {
        finish_child(path, children[oldest], &totals);
        oldest = (oldest + 1) % CHILDREN_CAP;
        live--;
    }
    if (totals.mismatches > PRINTED_CAP) {
        printf("save-fuzz: ... and %d more\n", totals.mismatches - PRINTED_CAP);
    }
    printf("save-fuzz: %s: %d save frames, %d screen-compared, %d dump-only, %d mismatches; largest record %d bytes at f%d\n", path,
           totals.save_frames, totals.screen_frames, totals.dump_only_frames, totals.mismatches, totals.largest_size, totals.largest_frame);
    if (totals.save_frames == 0) {
        printf("save-fuzz: %s: no frame wrote a save\n", path);
        return 1;
    }
    if (totals.screen_frames == 0) {
        printf("save-fuzz: %s: no save frame compared screens\n", path);
        return 1;
    }
    return totals.mismatches != 0;
}

int main(int argc, char **argv) {
    long online = sysconf(_SC_NPROCESSORS_ONLN);
    int cores = online < 1 ? 1 : (online > CHILDREN_CAP ? CHILDREN_CAP : (int)online);
    int failed = 0;
    _Static_assert(sizeof(fuzz_result_t) <= PIPE_BUF, "a child's result goes through the pipe in one atomic write");

    if (argc < 2) {
        fprintf(stderr, "usage: test_save_fuzz <events.json>...\n");
        return 2;
    }
    for (int i = 1; i < argc; i++) {
        failed |= fuzz_game(argv[i], cores);
    }
    return failed;
}
