#define _POSIX_C_SOURCE 200809L

#include "game/pad_nav.h"

#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "game/cart.h"
#include "game/economy_score.h"
#include "game/input_step.h"
#include "game/help.h"
#include "game/layout.h"
#include "game/main_economy.h"
#include "game/menu.h"
#include "game/message_box.h"
#include "game/planet.h"
#include "game/score_panel.h"
#include "game/score_panel_display.h"
#include "game/scroll.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "game/wormhole.h"
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

static int tagged_count(int tag) {
    const entity_list_t *tagged = entities_by_tag(tag);
    return tagged != NULL ? tagged->count : 0;
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

/* The × that starts the game is not a press edge in free mode: it places no probe. */
static void test_cross_held_across_the_click_leaves_nothing_held(void) {
    input_step_t step;
    start_title(&step);
    for (int i = 0; i < 2 * SETTLE_FRAMES; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == (i == 0 ? P8_MOUSE_LEFT : 0));
    }
    assert(strcmp(economy_score_difficulty()->name, "forgiving") == 0);
    assert(pad_nav_free_mode());
    assert(ui_left_held(main_ui()) == NULL);
    run_frame(&step, 0);
    assert(ui_left_held(main_ui()) == NULL);
    assert(tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0);
}

static void test_free_mode_holds_the_cursor_at_the_centre(void) {
    input_step_t step;
    start_title(&step);
    assert(!pad_nav_free_mode());
    run_frame(&step, P8_PAD_CROSS);
    assert(!pad_nav_panel_open());
    assert(pad_nav_free_mode());
    for (int i = 0; i < 24; i++) {
        run_frame(&step, (i * 0x2a5) & 2047);
        const p8_input_frame_t *played = input_step_played_frame(&step);
        assert(played->mouse_x == P8_SCREEN_WIDTH / 2 && played->mouse_y == P8_SCREEN_HEIGHT / 2);
        assert(played->mouse_buttons == 0 || ((i * 0x2a5) & P8_PAD_CROSS));
        assert(pad_nav_focus(&step.pad_nav) == NULL);
    }
}

enum { ITEMS_CAP = 32, PATH_CAP = 32 };

typedef struct {
    entity_t *item;
    int length;
    int moves[PATH_CAP];
} reached_t;

/* Breadth first over pad_nav_neighbor from the first item; returns how many items were reached. */
static int reach_all(reached_t reached[ITEMS_CAP]) {
    static const int directions[4] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT };
    entity_t *first = pad_nav_first_item();
    int count = 0;
    if (first == NULL) {
        return 0;
    }
    reached[count++] = (reached_t){ first, 0, { 0 } };
    for (int head = 0; head < count; head++) {
        for (int d = 0; d < 4; d++) {
            entity_t *next = pad_nav_neighbor(reached[head].item, directions[d]);
            int seen = next == NULL;
            for (int i = 0; i < count && !seen; i++) {
                seen = reached[i].item == next;
            }
            if (seen) {
                continue;
            }
            assert(count < ITEMS_CAP && reached[head].length < PATH_CAP);
            reached[count] = reached[head];
            reached[count].item = next;
            reached[count].moves[reached[count].length++] = directions[d];
            count++;
        }
    }
    return count;
}

/* Every expected item is reached from the first, and each, once focused through the input step, is the hovered entity. */
static void assert_reachable_and_hovered(input_step_t *step, entity_t *const expected[], int expected_count) {
    reached_t reached[ITEMS_CAP];
    int count = reach_all(reached);
    assert(expected_count >= 2);
    assert(count == expected_count);
    for (int i = 0; i < expected_count; i++) {
        int found = 0;
        for (int j = 0; j < count; j++) {
            found |= reached[j].item == expected[i];
        }
        assert(found);
    }
    for (int i = 0; i < count; i++) {
        pad_nav_init(&step->pad_nav);
        run_frame(step, 0);
        for (int m = 0; m < reached[i].length; m++) {
            tap(step, reached[i].moves[m]);
        }
        assert(pad_nav_focus(&step->pad_nav) == reached[i].item);
        assert(ui_hovered(main_ui()) == reached[i].item);
    }
}

