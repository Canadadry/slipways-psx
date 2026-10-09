/* usage: pad_author <script> <events.json> [--stop-at <label>] [--replay <replay binary>] [--no-aim-budget]
 *
 * Plays the script headless and writes the pad frames it pressed to
 * <events.json>, or, when a command fails, to tmp/pad-author/<name>.failed.json
 * with the state in the default place. STATE=<file>: the state of the last frame, as the replay
 * tool's STATE writes it (default tmp/pad-author/<name>.state.txt). Then
 * replays <events.json> with the replay binary (default tmp/replay) and exits
 * 1 unless its state file is byte-identical.
 *
 * Writes the per-frame mode log and the run's totals to
 * tmp/pad-author/<name>.modes.txt, the totals also to stdout. A whole run
 * (no --stop-at) fails when more than 20% of its frames are in aim mode,
 * unless --no-aim-budget. */

#define _POSIX_C_SOURCE 200809L

#include "devtools/pad_author.h"
#include "devtools/state.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "p8/assets.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define WORK_DIR "tmp/pad-author"
#define PATH_CHARS 512
#define NAME_CHARS 128

static pad_author_t author;

static void usage(void) {
    fprintf(stderr, "usage: pad_author <script> <events.json> [--stop-at <label>] [--replay <replay binary>] "
                    "[--no-aim-budget]\n");
}

static void base_name(char *out, size_t size, const char *path) {
    const char *slash = strrchr(path, '/');
    const char *name = slash != NULL ? slash + 1 : path;
    size_t length = strcspn(name, ".");
    snprintf(out, size, "%.*s", (int)length, name);
}

static int write_state(const char *path) {
    state_release();
    FILE *out = fopen(path, "w");
    if (out == NULL) {
        fprintf(stderr, "pad_author: cannot open the state file '%s'\n", path);
        return 0;
    }
    state_write(out);
    fprintf(out, "cheat activations %d\n", author.cheat_activations);
    return fclose(out) == 0;
}

/* Returns 1 when the replay ran to its end and wrote a state file. */
static int replay(const char *replay_binary, const char *events, const char *state_path, const char *dump_path) {
    remove(state_path);
    fflush(NULL);
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        if (setenv("STATE", state_path, 1) != 0) _exit(127);
        if (freopen("/dev/null", "w", stdout) == NULL) _exit(127);
        execl(replay_binary, replay_binary, events, "/dev/null", dump_path, (char *)NULL);
        _exit(127);
    }
    int status = 0;
    if (waitpid(child, &status, 0) != child) return 0;
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/* Returns 1 when both files exist and hold the same bytes. */
static int same_file(const char *left, const char *right) {
    FILE *a = fopen(left, "rb");
    FILE *b = fopen(right, "rb");
    int same = a != NULL && b != NULL;
    while (same) {
        int ca = fgetc(a);
        int cb = fgetc(b);
        same = ca == cb;
        if (ca == EOF) break;
    }
    if (a != NULL) fclose(a);
    if (b != NULL) fclose(b);
    return same;
}

int main(int argc, char **argv) {
    if (argc < 3 || argv[1][0] == '-' || argv[2][0] == '-') {
        usage();
        return 2;
    }
    const char *script_path = argv[1];
    const char *events_path = argv[2];
    const char *stop_label = NULL;
    const char *replay_binary = "tmp/replay";
    int aim_budget = 1;
    for (int a = 3; a < argc; a++) {
        if (strcmp(argv[a], "--stop-at") == 0 && a + 1 < argc) {
            stop_label = argv[++a];
        } else if (strcmp(argv[a], "--no-aim-budget") == 0) {
            aim_budget = 0;
        } else if (strcmp(argv[a], "--replay") == 0 && a + 1 < argc) {
            replay_binary = argv[++a];
        } else {
            usage();
            return 2;
        }
    }

    static char name[NAME_CHARS];
    static char own_state[PATH_CHARS];
    static char replay_state[PATH_CHARS];
    static char replay_dump[PATH_CHARS];
    static char mode_log_path[PATH_CHARS];
    base_name(name, sizeof name, events_path);
    mkdir("tmp", 0777);
    mkdir(WORK_DIR, 0777);
    const char *state_path = getenv("STATE");
    snprintf(own_state, sizeof own_state, "%s/%s.state.txt", WORK_DIR, name);
    if (state_path == NULL) state_path = own_state;
    snprintf(replay_state, sizeof replay_state, "%s/%s.replay-state.txt", WORK_DIR, name);
    snprintf(replay_dump, sizeof replay_dump, "%s/%s.replay-dump.txt", WORK_DIR, name);
    snprintf(mode_log_path, sizeof mode_log_path, "%s/%s.modes.txt", WORK_DIR, name);

    static p8_vram_t memory;
    static p8_graphics_t graphics;
    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    pad_author_init(&author);
    author.mode_log = fopen(mode_log_path, "w");
    if (author.mode_log == NULL) {
        fprintf(stderr, "pad_author: cannot open the mode log '%s'\n", mode_log_path);
        return 1;
    }
    cart_load();
    cart_init();

    state_write_on_exit(state_path, &author.frame_count);
    int played = pad_author_run(&author, script_path, stop_label);
    static char failed_events[PATH_CHARS];
    if (!played) {
        fprintf(stderr, "pad_author: %s\n", author.error);
        snprintf(failed_events, sizeof failed_events, "%s/%s.failed.json", WORK_DIR, name);
        events_path = failed_events;
        state_path = own_state;
    }
    pad_author_write_totals(&author, author.mode_log, "total ");
    fclose(author.mode_log);
    author.mode_log = NULL;
    if (author.frame_count == 0) {
        state_release();
        fprintf(stderr, "pad_author: %s pressed no frame, '%s' not written\n", script_path, events_path);
        return 1;
    }
    if (!p8_input_save(author.frames, author.frame_count, events_path)) {
        fprintf(stderr, "pad_author: cannot write '%s'\n", events_path);
        return 1;
    }
    if (!write_state(state_path)) return 1;
    printf("pad_author: %s -> %s, %d frame(s), cheat activations %d\n", script_path, events_path, author.frame_count,
           author.cheat_activations);
    pad_author_write_totals(&author, stdout, "pad_author: ");
    if (!played) return 1;
    int aim_frames = author.mode_frames[PAD_AUTHOR_MODE_AIM];
    if (aim_budget && stop_label == NULL && 5 * aim_frames > author.frame_count) {
        fprintf(stderr, "pad_author: %d of %d frames in aim mode, over the 20%% budget\n", aim_frames, author.frame_count);
        return 1;
    }

    if (!replay(replay_binary, events_path, replay_state, replay_dump)) {
        fprintf(stderr, "pad_author: replaying '%s' with %s failed\n", events_path, replay_binary);
        return 1;
    }
    if (!same_file(state_path, replay_state)) {
        fprintf(stderr, "pad_author: generator bug: %s and the replay's %s differ\n", state_path, replay_state);
        return 1;
    }
    printf("pad_author: self-check passed, %s replays to the same state\n", events_path);
    return 0;
}
