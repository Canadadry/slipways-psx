#define _POSIX_C_SOURCE 200809L

#include "game/input_step.h"

#include "devtools/replay_source.h"
#include "devtools/session_recorder.h"
#include "devtools/state.h"
#include "engine/dispatch_context.h"
#include "engine/entity_id.h"
#include "game/cart.h"
#include "game/main_economy.h"
#include "game/scroll.h"
#include "game/ui.h"
#include "p8/assets.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static p8_vram_t memory;
static p8_graphics_t graphics;

static void assert_frame_equals(const p8_input_frame_t *got, const p8_input_frame_t *want) {
    assert(got->mouse_x == want->mouse_x);
    assert(got->mouse_y == want->mouse_y);
    assert(got->mouse_buttons == want->mouse_buttons);
    assert(got->pad_buttons == want->pad_buttons);
}

/* The cart reads its input through the dispatch context: the played frame must be what it sees. */
static void assert_installed(const input_step_t *step) {
    const p8_input_t *installed = dispatch_context_input();
    const p8_input_frame_t *played = input_step_played_frame(step);
    p8_mouse_t mouse = p8_input_read_mouse(installed);
    assert(installed != NULL);
    assert(mouse.x == played->mouse_x && mouse.y == played->mouse_y && mouse.buttons == played->mouse_buttons);
}

#define NOISY_FRAME_COUNT 6
static const p8_input_frame_t noisy_frames[NOISY_FRAME_COUNT] = {
    { 10, 20, P8_MOUSE_LEFT, P8_PAD_CROSS },
    { 64, 64, 2, P8_PAD_UP | P8_PAD_LEFT },
    { 127, 0, 4, P8_PAD_DOWN | P8_PAD_RIGHT | P8_PAD_CIRCLE },
    { 0, 127, 7, 2047 },
    { 33, 44, 0, P8_PAD_START | P8_PAD_L1 | P8_PAD_R1 },
    { 5, 6, 0, 0 },
};

/* The synthetic mouse comes from pad navigation: two steps fed the same pad masks with different
 * raw mice must play the same frames. */
static void test_pad_keeps_only_the_pad_mask(void) {
    input_step_t step;
    input_step_t quiet_step;
    input_step_init(&step);
    input_step_init(&quiet_step);
    for (int i = 0; i < NOISY_FRAME_COUNT; i++) {
        p8_input_frame_t quiet = { 0, 0, 0, noisy_frames[i].pad_buttons };
        input_step_run(&quiet_step, &quiet);
        p8_input_frame_t want = *input_step_played_frame(&quiet_step);
        input_step_run(&step, &noisy_frames[i]);
        const p8_input_frame_t *played = input_step_played_frame(&step);
        assert(played->pad_buttons == noisy_frames[i].pad_buttons);
        assert_frame_equals(played, &want);
        assert_installed(&step);
    }
}

#define CHORD (P8_PAD_CROSS | P8_PAD_CIRCLE)

/* Each chord followed by a release frame; returns a bitmask of the chords on which the step reported the cheat. */
static int type_cheat(input_step_t *step) {
    static const p8_input_frame_t release = { 0, 0, 0, 0 };
    int fired_on = 0;
    for (int i = 0; i < CHEAT_CHORDS; i++) {
        p8_input_frame_t frame = { 0, 0, 0, CHORD };
        if (input_step_run(step, &frame)) {
            fired_on |= 1 << i;
        }
        assert(!input_step_run(step, &release));
    }
    return fired_on;
}

static int economy_cheated(void) {
    const economy_t *economy = main_economy_typed();
    return economy->credits == p8_fixed_from_int(200) && economy->research_points == p8_fixed_from_int(99);
}

static void test_pad_cheat_fires_on_cross_and_circle(void) {
    input_step_t step;
    cart_start_game(NULL);
    input_step_init(&step);
    assert(!economy_cheated());
    assert(type_cheat(&step) == 1 << (CHEAT_CHORDS - 1));
    assert(economy_cheated());
}

/* Noisy live frames with the cheat chords typed in the middle: the recording keeps only the
 * pad mask, so the replayed session must play and cheat exactly as the live one did. */