/* Returns how many entries the open box holds. */
static int box_entries(entity_t *entries[ITEMS_CAP]) {
    const entity_id_list_t *ephemerals = &main_ui()->ephemerals;
    assert(ephemerals->count <= ITEMS_CAP);
    for (int i = 0; i < ephemerals->count; i++) {
        entries[i] = entity_id_list_at(ephemerals, i);
        assert(entries[i] != NULL && entries[i]->class == button_class());
    }
    return ephemerals->count;
}

static void settle(input_step_t *step) {
    for (int i = 0; i < SETTLE_FRAMES; i++) {
        run_frame(step, 0);
    }
}

static void start_game(input_step_t *step) {
    start_title(step);
    cart_start_game((button_t *)title_button("forgiving"));
    settle(step);
    assert(main_economy_typed() != NULL && !pad_nav_panel_open());
}

/* Returns 1 in a child that copies the current world, 0 in the parent once that child ended cleanly;
 * the child ends with end_forked_copy. */
static int in_forked_copy(void) {
    int status;
    fflush(stdout);
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        return 1;
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    return 0;
}

static void end_forked_copy(void) {
    fflush(stdout);
    _exit(0);
}

static int absolute_int(int value) {
    return value < 0 ? -value : value;
}

static vec_t camera_at(void) {
    return camera_pos_before_shake(main_camera_typed());
}

static void assert_camera_still(void) {
    const camera_t *camera = main_camera_typed();
    assert(camera->velocity.x == 0 && camera->velocity.y == 0);
}

/* Holds the direction for the given frames, then releases it; returns the camera's move along its axis. */
static int hold_and_measure(input_step_t *step, int direction, int frames) {
    vec_t from = camera_at();
    for (int i = 0; i < frames; i++) {
        run_frame(step, direction);
        assert_camera_still();
        const p8_input_frame_t *played = input_step_played_frame(step);
        assert(played->mouse_x == P8_SCREEN_WIDTH / 2 && played->mouse_y == P8_SCREEN_HEIGHT / 2);
    }
    run_frame(step, 0);
    vec_t to = camera_at();
    vec_t moved = vec_sub(to, from);
    if (direction == P8_PAD_LEFT || direction == P8_PAD_RIGHT) {
        assert(moved.y == 0);
        return p8_fixed_flr_int(moved.x);
    }
    assert(moved.x == 0);
    return p8_fixed_flr_int(moved.y);
}

/* 10 × 1 + 30 × 2 + 10 × 4: the camera's start is far enough from every pad bound. */
static void test_free_mode_pans_with_the_speed_curve(void) {
    static const int directions[4] = { P8_PAD_RIGHT, P8_PAD_LEFT, P8_PAD_DOWN, P8_PAD_UP };
    static const int signs[4] = { 1, -1, 1, -1 };
    static const int frames[4] = { 1, 10, 11, 50 };
    static const int pixels[4] = { 1, 10, 12, 110 };
    input_step_t step;
    start_game(&step);
    for (int d = 0; d < 4; d++) {
        for (int f = 0; f < 4; f++) {
            if (in_forked_copy()) {
                assert(hold_and_measure(&step, directions[d], frames[f]) == signs[d] * pixels[f]);
                end_forked_copy();
            }
        }
    }
}

static void hold(input_step_t *step, int pad_buttons, int frames) {
    for (int i = 0; i < frames; i++) {
        run_frame(step, pad_buttons);
        assert_camera_still();
    }
}

static void assert_moved(vec_t from, int dx, int dy) {
    vec_t moved = vec_sub(camera_at(), from);
    assert(moved.x == p8_fixed_from_int(dx) && moved.y == p8_fixed_from_int(dy));
}

