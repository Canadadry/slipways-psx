#include "game/save_state.h"

#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "game/cart.h"
#include "game/economy_score.h"
#include "game/input_step.h"
#include "game/main_economy.h"
#include "game/planet.h"
#include "p8/sprite_sheet.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum { GAMES_CAP = 16 };

typedef struct {
    int size;
    int frame;
    int records;
} game_result_t;

static p8_vram_t memory;
static p8_graphics_t graphics;
static p8_input_t input;
static input_step_t step;
static save_state_t captured;
static save_state_t decoded;
static uint8_t first_bytes[SAVE_STATE_MAX_BYTES];
static uint8_t second_bytes[SAVE_STATE_MAX_BYTES];

static int game_is_running(void) {
    return main_economy_typed() != NULL && economy_score_difficulty() != NULL && planet_type_picker() != NULL &&
           planet_vanish_roll() != NULL && main_camera() != NULL;
}

static int wormhole_is_live(void) {
    const entity_list_t *wormholes = entities_by_tag(TAG_WORMHOLE);
    return wormholes != NULL && wormholes->count > 0;
}

static void check_frame(const char *path, int frame, game_result_t *result) {
    int size;
    save_state_capture(&captured);
    size = save_state_encode(&captured, first_bytes);
    if (!save_state_decode(first_bytes, size, &decoded)) {
        fprintf(stderr, "test_save_worst_case: %s f%d: the record of %d bytes does not decode\n", path, frame, size);
        _exit(1);
    }
    if (save_state_encode(&decoded, second_bytes) != size || memcmp(first_bytes, second_bytes, (size_t)size) != 0) {
        fprintf(stderr, "test_save_worst_case: %s f%d: the decoded record encodes to other bytes\n", path, frame);
        _exit(1);
    }
    result->records++;
    if (size > result->size) {
        result->size = size;
        result->frame = frame;
    }
}

/* Runs in its own child, forked before cart_load: each game starts from boot. */
static void replay_game(const char *path, int fd) {
    game_result_t result = {0, -1, 0};
    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    p8_sprite_sheet_load(&memory);
    p8_input_init(&input);
    assert(p8_input_load(&input, path));
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);
    cart_load();
    cart_init();
    input_step_init(&step, &bindings_compiled);
    for (int frame = 0; frame < input.frame_count; frame++) {
        input_step_run(&step, &input.frames[input.current_frame]);
        cart_update60();
        cart_draw();
        if (game_is_running() && !wormhole_is_live()) {
            check_frame(path, frame, &result);
        }
        if (frame < input.frame_count - 1) {
            p8_input_step(&input);
        }
    }
    assert(write(fd, &result, sizeof result) == (ssize_t)sizeof result);
}

typedef struct {
    pid_t pid;
    int fd;
} game_run_t;

static game_run_t start_game(const char *path) {
    game_run_t run;
    int fds[2];

    assert(pipe(fds) == 0);
    fflush(stdout);
    run.pid = fork();
    assert(run.pid >= 0);
    if (run.pid == 0) {
        close(fds[0]);
        replay_game(path, fds[1]);
        _exit(0);
    }
    close(fds[1]);
    run.fd = fds[0];
    return run;
}

/* Returns 0 and fills *result when the child replayed the game cleanly. */
static int finish_game(const char *path, game_run_t run, game_result_t *result) {
    int status;
    ssize_t got = read(run.fd, result, sizeof *result);
    close(run.fd);
    assert(waitpid(run.pid, &status, 0) == run.pid);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 || got != (ssize_t)sizeof *result) {
        printf("test_save_worst_case: %s stopped (%s %d)\n", path, WIFSIGNALED(status) ? "signal" : "exit",
               WIFSIGNALED(status) ? WTERMSIG(status) : WEXITSTATUS(status));
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    const char *largest_game = NULL;
    game_result_t largest = {0, -1, 0};
    int records = 0;

    game_run_t runs[GAMES_CAP];
    int failed = 0;

    if (argc < 2 || argc - 1 > GAMES_CAP) {
        fprintf(stderr, "usage: test_save_worst_case <events.json>... (at most %d)\n", GAMES_CAP);
        return 2;
    }
    for (int i = 1; i < argc; i++) {
        runs[i - 1] = start_game(argv[i]);
    }
    for (int i = 1; i < argc; i++) {
        game_result_t result;
        if (finish_game(argv[i], runs[i - 1], &result) != 0) {
            failed = 1;
            continue;
        }
        printf("test_save_worst_case: %s: %d records, largest %d bytes at f%d\n", argv[i], result.records, result.size, result.frame);
        records += result.records;
        if (result.size > largest.size) {
            largest = result;
            largest_game = argv[i];
        }
    }
    if (failed) {
        return 1;
    }
    if (largest_game == NULL) {
        printf("test_save_worst_case: no frame of a running game, nothing measured\n");
        return 1;
    }
    printf("test_save_worst_case: %d records round-trip; largest %d bytes, %s f%d, %d bytes left under %d\n", records, largest.size,
           largest_game, largest.frame, SAVE_STATE_MAX_BYTES - largest.size, SAVE_STATE_MAX_BYTES);
    return 0;
}