static void test_live_and_replayed_pad_frames_play_alike(void) {
    const char *path = "tmp/test_input_step_live.json";
    enum { LIVE_FRAME_COUNT = 40 };
    p8_input_frame_t live[LIVE_FRAME_COUNT];
    for (int i = 0; i < LIVE_FRAME_COUNT; i++) {
        p8_input_frame_t frame = noisy_frames[i % NOISY_FRAME_COUNT];
        frame.mouse_x = (frame.mouse_x + 7 * i) % 128;
        if (i >= 10 && i < 10 + 2 * CHEAT_CHORDS) {
            frame.pad_buttons = (i % 2 == 0) ? CHORD | P8_PAD_UP : P8_PAD_LEFT;
        } else if (i < 10) {
            frame.pad_buttons &= ~(P8_PAD_CROSS | P8_PAD_CIRCLE);
        }
        live[i] = frame;
    }
    cart_start_game(NULL);

    input_step_t live_step;
    session_recorder_t recorder;
    int live_fired[LIVE_FRAME_COUNT];
    p8_input_frame_t live_played[LIVE_FRAME_COUNT];
    int cheat_count = 0;
    input_step_init(&live_step);
    session_recorder_open(&recorder, path);
    for (int i = 0; i < LIVE_FRAME_COUNT; i++) {
        assert(session_recorder_record_frame(&recorder, &live[i]) == 1);
        live_fired[i] = input_step_run(&live_step, &live[i]);
        live_played[i] = *input_step_played_frame(&live_step);
        cheat_count += live_fired[i];
    }
    session_recorder_close(&recorder);
    assert(cheat_count == 1);

    replay_source_t replay;
    input_step_t replay_step;
    assert(replay_source_open(&replay, path) == REPLAY_SOURCE_LOADED);
    input_step_init(&replay_step);
    for (int i = 0; i < LIVE_FRAME_COUNT; i++) {
        p8_input_frame_t raw;
        assert(replay_source_next_frame(&replay, &raw) == 1);
        assert(input_step_run(&replay_step, &raw) == live_fired[i]);
        assert_frame_equals(input_step_played_frame(&replay_step), &live_played[i]);
    }
    replay_source_close(&replay);
}

enum { HEADLESS_FRAME_COUNT = 120 };

static p8_input_frame_t noisy_pad_session_frame(int i) {
    int first_half = i < HEADLESS_FRAME_COUNT / 2;
    p8_input_frame_t frame = { (i * 5) % 128, (i * 3) % 128, (i / 4) % 8, 0 };
    frame.pad_buttons = first_half ? P8_PAD_UP | P8_PAD_LEFT : P8_PAD_DOWN | P8_PAD_RIGHT;
    return frame;
}

static void write_snapshot(const char *path) {
    FILE *out = fopen(path, "w");
    const camera_t *camera = main_camera_typed();
    vec_t position = camera_pos_before_shake(camera);
    const entity_t *hovered = ui_hovered(main_ui());
    assert(out != NULL);
    fprintf(out, "camera %ld %ld\n", (long)position.x, (long)position.y);
    if (hovered == NULL) {
        fputs("hovered none\n", out);
    } else {
        entity_id_t id = entity_id_of(hovered);
        fprintf(out, "hovered %u %u\n", id.index, id.generation);
    }
    state_write(out);
    assert(fclose(out) == 0);
}

static void run_pad_session(const char *snapshot_path, int noisy) {
    input_step_t step;
    input_step_init(&step);
    for (int i = 0; i < HEADLESS_FRAME_COUNT; i++) {
        p8_input_frame_t raw = noisy_pad_session_frame(i);
        if (!noisy) {
            p8_input_frame_t quiet = { 0, 0, 0, raw.pad_buttons };
            raw = quiet;
        }
        input_step_run(&step, &raw);
        cart_update60();
    }
    /* One draw, not one per frame: drawing is most of the cost, and make test must stay fast. */
    cart_draw();
    write_snapshot(snapshot_path);
}

static void assert_files_identical(const char *a_path, const char *b_path) {
    FILE *a = fopen(a_path, "r");
    FILE *b = fopen(b_path, "r");
    int ca;
    int cb;
    assert(a != NULL && b != NULL);
    do {
        ca = fgetc(a);
        cb = fgetc(b);
        assert(ca == cb && "headless pad run: mouse noise changed the game");
    } while (ca != EOF);
    fclose(a);
    fclose(b);
}

/* D8: forked after the start, since the runtime's random state has no seed setter. */
static void test_pad_session_ignores_mouse_noise(void) {
    static const char *const paths[2] = { "tmp/test_input_step_quiet.txt", "tmp/test_input_step_noisy.txt" };
    pid_t children[2];
    cart_start_game(NULL);
    fflush(stdout);
    for (int noisy = 0; noisy < 2; noisy++) {
        children[noisy] = fork();
        assert(children[noisy] >= 0);
        if (children[noisy] == 0) {
            run_pad_session(paths[noisy], noisy);
            fflush(stdout);
            _exit(0);
        }
    }
    for (int noisy = 0; noisy < 2; noisy++) {
        int status;
        assert(waitpid(children[noisy], &status, 0) == children[noisy]);
        assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }
    assert_files_identical(paths[0], paths[1]);
}

static void in_fresh_child(void (*test)(void)) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        test();
        fflush(stdout);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

int main(void) {

    p8_vram_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    cart_load();
    cart_init();
    in_fresh_child(test_pad_keeps_only_the_pad_mask);
    in_fresh_child(test_pad_cheat_fires_on_cross_and_circle);
    in_fresh_child(test_live_and_replayed_pad_frames_play_alike);
    in_fresh_child(test_pad_session_ignores_mouse_noise);
    printf("test_input_step: ok\n");
    return 0;
}