static void test_free_mode_diagonal_cancel_and_reset(void) {
    input_step_t step;
    start_game(&step);
    vec_t from = camera_at();
    hold(&step, P8_PAD_UP | P8_PAD_LEFT, 12);
    assert_moved(from, -14, -14);

    hold(&step, 0, 1);
    from = camera_at();
    hold(&step, P8_PAD_LEFT | P8_PAD_RIGHT, 50);
    assert_moved(from, 0, 0);

    from = camera_at();
    hold(&step, 0, 1);
    hold(&step, P8_PAD_RIGHT, 1);
    assert_moved(from, 1, 0);

    hold(&step, 0, 1);
    from = camera_at();
    hold(&step, P8_PAD_RIGHT, 19);
    hold(&step, P8_PAD_RIGHT | P8_PAD_DOWN, 1);
    assert_moved(from, 10 + 2 * 10, 1);
}

/* Holds the direction until the camera stops; returns where it stopped. */
static vec_t pan_to_bound(input_step_t *step, int direction) {
    vec_t last = camera_at();
    for (int i = 0; i < 200; i++) {
        run_frame(step, direction);
        assert_camera_still();
        vec_t now = camera_at();
        if (now.x == last.x && now.y == last.y) {
            return now;
        }
        last = now;
    }
    assert(0 && "pan_to_bound: the camera never stopped");
    return last;
}

/* The centre reaches 16 to 368 on both axes, the galaxy's planet area: x -64..288, y -44..308. */
static void test_free_mode_bound_brings_every_planet_under_the_centre(void) {
    input_step_t step;
    start_game(&step);
    vec_t high = vec_make(pan_to_bound(&step, P8_PAD_RIGHT).x, pan_to_bound(&step, P8_PAD_DOWN).y);
    vec_t low = vec_make(pan_to_bound(&step, P8_PAD_LEFT).x, pan_to_bound(&step, P8_PAD_UP).y);
    assert(low.x == p8_fixed_from_int(-64) && high.x == p8_fixed_from_int(288));
    assert(low.y == p8_fixed_from_int(-44) && high.y == p8_fixed_from_int(308));
    vec_t centre = vec_make(p8_fixed_from_int(P8_SCREEN_WIDTH / 2), p8_fixed_from_int(P8_SCREEN_HEIGHT / 2));
    vec_t reach_low = vec_add(low, centre);
    vec_t reach_high = vec_add(high, centre);
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    assert(planets->count > 0);
    for (int i = 0; i < planets->count; i++) {
        vec_t p = entity_pos(planets->items[i]);
        assert(p.x >= reach_low.x && p.x <= reach_high.x && p.y >= reach_low.y && p.y <= reach_high.y);
    }
}

static void test_free_mode_other_buttons_do_nothing(void) {
    static const int buttons[6] = { P8_PAD_CIRCLE, P8_PAD_SQUARE, P8_PAD_TRIANGLE, P8_PAD_L1, P8_PAD_R1, P8_PAD_START };
    input_step_t step;
    start_game(&step);
    vec_t from = camera_at();
    int live = live_entities()->count;
    int boxes = tagged_count(TAG_MESSAGE_BOX);
    for (int b = 0; b < 6; b++) {
        for (int i = 0; i < 3; i++) {
            run_frame(&step, i == 1 ? 0 : buttons[b]);
            const p8_input_frame_t *played = input_step_played_frame(&step);
            assert(played->mouse_x == P8_SCREEN_WIDTH / 2 && played->mouse_y == P8_SCREEN_HEIGHT / 2);
            assert(played->mouse_buttons == 0);
            assert(pad_nav_free_mode());
        }
    }
    assert_moved(from, 0, 0);
    assert_camera_still();
    assert(live_entities()->count == live);
    assert(tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0);
    assert(tagged_count(TAG_MESSAGE_BOX) == boxes && main_ui()->ephemerals.count == 0);
}

/* The world point under the centre cursor, with the camera as the next frame's hit test sees it. */
static vec_t centre_world(void) {
    vec_t centre = vec_make(p8_fixed_from_int(P8_SCREEN_WIDTH / 2), p8_fixed_from_int(P8_SCREEN_HEIGHT / 2));
    return vec_add(centre, entity_pos(&main_camera_typed()->header));
}

static int planet_near_centre(int pixels) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    vec_t centre = centre_world();
    for (int i = 0; i < planets->count; i++) {
        vec_t gap = vec_sub(entity_pos(planets->items[i]), centre);
        int dx = absolute_int(p8_fixed_flr_int(gap.x));
        int dy = absolute_int(p8_fixed_flr_int(gap.y));
        if (dx <= pixels && dy <= pixels) {
            return 1;
        }
    }
    return 0;
}

