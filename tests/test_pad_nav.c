#define _POSIX_C_SOURCE 200809L

#include "game/pad_nav.h"

#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "game/cart.h"
#include "game/economy_score.h"
#include "game/input_step.h"
#include "game/main_economy.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "p8/assets.h"
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

enum { SETTLE_FRAMES = 20 };

/* NULL when no live button has that name. */
static entity_t *title_button(const char *name) {
    const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
    for (int i = 0; i < bucket->count; i++) {
        entity_t *e = bucket->items[i];
        if (e->class == button_class() && ((const button_t *)e)->name != NULL && strcmp(((const button_t *)e)->name, name) == 0) {
            return e;
        }
    }
    return NULL;
}

static void run_frame(input_step_t *step, int pad_buttons) {
    p8_input_frame_t raw = { 0, 0, 0, { 0, 0, 0, 0, 0, 0 }, pad_buttons };
    input_step_run(step, &raw);
    cart_update60();
}

/* Press on one frame, release on the next: the focus moves on the press. */
static void tap(input_step_t *step, int pad_buttons) {
    run_frame(step, pad_buttons);
    run_frame(step, 0);
}

static void start_title(input_step_t *step) {
    cart_init();
    input_step_init(step, P8_INPUT_PAD, 0);
    for (int i = 0; i < SETTLE_FRAMES; i++) {
        run_frame(step, 0);
    }
}

static void assert_focused_and_hovered(const input_step_t *step, const char *name) {
    entity_t *button = title_button(name);
    assert(button != NULL);
    assert(pad_nav_focus(&step->pad_nav) == button);
    assert(ui_hovered(main_ui()) == button);
}

static void test_title_focuses_the_first_difficulty(void) {
    input_step_t step;
    start_title(&step);
    assert(pad_nav_panel_open());
    assert(pad_nav_first_item() == title_button("forgiving"));
    assert_focused_and_hovered(&step, "forgiving");
}

static void test_title_down_and_up_move_between_rows(void) {
    input_step_t step;
    start_title(&step);
    tap(&step, P8_PAD_DOWN);
    assert_focused_and_hovered(&step, "reasonable");
    tap(&step, P8_PAD_DOWN);
    assert_focused_and_hovered(&step, "challenging");
    tap(&step, P8_PAD_UP);
    assert_focused_and_hovered(&step, "reasonable");
}

static void test_title_up_on_top_and_down_on_bottom_do_nothing(void) {
    input_step_t step;
    start_title(&step);
    tap(&step, P8_PAD_UP);
    assert_focused_and_hovered(&step, "forgiving");
    for (int i = 0; i < 3; i++) {
        tap(&step, P8_PAD_DOWN);
    }
    assert_focused_and_hovered(&step, "tough");
    tap(&step, P8_PAD_DOWN);
    assert_focused_and_hovered(&step, "tough");
}

static void test_title_left_and_right_do_nothing(void) {
    input_step_t step;
    start_title(&step);
    tap(&step, P8_PAD_DOWN);
    tap(&step, P8_PAD_LEFT);
    assert_focused_and_hovered(&step, "reasonable");
    tap(&step, P8_PAD_RIGHT);
    assert_focused_and_hovered(&step, "reasonable");
    assert(pad_nav_neighbor(title_button("tough"), P8_PAD_LEFT) == NULL);
    assert(pad_nav_neighbor(title_button("forgiving"), P8_PAD_RIGHT) == NULL);
}

static void test_title_held_direction_moves_once(void) {
    input_step_t step;
    start_title(&step);
    for (int i = 0; i < 10; i++) {
        run_frame(&step, P8_PAD_DOWN);
    }
    assert_focused_and_hovered(&step, "reasonable");
}

static void test_title_cross_starts_the_focused_difficulty(void) {
    input_step_t step;
    start_title(&step);
    tap(&step, P8_PAD_DOWN);
    assert(main_economy_typed() == NULL);
    run_frame(&step, P8_PAD_CROSS);
    assert(main_economy_typed() != NULL);
    assert(strcmp(economy_score_difficulty()->name, "reasonable") == 0);
}

/* A move and a × on the same frame click the newly focused row. */
static void test_title_cross_with_a_move_clicks_the_new_focus(void) {
    input_step_t step;
    start_title(&step);
    run_frame(&step, P8_PAD_DOWN | P8_PAD_CROSS);
    assert(main_economy_typed() != NULL);
    assert(strcmp(economy_score_difficulty()->name, "reasonable") == 0);
}

static void test_cross_held_across_the_click_leaves_nothing_held(void) {
    input_step_t step;
    start_title(&step);
    for (int i = 0; i < 10; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == (i == 0 ? P8_MOUSE_LEFT : 0));
    }
    assert(strcmp(economy_score_difficulty()->name, "forgiving") == 0);
    assert(ui_left_held(main_ui()) == NULL);
    run_frame(&step, 0);
    assert(ui_left_held(main_ui()) == NULL);
}

static void test_no_panel_open_leaves_the_cursor_still(void) {
    input_step_t step;
    start_title(&step);
    run_frame(&step, P8_PAD_CROSS);
    assert(!pad_nav_panel_open());
    const p8_input_frame_t still = *input_step_played_frame(&step);
    for (int i = 0; i < 24; i++) {
        run_frame(&step, (i * 0x2a5) & 2047);
        const p8_input_frame_t *played = input_step_played_frame(&step);
        assert(played->mouse_x == still.mouse_x && played->mouse_y == still.mouse_y);
        assert(played->mouse_buttons == 0);
        assert(pad_nav_focus(&step.pad_nav) == NULL);
    }
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
    in_fresh_child(test_title_focuses_the_first_difficulty);
    in_fresh_child(test_title_down_and_up_move_between_rows);
    in_fresh_child(test_title_up_on_top_and_down_on_bottom_do_nothing);
    in_fresh_child(test_title_left_and_right_do_nothing);
    in_fresh_child(test_title_held_direction_moves_once);
    in_fresh_child(test_title_cross_starts_the_focused_difficulty);
    in_fresh_child(test_title_cross_with_a_move_clicks_the_new_focus);
    in_fresh_child(test_cross_held_across_the_click_leaves_nothing_held);
    in_fresh_child(test_no_panel_open_leaves_the_cursor_still);
    printf("test_pad_nav: ok\n");
    return 0;
}