/* Pans right, then down, until no planet lies within 24 px of the centre on both axes. */
static void pan_to_empty_space(input_step_t *step) {
    for (int i = 0; i < 400 && planet_near_centre(24); i++) {
        tap(step, i % 2 ? P8_PAD_RIGHT : P8_PAD_DOWN);
    }
    assert(!planet_near_centre(24));
}

static void test_free_mode_first_cross_places_the_probe(void) {
    input_step_t step;
    start_game(&step);
    pan_to_empty_space(&step);
    tap(&step, P8_PAD_CROSS);
    assert(tagged_count(TAG_PROBE) == 1 && tagged_count(TAG_WORMHOLE) == 0);
    assert(pad_nav_free_mode());
    const p8_input_frame_t *played = input_step_played_frame(&step);
    assert(played->mouse_x == P8_SCREEN_WIDTH / 2 && played->mouse_y == P8_SCREEN_HEIGHT / 2);
    assert(played->mouse_buttons == 0);
}

static void test_free_mode_short_cross_hold_abandons_the_fold(void) {
    input_step_t step;
    start_game(&step);
    pan_to_empty_space(&step);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    p8_fixed_t credits = main_economy_typed()->credits;
    for (int i = 0; i < 10; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
        entity_t *held = ui_left_held(main_ui());
        assert(held != NULL && held->class == wormhole_class());
    }
    assert(tagged_count(TAG_WORMHOLE) == 1);
    run_frame(&step, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    settle(&step);
    assert(tagged_count(TAG_WORMHOLE) == 0 && tagged_count(TAG_PROBE) == 1);
    assert(main_economy_typed()->credits == credits);
    assert(pad_nav_free_mode());
}

/* The test puts the planet under the centre through the pan operation, as pad navigation itself moves the camera. */
static void test_free_mode_cross_on_a_planet_presses_nothing(void) {
    input_step_t step;
    start_game(&step);
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    for (int p = 0; p < planets->count; p += 7) {
        if (in_forked_copy()) {
            camera_t *camera = main_camera_typed();
            vec_t gap = vec_sub(entity_pos(planets->items[p]), centre_world());
            camera_pan(camera, p8_fixed_flr_int(gap.x), p8_fixed_flr_int(gap.y));
            run_frame(&step, 0);
            int live = live_entities()->count;
            for (int i = 0; i < 20; i++) {
                run_frame(&step, P8_PAD_CROSS);
                assert(input_step_played_frame(&step)->mouse_buttons == 0);
            }
            run_frame(&step, 0);
            assert(live_entities()->count == live);
            assert(tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0);
            assert(ui_left_held(main_ui()) == NULL && main_ui()->ephemerals.count == 0);
            assert(pad_nav_free_mode());
            end_forked_copy();
        }
    }
}

static void test_free_mode_pans_while_cross_is_held(void) {
    input_step_t step;
    start_game(&step);
    pan_to_empty_space(&step);
    tap(&step, P8_PAD_CROSS);
    run_frame(&step, P8_PAD_CROSS);
    vec_t from = camera_at();
    for (int i = 0; i < 12; i++) {
        run_frame(&step, P8_PAD_CROSS | P8_PAD_LEFT);
        assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
        assert_camera_still();
    }
    assert_moved(from, -14, 0);
    entity_t *held = ui_left_held(main_ui());
    assert(held != NULL && held->class == wormhole_class());
}

/* × engaged when the panel opens, then held again through the panel's close: back in free mode it is no press edge. */
static void test_free_mode_cross_engagement_ends_under_a_panel(void) {
    input_step_t step;
    start_game(&step);
    pan_to_empty_space(&step);
    tap(&step, P8_PAD_CROSS);
    hold(&step, P8_PAD_CROSS, 2);
    galaxy_menu_on_click(4)(NULL);
    run_frame(&step, 0);
    settle(&step);
    assert(pad_nav_panel_open());
    for (int i = 0; i < 3; i++) {
        tap(&step, P8_PAD_UP | P8_PAD_RIGHT);
    }
    int frames = 0;
    do {
        run_frame(&step, P8_PAD_CROSS);
    } while (pad_nav_panel_open() && ++frames < 100);
    assert(pad_nav_free_mode());
    for (int i = 0; i < 5; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
}

/* A held direction pans nothing under a panel, and its speed starts over once free mode resumes. */
static void test_panel_takes_precedence_then_the_cursor_returns_to_the_centre(void) {
    entity_t *entries[ITEMS_CAP];
    input_step_t step;
    start_game(&step);
    hold(&step, P8_PAD_RIGHT, 20);
    galaxy_menu_on_click(4)(NULL);
    vec_t from = camera_at();
    hold(&step, P8_PAD_RIGHT, SETTLE_FRAMES);
    assert(pad_nav_panel_open() && !pad_nav_free_mode());
    assert_moved(from, 0, 0);
    assert(box_entries(entries) == 4);
    const p8_input_frame_t *played = input_step_played_frame(&step);
    assert(played->mouse_x != P8_SCREEN_WIDTH / 2 || played->mouse_y != P8_SCREEN_HEIGHT / 2);
    assert(pad_nav_focus(&step.pad_nav) == pad_nav_first_item() && ui_hovered(main_ui()) == pad_nav_first_item());

    for (int i = 0; i < 3; i++) {
        tap(&step, P8_PAD_UP | P8_PAD_RIGHT);
        run_frame(&step, P8_PAD_RIGHT);
    }
    run_frame(&step, P8_PAD_CROSS | P8_PAD_RIGHT);
    while (pad_nav_panel_open()) {
        run_frame(&step, P8_PAD_RIGHT);
    }
    assert(pad_nav_free_mode());
    from = camera_at();
    run_frame(&step, P8_PAD_RIGHT);
    played = input_step_played_frame(&step);
    assert(played->mouse_x == P8_SCREEN_WIDTH / 2 && played->mouse_y == P8_SCREEN_HEIGHT / 2);
    assert_moved(from, 1, 0);
}

static void test_pause_menu_is_reachable(void) {
    input_step_t step;
    entity_t *entries[ITEMS_CAP];
    start_game(&step);
    galaxy_menu_on_click(4)(NULL);
    settle(&step);
    assert(pad_nav_panel_open());
    int count = box_entries(entries);
    assert(count == 4);
    assert_reachable_and_hovered(&step, entries, count);
}

static int has_text(const entity_t *e, const char *text) {
    const layout_row_t *row = ((const button_t *)e)->layout_row;
    for (int i = 0; row != NULL && i < row->placed_items_count; i++) {
        if (row->placed_items[i].kind == LAYOUT_ROW_ITEM_TEXT && strcmp(row->placed_items[i].text, text) == 0) {
            return 1;
        }
    }
    return 0;
}

/* NULL when no live entity has that class. */
static entity_t *live_of_class(const entity_class_t *class) {
    const entity_list_t *live = live_entities();
    for (int i = 0; i < live->count; i++) {
        if (live->items[i]->class == class) {
            return live->items[i];
        }
    }
    return NULL;
}

static int message_box_count(void) {
    const entity_list_t *boxes = entities_by_tag(TAG_MESSAGE_BOX);
    return boxes != NULL ? boxes->count : 0;
}

/* No help panel exists: "help" is the title screen, and the pause menu's "how to play?" opens nothing. */
static void test_help_is_the_title_screen(void) {
    input_step_t step;
    entity_t *rows[4];
    start_title(&step);
    entity_t *help = live_of_class(help_screen_class());
    assert(help != NULL && !pad_nav_is_item(help));
    assert(pad_nav_panel_open());
    rows[0] = title_button("forgiving");
    rows[1] = title_button("reasonable");
    rows[2] = title_button("challenging");
    rows[3] = title_button("tough");
    assert_reachable_and_hovered(&step, rows, 4);
}

static void test_how_to_play_is_reachable_and_opens_nothing(void) {
    input_step_t step;
    entity_t *entries[ITEMS_CAP];
    start_game(&step);
    galaxy_menu_on_click(4)(NULL);
    settle(&step);
    assert(box_entries(entries) == 4);
    entity_t *how_to_play = entries[3];
    assert(has_text(how_to_play, "how to play?"));
    for (int i = 0; i < 3; i++) {
        tap(&step, P8_PAD_UP);
    }
    assert(pad_nav_focus(&step.pad_nav) == how_to_play && ui_hovered(main_ui()) == how_to_play);
    int boxes = message_box_count();
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(!pad_nav_panel_open());
    assert(main_ui()->ephemerals.count == 0);
    assert(message_box_count() == boxes);
}

/* Some techs affordable, some too dear: rows without an action stay items. */
static void test_tech_panel_is_reachable(void) {
    input_step_t step;
    entity_t *entries[ITEMS_CAP];
    start_game(&step);
    main_economy_typed()->research_points = p8_fixed_from_int(5);
    galaxy_menu_on_click(2)(NULL);
    settle(&step);
    assert(pad_nav_panel_open());
    int count = box_entries(entries);
    int without_action = 0;
    for (int i = 0; i < count; i++) {
        without_action += ((const button_t *)entries[i])->on_click == NULL;
    }
    assert(without_action > 0 && without_action < count);
    assert_reachable_and_hovered(&step, entries, count);
}

/* During play the score panel is only its screen-wide backdrop: open, with no item. */
static void test_score_panel_during_play_has_no_item(void) {
    input_step_t step;
    start_game(&step);
    galaxy_menu_on_click(3)(NULL);
    settle(&step);
    assert(pad_nav_panel_open());
    assert(pad_nav_first_item() == NULL);
    const entity_list_t *live = live_entities();
    for (int i = 0; i < live->count; i++) {
        assert(!pad_nav_is_item(live->items[i]));
    }
    run_frame(&step, P8_PAD_DOWN);
    assert(pad_nav_focus(&step.pad_nav) == NULL);
}

static void end_game(input_step_t *step) {
    start_game(step);
    main_economy_typed()->end_status = "final score";
    main_economy_typed()->has_end_status = 1;
    entity_t *panel = &main_score_panel()->header;
    panel->class->states[ENTITY_STATE_NORMAL](panel, p8_fixed_from_int(0));
    settle(step);
}

/* Returns how many layer-15 buttons with a row are live: "again?" and back at the game's end. */
static int end_buttons(entity_t *buttons[ITEMS_CAP]) {
    const entity_list_t *live = live_entities();
    int count = 0;
    for (int i = 0; i < live->count; i++) {
        entity_t *e = live->items[i];
        if (e->class == button_class() && e->layer == p8_fixed_from_int(15) && ((const button_t *)e)->layout_row != NULL) {
            assert(count < ITEMS_CAP);
            buttons[count++] = e;
        }
    }
    return count;
}

static void test_score_panel_at_game_end_is_reachable(void) {
    input_step_t step;
    entity_t *buttons[ITEMS_CAP];
    end_game(&step);
    assert(pad_nav_panel_open());
    int count = end_buttons(buttons);
    assert(count == 2);
    assert_reachable_and_hovered(&step, buttons, count);
}

static void test_score_panel_at_game_end_left_and_right(void) {
    input_step_t step;
    entity_t *buttons[ITEMS_CAP];
    end_game(&step);
    assert(end_buttons(buttons) == 2);
    entity_t *again = has_text(buttons[0], " again? ") ? buttons[0] : buttons[1];
    entity_t *back = again == buttons[0] ? buttons[1] : buttons[0];
    assert(has_text(again, " again? "));
    assert(pad_nav_neighbor(again, P8_PAD_LEFT) == back && pad_nav_neighbor(back, P8_PAD_RIGHT) == again);
    assert(pad_nav_neighbor(again, P8_PAD_RIGHT) == NULL && pad_nav_neighbor(back, P8_PAD_LEFT) == NULL);
    assert(pad_nav_neighbor(again, P8_PAD_UP) == NULL && pad_nav_neighbor(again, P8_PAD_DOWN) == NULL);
}

static int distance_to(vec_t screen, int x, int y) {
    int dx = p8_fixed_flr_int(screen.x) - x;
    int dy = p8_fixed_flr_int(screen.y) - y;
    return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
}

/* The menu is world space (layer 9): the camera is moved off its start first, so a screen point that ignored it would miss. */
static void test_planet_menu_is_reachable(void) {
    input_step_t step;
    entity_t *entries[ITEMS_CAP];
    start_game(&step);
    camera_t *camera = main_camera_typed();
    vec_t start = entity_pos(&camera->header);
    entity_set_map_pos(&camera->header, vec_add(camera_pos_before_shake(camera), vec_make(p8_fixed_from_int(32), p8_fixed_from_int(24))));
    camera_set_velocity(camera, vec_make(0, 0));
    run_frame(&step, 0);
    vec_t moved = entity_pos(&camera->header);
    assert(moved.x != start.x && moved.y != start.y);

    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    planet_t *chosen = NULL;
    for (int i = 0; i < planets->count; i++) {
        planet_t *planet = (planet_t *)planets->items[i];
        if (chosen == NULL || distance_to(vec_sub(entity_pos(&planet->header), moved), 32, 32) <
                                  distance_to(vec_sub(entity_pos(&chosen->header), moved), 32, 32)) {
            chosen = planet;
        }
    }
    planet_roll_type(chosen);
    (void)planet_left_press(chosen);
    settle(&step);
    assert(pad_nav_panel_open());
    int count = box_entries(entries);
    for (int i = 0; i < count; i++) {
        assert(p8_fixed_flr_int(entries[i]->layer) < 10);
    }
    assert_reachable_and_hovered(&step, entries, count);
}

/* The year box has no button and is not modal: it opens no panel and changes no box's items. */
static void test_year_message_box_adds_and_removes_nothing(void) {
    input_step_t step;
    entity_t *entries[ITEMS_CAP];
    start_game(&step);
    int boxes = message_box_count();
    score_panel_new_year(p8_fixed_from_int(3421));
    assert(message_box_count() == boxes + 1);
    entity_t *year_box = &shown_message_box()->header;
    run_frame(&step, 0);
    assert(!pad_nav_panel_open() && pad_nav_first_item() == NULL && !pad_nav_is_item(year_box));

    galaxy_menu_on_click(4)(NULL);
    settle(&step);
    assert(entity_list_has(live_entities(), year_box));
    int count = box_entries(entries);
    assert(count == 4 && !pad_nav_is_item(year_box));
    assert_reachable_and_hovered(&step, entries, count);
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
    in_fresh_child(test_free_mode_holds_the_cursor_at_the_centre);
    in_fresh_child(test_free_mode_pans_with_the_speed_curve);
    in_fresh_child(test_free_mode_diagonal_cancel_and_reset);
    in_fresh_child(test_free_mode_bound_brings_every_planet_under_the_centre);
    in_fresh_child(test_free_mode_other_buttons_do_nothing);
    in_fresh_child(test_free_mode_first_cross_places_the_probe);
    in_fresh_child(test_free_mode_short_cross_hold_abandons_the_fold);
    in_fresh_child(test_free_mode_cross_on_a_planet_presses_nothing);
    in_fresh_child(test_free_mode_pans_while_cross_is_held);
    in_fresh_child(test_free_mode_cross_engagement_ends_under_a_panel);
    in_fresh_child(test_panel_takes_precedence_then_the_cursor_returns_to_the_centre);
    in_fresh_child(test_pause_menu_is_reachable);
    in_fresh_child(test_planet_menu_is_reachable);
    in_fresh_child(test_tech_panel_is_reachable);
    in_fresh_child(test_score_panel_during_play_has_no_item);
    in_fresh_child(test_score_panel_at_game_end_is_reachable);
    in_fresh_child(test_score_panel_at_game_end_left_and_right);
    in_fresh_child(test_year_message_box_adds_and_removes_nothing);
    in_fresh_child(test_help_is_the_title_screen);
    in_fresh_child(test_how_to_play_is_reachable_and_opens_nothing);
    printf("test_pad_nav: ok\n");
    return 0;
}
