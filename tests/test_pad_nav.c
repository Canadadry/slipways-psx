#define _POSIX_C_SOURCE 200809L

#include "game/pad_nav.h"

#include "engine/dispatch_context.h"
#include "engine/entity_id.h"
#include "engine/entity_registry.h"
#include "engine/geom.h"
#include "engine/pointer_list.h"
#include "game/cart.h"
#include "game/economy.h"
#include "game/economy_score.h"
#include "game/game_data.h"
#include "game/galaxy.h"
#include "game/input_step.h"
#include "game/help.h"
#include "game/layout.h"
#include "game/main_economy.h"
#include "game/menu.h"
#include "game/message_box.h"
#include "game/pad_panel.h"
#include "game/planet.h"
#include "game/score_panel.h"
#include "game/score_panel_display.h"
#include "game/research.h"
#include "game/save_state.h"
#include "game/save_storage.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "game/structure_icon.h"
#include "game/targeting.h"
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
    p8_input_frame_t raw = { 0, 0, 0, pad_buttons };
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
    input_step_init(step);
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

static save_state_t g_saved;

/* A reasonable game saved at its first end turn, then the title again: g_saved holds the record. */
static void start_title_with_a_save(input_step_t *step) {
    static uint8_t bytes[SAVE_STATE_MAX_BYTES];
    int count;

    start_title(step);
    cart_start_game((button_t *)title_button("reasonable"));
    run_frame(step, 0);
    economy_new_year(main_economy_typed());
    run_frame(step, 0);
    count = save_storage_read(bytes, (int)sizeof bytes);
    assert(count > 0 && save_state_decode(bytes, count, &g_saved));
    start_title(step);
    assert(title_button("resume") != NULL);
}

static void test_title_with_a_save_still_focuses_forgiving(void) {
    input_step_t step;
    start_title_with_a_save(&step);
    assert(pad_nav_first_item() == title_button("forgiving"));
    assert_focused_and_hovered(&step, "forgiving");
}

static void test_title_up_reaches_resume_and_down_comes_back(void) {
    input_step_t step;
    start_title_with_a_save(&step);
    tap(&step, P8_PAD_UP);
    assert_focused_and_hovered(&step, "resume");
    tap(&step, P8_PAD_UP);
    assert_focused_and_hovered(&step, "resume");
    tap(&step, P8_PAD_DOWN);
    assert_focused_and_hovered(&step, "forgiving");
}

static void test_title_cross_on_resume_resumes_the_saved_game(void) {
    input_step_t step;
    start_title_with_a_save(&step);
    tap(&step, P8_PAD_UP);
    assert(main_economy_typed() == NULL);
    run_frame(&step, P8_PAD_CROSS);
    assert(main_economy_typed() != NULL);
    assert(strcmp(economy_score_difficulty()->name, difficulty_options_table[g_saved.difficulty - 1].name) == 0);
    assert(main_economy_typed()->year == g_saved.economy.year);
    assert(entities_by_tag(TAG_NODE)->count == g_saved.node_count);
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
    assert(pad_nav_free_mode(&step.pad_nav));
    assert(ui_left_held(main_ui()) == NULL);
    run_frame(&step, 0);
    assert(ui_left_held(main_ui()) == NULL);
    assert(tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0);
}

static void test_free_mode_holds_the_cursor_at_the_centre(void) {
    input_step_t step;
    start_title(&step);
    assert(!pad_nav_free_mode(&step.pad_nav));
    run_frame(&step, P8_PAD_CROSS);
    assert(!pad_nav_panel_open());
    assert(pad_nav_free_mode(&step.pad_nav));
    for (int i = 0; i < 24; i++) {
        int pad = (i * 0x2a5) & 2047 & ~(P8_PAD_L1 | P8_PAD_R1 | P8_PAD_START);
        run_frame(&step, pad);
        const p8_input_frame_t *played = input_step_played_frame(&step);
        assert(played->mouse_x == P8_SCREEN_WIDTH / 2 && played->mouse_y == P8_SCREEN_HEIGHT / 2);
        assert(played->mouse_buttons == 0 || (pad & P8_PAD_CROSS));
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

/* Holds the direction for the given frames, then releases it; returns the camera's move along its axis. */
static int hold_and_measure(input_step_t *step, int direction, int frames) {
    vec_t from = camera_at();
    for (int i = 0; i < frames; i++) {
        run_frame(step, direction);
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
    static const int buttons[2] = { P8_PAD_CIRCLE, P8_PAD_SQUARE };
    input_step_t step;
    start_game(&step);
    vec_t from = camera_at();
    int live = live_entities()->count;
    int boxes = tagged_count(TAG_MESSAGE_BOX);
    for (int b = 0; b < 2; b++) {
        for (int i = 0; i < 3; i++) {
            run_frame(&step, i == 1 ? 0 : buttons[b]);
            const p8_input_frame_t *played = input_step_played_frame(&step);
            assert(played->mouse_x == P8_SCREEN_WIDTH / 2 && played->mouse_y == P8_SCREEN_HEIGHT / 2);
            assert(played->mouse_buttons == 0);
            assert(pad_nav_free_mode(&step.pad_nav));
        }
    }
    assert_moved(from, 0, 0);
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
    assert(pad_nav_free_mode(&step.pad_nav));
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
    assert(pad_nav_free_mode(&step.pad_nav));
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
            assert(pad_nav_free_mode(&step.pad_nav));
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
    assert(pad_nav_free_mode(&step.pad_nav));
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
    assert(pad_nav_panel_open() && !pad_nav_free_mode(&step.pad_nav));
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
    assert(pad_nav_free_mode(&step.pad_nav));
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

/* NULL when no planet-family entity is live. */
static planet_t *planet_closest_to_centre(void) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    vec_t centre = centre_world();
    planet_t *chosen = NULL;
    int best = 0;
    for (int i = 0; i < planets->count; i++) {
        vec_t gap = vec_sub(entity_pos(planets->items[i]), centre);
        int distance = absolute_int(p8_fixed_flr_int(gap.x)) + absolute_int(p8_fixed_flr_int(gap.y));
        if (chosen == NULL || distance < best) {
            chosen = (planet_t *)planets->items[i];
            best = distance;
        }
    }
    return chosen;
}

/* Keeps held_buttons down across the frame it runs. */
static void pan_planet_under_centre(input_step_t *step, const planet_t *planet, int held_buttons) {
    vec_t gap = vec_sub(entity_pos(&planet->header), centre_world());
    camera_pan(main_camera_typed(), p8_fixed_flr_int(gap.x), p8_fixed_flr_int(gap.y));
    run_frame(step, held_buttons);
    gap = vec_sub(entity_pos(&planet->header), centre_world());
    assert(absolute_int(p8_fixed_flr_int(gap.x)) <= 1 && absolute_int(p8_fixed_flr_int(gap.y)) <= 1);
}

/* Rolls the planet's type until it lands known, as a fold reveals it. */
static planet_t *known_planet_closest_to_centre(void) {
    planet_t *planet = planet_closest_to_centre();
    assert(planet != NULL);
    for (int i = 0; i < 64 && planet->header.state != ENTITY_STATE_KNOWN; i++) {
        planet_roll_type(planet);
    }
    assert(planet->header.state == ENTITY_STATE_KNOWN);
    return planet;
}

static planet_t *known_planet_under_centre(input_step_t *step) {
    planet_t *planet = known_planet_closest_to_centre();
    pan_planet_under_centre(step, planet, 0);
    return planet;
}

static void assert_cursor_on(const input_step_t *step, const entity_t *e) {
    vec_t at = vec_sub(entity_pos(e), entity_pos(&main_camera_typed()->header));
    const p8_input_frame_t *played = input_step_played_frame(step);
    assert(played->mouse_x == p8_fixed_flr_int(at.x) && played->mouse_y == p8_fixed_flr_int(at.y));
}

static void assert_cursor_at_centre(const input_step_t *step) {
    const p8_input_frame_t *played = input_step_played_frame(step);
    assert(played->mouse_x == P8_SCREEN_WIDTH / 2 && played->mouse_y == P8_SCREEN_HEIGHT / 2);
}

static void assert_network_on(const input_step_t *step, const planet_t *planet) {
    assert(pad_nav_selection(&step->pad_nav) == &planet->header);
    assert(!pad_nav_free_mode(&step->pad_nav));
}

/* △ opens the menu, then ○ closes it: network mode on a known planet, nothing charged. */
static planet_t *network_on_known_planet(input_step_t *step) {
    planet_t *planet = known_planet_under_centre(step);
    tap(step, P8_PAD_TRIANGLE);
    assert(main_ui()->ephemerals.count > 0);
    tap(step, P8_PAD_CIRCLE);
    assert(main_ui()->ephemerals.count == 0);
    return planet;
}

static int is_button_of(const entity_t *e, const planet_t *planet) {
    return e->class == button_class() && entity_get(((const button_t *)e)->project_planet) == &planet->header;
}

static void test_triangle_on_a_known_planet_opens_its_menu(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = known_planet_under_centre(&step);
    (void)planet_left_press(planet);
    int rows = main_ui()->ephemerals.count;
    assert(rows > 0);
    ui_clear_ephemerals(main_ui());

    run_frame(&step, P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    assert(main_ui()->ephemerals.count == rows);
    run_frame(&step, P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert(pad_nav_focus(&step.pad_nav) == pad_nav_first_item() && pad_nav_first_item() != NULL);
    run_frame(&step, 0);
    assert(ui_hovered(main_ui()) == pad_nav_first_item());
    assert(pad_nav_selection(&step.pad_nav) == &planet->header);
    assert(!pad_nav_free_mode(&step.pad_nav));
}

static void assert_triangle_does_nothing(input_step_t *step) {
    int live = live_entities()->count;
    for (int i = 0; i < 2; i++) {
        run_frame(step, i == 0 ? P8_PAD_TRIANGLE : 0);
        assert(input_step_played_frame(step)->mouse_buttons == 0);
        assert_cursor_at_centre(step);
    }
    settle(step);
    assert(live_entities()->count == live);
    assert(tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0);
    assert(main_ui()->ephemerals.count == 0 && ui_left_held(main_ui()) == NULL);
    assert(pad_nav_free_mode(&step->pad_nav) && pad_nav_selection(&step->pad_nav) == NULL);
}

static void test_triangle_on_empty_space_or_an_unexplored_planet_does_nothing(void) {
    input_step_t step;
    start_game(&step);
    if (in_forked_copy()) {
        pan_to_empty_space(&step);
        assert_triangle_does_nothing(&step);
        end_forked_copy();
    }
    planet_t *planet = planet_closest_to_centre();
    assert(planet->header.state != ENTITY_STATE_KNOWN && planet->header.state != ENTITY_STATE_SETTLED);
    pan_planet_under_centre(&step, planet, 0);
    assert_triangle_does_nothing(&step);
}

/* The colony is made through its menu row's own click, as the mouse makes it; the pad stays in free mode. */
static void test_triangle_on_a_settled_planet_opens_its_inspection_panel(void) {
    entity_t *rows[ITEMS_CAP];
    input_step_t step;
    start_game(&step);
    planet_t *planet = known_planet_closest_to_centre();
    (void)planet_left_press(planet);
    int count = box_entries(rows);
    for (int i = 0; i < count && planet->header.state == ENTITY_STATE_KNOWN; i++) {
        button_t *row = (button_t *)rows[i];
        if (row->on_click != NULL) {
            row->on_click(row);
        }
    }
    ui_clear_ephemerals(main_ui());
    settle(&step);
    assert(planet->header.state == ENTITY_STATE_SETTLED);
    pan_planet_under_centre(&step, planet, 0);
    assert(pad_nav_free_mode(&step.pad_nav));
    p8_fixed_t credits = main_economy_typed()->credits;
    int slipways = tagged_count(TAG_SLIPWAY);

    run_frame(&step, P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    run_frame(&step, P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert(main_ui()->ephemerals.count > 0);
    settle(&step);
    assert(tagged_count(TAG_SLIPWAY) == slipways && ui_left_held(main_ui()) == NULL);
    assert(main_economy_typed()->credits == credits);
    assert_network_on(&step, planet);
}

static void test_colonising_through_the_menu_lands_network_mode_on_the_planet(void) {
    entity_t *rows[ITEMS_CAP];
    input_step_t step;
    start_game(&step);
    planet_t *planet = known_planet_under_centre(&step);
    p8_fixed_t credits = main_economy_typed()->credits;
    tap(&step, P8_PAD_TRIANGLE);
    settle(&step);
    int count = box_entries(rows);
    assert(count >= 3 && ((button_t *)rows[0])->on_click == NULL && is_button_of(rows[1], planet));
    assert(pad_nav_focus(&step.pad_nav) == rows[0]);
    tap(&step, P8_PAD_DOWN);
    tap(&step, P8_PAD_DOWN);
    assert(pad_nav_focus(&step.pad_nav) == rows[2]);
    tap(&step, P8_PAD_UP);
    assert(pad_nav_focus(&step.pad_nav) == rows[1] && ui_hovered(main_ui()) == rows[1]);
    p8_fixed_t cost = ((button_t *)rows[1])->project_cost;
    assert(cost > 0);

    for (int i = 0; i < 4 && main_ui()->ephemerals.count > 0; i++) {
        run_frame(&step, P8_PAD_CROSS);
    }
    assert(main_ui()->ephemerals.count == 0);
    assert(planet->header.state == ENTITY_STATE_SETTLED);
    assert(main_economy_typed()->credits == p8_fixed_sub(credits, cost));
    for (int i = 0; i < 3; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        assert_cursor_on(&step, &planet->header);
        assert(ui_hovered(main_ui()) == &planet->header);
        assert_network_on(&step, planet);
    }
    assert(main_ui()->ephemerals.count == 0 && ui_left_held(main_ui()) == NULL);
    assert(tagged_count(TAG_SLIPWAY) == 0);
    camera_pan(main_camera_typed(), 13, -9);
    run_frame(&step, 0);
    assert_cursor_on(&step, &planet->header);
    assert(ui_hovered(main_ui()) == &planet->header);
}

static void test_circle_closes_the_menu_and_stays_on_the_planet(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = known_planet_under_centre(&step);
    p8_fixed_t credits = main_economy_typed()->credits;
    tap(&step, P8_PAD_TRIANGLE);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(main_ui()->ephemerals.count == 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert_cursor_on(&step, &planet->header);
    run_frame(&step, 0);
    settle(&step);
    assert(planet->header.state == ENTITY_STATE_KNOWN);
    assert(main_economy_typed()->credits == credits);
    assert_network_on(&step, planet);
    assert(ui_hovered(main_ui()) == &planet->header);
}

static void test_network_mode_ignores_cross_directions_and_the_unbound_buttons(void) {
    static const int buttons[5] = { P8_PAD_CROSS, P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT };
    input_step_t step;
    start_game(&step);
    planet_t *planet = network_on_known_planet(&step);
    vec_t from = camera_at();
    int live = live_entities()->count;
    p8_fixed_t credits = main_economy_typed()->credits;
    for (int b = 0; b < 5; b++) {
        for (int i = 0; i < 12; i++) {
            run_frame(&step, i < 10 ? buttons[b] : 0);
            assert(input_step_played_frame(&step)->mouse_buttons == 0);
            assert_cursor_on(&step, &planet->header);
        }
    }
    assert_moved(from, 0, 0);
    assert(live_entities()->count == live && main_economy_typed()->credits == credits);
    assert(main_ui()->ephemerals.count == 0 && ui_left_held(main_ui()) == NULL);
    assert(planet->header.state == ENTITY_STATE_KNOWN);
    assert_network_on(&step, planet);
}

/* × held across the △: the click still releases on the next frame. */
static void test_triangle_click_releases_even_with_cross_held(void) {
    input_step_t step;
    start_game(&step);
    (void)network_on_known_planet(&step);
    run_frame(&step, P8_PAD_CROSS);
    run_frame(&step, P8_PAD_CROSS | P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    run_frame(&step, P8_PAD_CROSS);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
}

static void test_triangle_in_network_mode_reopens_the_menu(void) {
    entity_t *rows[ITEMS_CAP];
    input_step_t step;
    start_game(&step);
    planet_t *planet = network_on_known_planet(&step);
    tap(&step, P8_PAD_TRIANGLE);
    int count = box_entries(rows);
    assert(count > 0 && is_button_of(rows[count - 1], planet));
    assert_network_on(&step, planet);
}

static void test_circle_on_the_title_does_nothing(void) {
    input_step_t step;
    start_title(&step);
    tap(&step, P8_PAD_DOWN);
    int items = main_ui()->ephemerals.count;
    tap(&step, P8_PAD_CIRCLE);
    settle(&step);
    assert(main_ui()->ephemerals.count == items && pad_nav_panel_open());
    assert(main_economy_typed() == NULL);
    assert_focused_and_hovered(&step, "reasonable");
}

static void test_circle_closes_the_pause_menu(void) {
    input_step_t step;
    start_game(&step);
    int live = live_entities()->count;
    galaxy_menu_on_click(4)(NULL);
    settle(&step);
    assert(pause_menu_shown());
    tap(&step, P8_PAD_CIRCLE);
    settle(&step);
    assert(!pause_menu_shown() && main_ui()->ephemerals.count == 0 && !pad_nav_panel_open());
    assert(live_entities()->count == live && ui_left_held(main_ui()) == NULL);
    assert(tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0);
    assert(pad_nav_free_mode(&step.pad_nav));
    assert_cursor_at_centre(&step);
}

static void test_triangle_while_cross_holds_a_fold_does_nothing(void) {
    input_step_t step;
    start_game(&step);
    pan_to_empty_space(&step);
    tap(&step, P8_PAD_CROSS);
    hold(&step, P8_PAD_CROSS, 3);
    entity_t *held = ui_left_held(main_ui());
    assert(held != NULL && held->class == wormhole_class());
    planet_t *planet = known_planet_closest_to_centre();
    pan_planet_under_centre(&step, planet, P8_PAD_CROSS);
    for (int i = 0; i < 4; i++) {
        run_frame(&step, P8_PAD_CROSS | (i % 2 ? 0 : P8_PAD_TRIANGLE));
        assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
        assert_cursor_at_centre(&step);
    }
    assert(ui_left_held(main_ui()) == held && main_ui()->ephemerals.count == 0);
    assert(pad_nav_free_mode(&step.pad_nav) && pad_nav_selection(&step.pad_nav) == NULL);
}

static void test_a_dead_selection_falls_back_to_free_mode(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = network_on_known_planet(&step);
    vec_t from = camera_at();
    planet->header.is_dead = 1;
    assert(pad_nav_selection(&step.pad_nav) == NULL);
    run_frame(&step, 0);
    assert(pad_nav_free_mode(&step.pad_nav));
    assert_cursor_at_centre(&step);
    assert_moved(from, 0, 0);
}

static void test_a_new_game_starts_in_free_mode(void) {
    input_step_t step;
    start_game(&step);
    (void)network_on_known_planet(&step);
    cart_init();
    for (int i = 0; i < SETTLE_FRAMES; i++) {
        cart_update60();
    }
    cart_start_game((button_t *)title_button("forgiving"));
    run_frame(&step, 0);
    assert(main_economy_typed() != NULL && !pad_nav_panel_open());
    assert(pad_nav_free_mode(&step.pad_nav) && pad_nav_selection(&step.pad_nav) == NULL);
    assert_cursor_at_centre(&step);
}

/* The planet bobs by a fraction of a pixel, so its floored point may sit one pixel off the centre. */
static void assert_centred_on(const entity_t *e) {
    vec_t at = entity_pos(e);
    vec_t camera = camera_at();
    int dx = p8_fixed_flr_int(at.x) - p8_fixed_flr_int(camera.x) - P8_SCREEN_WIDTH / 2;
    int dy = p8_fixed_flr_int(at.y) - p8_fixed_flr_int(camera.y) - P8_SCREEN_HEIGHT / 2;
    assert(absolute_int(dx) <= 1 && absolute_int(dy) <= 1);
}

static void assert_free_from(const input_step_t *step, const planet_t *start) {
    assert(pad_nav_free_mode(&step->pad_nav));
    assert(pad_nav_starting_planet(&step->pad_nav) == &start->header);
    assert(pad_nav_selection(&step->pad_nav) == NULL);
}

static void test_square_in_network_mode_enters_free_mode_on_the_planet(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = network_on_known_planet(&step);
    camera_pan(main_camera_typed(), 13, -9);
    run_frame(&step, 0);
    assert(pad_nav_starting_planet(&step.pad_nav) == NULL);
    run_frame(&step, P8_PAD_SQUARE);
    assert_free_from(&step, planet);
    assert_cursor_at_centre(&step);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert_centred_on(&planet->header);
    assert(ui_pad_selection(main_ui()) == NULL);
    run_frame(&step, 0);
    assert_free_from(&step, planet);
    assert(ui_hovered(main_ui()) == &planet->header);
}

/* From a known starting planet no drag holds: □ still forgets the planet, and a second □ changes nothing. */
static void test_square_in_free_mode_forgets_the_starting_planet_then_changes_nothing(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = network_on_known_planet(&step);
    tap(&step, P8_PAD_SQUARE);
    assert_free_from(&step, planet);
    hold(&step, P8_PAD_RIGHT, 30);
    run_frame(&step, 0);
    vec_t from = camera_at();
    for (int i = 0; i < 6; i++) {
        run_frame(&step, i % 2 ? 0 : P8_PAD_SQUARE);
        assert_cursor_at_centre(&step);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        assert(pad_nav_free_mode(&step.pad_nav) && pad_nav_starting_planet(&step.pad_nav) == NULL);
        assert(pad_nav_selection(&step.pad_nav) == NULL);
    }
    assert_moved(from, 0, 0);
}

/* Clicks the first row that colonises in place: a row whose project names a class replaces the planet. */
static int click_colonise_row(void) {
    entity_t *rows[ITEMS_CAP];
    int count = box_entries(rows);
    for (int i = 0; i < count; i++) {
        button_t *row = (button_t *)rows[i];
        const research_project_t *project = research_project_get(row->project);
        if (row->on_click != NULL && (project == NULL || project->class_name == NULL)) {
            row->on_click(row);
            return 1;
        }
    }
    return 0;
}

/* Rolls the planet's type until its menu can colonise it, then colonises it through that row's own click, as the mouse does. */
static void settle_planet(input_step_t *step, planet_t *planet) {
    for (int i = 0; i < 64 && planet->header.state != ENTITY_STATE_SETTLED; i++) {
        planet_roll_type(planet);
        if (planet->header.state == ENTITY_STATE_KNOWN) {
            (void)planet_left_press(planet);
            (void)click_colonise_row();
            ui_clear_ephemerals(main_ui());
            run_frame(step, 0);
        }
    }
    assert(!planet->header.is_dead && planet->header.state == ENTITY_STATE_SETTLED);
}

typedef int (*pair_accept_t)(int dx, int dy, p8_fixed_t squared);

/* The two planets' pixel gap and squared map distance pass accept, and no other planet lies near the segment between them. */
static void find_pair(pair_accept_t accept, planet_t **start, planet_t **target) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    for (int a = 0; a < planets->count; a++) {
        for (int b = 0; b < planets->count; b++) {
            entity_t *from = planets->items[a];
            entity_t *to = planets->items[b];
            vec_t gap = vec_sub(entity_pos(to), entity_pos(from));
            p8_fixed_t squared = vec_length_squared(vec_sub(entity_map_pos(to), entity_map_pos(from)));
            if (a == b || !accept(p8_fixed_flr_int(gap.x), p8_fixed_flr_int(gap.y), squared)) {
                continue;
            }
            int clear = 1;
            for (int c = 0; c < planets->count && clear; c++) {
                clear = c == a || c == b ||
                        !geom_near_segment(entity_map_pos(from), entity_map_pos(to), entity_map_pos(planets->items[c]),
                                           P8_FIXED_LIT(0x0000028f, 0.01));
            }
            if (clear) {
                *start = (planet_t *)from;
                *target = (planet_t *)to;
                return;
            }
        }
    }
    assert(0 && "find_pair: no pair of planets passes");
}

/* The start stays on screen, clear of the HUD's edges, while the target is under the centre. */
static int near_pair(int dx, int dy, p8_fixed_t squared) {
    return squared < P8_FIXED_LIT(0x00006666, 0.4) && absolute_int(dx) <= 50 && absolute_int(dy) <= 30 &&
           absolute_int(dx) + absolute_int(dy) >= 20;
}

/* The start is off screen, below or above it, while the target is under the centre. */
static int off_screen_pair(int dx, int dy, p8_fixed_t squared) {
    (void)dx;
    return squared < P8_FIXED_LIT(0x00007333, 0.45) && absolute_int(dy) >= 66;
}

/* Beyond the squared 0.5 slipway reach the game has before "space folding". */
static int too_far_pair(int dx, int dy, p8_fixed_t squared) {
    (void)dx;
    (void)dy;
    return squared > P8_FIXED_LIT(0x00009999, 0.6) && squared < P8_FIXED_LIT(0x0000e666, 0.9);
}

typedef struct {
    p8_fixed_t credits;
    p8_fixed_t year_ticks;
    p8_fixed_t year;
    int slipways;
    int probes;
    int wormholes;
} world_count_t;

/* A slipway still dragged is in the placing state: only built ones count. */
static int built_slipways(void) {
    const entity_list_t *slipways = entities_by_tag(TAG_SLIPWAY);
    int count = 0;
    for (int i = 0; slipways != NULL && i < slipways->count; i++) {
        count += !slipways->items[i]->is_dead && slipways->items[i]->state != ENTITY_STATE_PLACING;
    }
    return count;
}

static world_count_t world_count(void) {
    const economy_t *economy = main_economy_typed();
    world_count_t count = { economy->credits, economy->year_ticks, economy->year, built_slipways(),
                            tagged_count(TAG_PROBE), tagged_count(TAG_WORMHOLE) };
    return count;
}

/* A placing slipway the ui no longer holds would stay drawn: a cancel must drop it. */
static int slipways_left_placing(void) {
    const entity_list_t *slipways = entities_by_tag(TAG_SLIPWAY);
    int count = 0;
    for (int i = 0; slipways != NULL && i < slipways->count; i++) {
        entity_t *e = slipways->items[i];
        count += !e->is_dead && e->state == ENTITY_STATE_PLACING && e != ui_left_held(main_ui());
    }
    return count;
}

static void assert_world_unchanged(world_count_t before) {
    assert(slipways_left_placing() == 0);
    world_count_t now = world_count();
    assert(now.credits == before.credits && now.year_ticks == before.year_ticks && now.year == before.year);
    assert(now.slipways == before.slipways && now.probes == before.probes && now.wormholes == before.wormholes);
}

/* Both planets settled, network mode on the start through △ and ○, then □: free mode from the start. */
static void free_from_settled(input_step_t *step, planet_t *start, planet_t *target, int settle_target) {
    settle_planet(step, start);
    if (settle_target) {
        settle_planet(step, target);
    }
    pan_planet_under_centre(step, start, 0);
    tap(step, P8_PAD_TRIANGLE);
    assert(main_ui()->ephemerals.count > 0);
    tap(step, P8_PAD_CIRCLE);
    assert(main_ui()->ephemerals.count == 0);
    assert_network_on(step, start);
    tap(step, P8_PAD_SQUARE);
    assert_free_from(step, start);
}

/* The ui holds the slipway pressed out of start, and the pad holds the left button at the centre. */
static void assert_dragging(const input_step_t *step, const planet_t *start) {
    entity_t *held = ui_left_held(main_ui());
    assert(held != NULL && held->class == slipway_class() && slipway_start((const slipway_t *)held) == &start->header);
    assert(input_step_played_frame(step)->mouse_buttons == P8_MOUSE_LEFT);
    assert_cursor_at_centre(step);
    assert_free_from(step, start);
}

static void test_square_on_a_colony_holds_its_slipway_at_the_centre(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    settle_planet(&step, start);
    pan_planet_under_centre(&step, start, 0);
    tap(&step, P8_PAD_TRIANGLE);
    tap(&step, P8_PAD_CIRCLE);
    assert_network_on(&step, start);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_SQUARE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    assert_cursor_on(&step, &start->header);
    for (int i = 0; i < 12; i++) {
        run_frame(&step, i < 2 ? 0 : P8_PAD_RIGHT);
        assert_dragging(&step, start);
    }
    entity_t *held = ui_left_held(main_ui());
    vec_t end = entity_pos(slipway_end((const slipway_t *)held));
    vec_t centre = centre_world();
    assert(absolute_int(p8_fixed_flr_int(end.x) - p8_fixed_flr_int(centre.x)) <= 2);
    assert(((const slipway_t *)held)->targeting.cost > 0);
    assert_world_unchanged(before);
    assert(main_ui()->ephemerals.count == 0);
}

static int off_screen(int x, int y) {
    return x < 0 || x >= P8_SCREEN_WIDTH || y < 0 || y >= P8_SCREEN_HEIGHT;
}

/* Target under the centre while the drag holds, then × lets the button go there; returns whether the start was off screen. */
static int drop_on(input_step_t *step, const planet_t *start, const planet_t *target) {
    pan_planet_under_centre(step, target, 0);
    assert_dragging(step, start);
    vec_t at = vec_sub(entity_pos(&start->header), camera_at());
    int start_off_screen = off_screen(p8_fixed_flr_int(at.x), p8_fixed_flr_int(at.y));
    run_frame(step, P8_PAD_CROSS);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    assert_cursor_at_centre(step);
    return start_off_screen;
}

/* NULL when no slipway joins the two planets. */
static entity_t *slipway_between(const entity_t *start, const entity_t *target) {
    entity_id_list_iter_t it = entity_id_list_all(&((const planet_t *)start)->slipways);
    entity_t *slipway;
    while (entity_id_list_iter_next(&it, &slipway)) {
        if (slipway_other_end((const slipway_t *)slipway, start) == target) {
            return slipway;
        }
    }
    return NULL;
}

/* One slipway joins the two planets, paid and clocked; the pad lands on the target with it selected. */
static void assert_built_and_landed(input_step_t *step, const planet_t *start, const planet_t *target, world_count_t before) {
    entity_t *slipway = slipway_between(&start->header, &target->header);
    assert(slipway != NULL);
    world_count_t now = world_count();
    assert(now.slipways == before.slipways + 1);
    assert(now.credits == p8_fixed_sub(before.credits, ((const slipway_t *)slipway)->targeting.cost));
    assert(((const slipway_t *)slipway)->targeting.cost > 0);
    assert(now.year_ticks != before.year_ticks || now.year != before.year);
    assert_network_on(step, target);
    assert(pad_nav_selected_slipway(&step->pad_nav) == slipway);
    assert(pad_nav_starting_planet(&step->pad_nav) == NULL);
    assert_centred_on(&target->header);
    assert(main_ui()->ephemerals.count == 0 && ui_left_held(main_ui()) == NULL);
}

static void test_cross_on_a_colony_builds_a_slipway_and_lands_on_it(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    world_count_t before = world_count();
    assert(!drop_on(&step, start, target));
    run_frame(&step, P8_PAD_CROSS);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert_built_and_landed(&step, start, target, before);
    run_frame(&step, 0);
    assert_cursor_on(&step, &target->header);
    assert(ui_pad_selection(main_ui()) == &target->header);
    assert(ui_left_held(main_ui()) == NULL);
}

static void test_cross_builds_from_an_off_screen_starting_planet(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(off_screen_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    world_count_t before = world_count();
    assert(drop_on(&step, start, target));
    run_frame(&step, 0);
    assert_built_and_landed(&step, start, target, before);
}

/* × on the target drops nothing of the world; the next frame presses the start again and the drag holds once more. */
static void assert_drop_refused(input_step_t *step, const planet_t *start, const planet_t *target) {
    world_count_t before = world_count();
    (void)drop_on(step, start, target);
    run_frame(step, P8_PAD_CROSS);
    assert(input_step_played_frame(step)->mouse_buttons == P8_MOUSE_LEFT);
    assert_cursor_on(step, &start->header);
    assert_free_from(step, start);
    for (int i = 0; i < 20; i++) {
        run_frame(step, P8_PAD_CROSS);
        assert_dragging(step, start);
    }
    settle(step);
    assert_dragging(step, start);
    assert_world_unchanged(before);
    assert(main_ui()->ephemerals.count == 0);
}

static void test_cross_on_a_known_planet_is_refused(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 0);
    for (int i = 0; i < 64 && target->header.state != ENTITY_STATE_KNOWN; i++) {
        planet_roll_type(target);
    }
    assert(target->header.state == ENTITY_STATE_KNOWN);
    assert_drop_refused(&step, start, target);
}

static void test_cross_on_a_planet_too_far_is_refused(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(too_far_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    assert_drop_refused(&step, start, target);
}

static void test_cross_without_the_credits_is_refused(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    main_economy_typed()->credits = p8_fixed_from_int(0);
    assert_drop_refused(&step, start, target);
}

static void test_cross_on_an_already_linked_planet_is_refused(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    (void)drop_on(&step, start, target);
    run_frame(&step, 0);
    assert_network_on(&step, target);
    tap(&step, P8_PAD_SQUARE);
    assert_dragging(&step, target);
    assert_drop_refused(&step, target, start);
}

/* × presses nothing: no button, no box, nothing held. */
static void assert_cross_presses_nothing(input_step_t *step) {
    world_count_t before = world_count();
    for (int i = 0; i < 3; i++) {
        run_frame(step, P8_PAD_CROSS);
        assert(input_step_played_frame(step)->mouse_buttons == 0);
        assert_cursor_at_centre(step);
    }
    run_frame(step, 0);
    settle(step);
    assert_world_unchanged(before);
    assert(main_ui()->ephemerals.count == 0 && ui_left_held(main_ui()) == NULL);
    assert(pad_nav_free_mode(&step->pad_nav));
}

static void test_cross_on_the_starting_planet_itself_keeps_the_drag(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 0);
    world_count_t before = world_count();
    for (int i = 0; i < 4; i++) {
        run_frame(&step, i % 2 ? 0 : P8_PAD_CROSS);
        assert_dragging(&step, start);
    }
    assert_world_unchanged(before);
    assert(main_ui()->ephemerals.count == 0);
}

static void test_cross_from_a_known_starting_planet_presses_nothing(void) {
    input_step_t step;
    start_game(&step);
    planet_t *start = network_on_known_planet(&step);
    run_frame(&step, P8_PAD_SQUARE);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    run_frame(&step, 0);
    assert_free_from(&step, start);
    assert(ui_left_held(main_ui()) == NULL);
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    planet_t *target = (planet_t *)planets->items[planets->items[0] == &start->header ? 1 : 0];
    pan_planet_under_centre(&step, target, 0);
    assert_cross_presses_nothing(&step);
    assert_free_from(&step, start);
}

/* Moves the icon so its hit box centre sits on the planet's screen point. */
static void cover_with_icon(entity_t *icon, const planet_t *planet) {
    vec_t at = vec_sub(entity_pos(&planet->header), entity_pos(&main_camera_typed()->header));
    vec_t middle = vec_make(p8_fixed_div(p8_fixed_add(icon->hit_box.left, icon->hit_box.right), p8_fixed_from_int(2)),
                            p8_fixed_div(p8_fixed_add(icon->hit_box.top, icon->hit_box.bottom), p8_fixed_from_int(2)));
    entity_set_pos(icon, vec_sub(vec_make(p8_fixed_flr(at.x), p8_fixed_flr(at.y)), middle));
}

/* □ centres the camera before its press, so a HUD icon over the planet's old screen point takes nothing. */
static void test_square_presses_a_starting_planet_the_hud_covered(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    tap(&step, P8_PAD_CIRCLE);
    assert_network_on(&step, start);
    entity_t *icon = live_of_class(structure_icon_class());
    assert(icon != NULL);
    int x, y;
    pad_panel_focus_point(icon, &x, &y);
    camera_pan(main_camera_typed(), P8_SCREEN_WIDTH / 2 - x, P8_SCREEN_HEIGHT / 2 - y);
    run_frame(&step, 0);
    assert_cursor_on(&step, &start->header);
    assert(pad_panel_entity_under(input_step_played_frame(&step)->mouse_x, input_step_played_frame(&step)->mouse_y,
                                  p8_fixed_flr_int(start->header.layer), pad_panel_is_candidate) != NULL);
    run_frame(&step, P8_PAD_SQUARE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    assert_cursor_on(&step, &start->header);
    run_frame(&step, 0);
    assert_dragging(&step, start);
}

/* In free mode the starting planet moves on screen with the camera: an icon over it holds the press back. */
static void test_no_drag_starts_again_while_the_hud_covers_the_starting_planet(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    pan_to_empty_space(&step);
    run_frame(&step, P8_PAD_CROSS);
    run_frame(&step, P8_PAD_CROSS);
    assert(ui_left_held(main_ui()) != NULL && ui_left_held(main_ui())->class == probe_class());
    entity_t *icon = live_of_class(structure_icon_class());
    assert(icon != NULL);
    vec_t home = entity_pos(icon);
    cover_with_icon(icon, start);
    for (int i = 0; i < 4; i++) {
        run_frame(&step, 0);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        assert(i == 0 || ui_left_held(main_ui()) == NULL);
    }
    assert_free_from(&step, start);
    entity_set_pos(icon, home);
    run_frame(&step, 0);
    run_frame(&step, 0);
    assert_dragging(&step, start);
}

static void test_cross_on_empty_space_drops_the_slipway_then_places_the_probe(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    pan_to_empty_space(&step);
    assert_dragging(&step, start);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_CROSS);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert_cursor_at_centre(&step);
    assert_world_unchanged(before);
    for (int i = 0; i < 3; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
        assert_cursor_at_centre(&step);
        entity_t *held = ui_left_held(main_ui());
        assert(held != NULL && held->class == probe_class());
    }
    run_frame(&step, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert(tagged_count(TAG_PROBE) == 1 && tagged_count(TAG_WORMHOLE) == 0);
    world_count_t now = world_count();
    assert(now.slipways == before.slipways);
    run_frame(&step, 0);
    run_frame(&step, 0);
    assert_dragging(&step, start);
}

/* A one-frame × on empty space still holds the probe's press for a frame. */
static void test_a_one_frame_cross_on_empty_space_places_the_probe(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    pan_to_empty_space(&step);
    run_frame(&step, P8_PAD_CROSS);
    run_frame(&step, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    run_frame(&step, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert(tagged_count(TAG_PROBE) == 1);
}

static void test_circle_drops_the_drag_and_returns_to_the_starting_planet_and_its_slipway(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    (void)drop_on(&step, start, target);
    run_frame(&step, 0);
    entity_t *slipway = pad_nav_selected_slipway(&step.pad_nav);
    assert(slipway != NULL);
    tap(&step, P8_PAD_SQUARE);
    assert_dragging(&step, target);
    assert(pad_nav_selected_slipway(&step.pad_nav) == NULL);
    hold(&step, P8_PAD_RIGHT, 60);
    hold(&step, P8_PAD_DOWN, 30);
    vec_t at = vec_sub(entity_pos(&target->header), camera_at());
    assert(off_screen(p8_fixed_flr_int(at.x), p8_fixed_flr_int(at.y)));
    assert_dragging(&step, target);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_CIRCLE);
    const p8_input_frame_t *played = input_step_played_frame(&step);
    assert(played->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
    assert_network_on(&step, target);
    assert(pad_nav_selected_slipway(&step.pad_nav) == slipway);
    assert_centred_on(&target->header);
    run_frame(&step, 0);
    assert_cursor_on(&step, &target->header);
    assert(ui_pad_selection(main_ui()) == &target->header);
    settle(&step);
    assert(ui_left_held(main_ui()) == NULL && main_ui()->ephemerals.count == 0);
    assert_world_unchanged(before);
}

static void test_circle_first_closes_a_box_then_returns(void) {
    input_step_t step;
    start_game(&step);
    planet_t *start = network_on_known_planet(&step);
    tap(&step, P8_PAD_SQUARE);
    (void)planet_left_press(start);
    assert(main_ui()->ephemerals.count > 0);
    tap(&step, P8_PAD_CIRCLE);
    assert(main_ui()->ephemerals.count == 0);
    assert_free_from(&step, start);
    tap(&step, P8_PAD_CIRCLE);
    assert_network_on(&step, start);
}

/* △ lets the drag go nowhere, then clicks the planet under the centre on the next frame. */
static void test_triangle_while_dragging_opens_the_menu_under_the_centre(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 0);
    for (int i = 0; i < 64 && target->header.state != ENTITY_STATE_KNOWN; i++) {
        planet_roll_type(target);
    }
    pan_planet_under_centre(&step, target, 0);
    assert_dragging(&step, start);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_TRIANGLE);
    const p8_input_frame_t *played = input_step_played_frame(&step);
    assert(played->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
    run_frame(&step, P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    assert_cursor_at_centre(&step);
    run_frame(&step, 0);
    assert(main_ui()->ephemerals.count > 0);
    assert_network_on(&step, target);
    assert(ui_left_held(main_ui()) == NULL);
    assert_world_unchanged(before);
    vec_t camera = camera_at();
    run_frame(&step, P8_PAD_CIRCLE);
    assert(main_ui()->ephemerals.count == 0);
    run_frame(&step, 0);
    run_frame(&step, 0);
    assert_dragging(&step, start);
    assert_moved(camera, 0, 0);
    assert_world_unchanged(before);
}

/* Colonising the planet △ opened gives the drag back from the starting planet, aimed at the new colony. */
static void test_colonising_from_a_drag_resumes_it_aimed_at_the_colony(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 0);
    main_economy_typed()->credits = p8_fixed_from_int(999);
    for (int i = 0; i < 64; i++) {
        planet_roll_type(target);
        if (target->header.state != ENTITY_STATE_KNOWN) {
            continue;
        }
        (void)planet_left_press(target);
        int colonises = 0;
        entity_t *rows[ITEMS_CAP];
        int count = box_entries(rows);
        for (int r = 0; r < count; r++) {
            const research_project_t *project = research_project_get(((button_t *)rows[r])->project);
            colonises |= ((button_t *)rows[r])->on_click != NULL && (project == NULL || project->class_name == NULL);
        }
        ui_clear_ephemerals(main_ui());
        run_frame(&step, 0);
        if (colonises) {
            break;
        }
    }
    run_frame(&step, 0);
    pan_planet_under_centre(&step, target, 0);
    assert_dragging(&step, start);
    tap(&step, P8_PAD_TRIANGLE);
    run_frame(&step, 0);
    assert(main_ui()->ephemerals.count > 0);
    assert(click_colonise_row());
    ui_clear_ephemerals(main_ui());
    assert(target->header.state == ENTITY_STATE_SETTLED);
    for (int i = 0; i < 3; i++) {
        run_frame(&step, 0);
    }
    assert_dragging(&step, start);
    entity_t *held = ui_left_held(main_ui());
    assert(slipway_end((const slipway_t *)held) == &target->header);
    run_frame(&step, P8_PAD_CROSS);
    run_frame(&step, 0);
    assert_network_on(&step, target);
    assert(slipway_between(&start->header, &target->header) != NULL);
}

static void test_a_dead_starting_planet_drops_the_drag_and_is_forgotten(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    pan_planet_under_centre(&step, target, 0);
    assert_dragging(&step, start);
    start->header.is_dead = 1;
    assert(pad_nav_starting_planet(&step.pad_nav) == NULL);
    run_frame(&step, 0);
    const p8_input_frame_t *played = input_step_played_frame(&step);
    assert(played->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
    for (int i = 0; i < 3; i++) {
        run_frame(&step, i % 2 ? 0 : P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        assert_cursor_at_centre(&step);
    }
    tap(&step, P8_PAD_CIRCLE);
    assert(pad_nav_free_mode(&step.pad_nav) && pad_nav_selection(&step.pad_nav) == NULL);
    assert(world_count().slipways == 0 && ui_left_held(main_ui()) == NULL && main_ui()->ephemerals.count == 0);
}

static void test_a_panel_opening_drops_the_drag(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 0);
    for (int i = 0; i < 64 && target->header.state != ENTITY_STATE_KNOWN; i++) {
        planet_roll_type(target);
    }
    pan_planet_under_centre(&step, target, 0);
    world_count_t before = world_count();
    (void)planet_left_press(target);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    run_frame(&step, 0);
    assert(ui_left_held(main_ui()) == NULL);
    tap(&step, P8_PAD_CIRCLE);
    assert(main_ui()->ephemerals.count == 0);
    run_frame(&step, 0);
    run_frame(&step, 0);
    assert_dragging(&step, start);
    assert_world_unchanged(before);
}

static void assert_plain_free_mode(const input_step_t *step) {
    assert(pad_nav_free_mode(&step->pad_nav));
    assert(pad_nav_starting_planet(&step->pad_nav) == NULL && pad_nav_selection(&step->pad_nav) == NULL);
    assert(ui_left_held(main_ui()) == NULL);
}

/* Free mode dragging from start, panned right for pan_frames, then □: the drag is dropped where the camera stands. */
static void plain_free_from_a_drag(input_step_t *step, planet_t **start, planet_t **target, int pan_frames) {
    find_pair(near_pair, start, target);
    free_from_settled(step, *start, *target, 1);
    hold(step, P8_PAD_RIGHT, pan_frames);
    run_frame(step, 0);
    assert_dragging(step, *start);
    vec_t from = camera_at();
    world_count_t before = world_count();
    run_frame(step, P8_PAD_SQUARE);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    assert_cursor_at_centre(step);
    assert_plain_free_mode(step);
    run_frame(step, 0);
    run_frame(step, 0);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    assert_plain_free_mode(step);
    assert_moved(from, 0, 0);
    assert_world_unchanged(before);
    assert(main_ui()->ephemerals.count == 0);
}

/* Over the starting planet a plain release would open its panel: the drag must be dropped, not let go. */
static void test_square_while_dragging_drops_the_drag_into_plain_free_mode(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    plain_free_from_a_drag(&step, &start, &target, 0);
    assert(main_ui()->pad_crosshair);
    settle(&step);
    assert_plain_free_mode(&step);
    assert(main_ui()->ephemerals.count == 0);
}

static void test_cross_in_plain_free_mode_places_the_probe(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    plain_free_from_a_drag(&step, &start, &target, 20);
    pan_to_empty_space(&step);
    world_count_t before = world_count();
    tap(&step, P8_PAD_CROSS);
    assert(tagged_count(TAG_PROBE) == before.probes + 1 && tagged_count(TAG_WORMHOLE) == 0);
    assert(world_count().slipways == before.slipways);
    assert_plain_free_mode(&step);
}

static void test_triangle_in_plain_free_mode_opens_the_menu_and_lands_on_the_planet(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    plain_free_from_a_drag(&step, &start, &target, 20);
    world_count_t before = world_count();
    pan_planet_under_centre(&step, target, 0);
    tap(&step, P8_PAD_TRIANGLE);
    assert(main_ui()->ephemerals.count > 0);
    assert_network_on(&step, target);
    tap(&step, P8_PAD_CIRCLE);
    assert(main_ui()->ephemerals.count == 0);
    settle(&step);
    assert_network_on(&step, target);
    assert(ui_left_held(main_ui()) == NULL);
    assert_world_unchanged(before);
}

/* Away from the map's edges, so the camera can centre every planet placed around it. */
static planet_t *most_isolated_planet(void) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    planet_t *chosen = NULL;
    p8_fixed_t best = 0;
    for (int i = 0; i < planets->count; i++) {
        entity_t *planet = planets->items[i];
        vec_t map = entity_map_pos(planet);
        if (map.x < p8_fixed_from_int(1) || map.x > p8_fixed_from_int(2) || map.y < p8_fixed_from_int(1) ||
            map.y > p8_fixed_from_int(2)) {
            continue;
        }
        p8_fixed_t nearest = 0;
        for (int j = 0; j < planets->count; j++) {
            p8_fixed_t squared = vec_length_squared(vec_sub(entity_map_pos(planets->items[j]), map));
            if (j != i && (nearest == 0 || squared < nearest)) {
                nearest = squared;
            }
        }
        if (chosen == NULL || nearest > best) {
            chosen = (planet_t *)planet;
            best = nearest;
        }
    }
    assert(chosen != NULL && best > P8_FIXED_LIT(0x00000666, 0.025) && "most_isolated_planet: no planet 20 px clear of the others");
    return chosen;
}

static void place_near(planet_t *planet, const planet_t *hub, int dx, int dy) {
    vec_t at = vec_add(entity_pos(&hub->header), vec_make(p8_fixed_from_int(dx), p8_fixed_from_int(dy)));
    entity_set_pos(&planet->header, at);
    entity_set_map_pos(&planet->header, vec_div(at, p8_fixed_from_int(GALAXY_MAP_SCALE)));
}

/* Dragged over the end and dropped, as the mouse builds it; paid from the credits. */
static entity_t *link_planets(planet_t *start, planet_t *end) {
    slipway_keys_t keys = { 0 };
    keys.start = entity_id_link(&start->header);
    entity_t *slipway = &slipway_new(&keys)->header;
    mouse_pos_t nowhere = { 0 };
    main_ui()->hovered = entity_id_link(&end->header);
    slipway->class->drag(slipway, nowhere);
    slipway->class->drop(slipway, &end->header);
    assert(!slipway->is_dead && slipway_other_end((const slipway_t *)slipway, &start->header) == &end->header);
    return slipway;
}

/* ends[i] is placed at offsets[i] from the hub and linked to it, in that order; slipways[i] joins them. */
static void star_around(planet_t *hub, const int offsets[][2], int count, planet_t *ends[], entity_t *slipways[]) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    main_economy_typed()->credits = p8_fixed_from_int(999);
    int next = 0;
    for (int i = 0; i < count; i++) {
        if (planets->items[next] == &hub->header) {
            next++;
        }
        ends[i] = (planet_t *)planets->items[next++];
        place_near(ends[i], hub, offsets[i][0], offsets[i][1]);
        slipways[i] = link_planets(hub, ends[i]);
    }
}

/* Built out of clockwise order, so the order cannot come from the slipways list. */
static void test_slipways_turn_clockwise_from_straight_up(void) {
    static const int offsets[4][2] = { { 0, 12 }, { -12, 0 }, { 0, -12 }, { 9, 9 } };
    static const int clockwise[4] = { 2, 3, 0, 1 };
    input_step_t step;
    planet_t *ends[4];
    entity_t *slipways[4];
    start_game(&step);
    planet_t *hub = most_isolated_planet();
    star_around(hub, offsets, 4, ends, slipways);
    assert(pad_nav_next_slipway(&hub->header, NULL, 1) == slipways[clockwise[0]]);
    for (int i = 0; i < 4; i++) {
        entity_t *current = slipways[clockwise[i]];
        assert(pad_nav_next_slipway(&hub->header, current, 1) == slipways[clockwise[(i + 1) % 4]]);
        assert(pad_nav_next_slipway(&hub->header, current, -1) == slipways[clockwise[(i + 3) % 4]]);
        assert(pad_nav_next_slipway(&hub->header, current, 0) == current);
    }
    assert(pad_nav_next_slipway(&ends[0]->header, slipways[1], 1) == slipways[0]);
    assert(pad_nav_next_slipway(&ends[0]->header, NULL, -1) == slipways[0]);
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    entity_t *lone = planets->items[planets->count - 1] != &hub->header ? planets->items[planets->count - 1] : planets->items[planets->count - 2];
    assert(((const planet_t *)lone)->slipways.count == 0);
    assert(pad_nav_next_slipway(lone, NULL, 1) == NULL && pad_nav_next_slipway(lone, slipways[0], 0) == NULL);
}

/* Built apart, then moved one pixel either side of straight up: dx +1 is the right half, so it comes first. */
static void test_straight_up_splits_the_two_halves(void) {
    static const int offsets[2][2] = { { -9, -12 }, { 9, -12 } };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = most_isolated_planet();
    star_around(hub, offsets, 2, ends, slipways);
    place_near(ends[0], hub, -1, -12);
    place_near(ends[1], hub, 1, -12);
    assert(pad_nav_next_slipway(&hub->header, NULL, 1) == slipways[1]);
    assert(pad_nav_next_slipway(&hub->header, slipways[1], 1) == slipways[0]);
    assert(pad_nav_next_slipway(&hub->header, slipways[1], -1) == slipways[0]);
}

static planet_t *network_on_star(input_step_t *step, const int offsets[][2], int count, planet_t *ends[], entity_t *slipways[]) {
    planet_t *hub = most_isolated_planet();
    for (int i = 0; i < 64 && hub->header.state != ENTITY_STATE_KNOWN; i++) {
        planet_roll_type(hub);
    }
    assert(hub->header.state == ENTITY_STATE_KNOWN);
    star_around(hub, offsets, count, ends, slipways);
    pan_planet_under_centre(step, hub, 0);
    tap(step, P8_PAD_TRIANGLE);
    assert(main_ui()->ephemerals.count > 0);
    tap(step, P8_PAD_CIRCLE);
    assert(main_ui()->ephemerals.count == 0);
    assert_network_on(step, hub);
    return hub;
}

static void assert_selected(const input_step_t *step, const planet_t *planet, const entity_t *slipway) {
    assert_network_on(step, planet);
    assert(pad_nav_selected_slipway(&step->pad_nav) == slipway);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    assert_cursor_on(step, &planet->header);
}

static void test_left_and_right_turn_the_selected_slipway(void) {
    static const int offsets[3][2] = { { 0, -12 }, { 9, 9 }, { -12, 0 } };
    input_step_t step;
    planet_t *ends[3];
    entity_t *slipways[3];
    start_game(&step);
    planet_t *hub = network_on_star(&step, offsets, 3, ends, slipways);
    vec_t from = camera_at();
    run_frame(&step, 0);
    assert_selected(&step, hub, slipways[0]);
    for (int i = 0; i < 10; i++) {
        run_frame(&step, P8_PAD_RIGHT);
        assert_selected(&step, hub, slipways[1]);
    }
    run_frame(&step, 0);
    tap(&step, P8_PAD_RIGHT);
    assert_selected(&step, hub, slipways[2]);
    tap(&step, P8_PAD_RIGHT);
    assert_selected(&step, hub, slipways[0]);
    tap(&step, P8_PAD_LEFT);
    assert_selected(&step, hub, slipways[2]);
    tap(&step, P8_PAD_LEFT);
    assert_selected(&step, hub, slipways[1]);
    assert_moved(from, 0, 0);
}

static void assert_travelled(const input_step_t *step, const planet_t *planet, const entity_t *slipway) {
    assert_selected(step, planet, slipway);
    assert_centred_on(&planet->header);
    assert(ui_pad_selection(main_ui()) == &planet->header && main_ui()->pad_selection_frames == 0);
}

static void test_cross_travels_along_the_selected_slipway(void) {
    static const int offsets[2][2] = { { 0, -12 }, { -12, 0 } };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = network_on_star(&step, offsets, 2, ends, slipways);
    settle(&step);
    assert_selected(&step, hub, slipways[0]);
    assert(main_ui()->pad_selection_frames > 0);
    run_frame(&step, P8_PAD_CROSS);
    assert_travelled(&step, ends[0], slipways[0]);
    for (int i = 0; i < 10; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert_selected(&step, ends[0], slipways[0]);
    }
    run_frame(&step, 0);
    run_frame(&step, P8_PAD_CROSS);
    assert_travelled(&step, hub, slipways[0]);
    run_frame(&step, 0);
    run_frame(&step, P8_PAD_CROSS | P8_PAD_RIGHT);
    assert_travelled(&step, ends[0], slipways[0]);
    run_frame(&step, 0);
    tap(&step, P8_PAD_CROSS);
    assert_selected(&step, hub, slipways[0]);
    tap(&step, P8_PAD_LEFT);
    assert_selected(&step, hub, slipways[1]);
    run_frame(&step, P8_PAD_CROSS);
    assert_travelled(&step, ends[1], slipways[1]);
}

/* Network mode reads only Left/Right off the D-pad: a diagonal still turns. */
static void test_up_and_down_do_nothing_in_network_mode(void) {
    static const int offsets[2][2] = { { 0, -12 }, { -12, 0 } };
    static const int directions[2] = { P8_PAD_UP, P8_PAD_DOWN };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = network_on_star(&step, offsets, 2, ends, slipways);
    run_frame(&step, 0);
    vec_t from = camera_at();
    for (int d = 0; d < 2; d++) {
        for (int i = 0; i < 12; i++) {
            run_frame(&step, i < 10 ? directions[d] : 0);
            assert_selected(&step, hub, slipways[0]);
        }
    }
    assert_moved(from, 0, 0);
    tap(&step, P8_PAD_UP | P8_PAD_RIGHT);
    assert_selected(&step, hub, slipways[1]);
    tap(&step, P8_PAD_DOWN | P8_PAD_LEFT);
    assert_selected(&step, hub, slipways[0]);
    assert_moved(from, 0, 0);
}

/* □ saved no slipway, the planet has gained one since: ○ lands with it selected, and the circle shows from the next frame. */
static void test_circle_lands_with_a_slipway_selected(void) {
    static const int offsets[1][2] = { { 12, 0 } };
    input_step_t step;
    planet_t *ends[1];
    entity_t *slipways[1];
    start_game(&step);
    planet_t *hub = network_on_star(&step, offsets, 0, ends, slipways);
    tap(&step, P8_PAD_SQUARE);
    assert_free_from(&step, hub);
    star_around(hub, offsets, 1, ends, slipways);
    hold(&step, P8_PAD_LEFT, 30);
    run_frame(&step, P8_PAD_CIRCLE);
    assert_network_on(&step, hub);
    assert(pad_nav_selected_slipway(&step.pad_nav) == slipways[0]);
    assert_centred_on(&hub->header);
    assert(ui_pad_selection(main_ui()) == NULL);
    run_frame(&step, 0);
    assert_selected(&step, hub, slipways[0]);
    assert(ui_pad_selection(main_ui()) == &hub->header && main_ui()->pad_selection_frames == 0);
}

static void test_circle_restores_the_slipway_turned_to_before_square(void) {
    static const int offsets[2][2] = { { 0, -12 }, { -12, 0 } };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = network_on_star(&step, offsets, 2, ends, slipways);
    tap(&step, P8_PAD_RIGHT);
    assert_selected(&step, hub, slipways[1]);
    tap(&step, P8_PAD_SQUARE);
    assert_free_from(&step, hub);
    hold(&step, P8_PAD_DOWN, 30);
    tap(&step, P8_PAD_CIRCLE);
    assert_selected(&step, hub, slipways[1]);
}

static void test_circle_in_plain_free_mode_returns_to_the_last_planet_and_its_slipway(void) {
    static const int offsets[2][2] = { { 0, -12 }, { -12, 0 } };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = network_on_star(&step, offsets, 2, ends, slipways);
    tap(&step, P8_PAD_RIGHT);
    assert_selected(&step, hub, slipways[1]);
    tap(&step, P8_PAD_SQUARE);
    assert_free_from(&step, hub);
    tap(&step, P8_PAD_SQUARE);
    assert(pad_nav_free_mode(&step.pad_nav) && pad_nav_starting_planet(&step.pad_nav) == NULL);
    hold(&step, P8_PAD_DOWN, 30);
    run_frame(&step, P8_PAD_CIRCLE);
    assert_network_on(&step, hub);
    assert_centred_on(&hub->header);
    run_frame(&step, 0);
    assert_selected(&step, hub, slipways[1]);
    tap(&step, P8_PAD_SQUARE);
    assert_free_from(&step, hub);
}

static void test_circle_in_plain_free_mode_does_nothing_once_the_last_planet_is_dead(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = network_on_known_planet(&step);
    tap(&step, P8_PAD_SQUARE);
    tap(&step, P8_PAD_SQUARE);
    hold(&step, P8_PAD_RIGHT, 20);
    run_frame(&step, 0);
    planet->header.is_dead = 1;
    vec_t from = camera_at();
    for (int i = 0; i < 4; i++) {
        run_frame(&step, i % 2 ? 0 : P8_PAD_CIRCLE);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        assert_cursor_at_centre(&step);
        assert(pad_nav_free_mode(&step.pad_nav) && pad_nav_selection(&step.pad_nav) == NULL);
    }
    assert_moved(from, 0, 0);
}

/* Planted a few pixels off the centre, still under it: the landing centres the camera. */
static void put_planet_under_centre_off_by(input_step_t *step, const planet_t *planet, int dx, int dy) {
    pan_planet_under_centre(step, planet, 0);
    camera_pan(main_camera_typed(), dx, dy);
    run_frame(step, 0);
}

/* Plain free mode is where every game starts. Built out of clockwise order: the first clockwise is slipways[1]. */
static void test_square_in_plain_free_mode_selects_the_settled_planet_under_the_centre(void) {
    static const int offsets[2][2] = { { -12, 0 }, { 0, -12 } };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = most_isolated_planet();
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    settle_planet(&step, hub);
    main_economy_typed()->credits = p8_fixed_from_int(999);
    for (int i = 0, next = 0; i < 2; i++, next++) {
        next += planets->items[next] == &hub->header;
        ends[i] = (planet_t *)planets->items[next];
        settle_planet(&step, ends[i]);
        place_near(ends[i], hub, offsets[i][0], offsets[i][1]);
        slipways[i] = link_planets(hub, ends[i]);
    }
    put_planet_under_centre_off_by(&step, hub, 3, -2);
    assert_plain_free_mode(&step);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_SQUARE);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert_network_on(&step, hub);
    assert(pad_nav_selected_slipway(&step.pad_nav) == slipways[1]);
    assert_centred_on(&hub->header);
    assert(ui_pad_selection(main_ui()) == NULL);
    run_frame(&step, 0);
    assert_selected(&step, hub, slipways[1]);
    assert(ui_pad_selection(main_ui()) == &hub->header && main_ui()->pad_selection_frames == 0);
    assert(!main_ui()->pad_crosshair);
    settle(&step);
    assert_selected(&step, hub, slipways[1]);
    assert_world_unchanged(before);
    assert(main_ui()->ephemerals.count == 0 && ui_left_held(main_ui()) == NULL);
}

/* The drag goes through the slipway's own null drop: snapped on the target, a plain release would build it. */
static void test_square_while_dragging_selects_another_settled_planet_under_the_centre(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    put_planet_under_centre_off_by(&step, target, -2, 3);
    assert_dragging(&step, start);
    assert(slipway_end((const slipway_t *)ui_left_held(main_ui())) == &target->header);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_SQUARE);
    assert(input_step_played_frame(&step)->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
    assert_network_on(&step, target);
    assert(pad_nav_starting_planet(&step.pad_nav) == NULL);
    assert_centred_on(&target->header);
    run_frame(&step, 0);
    assert_selected(&step, target, NULL);
    assert(ui_pad_selection(main_ui()) == &target->header && main_ui()->pad_selection_frames == 0);
    settle(&step);
    assert_selected(&step, target, NULL);
    assert_world_unchanged(before);
    assert(slipway_between(&start->header, &target->header) == NULL);
    assert(main_ui()->ephemerals.count == 0 && ui_left_held(main_ui()) == NULL);
}

static void test_square_while_dragging_over_a_known_planet_leaves_for_plain_free_mode(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 0);
    for (int i = 0; i < 64 && target->header.state != ENTITY_STATE_KNOWN; i++) {
        planet_roll_type(target);
    }
    pan_planet_under_centre(&step, target, 0);
    assert_dragging(&step, start);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_SQUARE);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert_plain_free_mode(&step);
    settle(&step);
    assert_plain_free_mode(&step);
    assert_world_unchanged(before);
}

/* The fold holds the left button: □ must not land in network mode under it. */
static void test_square_while_cross_holds_a_fold_selects_nothing(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = most_isolated_planet();
    settle_planet(&step, planet);
    pan_to_empty_space(&step);
    tap(&step, P8_PAD_CROSS);
    assert(tagged_count(TAG_PROBE) == 1);
    run_frame(&step, P8_PAD_CROSS);
    pan_planet_under_centre(&step, planet, P8_PAD_CROSS);
    entity_t *held = ui_left_held(main_ui());
    assert(held != NULL && held->class == wormhole_class());
    for (int i = 0; i < 4; i++) {
        run_frame(&step, P8_PAD_CROSS | (i % 2 ? 0 : P8_PAD_SQUARE));
        assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
        assert(ui_left_held(main_ui()) == held);
        assert(pad_nav_free_mode(&step.pad_nav) && pad_nav_selection(&step.pad_nav) == NULL);
    }
}

static void test_shoulders_in_plain_free_mode_switch_and_come_back_to_it(void) {
    input_step_t step;
    start_game(&step);
    (void)network_on_known_planet(&step);
    tap(&step, P8_PAD_SQUARE);
    tap(&step, P8_PAD_SQUARE);
    hold(&step, P8_PAD_RIGHT, 20);
    run_frame(&step, 0);
    vec_t from = camera_at();
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    run_frame(&step, 0);
    tap(&step, P8_PAD_L1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
    assert(pad_nav_free_mode(&step.pad_nav) && pad_nav_starting_planet(&step.pad_nav) == NULL);
    assert(pad_nav_selection(&step.pad_nav) == NULL);
    assert_moved(from, 0, 0);
}

/* A settled planet's △ holds its slipway until the release: a travel then would drop it on the other end. */
static void test_cross_on_the_triangle_release_frame_is_ignored(void) {
    input_step_t step;
    start_game(&step);
    planet_t *hub = most_isolated_planet();
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    planet_t *end = (planet_t *)planets->items[planets->items[0] == &hub->header ? 1 : 0];
    settle_planet(&step, hub);
    settle_planet(&step, end);
    place_near(end, hub, 0, -12);
    main_economy_typed()->credits = p8_fixed_from_int(999);
    (void)link_planets(hub, end);
    world_count_t before = world_count();
    pan_planet_under_centre(&step, hub, 0);
    run_frame(&step, P8_PAD_TRIANGLE);
    run_frame(&step, P8_PAD_CROSS);
    assert_network_on(&step, hub);
    assert_cursor_on(&step, &hub->header);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, 0);
    assert_world_unchanged(before);
}

static void test_the_ui_pad_selection_is_the_planet_only_in_network_mode_without_a_box(void) {
    input_step_t step;
    start_game(&step);
    assert(ui_pad_selection(main_ui()) == NULL);
    planet_t *planet = known_planet_under_centre(&step);
    assert(ui_pad_selection(main_ui()) == NULL);
    run_frame(&step, P8_PAD_TRIANGLE);
    assert(main_ui()->ephemerals.count > 0 && ui_pad_selection(main_ui()) == NULL);
    run_frame(&step, 0);
    assert(main_ui()->ephemerals.count > 0 && ui_pad_selection(main_ui()) == NULL);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(ui_pad_selection(main_ui()) == &planet->header);
    run_frame(&step, 0);
    assert(ui_pad_selection(main_ui()) == &planet->header);
    planet->header.is_dead = 1;
    run_frame(&step, 0);
    assert(pad_nav_free_mode(&step.pad_nav) && ui_pad_selection(main_ui()) == NULL);
}

static void test_the_ui_pad_slipway_is_shown_with_the_planet(void) {
    static const int offsets[2][2] = { { 0, -12 }, { 0, 12 } };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    assert(ui_pad_slipway(main_ui()) == NULL);
    planet_t *hub = network_on_star(&step, offsets, 2, ends, slipways);
    assert(ui_pad_slipway(main_ui()) == slipways[0]);
    tap(&step, P8_PAD_RIGHT);
    assert(ui_pad_slipway(main_ui()) == slipways[1]);
    run_frame(&step, P8_PAD_TRIANGLE);
    assert(ui_pad_selection(main_ui()) == NULL && ui_pad_slipway(main_ui()) == NULL);
    run_frame(&step, 0);
    assert(main_ui()->ephemerals.count > 0 && ui_pad_slipway(main_ui()) == NULL);
    run_frame(&step, 0);
    assert(ui_pad_slipway(main_ui()) == NULL);
    tap(&step, P8_PAD_CIRCLE);
    assert(ui_pad_selection(main_ui()) == &hub->header && ui_pad_slipway(main_ui()) == slipways[1]);
    tap(&step, P8_PAD_SQUARE);
    assert(pad_nav_free_mode(&step.pad_nav) && ui_pad_slipway(main_ui()) == NULL);
}

enum { PAD_CIRCLE_RADIUS = 9, PAD_CIRCLE_COLOUR = 7 };

/* Of the radius-9 circle's pixels around the point, how many show colour 7; *total gets their number. */
static int circle_pixels_lit(int x, int y, int *total) {
    static p8_vram_t pattern;
    p8_graphics_t pattern_graphics;
    p8_vram_init(&pattern);
    p8_gfx_init(&pattern_graphics);
    p8_circ(&pattern_graphics, &pattern, x, y, PAD_CIRCLE_RADIUS, 1);
    int lit = 0;
    *total = 0;
    for (int row = 0; row < P8_SCREEN_HEIGHT; row++) {
        for (int column = 0; column < P8_SCREEN_WIDTH; column++) {
            if (pattern.screen[row][column] == 1) {
                (*total)++;
                lit += memory.screen[row][column] == PAD_CIRCLE_COLOUR;
            }
        }
    }
    return lit;
}

/* The draw floors the planet and the camera apart, the cursor floors their difference: the two can be a pixel apart. */
static int circle_pixels_lit_near(int x, int y, int *total) {
    int best = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int lit = circle_pixels_lit(x + dx, y + dy, total);
            best = lit > best ? lit : best;
        }
    }
    return best;
}

enum { CROSSHAIR_COLOUR = 7, CROSSHAIR_ARM = 2 };

/* How many of the crosshair's arm pixels around the screen centre show its colour, out of 4 * CROSSHAIR_ARM. */
static int crosshair_pixels_lit(void) {
    int x = P8_SCREEN_WIDTH / 2;
    int y = P8_SCREEN_HEIGHT / 2;
    int lit = 0;
    for (int d = 1; d <= CROSSHAIR_ARM; d++) {
        lit += memory.screen[y][x - d] == CROSSHAIR_COLOUR;
        lit += memory.screen[y][x + d] == CROSSHAIR_COLOUR;
        lit += memory.screen[y - d][x] == CROSSHAIR_COLOUR;
        lit += memory.screen[y + d][x] == CROSSHAIR_COLOUR;
    }
    return lit;
}

static void test_network_mode_hides_the_cursor_and_blinks_a_circle(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = network_on_known_planet(&step);
    assert_cursor_on(&step, &planet->header);
    int x = input_step_played_frame(&step)->mouse_x;
    int y = input_step_played_frame(&step)->mouse_y;
    int total;
    cart_draw();
    int lit = circle_pixels_lit_near(x, y, &total);
    assert(lit == total);
    for (int i = 0; i < UI_PAD_BLINK_PERIOD / 2; i++) {
        run_frame(&step, 0);
    }
    assert_cursor_on(&step, &planet->header);
    cart_draw();
    lit = circle_pixels_lit_near(x, y, &total);
    assert(lit == 0);
}

enum { CORE_REACH = 30, SLIPWAY_COLOUR = 1, BRIGHT_SLIPWAY_COLOUR = 12 };

static const int vertical_star[2][2] = { { 0, -CORE_REACH }, { 0, CORE_REACH } };

/* Pixels of that colour on the middle of the vertical slipway from the hub towards side (-1 up, +1 down),
 * clear of the planets and the selection circle. */
static int core_pixels(const planet_t *hub, int side, int colour) {
    vec_t at = vec_sub(entity_pos(&hub->header), entity_pos(&main_camera_typed()->header));
    int x = p8_fixed_flr_int(at.x);
    int y = p8_fixed_flr_int(at.y);
    int count = 0;
    for (int distance = PAD_CIRCLE_RADIUS + 2; distance <= CORE_REACH - 9; distance++) {
        for (int column = x - 3; column <= x + 4; column++) {
            count += memory.screen[y + side * distance][column] == colour;
        }
    }
    return count;
}

static void finish_fade_in(entity_t *const slipways[], int count) {
    for (int i = 0; i < count; i++) {
        ((slipway_t *)slipways[i])->fade_in_step = p8_fixed_from_int(20);
    }
}

static void test_the_selected_slipway_is_drawn_bright(void) {
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = network_on_star(&step, vertical_star, 2, ends, slipways);
    assert(ui_pad_slipway(main_ui()) == slipways[0]);
    finish_fade_in(slipways, 2);
    cart_draw();
    assert(core_pixels(hub, -1, BRIGHT_SLIPWAY_COLOUR) > 0 && core_pixels(hub, -1, SLIPWAY_COLOUR) == 0);
    assert(core_pixels(hub, 1, BRIGHT_SLIPWAY_COLOUR) == 0 && core_pixels(hub, 1, SLIPWAY_COLOUR) > 0);
}

enum { DRAG_REACH = 36, REFUSED_COLOUR = 2, COST_COLOUR = 8 };

/* A unit step of 8 headings whose DRAG_REACH segment from the hub passes no other planet. */
static void clear_heading(const planet_t *hub, int *ux, int *uy) {
    static const int headings[8][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 } };
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    vec_t from = entity_map_pos(&hub->header);
    for (int h = 0; h < 8; h++) {
        vec_t to = vec_add(entity_pos(&hub->header), vec_make(p8_fixed_from_int(headings[h][0] * DRAG_REACH),
                                                               p8_fixed_from_int(headings[h][1] * DRAG_REACH)));
        to = vec_div(to, p8_fixed_from_int(GALAXY_MAP_SCALE));
        int clear = 1;
        for (int i = 0; i < planets->count && clear; i++) {
            clear = planets->items[i] == &hub->header ||
                    !geom_near_segment(from, to, entity_map_pos(planets->items[i]), P8_FIXED_LIT(0x00000a3d, 0.04));
        }
        if (clear) {
            *ux = headings[h][0];
            *uy = headings[h][1];
            return;
        }
    }
    assert(0 && "clear_heading: every heading passes a planet");
}

/* Dragging from the isolated hub with the centre DRAG_REACH px along a clear heading, past the fade-in. */
static planet_t *drag_from_hub(input_step_t *step, int *ux, int *uy) {
    planet_t *hub = most_isolated_planet();
    settle_planet(step, hub);
    pan_planet_under_centre(step, hub, 0);
    tap(step, P8_PAD_TRIANGLE);
    tap(step, P8_PAD_CIRCLE);
    tap(step, P8_PAD_SQUARE);
    assert_dragging(step, hub);
    clear_heading(hub, ux, uy);
    camera_pan(main_camera_typed(), *ux * DRAG_REACH, *uy * DRAG_REACH);
    settle(step);
    settle(step);
    assert_dragging(step, hub);
    return hub;
}

/* Points of the segment from the hub's screen point to the centre, from `from` px out to 4 px short, with a 2x2 block of
 * that colour within 2 px: the line is two pixels thick and its ends wobble 1.5 px, the background grid's dots are single. */
static int drag_line_pixels(const planet_t *hub, int from, int colour) {
    vec_t at = vec_sub(entity_pos(&hub->header), camera_at());
    int x0 = p8_fixed_flr_int(at.x), y0 = p8_fixed_flr_int(at.y);
    int length = absolute_int(P8_SCREEN_WIDTH / 2 - x0) > absolute_int(P8_SCREEN_HEIGHT / 2 - y0)
                     ? absolute_int(P8_SCREEN_WIDTH / 2 - x0)
                     : absolute_int(P8_SCREEN_HEIGHT / 2 - y0);
    int count = 0;
    for (int d = from; d <= length - 4; d++) {
        int x = x0 + (P8_SCREEN_WIDTH / 2 - x0) * d / length;
        int y = y0 + (P8_SCREEN_HEIGHT / 2 - y0) * d / length;
        int hit = 0;
        for (int dy = -2; dy <= 1; dy++) {
            for (int dx = -2; dx <= 1; dx++) {
                hit |= memory.screen[y + dy][x + dx] == colour && memory.screen[y + dy][x + dx + 1] == colour &&
                       memory.screen[y + dy + 1][x + dx] == colour && memory.screen[y + dy + 1][x + dx + 1] == colour;
            }
        }
        count += hit;
    }
    return count;
}

static int pixels_in_box(int left, int top, int right, int bottom, int colour) {
    int count = 0;
    for (int y = top; y <= bottom; y++) {
        for (int x = left; x <= right; x++) {
            count += memory.screen[y][x] == colour;
        }
    }
    return count;
}

/* As the mouse shows it: the slipway's line from the planet to the aim point, and its cost above that point. */
static void test_a_drag_draws_the_partial_slipway_and_its_cost(void) {
    input_step_t step;
    int ux, uy;
    start_game(&step);
    planet_t *hub = drag_from_hub(&step, &ux, &uy);
    cart_draw();
    int along = DRAG_REACH - 14;
    assert(drag_line_pixels(hub, 11, SLIPWAY_COLOUR) >= along);
    assert(drag_line_pixels(hub, 11, REFUSED_COLOUR) == 0);
    int x = P8_SCREEN_WIDTH / 2, y = P8_SCREEN_HEIGHT / 2;
    assert(pixels_in_box(x - 8, y - 8, x + 8, y - 4, COST_COLOUR) >= 6);
    assert(crosshair_pixels_lit() == 4 * CROSSHAIR_ARM);
}

/* A planet on the segment refuses it: the line turns colour 2 and the planet gets the dithered colour-2 disc. */
static void test_a_drag_across_a_planet_draws_the_refused_feedback(void) {
    input_step_t step;
    int ux, uy;
    start_game(&step);
    planet_t *hub = drag_from_hub(&step, &ux, &uy);
    planet_t *blocker = NULL;
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    blocker = (planet_t *)planets->items[planets->items[0] == &hub->header ? 1 : 0];
    for (int i = 0; i < 64 && blocker->header.state != ENTITY_STATE_KNOWN; i++) {
        planet_roll_type(blocker);
    }
    assert(blocker->header.state == ENTITY_STATE_KNOWN);
    place_near(blocker, hub, ux * DRAG_REACH / 2, uy * DRAG_REACH / 2);
    run_frame(&step, 0);
    run_frame(&step, 0);
    entity_t *held = ui_left_held(main_ui());
    assert(held != NULL && targeting_target(&((slipway_t *)held)->targeting) == &blocker->header);
    cart_draw();
    assert(drag_line_pixels(hub, DRAG_REACH / 2 + 10, REFUSED_COLOUR) > 0);
    assert(drag_line_pixels(hub, DRAG_REACH / 2 + 10, SLIPWAY_COLOUR) == 0);
    vec_t at = vec_sub(entity_pos(&blocker->header), camera_at());
    int bx = p8_fixed_flr_int(at.x), by = p8_fixed_flr_int(at.y);
    assert(pixels_in_box(bx - 8, by - 8, bx + 8, by + 8, REFUSED_COLOUR) >= 30);
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

static void test_shoulders_switch_and_free_mode_pans_under_the_year_box(void) {
    input_step_t step;
    start_game(&step);
    score_panel_new_year(p8_fixed_from_int(3421));
    entity_t *year_box = &shown_message_box()->header;
    run_frame(&step, 0);
    tap(&step, P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    tap(&step, P8_PAD_L1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP && pad_nav_free_mode(&step.pad_nav));
    vec_t from = camera_at();
    run_frame(&step, P8_PAD_RIGHT);
    assert_moved(from, 1, 0);
    assert(entity_list_has(live_entities(), year_box) && shown_message_box() != NULL);
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

static const pad_nav_tab_t forward_cycle[5] = { PAD_NAV_TAB_BUILD, PAD_NAV_TAB_END_TURN, PAD_NAV_TAB_TECH,
                                                  PAD_NAV_TAB_SCORE, PAD_NAV_TAB_MAP };

static void test_r1_and_l1_cycle_the_tabs(void) {
    input_step_t step;
    start_game(&step);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
    for (int i = 0; i < 5; i++) {
        tap(&step, P8_PAD_R1);
        assert(pad_nav_tab(&step.pad_nav) == forward_cycle[i]);
    }
    /* score to tech clicks the tech button on its third frame and releases it on the fourth: L1 waits until then. */
    for (int i = 3; i >= -1; i--) {
        tap(&step, P8_PAD_L1);
        tap(&step, 0);
        assert(pad_nav_tab(&step.pad_nav) == (i >= 0 ? forward_cycle[i] : PAD_NAV_TAB_MAP));
    }
}

static void test_a_held_shoulder_moves_one_tab_and_both_together_move_none(void) {
    input_step_t step;
    start_game(&step);
    hold(&step, P8_PAD_R1, 10);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    run_frame(&step, 0);
    tap(&step, P8_PAD_L1 | P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
}

static void go_to_tab(input_step_t *step, pad_nav_tab_t tab) {
    for (int i = 0; i < 5 && pad_nav_tab(&step->pad_nav) != tab; i++) {
        tap(step, P8_PAD_R1);
    }
    assert(pad_nav_tab(&step->pad_nav) == tab);
}

static void assert_played(const input_step_t *step, int x, int y, int buttons) {
    const p8_input_frame_t *played = input_step_played_frame(step);
    assert(played->mouse_x == x && played->mouse_y == y && played->mouse_buttons == buttons);
}

/* NULL when no revealed icon sits at that screen x. */
static entity_t *revealed_icon_at(int x) {
    const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
    for (int i = 0; i < bucket->count; i++) {
        entity_t *e = bucket->items[i];
        if (structure_icon_is_revealed(e) && p8_fixed_flr_int(entity_pos(e).x) == x) {
            return e;
        }
    }
    return NULL;
}

static entity_t *basic_structure_icon(void) {
    entity_t *icon = revealed_icon_at(p8_fixed_flr_int(structure_icon_rest_cursor()->x));
    assert(icon != NULL);
    return icon;
}

static entity_t *processor_icon(void) {
    entity_t *icon = revealed_icon_at(p8_fixed_flr_int(structure_icon_rest_cursor()->x) - 10);
    assert(icon != NULL);
    return icon;
}

static void assert_build_focus(const input_step_t *step, entity_t *icon) {
    int x, y;
    pad_panel_focus_point(icon, &x, &y);
    assert(pad_nav_build_icon(&step->pad_nav) == icon);
    assert_played(step, x, y, 0);
    assert(ui_hovered(main_ui()) == icon);
}

static void test_build_focuses_the_basic_structure_icon_from_the_tab_change_frame(void) {
    input_step_t step;
    start_game(&step);
    assert(pad_nav_build_icon(&step.pad_nav) == NULL);
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    assert_build_focus(&step, basic_structure_icon());
    assert(ui_pad_tab(main_ui()) == NULL);
    run_frame(&step, 0);
    assert_build_focus(&step, basic_structure_icon());
    assert(ui_pad_tab(main_ui()) == NULL);
}

static void test_build_left_and_right_move_between_the_icons_without_wrapping(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    tap(&step, P8_PAD_LEFT);
    assert_build_focus(&step, processor_icon());
    tap(&step, P8_PAD_LEFT);
    assert_build_focus(&step, processor_icon());
    tap(&step, P8_PAD_RIGHT);
    assert_build_focus(&step, basic_structure_icon());
    tap(&step, P8_PAD_RIGHT);
    assert_build_focus(&step, basic_structure_icon());
}

static void test_build_left_reaches_every_revealed_icon_in_decreasing_x(void) {
    static char *const techs[2] = { "slipgates", "ascension" };
    input_step_t step;
    start_game(&step);
    for (int i = 0; i < 2; i++) {
        pointer_list_add(&main_economy_typed()->invented, techs[i]);
    }
    const entity_list_t *refresh = entities_by_message(ENTITY_MESSAGE_REFRESH);
    for (int i = 0; i < refresh->count; i++) {
        if (refresh->items[i]->class->refresh != NULL) {
            refresh->items[i]->class->refresh(refresh->items[i]);
        }
    }
    int revealed = 0, hidden = 0;
    const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
    for (int i = 0; i < bucket->count; i++) {
        revealed += structure_icon_is_revealed(bucket->items[i]);
        hidden += bucket->items[i]->class == structure_icon_class() && !structure_icon_is_revealed(bucket->items[i]);
    }
    assert(revealed == 4 && hidden == 3);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    int rest_x = p8_fixed_flr_int(structure_icon_rest_cursor()->x);
    for (int i = 0; i < revealed; i++) {
        assert_build_focus(&step, revealed_icon_at(rest_x - 10 * i));
        tap(&step, P8_PAD_LEFT);
    }
    assert_build_focus(&step, revealed_icon_at(rest_x - 10 * (revealed - 1)));
}

static void test_build_remembers_its_icon_until_a_new_game(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    tap(&step, P8_PAD_LEFT);
    tap(&step, P8_PAD_L1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP && pad_nav_build_icon(&step.pad_nav) == NULL);
    run_frame(&step, P8_PAD_R1);
    assert_build_focus(&step, processor_icon());
    cart_init();
    settle(&step);
    cart_start_game((button_t *)title_button("forgiving"));
    settle(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    assert_build_focus(&step, basic_structure_icon());
}

static void test_build_up_down_square_and_triangle_press_nothing(void) {
    static const int buttons[4] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_SQUARE, P8_PAD_TRIANGLE };
    input_step_t step;
    start_game(&step);
    vec_t from = camera_at();
    int live = live_entities()->count;
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    for (int b = 0; b < 4; b++) {
        hold(&step, buttons[b], 3);
        assert_build_focus(&step, basic_structure_icon());
        run_frame(&step, 0);
    }
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    assert_moved(from, 0, 0);
    assert(live_entities()->count == live && main_ui()->ephemerals.count == 0);
    assert(tagged_count(TAG_PROBE) == 0);
}

/* score leaves through a click on its backdrop: its own test covers it. */
static void test_circle_on_a_tab_returns_to_the_map(void) {
    static const pad_nav_tab_t tabs[3] = { PAD_NAV_TAB_BUILD, PAD_NAV_TAB_END_TURN, PAD_NAV_TAB_TECH };
    input_step_t step;
    start_game(&step);
    for (int t = 0; t < 3; t++) {
        if (in_forked_copy()) {
            go_to_tab(&step, tabs[t]);
            run_frame(&step, P8_PAD_CIRCLE | P8_PAD_CROSS);
            assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
            assert_played(&step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
            run_frame(&step, P8_PAD_CROSS);
            assert(pad_nav_free_mode(&step.pad_nav));
            assert_played(&step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
            end_forked_copy();
        }
    }
}

static void assert_carrying(const input_step_t *step, const entity_t *structure) {
    assert(pad_nav_tab(&step->pad_nav) == PAD_NAV_TAB_MAP);
    assert(pad_nav_placing(&step->pad_nav) == structure && ui_left_held(main_ui()) == structure);
    assert(planet_is_family(structure) && structure->state == ENTITY_STATE_PLACING);
    assert_played(step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, P8_MOUSE_LEFT);
}

/* × tapped on build: pressed at the icon on that frame, carried on the map from the next. */
static entity_t *take_structure(input_step_t *step) {
    entity_t *icon = pad_nav_build_icon(&step->pad_nav);
    assert(icon != NULL);
    int x, y;
    pad_panel_focus_point(icon, &x, &y);
    run_frame(step, P8_PAD_CROSS);
    assert_played(step, x, y, P8_MOUSE_LEFT);
    run_frame(step, 0);
    entity_t *structure = pad_nav_placing(&step->pad_nav);
    assert(structure != NULL);
    assert_carrying(step, structure);
    return structure;
}

static void pan_clear_of_every_node(input_step_t *step, const entity_t *structure) {
    for (int i = 0; i < 120 && targeting_of((entity_t *)structure)->target.kind != TARGETING_TARGET_NONE; i++) {
        run_frame(step, P8_PAD_DOWN);
        assert_carrying(step, structure);
    }
    assert(targeting_of((entity_t *)structure)->target.kind == TARGETING_TARGET_NONE);
    run_frame(step, 0);
    assert_carrying(step, structure);
}

static void assert_placement_over(const input_step_t *step) {
    assert(pad_nav_placing(&step->pad_nav) == NULL && ui_left_held(main_ui()) == NULL);
    assert(pad_nav_free_mode(&step->pad_nav));
}

static void test_cross_takes_the_structure_and_a_second_cross_places_it(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    entity_t *structure = take_structure(&step);
    pan_clear_of_every_node(&step, structure);
    world_count_t before = world_count();
    p8_fixed_t cost = targeting_of(structure)->cost;
    assert(cost > 0);
    run_frame(&step, P8_PAD_CROSS);
    assert_played(&step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
    run_frame(&step, 0);
    assert(!structure->is_dead && structure->state == ENTITY_STATE_SETTLED);
    world_count_t now = world_count();
    assert(now.credits == p8_fixed_sub(before.credits, cost));
    assert(now.year_ticks != before.year_ticks || now.year != before.year);
    assert_placement_over(&step);
}

static void test_the_cross_that_takes_the_structure_drops_nothing_while_held(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    run_frame(&step, P8_PAD_CROSS);
    entity_t *structure = ui_left_held(main_ui());
    assert(structure != NULL);
    for (int i = 0; i < 20; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert_carrying(&step, structure);
    }
}

/* At the start centre, a planet lies within the structure's attach distance: the cart refuses the drop. */
static void test_a_refused_drop_kills_the_structure_and_charges_nothing(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    entity_t *structure = take_structure(&step);
    assert(targeting_of(structure)->target.kind != TARGETING_TARGET_NONE);
    world_count_t before = world_count();
    tap(&step, P8_PAD_CROSS);
    assert(structure->is_dead);
    assert_world_unchanged(before);
    assert_placement_over(&step);
}

/* The release lands on the icon, outside the map area, where the cart refuses every drop. */
static void test_circle_cancels_the_structure_and_charges_nothing(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    int x, y;
    pad_panel_focus_point(basic_structure_icon(), &x, &y);
    entity_t *structure = take_structure(&step);
    pan_clear_of_every_node(&step, structure);
    world_count_t before = world_count();
    vec_t from = camera_at();
    run_frame(&step, P8_PAD_CIRCLE);
    assert_played(&step, x, y, 0);
    run_frame(&step, 0);
    assert(structure->is_dead);
    assert_world_unchanged(before);
    assert_placement_over(&step);
    assert(pad_nav_starting_planet(&step.pad_nav) == NULL);
    assert_moved(from, 0, 0);
    assert_played(&step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
}

/* Start suspends the carry before pause opens, so the pause menu is opened directly. */
static void test_a_panel_opened_while_carrying_releases_at_the_icon(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    int x, y;
    pad_panel_focus_point(basic_structure_icon(), &x, &y);
    (void)take_structure(&step);
    galaxy_menu_on_click(4)(NULL);
    assert(pad_nav_panel_open());
    run_frame(&step, 0);
    assert_played(&step, x, y, 0);
}

static void test_taking_from_network_mode_enters_free_mode_from_the_selected_planet(void) {
    input_step_t step;
    start_game(&step);
    planet_t *colony = known_planet_closest_to_centre();
    settle_planet(&step, colony);
    pan_planet_under_centre(&step, colony, 0);
    tap(&step, P8_PAD_TRIANGLE);
    tap(&step, P8_PAD_CIRCLE);
    assert_network_on(&step, colony);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    camera_pan(main_camera_typed(), 30, 0);
    run_frame(&step, 0);
    entity_t *structure = take_structure(&step);
    assert_free_from(&step, colony);
    assert_centred_on(&colony->header);
    tap(&step, P8_PAD_CIRCLE);
    assert(structure->is_dead);
    assert_free_from(&step, colony);
    tap(&step, P8_PAD_CIRCLE);
    assert_network_on(&step, colony);
}

static void test_triangle_and_square_wait_while_carrying(void) {
    static const int buttons[2] = { P8_PAD_TRIANGLE, P8_PAD_SQUARE };
    input_step_t step;
    start_game(&step);
    planet_t *planet = known_planet_under_centre(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    entity_t *structure = take_structure(&step);
    for (int b = 0; b < 2; b++) {
        run_frame(&step, buttons[b]);
        assert_carrying(&step, structure);
        run_frame(&step, 0);
        assert_carrying(&step, structure);
    }
    assert(main_ui()->ephemerals.count == 0 && pad_nav_starting_planet(&step.pad_nav) == NULL);
    assert(pad_nav_selection(&step.pad_nav) == NULL && planet->header.state == ENTITY_STATE_KNOWN);
}

/* The release lands on the icon, as ○'s does: the cart refuses the drop. */
static void test_shoulders_cancel_the_carried_structure_then_switch(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    int x, y;
    pad_panel_focus_point(basic_structure_icon(), &x, &y);
    entity_t *structure = take_structure(&step);
    pan_clear_of_every_node(&step, structure);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_R1);
    assert_played(&step, x, y, 0);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    run_frame(&step, 0);
    assert(structure->is_dead && ui_left_held(main_ui()) == NULL && pad_nav_placing(&step.pad_nav) == NULL);
    assert_world_unchanged(before);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    assert_played(&step, x, y, 0);
    tap(&step, P8_PAD_L1);
    assert(pad_nav_free_mode(&step.pad_nav) && pad_nav_starting_planet(&step.pad_nav) == NULL);
}

static void test_shoulders_cancel_a_structure_taken_in_network_mode_and_land_on_its_planet(void) {
    input_step_t step;
    start_game(&step);
    planet_t *colony = known_planet_closest_to_centre();
    settle_planet(&step, colony);
    pan_planet_under_centre(&step, colony, 0);
    tap(&step, P8_PAD_TRIANGLE);
    tap(&step, P8_PAD_CIRCLE);
    assert_network_on(&step, colony);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    entity_t *structure = take_structure(&step);
    assert_free_from(&step, colony);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_L1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_SCORE && input_step_played_frame(&step)->mouse_buttons == 0);
    run_frame(&step, 0);
    assert(structure->is_dead && pad_nav_placing(&step.pad_nav) == NULL);
    assert_world_unchanged(before);
    run_frame(&step, 0);
    tap(&step, P8_PAD_CIRCLE);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
    assert_network_on(&step, colony);
    assert(pad_nav_starting_planet(&step.pad_nav) == NULL);
    assert_centred_on(&colony->header);
}

static void test_cross_held_on_arrival_on_build_takes_nothing(void) {
    input_step_t step;
    start_game(&step);
    int live = live_entities()->count;
    run_frame(&step, P8_PAD_CROSS | P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    for (int i = 0; i < 10; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert(live_entities()->count == live && ui_left_held(main_ui()) == NULL);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    run_frame(&step, 0);
    (void)take_structure(&step);
}

/* The icon leaves the registry between the press and the ui's dispatch: the press reaches the background, which
 * hands over a probe, no structure. */
static void test_a_press_that_hands_over_no_structure_stays_on_build(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    p8_input_frame_t raw = { 0, 0, 0, P8_PAD_CROSS };
    input_step_run(&step, &raw);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    entity_unregister(pad_nav_build_icon(&step.pad_nav));
    cart_update60();
    assert(ui_left_held(main_ui()) != NULL && !planet_is_family(ui_left_held(main_ui())));
    for (int i = 0; i < 3; i++) {
        run_frame(&step, 0);
        assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD && pad_nav_placing(&step.pad_nav) == NULL);
        assert(input_step_played_frame(&step)->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
    }
    tap(&step, P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_END_TURN);
}

static void year_skip_point(int *x, int *y) {
    const entity_t *button = &galaxy_menu_find_button(1)->header;
    vec_t at = entity_pos(button);
    *x = p8_fixed_flr_int(p8_fixed_div(p8_fixed_add(p8_fixed_add(at.x, button->hit_box.left), p8_fixed_add(at.x, button->hit_box.right)),
                                       p8_fixed_from_int(2)));
    *y = p8_fixed_flr_int(p8_fixed_div(p8_fixed_add(p8_fixed_add(at.y, button->hit_box.top), p8_fixed_add(at.y, button->hit_box.bottom)),
                                       p8_fixed_from_int(2)));
}

typedef struct {
    p8_fixed_t year;
    p8_fixed_t research_points;
    p8_fixed_t credits;
    p8_fixed_t income;
} year_facts_t;

static year_facts_t year_facts(void) {
    const economy_t *economy = main_economy_typed();
    year_facts_t facts = { economy->year, economy->research_points, economy->credits, economy->income };
    return facts;
}

static void assert_years_ended(year_facts_t before, int years) {
    year_facts_t now = year_facts();
    assert(now.year == p8_fixed_add(before.year, p8_fixed_from_int(years)));
    assert(now.research_points == p8_fixed_add(before.research_points, p8_fixed_from_int(years)));
    assert(now.credits == p8_fixed_add(before.credits, p8_fixed_mul(before.income, p8_fixed_from_int(years))));
}

static void test_end_turn_focuses_and_hovers_the_year_skip_button(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_END_TURN);
    int x, y;
    year_skip_point(&x, &y);
    assert_played(&step, x, y, 0);
    assert(ui_hovered(main_ui()) == &galaxy_menu_find_button(1)->header);
}

static void test_cross_on_end_turn_ends_one_year_per_press(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_END_TURN);
    vec_t from = camera_at();
    year_facts_t before = year_facts();
    int x, y;
    year_skip_point(&x, &y);
    for (int i = 0; i < 20; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert_played(&step, x, y, P8_MOUSE_LEFT);
    }
    run_frame(&step, 0);
    assert_years_ended(before, 1);
    tap(&step, P8_PAD_CROSS);
    assert_years_ended(before, 2);
    static const int directions[4] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT };
    for (int d = 0; d < 4; d++) {
        hold(&step, directions[d], 12);
    }
    assert_moved(from, 0, 0);
    assert_years_ended(before, 2);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_END_TURN);
}

static void test_cross_held_on_arrival_ends_no_year(void) {
    input_step_t step;
    start_game(&step);
    year_facts_t before = year_facts();
    run_frame(&step, P8_PAD_CROSS | P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    run_frame(&step, P8_PAD_CROSS);
    run_frame(&step, P8_PAD_CROSS | P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_END_TURN);
    for (int i = 0; i < 20; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert_years_ended(before, 0);
    run_frame(&step, 0);
    tap(&step, P8_PAD_CROSS);
    assert_years_ended(before, 1);
}

static void test_a_new_game_starts_on_the_map(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_END_TURN);
    cart_init();
    settle(&step);
    cart_start_game((button_t *)title_button("forgiving"));
    run_frame(&step, 0);
    assert(main_economy_typed() != NULL);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP && pad_nav_free_mode(&step.pad_nav));
    assert_cursor_at_centre(&step);
}

static void visit_two_tabs_and_come_back(input_step_t *step) {
    tap(step, P8_PAD_R1);
    tap(step, P8_PAD_R1);
    run_frame(step, P8_PAD_CIRCLE);
    assert(pad_nav_tab(&step->pad_nav) == PAD_NAV_TAB_MAP);
}

static void test_network_mode_survives_a_tab_visit(void) {
    static const int offsets[2][2] = { { 0, -12 }, { -12, 0 } };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = network_on_star(&step, offsets, 2, ends, slipways);
    tap(&step, P8_PAD_RIGHT);
    assert_selected(&step, hub, slipways[1]);
    vec_t from = camera_at();
    visit_two_tabs_and_come_back(&step);
    assert_cursor_on(&step, &hub->header);
    run_frame(&step, 0);
    assert_selected(&step, hub, slipways[1]);
    assert_moved(from, 0, 0);
}

static void test_free_mode_survives_a_tab_visit_and_pans_from_slow(void) {
    input_step_t step;
    start_game(&step);
    hold(&step, P8_PAD_RIGHT, 30);
    vec_t from = camera_at();
    run_frame(&step, P8_PAD_RIGHT | P8_PAD_R1);
    run_frame(&step, P8_PAD_RIGHT);
    tap(&step, P8_PAD_R1);
    run_frame(&step, P8_PAD_RIGHT | P8_PAD_CIRCLE);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
    assert_moved(from, 0, 0);
    run_frame(&step, P8_PAD_RIGHT);
    assert(pad_nav_free_mode(&step.pad_nav));
    assert_moved(from, 1, 0);
}

static void assert_shoulders_wait(input_step_t *step) {
    pad_nav_tab_t tab = pad_nav_tab(&step->pad_nav);
    tap(step, P8_PAD_R1);
    assert(pad_nav_tab(&step->pad_nav) == tab);
    tap(step, P8_PAD_L1);
    assert(pad_nav_tab(&step->pad_nav) == tab);
}

static void assert_final_score_unchanged(const input_step_t *step, entity_t *again) {
    entity_t *buttons[ITEMS_CAP];
    assert(pad_nav_tab(&step->pad_nav) == PAD_NAV_TAB_END_TURN && pad_nav_panel_open());
    assert(end_buttons(buttons) == 2 && !again->is_dead && pad_nav_focus(&step->pad_nav) == again);
    assert(!pause_menu_shown() && main_economy_typed() != NULL);
}

static void test_final_score_ignores_shoulders_circle_and_start(void) {
    static const int buttons[6] = { P8_PAD_L1, P8_PAD_R1, P8_PAD_CIRCLE, P8_PAD_START, P8_PAD_TRIANGLE, P8_PAD_SQUARE };
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_END_TURN);
    main_economy_typed()->year = p8_fixed_from_int(3425);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed()->year == p8_fixed_from_int(3426));
    entity_t *again = pad_nav_first_item();
    assert(again != NULL && has_text(again, " again? "));
    assert_final_score_unchanged(&step, again);
    for (int i = 0; i < 6; i++) {
        tap(&step, buttons[i]);
        tap(&step, 0);
        assert_final_score_unchanged(&step, again);
    }
}

static void test_shoulders_close_a_planet_menu_then_switch(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = network_on_known_planet(&step);
    tap(&step, P8_PAD_TRIANGLE);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, P8_PAD_R1);
    assert(main_ui()->ephemerals.count == 0 && pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    run_frame(&step, 0);
    assert(main_ui()->ephemerals.count == 0);
    tap(&step, P8_PAD_L1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
    assert_network_on(&step, planet);
}

static void test_shoulders_wait_under_the_pause_menu(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_END_TURN);
    galaxy_menu_on_click(4)(NULL);
    settle(&step);
    assert(pause_menu_shown());
    assert_shoulders_wait(&step);
    assert(pause_menu_shown());
    tap(&step, P8_PAD_CIRCLE);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_END_TURN && !pause_menu_shown());
}

static void test_shoulders_return_to_the_starting_planet_then_switch(void) {
    static const int offsets[2][2] = { { 0, -12 }, { -12, 0 } };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = network_on_star(&step, offsets, 2, ends, slipways);
    tap(&step, P8_PAD_RIGHT);
    assert_selected(&step, hub, slipways[1]);
    tap(&step, P8_PAD_SQUARE);
    assert_free_from(&step, hub);
    hold(&step, P8_PAD_RIGHT, 30);
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    assert_centred_on(&hub->header);
    run_frame(&step, 0);
    tap(&step, P8_PAD_L1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
    assert_selected(&step, hub, slipways[1]);
    assert(pad_nav_starting_planet(&step.pad_nav) == NULL);
    assert_centred_on(&hub->header);
}

/* A settled planet opens its panel on the release, so no panel is up yet on that frame. */
static void test_shoulders_wait_on_the_triangle_release_frame(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = known_planet_closest_to_centre();
    settle_planet(&step, planet);
    pan_planet_under_centre(&step, planet, 0);
    run_frame(&step, P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT && main_ui()->ephemerals.count == 0);
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
    assert(input_step_played_frame(&step)->mouse_buttons == 0 && main_ui()->ephemerals.count > 0);
}

static void test_the_ui_pad_tab_is_the_tab_hud_button(void) {
    input_step_t step;
    start_game(&step);
    assert(ui_pad_tab(main_ui()) == NULL);
    const entity_t *lit[5] = { NULL, &galaxy_menu_find_button(1)->header, &galaxy_menu_find_button(2)->header,
                               &galaxy_menu_find_button(3)->header, NULL };
    for (int i = 0; i < 5; i++) {
        tap(&step, P8_PAD_R1);
        assert(pad_nav_tab(&step.pad_nav) == forward_cycle[i]);
        assert(ui_pad_tab(main_ui()) == lit[i]);
    }
    go_to_tab(&step, PAD_NAV_TAB_TECH);
    cart_init();
    settle(&step);
    cart_start_game((button_t *)title_button("forgiving"));
    run_frame(&step, 0);
    assert(ui_pad_tab(main_ui()) == NULL);
}

static void test_no_pad_selection_is_shown_behind_a_tab(void) {
    static const int offsets[2][2] = { { 0, -12 }, { -12, 0 } };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = network_on_star(&step, offsets, 2, ends, slipways);
    assert(ui_pad_selection(main_ui()) == &hub->header && ui_pad_slipway(main_ui()) == slipways[0]);
    for (int i = 0; i < 4; i++) {
        tap(&step, P8_PAD_R1);
        assert(pad_nav_tab(&step.pad_nav) == forward_cycle[i]);
        assert(ui_pad_selection(main_ui()) == NULL && ui_pad_slipway(main_ui()) == NULL);
    }
    /* ○ leaves score through a backdrop click: its release frame shows no selection either. */
    run_frame(&step, P8_PAD_CIRCLE);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP && ui_pad_selection(main_ui()) == NULL);
    run_frame(&step, 0);
    assert(ui_pad_selection(main_ui()) == NULL);
    run_frame(&step, 0);
    assert(ui_pad_selection(main_ui()) == &hub->header && ui_pad_slipway(main_ui()) == slipways[0]);
    assert(main_ui()->pad_selection_frames == 0);
}

enum { HOVER_FILL = 12, IDLE_FILL = 13 };

/* The fill's right column, past the row drawn from one pixel in: whether all of it below the top line has that colour. */
static int filled_with(int index, int colour) {
    const button_t *button = galaxy_menu_find_button(index);
    vec_t at = entity_pos(&button->header);
    int top = p8_fixed_flr_int(at.y);
    int right = p8_fixed_flr_int(p8_fixed_sub(p8_fixed_add(at.x, button->width), p8_fixed_from_int(1)));
    for (int y = top + 1; y <= top + 9; y++) {
        if (memory.screen[y][right] != colour) {
            return 0;
        }
    }
    return 1;
}

static void assert_tech_panel_up_and_its_button_not_hovered(void) {
    assert(pad_nav_panel_open() && ui_hovered(main_ui()) != &galaxy_menu_find_button(2)->header);
}

static void test_the_tech_tab_lights_the_tech_button_under_its_panel(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_TECH);
    settle(&step);
    assert_tech_panel_up_and_its_button_not_hovered();
    cart_draw();
    assert(filled_with(2, HOVER_FILL) && filled_with(1, IDLE_FILL));
}

static void assert_clicked(const input_step_t *step, int index) {
    int x, y;
    pad_panel_focus_point(&galaxy_menu_find_button(index)->header, &x, &y);
    assert_played(step, x, y, P8_MOUSE_LEFT);
}

static int invented_count(void) {
    return main_economy_typed()->invented.count;
}

/* Stops on the first row with an action below the header the focus starts on. */
static const entity_t *focus_affordable_row(input_step_t *step) {
    for (int i = 0; i < ITEMS_CAP; i++) {
        const entity_t *focus = pad_nav_focus(&step->pad_nav);
        assert(focus != NULL);
        if (((const button_t *)focus)->on_click != NULL) {
            return focus;
        }
        tap(step, P8_PAD_DOWN);
    }
    assert(0 && "focus_affordable_row: no affordable row in the tech box");
    return NULL;
}

static void test_landing_on_tech_opens_its_panel_and_an_invention_returns_to_the_map(void) {
    input_step_t step;
    start_game(&step);
    main_economy_typed()->research_points = p8_fixed_from_int(5);
    go_to_tab(&step, PAD_NAV_TAB_END_TURN);
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_TECH);
    assert_clicked(&step, 2);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, 0);
    assert(pad_nav_panel_open() && pad_nav_focus(&step.pad_nav) == pad_nav_first_item());
    assert(((const button_t *)pad_nav_focus(&step.pad_nav))->on_click == NULL);
    tap(&step, P8_PAD_DOWN);
    (void)focus_affordable_row(&step);
    p8_fixed_t research = main_economy_typed()->research_points;
    run_frame(&step, P8_PAD_CROSS);
    assert(invented_count() == 1 && main_economy_typed()->research_points < research);
    assert(main_ui()->ephemerals.count == 0 && pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_TECH);
    run_frame(&step, 0);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
}

static void test_cross_on_the_tech_header_closes_the_box_and_returns_to_the_map(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_TECH);
    assert(main_ui()->ephemerals.count > 0 && ((const button_t *)pad_nav_focus(&step.pad_nav))->on_click == NULL);
    tap(&step, P8_PAD_CROSS);
    assert(invented_count() == 0 && main_ui()->ephemerals.count == 0);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
}

static void test_circle_on_tech_closes_the_box_and_returns_to_the_map_in_one_press(void) {
    static const int offsets[2][2] = { { 0, -12 }, { -12, 0 } };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = network_on_star(&step, offsets, 2, ends, slipways);
    tap(&step, P8_PAD_RIGHT);
    vec_t from = camera_at();
    go_to_tab(&step, PAD_NAV_TAB_TECH);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP && main_ui()->ephemerals.count == 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    run_frame(&step, P8_PAD_CIRCLE);
    run_frame(&step, 0);
    assert(main_ui()->ephemerals.count == 0);
    assert_selected(&step, hub, slipways[1]);
    assert_moved(from, 0, 0);
}

static int score_panel_shown(void) {
    const score_panel_t *panel = main_score_panel();
    return panel->has_is_shown && panel->is_shown;
}

/* NULL once the score panel's backdrop is gone. */
static entity_t *backdrop(void) {
    return live_of_class(modal_backdrop_class());
}

static void open_score_from_the_map(input_step_t *step) {
    assert(!score_panel_shown() && backdrop() == NULL);
    run_frame(step, P8_PAD_L1);
    assert(pad_nav_tab(&step->pad_nav) == PAD_NAV_TAB_SCORE);
    assert_clicked(step, 3);
    assert(score_panel_shown() && backdrop() != NULL);
    run_frame(step, 0);
}

static void test_landing_on_score_shows_its_panel_and_cross_presses_nothing(void) {
    static const int buttons[5] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT, P8_PAD_CROSS };
    input_step_t step;
    start_game(&step);
    open_score_from_the_map(&step);
    entity_t *shown_backdrop = backdrop();
    for (int b = 0; b < 5; b++) {
        hold(&step, buttons[b], 3);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        run_frame(&step, 0);
    }
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_SCORE && score_panel_shown() && backdrop() == shown_backdrop);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP && !score_panel_shown());
    run_frame(&step, 0);
    assert(backdrop() == NULL && pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP && pad_nav_free_mode(&step.pad_nav));
}

static void test_l1_on_score_hands_over_to_tech_on_the_third_frame(void) {
    input_step_t step;
    start_game(&step);
    open_score_from_the_map(&step);
    run_frame(&step, P8_PAD_L1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_TECH && !score_panel_shown());
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    run_frame(&step, 0);
    assert(backdrop() == NULL && main_ui()->ephemerals.count == 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    run_frame(&step, 0);
    assert_clicked(&step, 2);
    assert(main_ui()->ephemerals.count > 0 && pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_TECH);
}

static void test_r1_on_tech_closes_the_box_and_shows_score_on_the_same_frame(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_TECH);
    assert(main_ui()->ephemerals.count > 0 && !score_panel_shown());
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_SCORE);
    assert_clicked(&step, 3);
    assert(main_ui()->ephemerals.count == 0 && score_panel_shown() && backdrop() != NULL);
}

static void test_r1_on_score_hides_it_and_returns_to_the_map(void) {
    input_step_t step;
    start_game(&step);
    open_score_from_the_map(&step);
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP && !score_panel_shown());
    run_frame(&step, 0);
    assert(backdrop() == NULL && pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
}

static void test_a_tab_whose_button_is_gone_returns_to_the_map(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_END_TURN);
    galaxy_menu_find_button(2)->header.is_dead = 1;
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_TECH && input_step_played_frame(&step)->mouse_buttons == 0);
    run_frame(&step, 0);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP && main_ui()->ephemerals.count == 0);
}

static void test_circle_leaving_score_is_back_in_free_mode_before_the_next_frame(void) {
    input_step_t step;
    start_game(&step);
    open_score_from_the_map(&step);
    run_frame(&step, P8_PAD_CIRCLE);
    entity_t *dead = backdrop();
    assert(dead != NULL && dead->is_dead);
    assert(!pad_nav_panel_open() && pad_nav_free_mode(&step.pad_nav));
}

/* ○ leaves score by a backdrop click; its release frame on the map must emit no press. */
static void leave_score_then_triangle_on_the_release_frame(input_step_t *step) {
    open_score_from_the_map(step);
    run_frame(step, P8_PAD_CIRCLE);
    assert(pad_nav_tab(&step->pad_nav) == PAD_NAV_TAB_MAP);
    assert(!pad_nav_panel_open());
    run_frame(step, P8_PAD_TRIANGLE | P8_PAD_CROSS);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    run_frame(step, 0);
    assert(input_step_played_frame(step)->mouse_buttons == 0 && main_ui()->ephemerals.count == 0);
    assert(backdrop() == NULL && tagged_count(TAG_PROBE) == 0);
}

static void test_triangle_on_the_release_frame_of_leaving_score_does_nothing(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = known_planet_under_centre(&step);
    if (in_forked_copy()) {
        leave_score_then_triangle_on_the_release_frame(&step);
        assert(pad_nav_free_mode(&step.pad_nav) && pad_nav_selection(&step.pad_nav) == NULL);
        tap(&step, P8_PAD_TRIANGLE);
        assert(main_ui()->ephemerals.count > 0 && pad_nav_selection(&step.pad_nav) == &planet->header);
        end_forked_copy();
    }
    (void)network_on_known_planet(&step);
    run_frame(&step, 0);
    leave_score_then_triangle_on_the_release_frame(&step);
    assert(pad_nav_selection(&step.pad_nav) == &planet->header);
    tap(&step, P8_PAD_TRIANGLE);
    assert(main_ui()->ephemerals.count > 0);
}

static void pause_button_point(int *x, int *y) {
    pad_panel_focus_point(&galaxy_menu_find_button(4)->header, x, y);
}

static void assert_restart_focused_and_hovered(const input_step_t *step) {
    entity_t *restart = pad_nav_first_item();
    assert(pause_menu_shown() && restart != NULL && ((const button_t *)restart)->on_click == score_panel_init_field_value());
    assert(pad_nav_focus(&step->pad_nav) == restart && ui_hovered(main_ui()) == restart);
}

/* Start tapped with the left button up: the pause button pressed on that frame, released on the next, and the
 * cursor on "restart" from that release, hovered from the frame after. */
static void open_pause(input_step_t *step) {
    int x, y;
    pause_button_point(&x, &y);
    assert(!pause_menu_shown());
    run_frame(step, P8_PAD_START);
    assert_played(step, x, y, P8_MOUSE_LEFT);
    run_frame(step, 0);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    assert(pause_menu_shown() && pad_nav_focus(&step->pad_nav) == pad_nav_first_item());
    run_frame(step, 0);
    assert_restart_focused_and_hovered(step);
}

static void test_restart_from_pause_with_a_save_shows_resume(void) {
    input_step_t step;
    start_game(&step);
    economy_new_year(main_economy_typed());
    run_frame(&step, 0);
    open_pause(&step);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() == NULL);
    assert(title_button("resume") != NULL);
    assert_focused_and_hovered(&step, "forgiving");
}

static void test_start_opens_pause_from_free_mode(void) {
    input_step_t step;
    start_game(&step);
    vec_t from = camera_at();
    open_pause(&step);
    settle(&step);
    assert_restart_focused_and_hovered(&step);
    assert_moved(from, 0, 0);
    assert(tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0 && ui_left_held(main_ui()) == NULL);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
}

/* Start or ○ with pause up: a press at the centre, off every row, and its release on the next frame. */
static void close_pause(input_step_t *step, int button) {
    assert(pause_menu_shown());
    run_frame(step, button);
    assert_played(step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, P8_MOUSE_LEFT);
    assert(!pause_menu_shown() && main_ui()->ephemerals.count == 0);
    run_frame(step, 0);
    assert(input_step_played_frame(step)->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
}

static void test_start_and_circle_close_pause_pressing_nothing_on_the_map(void) {
    static const int buttons[2] = { P8_PAD_START, P8_PAD_CIRCLE };
    input_step_t step;
    start_game(&step);
    for (int b = 0; b < 2; b++) {
        if (in_forked_copy()) {
            pan_to_empty_space(&step);
            vec_t from = camera_at();
            world_count_t before = world_count();
            int live = live_entities()->count;
            open_pause(&step);
            close_pause(&step, buttons[b]);
            settle(&step);
            assert(live_entities()->count == live && tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0);
            assert(main_ui()->ephemerals.count == 0 && ui_left_held(main_ui()) == NULL && !pad_nav_panel_open());
            assert(main_economy_typed()->credits == before.credits);
            assert(pad_nav_free_mode(&step.pad_nav));
            assert_cursor_at_centre(&step);
            assert_moved(from, 0, 0);
            end_forked_copy();
        }
    }
}

static void test_pause_from_network_mode_gives_back_the_planet_slipway_and_camera(void) {
    static const int offsets[2][2] = { { 0, -12 }, { -12, 0 } };
    input_step_t step;
    planet_t *ends[2];
    entity_t *slipways[2];
    start_game(&step);
    planet_t *hub = network_on_star(&step, offsets, 2, ends, slipways);
    tap(&step, P8_PAD_RIGHT);
    assert_selected(&step, hub, slipways[1]);
    vec_t from = camera_at();
    open_pause(&step);
    tap(&step, P8_PAD_UP);
    close_pause(&step, P8_PAD_START);
    assert_cursor_on(&step, &hub->header);
    run_frame(&step, 0);
    assert_selected(&step, hub, slipways[1]);
    assert(ui_pad_selection(main_ui()) == &hub->header);
    assert_moved(from, 0, 0);
}

static void test_pause_from_end_turn_and_build_keeps_the_tab_and_its_cursor(void) {
    input_step_t step;
    start_game(&step);
    if (in_forked_copy()) {
        go_to_tab(&step, PAD_NAV_TAB_END_TURN);
        year_facts_t before = year_facts();
        int x, y;
        year_skip_point(&x, &y);
        open_pause(&step);
        close_pause(&step, P8_PAD_START);
        assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_END_TURN);
        assert_played(&step, x, y, 0);
        run_frame(&step, 0);
        assert(ui_hovered(main_ui()) == &galaxy_menu_find_button(1)->header);
        assert_years_ended(before, 0);
        end_forked_copy();
    }
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    tap(&step, P8_PAD_LEFT);
    entity_t *icon = processor_icon();
    assert(pad_nav_build_icon(&step.pad_nav) == icon);
    int live = live_entities()->count;
    open_pause(&step);
    close_pause(&step, P8_PAD_START);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    run_frame(&step, 0);
    assert_build_focus(&step, icon);
    assert(live_entities()->count == live && pad_nav_placing(&step.pad_nav) == NULL);
}

static void test_pause_keeps_the_starting_planet_for_the_next_circle(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = network_on_known_planet(&step);
    tap(&step, P8_PAD_SQUARE);
    hold(&step, P8_PAD_RIGHT, 20);
    run_frame(&step, 0);
    vec_t from = camera_at();
    assert_free_from(&step, planet);
    open_pause(&step);
    close_pause(&step, P8_PAD_CIRCLE);
    assert_free_from(&step, planet);
    assert_moved(from, 0, 0);
    tap(&step, P8_PAD_CIRCLE);
    assert_network_on(&step, planet);
}

/* × on "toggle music", "toggle sfx" and "how to play?" closes pause as the mouse click does, and opens nothing. */
static void test_cross_on_each_row_but_restart_closes_pause(void) {
    input_step_t step;
    start_game(&step);
    for (int up = 1; up <= 3; up++) {
        if (in_forked_copy()) {
            int boxes = message_box_count();
            open_pause(&step);
            for (int i = 0; i < up; i++) {
                tap(&step, P8_PAD_UP);
            }
            entity_t *row = pad_nav_focus(&step.pad_nav);
            assert(row != NULL && ((const button_t *)row)->on_click != score_panel_init_field_value());
            tap(&step, P8_PAD_CROSS);
            settle(&step);
            assert(!pause_menu_shown() && !pad_nav_panel_open() && main_economy_typed() != NULL);
            assert(message_box_count() == boxes && tagged_count(TAG_PROBE) == 0);
            assert(pad_nav_free_mode(&step.pad_nav));
            end_forked_copy();
        }
    }
}

static void test_cross_on_restart_gives_the_title_then_a_new_game_on_the_map(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_END_TURN);
    open_pause(&step);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() == NULL && pad_nav_panel_open());
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
    assert_focused_and_hovered(&step, "forgiving");
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() != NULL && !pad_nav_panel_open());
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP && pad_nav_free_mode(&step.pad_nav));
}

static void test_the_d_pad_on_pause_moves_among_its_rows_and_no_camera(void) {
    entity_t *entries[ITEMS_CAP];
    input_step_t step;
    start_game(&step);
    vec_t from = camera_at();
    open_pause(&step);
    assert(box_entries(entries) == 4);
    entity_t *restart = pad_nav_focus(&step.pad_nav);
    tap(&step, P8_PAD_DOWN);
    assert(pad_nav_focus(&step.pad_nav) == restart);
    hold(&step, P8_PAD_LEFT, 12);
    hold(&step, P8_PAD_RIGHT, 12);
    run_frame(&step, 0);
    assert(pad_nav_focus(&step.pad_nav) == restart);
    entity_t *seen[4] = { restart, NULL, NULL, NULL };
    for (int i = 1; i < 4; i++) {
        hold(&step, P8_PAD_UP, 12);
        run_frame(&step, 0);
        seen[i] = pad_nav_focus(&step.pad_nav);
        assert(seen[i] != NULL && seen[i] != seen[i - 1] && ui_hovered(main_ui()) == seen[i]);
    }
    tap(&step, P8_PAD_UP);
    assert(pad_nav_focus(&step.pad_nav) == seen[3]);
    for (int i = 2; i >= 0; i--) {
        tap(&step, P8_PAD_DOWN);
        assert(pad_nav_focus(&step.pad_nav) == seen[i]);
    }
    assert(pause_menu_shown());
    assert_moved(from, 0, 0);
}

/* On end turn × is the held left button: Start lets it up first, and the click on the pause button follows a frame later. */
static void test_cross_held_into_pause_presses_no_row_until_pressed_again(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_END_TURN);
    year_facts_t before = year_facts();
    hold(&step, P8_PAD_CROSS, 5);
    assert_years_ended(before, 1);
    int x, y;
    pause_button_point(&x, &y);
    run_frame(&step, P8_PAD_CROSS | P8_PAD_START);
    assert(input_step_played_frame(&step)->mouse_buttons == 0 && !pause_menu_shown());
    run_frame(&step, P8_PAD_CROSS);
    assert_played(&step, x, y, P8_MOUSE_LEFT);
    for (int i = 0; i < 20; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        assert(pause_menu_shown());
    }
    assert_restart_focused_and_hovered(&step);
    assert_years_ended(before, 1);
    run_frame(&step, 0);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() == NULL);
}

static void test_the_cross_that_closes_pause_on_end_turn_ends_no_year(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_END_TURN);
    year_facts_t before = year_facts();
    open_pause(&step);
    tap(&step, P8_PAD_UP);
    for (int i = 0; i < 20; i++) {
        run_frame(&step, P8_PAD_CROSS);
    }
    assert(!pause_menu_shown() && pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_END_TURN);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert_years_ended(before, 0);
    run_frame(&step, 0);
    tap(&step, P8_PAD_CROSS);
    assert_years_ended(before, 1);
}

/* Start's edge goes on this frame and the next stays clear: no pause click follows. */
static void assert_start_ignored(input_step_t *step, int held) {
    run_frame(step, held | P8_PAD_START);
    run_frame(step, held);
    assert(!pause_menu_shown());
}

static void test_start_is_ignored_on_the_title(void) {
    input_step_t step;
    start_title(&step);
    tap(&step, P8_PAD_DOWN);
    assert_start_ignored(&step, 0);
    settle(&step);
    assert(main_economy_typed() == NULL && pad_nav_panel_open());
    assert_focused_and_hovered(&step, "reasonable");
}

static void test_start_is_ignored_under_a_planet_menu_and_on_tech_and_score(void) {
    input_step_t step;
    start_game(&step);
    if (in_forked_copy()) {
        planet_t *planet = network_on_known_planet(&step);
        tap(&step, P8_PAD_TRIANGLE);
        int rows = main_ui()->ephemerals.count;
        assert(rows > 0);
        assert_start_ignored(&step, 0);
        settle(&step);
        assert(main_ui()->ephemerals.count == rows && !pause_menu_shown());
        assert_network_on(&step, planet);
        end_forked_copy();
    }
    if (in_forked_copy()) {
        go_to_tab(&step, PAD_NAV_TAB_TECH);
        int rows = main_ui()->ephemerals.count;
        assert(rows > 0);
        assert_start_ignored(&step, 0);
        settle(&step);
        assert(main_ui()->ephemerals.count == rows && !pause_menu_shown());
        assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_TECH);
        end_forked_copy();
    }
    open_score_from_the_map(&step);
    assert_start_ignored(&step, 0);
    settle(&step);
    assert(score_panel_shown() && !pause_menu_shown() && pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_SCORE);
}

static void test_start_is_ignored_while_cross_holds_a_fold(void) {
    input_step_t step;
    start_game(&step);
    pan_to_empty_space(&step);
    tap(&step, P8_PAD_CROSS);
    hold(&step, P8_PAD_CROSS, 3);
    entity_t *held = ui_left_held(main_ui());
    assert(held != NULL && held->class == wormhole_class());
    assert_start_ignored(&step, P8_PAD_CROSS);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    assert_cursor_at_centre(&step);
    assert(ui_left_held(main_ui()) == held && main_ui()->ephemerals.count == 0);
}

static void test_start_is_ignored_on_the_triangle_release_frame(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = known_planet_closest_to_centre();
    settle_planet(&step, planet);
    pan_planet_under_centre(&step, planet, 0);
    run_frame(&step, P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT && main_ui()->ephemerals.count == 0);
    run_frame(&step, P8_PAD_START);
    assert_cursor_on(&step, &planet->header);
    run_frame(&step, 0);
    assert(!pause_menu_shown() && main_ui()->ephemerals.count > 0);
    assert_network_on(&step, planet);
}

/* Once an invention closes the tech box, the tab returns to the map on the next frame: Start there opens nothing. */
static void test_start_is_ignored_on_the_frame_tech_returns_to_the_map(void) {
    input_step_t step;
    start_game(&step);
    main_economy_typed()->research_points = p8_fixed_from_int(5);
    go_to_tab(&step, PAD_NAV_TAB_TECH);
    tap(&step, P8_PAD_DOWN);
    (void)focus_affordable_row(&step);
    run_frame(&step, P8_PAD_CROSS);
    assert(invented_count() == 1 && main_ui()->ephemerals.count == 0 && pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_TECH);
    assert_start_ignored(&step, 0);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP && pad_nav_free_mode(&step.pad_nav));
}

/* The frame after the drop is the landing's: Start and the shoulders wait for it. */
static void test_start_and_shoulders_wait_on_the_landing_frame(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    for (int at = 0; at < 2; at++) {
        if (in_forked_copy()) {
            world_count_t before = world_count();
            (void)drop_on(&step, start, target);
            run_frame(&step, P8_PAD_CROSS | (at == 0 ? P8_PAD_START : P8_PAD_R1));
            assert(!pause_menu_shown() && pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
            run_frame(&step, 0);
            assert(!pause_menu_shown() && pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
            assert_built_and_landed(&step, start, target, before);
            end_forked_copy();
        }
    }
}

/* Start lets the drag go nowhere, then opens pause; closing pause presses the start again. */
static void test_start_while_dragging_drops_the_slipway_opens_pause_and_drags_again_after(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    pan_planet_under_centre(&step, target, 0);
    assert_dragging(&step, start);
    vec_t camera = camera_at();
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_START);
    const p8_input_frame_t *played = input_step_played_frame(&step);
    assert(played->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
    int x, y;
    run_frame(&step, 0);
    pause_button_point(&x, &y);
    assert_played(&step, x, y, P8_MOUSE_LEFT);
    run_frame(&step, 0);
    assert(pause_menu_shown() && ui_left_held(main_ui()) == NULL);
    close_pause(&step, P8_PAD_CIRCLE);
    assert(!pause_menu_shown());
    for (int i = 0; i < 3; i++) {
        run_frame(&step, 0);
    }
    assert_dragging(&step, start);
    assert_moved(camera, 0, 0);
    assert_world_unchanged(before);
}

/* The shoulders let the drag go nowhere, return to the starting planet, then switch. */
static void test_shoulders_drop_the_drag_return_to_the_starting_planet_then_switch(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game(&step);
    find_pair(near_pair, &start, &target);
    free_from_settled(&step, start, target, 1);
    pan_planet_under_centre(&step, target, 0);
    assert_dragging(&step, start);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_R1);
    const p8_input_frame_t *played = input_step_played_frame(&step);
    assert(played->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
    run_frame(&step, 0);
    assert(ui_left_held(main_ui()) == NULL);
    tap(&step, P8_PAD_L1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
    assert_network_on(&step, start);
    settle(&step);
    assert(ui_left_held(main_ui()) == NULL && main_ui()->ephemerals.count == 0);
    assert_world_unchanged(before);
}

static void test_start_with_r1_opens_pause_and_keeps_the_tab(void) {
    input_step_t step;
    start_game(&step);
    int x, y;
    pause_button_point(&x, &y);
    run_frame(&step, P8_PAD_START | P8_PAD_R1);
    assert_played(&step, x, y, P8_MOUSE_LEFT);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
    run_frame(&step, 0);
    run_frame(&step, 0);
    assert_restart_focused_and_hovered(&step);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_MAP);
}

static void test_start_opens_pause_over_the_year_box(void) {
    input_step_t step;
    start_game(&step);
    score_panel_new_year(p8_fixed_from_int(3421));
    entity_t *year_box = &shown_message_box()->header;
    run_frame(&step, 0);
    open_pause(&step);
    assert(entity_list_has(live_entities(), year_box) && !pad_nav_is_item(year_box));
    close_pause(&step, P8_PAD_START);
    assert(pad_nav_free_mode(&step.pad_nav));
}

/* Start lets the carried structure go before the pause click, so nothing is dropped on the pause button. */
static void suspend_into_pause(input_step_t *step, const entity_t *structure) {
    int x, y;
    pause_button_point(&x, &y);
    run_frame(step, P8_PAD_START);
    assert_played(step, x, y, 0);
    assert(ui_left_held(main_ui()) == NULL && pad_nav_placing(&step->pad_nav) == NULL);
    run_frame(step, 0);
    assert_played(step, x, y, P8_MOUSE_LEFT);
    run_frame(step, 0);
    run_frame(step, 0);
    assert_restart_focused_and_hovered(step);
    assert(ui_left_held(main_ui()) == NULL && pad_nav_placing(&step->pad_nav) == NULL);
    assert(!structure->is_dead && structure->state == ENTITY_STATE_PLACING);
}

static void close_pause_into_the_carry(input_step_t *step, const entity_t *structure) {
    run_frame(step, P8_PAD_START);
    assert_played(step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, P8_MOUSE_LEFT);
    assert(!pause_menu_shown());
    run_frame(step, 0);
    assert_carrying(step, structure);
}

static void test_start_while_carrying_suspends_the_structure_under_pause(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    entity_t *structure = take_structure(&step);
    pan_clear_of_every_node(&step, structure);
    vec_t at = entity_pos(structure);
    world_count_t before = world_count();
    suspend_into_pause(&step, structure);
    for (int i = 0; i < 3; i++) {
        hold(&step, P8_PAD_UP, 12);
        run_frame(&step, 0);
    }
    hold(&step, P8_PAD_DOWN, 12);
    settle(&step);
    assert(pause_menu_shown() && pad_nav_focus(&step.pad_nav) != pad_nav_first_item());
    vec_t now = entity_pos(structure);
    assert(now.x == at.x && now.y == at.y);
    assert(!structure->is_dead && structure->state == ENTITY_STATE_PLACING);
    assert_world_unchanged(before);
    assert(ui_left_held(main_ui()) == NULL && pad_nav_placing(&step.pad_nav) == NULL);
}

/* Start and ○ press off the rows at the centre, × presses "toggle music": the left button is down on the closing frame. */
static void test_closing_pause_resumes_the_carry_where_it_was(void) {
    static const int buttons[3] = { P8_PAD_START, P8_PAD_CIRCLE, P8_PAD_CROSS };
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    entity_t *structure = take_structure(&step);
    pan_clear_of_every_node(&step, structure);
    for (int b = 0; b < 3; b++) {
        if (!in_forked_copy()) {
            continue;
        }
        world_count_t before = world_count();
        suspend_into_pause(&step, structure);
        if (buttons[b] == P8_PAD_CROSS) {
            tap(&step, P8_PAD_UP);
            assert(has_text(pad_nav_focus(&step.pad_nav), "toggle music"));
        }
        run_frame(&step, buttons[b]);
        assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT && !pause_menu_shown());
        assert(ui_left_held(main_ui()) == NULL && !structure->is_dead);
        run_frame(&step, 0);
        assert_carrying(&step, structure);
        assert_world_unchanged(before);
        if (in_forked_copy()) {
            run_frame(&step, P8_PAD_DOWN);
            assert_carrying(&step, structure);
            pan_clear_of_every_node(&step, structure);
            p8_fixed_t cost = targeting_of(structure)->cost;
            tap(&step, P8_PAD_CROSS);
            assert(!structure->is_dead && structure->state == ENTITY_STATE_SETTLED);
            assert(main_economy_typed()->credits == p8_fixed_sub(before.credits, cost));
            assert_placement_over(&step);
            end_forked_copy();
        }
        tap(&step, P8_PAD_CIRCLE);
        assert(structure->is_dead && main_economy_typed()->credits == before.credits);
        assert_placement_over(&step);
        end_forked_copy();
    }
}

/* The suspension waits on a placement phase: left behind, R1 would keep waiting on the map of the new game. */
static void test_restart_under_pause_drops_the_suspended_structure(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    entity_t *structure = take_structure(&step);
    suspend_into_pause(&step, structure);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() == NULL && pad_nav_placing(&step.pad_nav) == NULL && ui_left_held(main_ui()) == NULL);
    assert_focused_and_hovered(&step, "forgiving");
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() != NULL && pad_nav_free_mode(&step.pad_nav));
    assert(pad_nav_placing(&step.pad_nav) == NULL && ui_left_held(main_ui()) == NULL);
    tap(&step, P8_PAD_R1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_BUILD);
}

static void test_pause_while_carrying_from_network_mode_keeps_the_starting_planet(void) {
    input_step_t step;
    start_game(&step);
    planet_t *colony = known_planet_closest_to_centre();
    settle_planet(&step, colony);
    pan_planet_under_centre(&step, colony, 0);
    tap(&step, P8_PAD_TRIANGLE);
    tap(&step, P8_PAD_CIRCLE);
    assert_network_on(&step, colony);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    entity_t *structure = take_structure(&step);
    assert_free_from(&step, colony);
    suspend_into_pause(&step, structure);
    assert(pad_nav_starting_planet(&step.pad_nav) == &colony->header);
    close_pause_into_the_carry(&step, structure);
    assert_free_from(&step, colony);
    tap(&step, P8_PAD_CIRCLE);
    assert(structure->is_dead);
    assert_free_from(&step, colony);
    tap(&step, P8_PAD_CIRCLE);
    assert_network_on(&step, colony);
}

/* ui_clear_ephemerals would close pause there with the left button up, so the resume's held button would be a press on the map. */
static void test_circle_on_the_release_frame_of_the_pause_click_is_dropped(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    entity_t *structure = take_structure(&step);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_START);
    run_frame(&step, 0);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    run_frame(&step, 0);
    run_frame(&step, 0);
    assert_restart_focused_and_hovered(&step);
    assert(!structure->is_dead && structure->state == ENTITY_STATE_PLACING && ui_left_held(main_ui()) == NULL);
    assert_world_unchanged(before);
    close_pause_into_the_carry(&step, structure);
}

/* On the map, L1 to score clicks the score button on its own frame; a cancel's release takes that frame first. */
static void test_the_landing_after_a_shoulder_cancel_waits_for_its_release(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    int x, y;
    pad_panel_focus_point(basic_structure_icon(), &x, &y);
    entity_t *structure = take_structure(&step);
    run_frame(&step, P8_PAD_L1);
    assert(pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_SCORE);
    assert_played(&step, x, y, 0);
    assert(!score_panel_shown());
    run_frame(&step, 0);
    assert(structure->is_dead);
    assert_clicked(&step, 3);
    assert(score_panel_shown() && backdrop() != NULL);
    run_frame(&step, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0 && pad_nav_tab(&step.pad_nav) == PAD_NAV_TAB_SCORE);
}

static void test_pad_free_mode_draws_a_crosshair_at_the_centre(void) {
    input_step_t step;
    start_game(&step);
    assert(pad_nav_free_mode(&step.pad_nav));
    assert_played(&step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
    cart_draw();
    assert(crosshair_pixels_lit() == 4 * CROSSHAIR_ARM);
}

static void assert_no_crosshair_drawn(void) {
    cart_draw();
    assert(crosshair_pixels_lit() < 4 * CROSSHAIR_ARM);
}

static void test_pad_pause_draws_no_crosshair(void) {
    input_step_t step;
    start_game(&step);
    open_pause(&step);
    assert_no_crosshair_drawn();
}

static void test_a_carried_structure_draws_no_crosshair(void) {
    input_step_t step;
    start_game(&step);
    go_to_tab(&step, PAD_NAV_TAB_BUILD);
    entity_t *structure = take_structure(&step);
    pan_clear_of_every_node(&step, structure);
    assert(pad_nav_free_mode(&step.pad_nav));
    assert_no_crosshair_drawn();
}

static void test_network_mode_draws_no_crosshair(void) {
    input_step_t step;
    start_game(&step);
    network_on_known_planet(&step);
    assert_no_crosshair_drawn();
}

static void test_a_planet_menu_draws_no_crosshair(void) {
    input_step_t step;
    start_game(&step);
    known_planet_under_centre(&step);
    tap(&step, P8_PAD_TRIANGLE);
    run_frame(&step, 0);
    assert(pad_nav_panel_open() && ui_hovered(main_ui()) == pad_nav_first_item());
    assert_no_crosshair_drawn();
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
    in_fresh_child(test_title_with_a_save_still_focuses_forgiving);
    in_fresh_child(test_title_up_reaches_resume_and_down_comes_back);
    in_fresh_child(test_title_cross_on_resume_resumes_the_saved_game);
    in_fresh_child(test_restart_from_pause_with_a_save_shows_resume);
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
    in_fresh_child(test_triangle_on_a_known_planet_opens_its_menu);
    in_fresh_child(test_triangle_on_empty_space_or_an_unexplored_planet_does_nothing);
    in_fresh_child(test_triangle_on_a_settled_planet_opens_its_inspection_panel);
    in_fresh_child(test_colonising_through_the_menu_lands_network_mode_on_the_planet);
    in_fresh_child(test_circle_closes_the_menu_and_stays_on_the_planet);
    in_fresh_child(test_network_mode_ignores_cross_directions_and_the_unbound_buttons);
    in_fresh_child(test_triangle_in_network_mode_reopens_the_menu);
    in_fresh_child(test_triangle_click_releases_even_with_cross_held);
    in_fresh_child(test_circle_on_the_title_does_nothing);
    in_fresh_child(test_circle_closes_the_pause_menu);
    in_fresh_child(test_triangle_while_cross_holds_a_fold_does_nothing);
    in_fresh_child(test_a_dead_selection_falls_back_to_free_mode);
    in_fresh_child(test_a_new_game_starts_in_free_mode);
    in_fresh_child(test_the_ui_pad_selection_is_the_planet_only_in_network_mode_without_a_box);
    in_fresh_child(test_square_in_network_mode_enters_free_mode_on_the_planet);
    in_fresh_child(test_square_in_free_mode_forgets_the_starting_planet_then_changes_nothing);
    in_fresh_child(test_square_on_a_colony_holds_its_slipway_at_the_centre);
    in_fresh_child(test_cross_on_a_colony_builds_a_slipway_and_lands_on_it);
    in_fresh_child(test_cross_builds_from_an_off_screen_starting_planet);
    in_fresh_child(test_cross_on_a_known_planet_is_refused);
    in_fresh_child(test_cross_on_a_planet_too_far_is_refused);
    in_fresh_child(test_cross_without_the_credits_is_refused);
    in_fresh_child(test_cross_on_an_already_linked_planet_is_refused);
    in_fresh_child(test_cross_on_the_starting_planet_itself_keeps_the_drag);
    in_fresh_child(test_cross_from_a_known_starting_planet_presses_nothing);
    in_fresh_child(test_square_presses_a_starting_planet_the_hud_covered);
    in_fresh_child(test_no_drag_starts_again_while_the_hud_covers_the_starting_planet);
    in_fresh_child(test_cross_on_empty_space_drops_the_slipway_then_places_the_probe);
    in_fresh_child(test_a_one_frame_cross_on_empty_space_places_the_probe);
    in_fresh_child(test_circle_drops_the_drag_and_returns_to_the_starting_planet_and_its_slipway);
    in_fresh_child(test_triangle_while_dragging_opens_the_menu_under_the_centre);
    in_fresh_child(test_colonising_from_a_drag_resumes_it_aimed_at_the_colony);
    in_fresh_child(test_a_panel_opening_drops_the_drag);
    in_fresh_child(test_square_while_dragging_drops_the_drag_into_plain_free_mode);
    in_fresh_child(test_cross_in_plain_free_mode_places_the_probe);
    in_fresh_child(test_triangle_in_plain_free_mode_opens_the_menu_and_lands_on_the_planet);
    in_fresh_child(test_circle_in_plain_free_mode_returns_to_the_last_planet_and_its_slipway);
    in_fresh_child(test_circle_in_plain_free_mode_does_nothing_once_the_last_planet_is_dead);
    in_fresh_child(test_square_in_plain_free_mode_selects_the_settled_planet_under_the_centre);
    in_fresh_child(test_square_while_dragging_selects_another_settled_planet_under_the_centre);
    in_fresh_child(test_square_while_dragging_over_a_known_planet_leaves_for_plain_free_mode);
    in_fresh_child(test_square_while_cross_holds_a_fold_selects_nothing);
    in_fresh_child(test_shoulders_in_plain_free_mode_switch_and_come_back_to_it);
    in_fresh_child(test_circle_first_closes_a_box_then_returns);
    in_fresh_child(test_a_dead_starting_planet_drops_the_drag_and_is_forgotten);
    in_fresh_child(test_slipways_turn_clockwise_from_straight_up);
    in_fresh_child(test_straight_up_splits_the_two_halves);
    in_fresh_child(test_left_and_right_turn_the_selected_slipway);
    in_fresh_child(test_cross_travels_along_the_selected_slipway);
    in_fresh_child(test_up_and_down_do_nothing_in_network_mode);
    in_fresh_child(test_circle_lands_with_a_slipway_selected);
    in_fresh_child(test_circle_restores_the_slipway_turned_to_before_square);
    in_fresh_child(test_cross_on_the_triangle_release_frame_is_ignored);
    in_fresh_child(test_the_ui_pad_slipway_is_shown_with_the_planet);
    in_fresh_child(test_network_mode_hides_the_cursor_and_blinks_a_circle);
    in_fresh_child(test_the_selected_slipway_is_drawn_bright);
    in_fresh_child(test_r1_and_l1_cycle_the_tabs);
    in_fresh_child(test_a_held_shoulder_moves_one_tab_and_both_together_move_none);
    in_fresh_child(test_build_focuses_the_basic_structure_icon_from_the_tab_change_frame);
    in_fresh_child(test_build_left_and_right_move_between_the_icons_without_wrapping);
    in_fresh_child(test_build_left_reaches_every_revealed_icon_in_decreasing_x);
    in_fresh_child(test_build_remembers_its_icon_until_a_new_game);
    in_fresh_child(test_build_up_down_square_and_triangle_press_nothing);
    in_fresh_child(test_circle_on_a_tab_returns_to_the_map);
    in_fresh_child(test_cross_takes_the_structure_and_a_second_cross_places_it);
    in_fresh_child(test_the_cross_that_takes_the_structure_drops_nothing_while_held);
    in_fresh_child(test_a_refused_drop_kills_the_structure_and_charges_nothing);
    in_fresh_child(test_circle_cancels_the_structure_and_charges_nothing);
    in_fresh_child(test_a_panel_opened_while_carrying_releases_at_the_icon);
    in_fresh_child(test_taking_from_network_mode_enters_free_mode_from_the_selected_planet);
    in_fresh_child(test_triangle_and_square_wait_while_carrying);
    in_fresh_child(test_shoulders_cancel_the_carried_structure_then_switch);
    in_fresh_child(test_shoulders_cancel_a_structure_taken_in_network_mode_and_land_on_its_planet);
    in_fresh_child(test_cross_held_on_arrival_on_build_takes_nothing);
    in_fresh_child(test_a_press_that_hands_over_no_structure_stays_on_build);
    in_fresh_child(test_end_turn_focuses_and_hovers_the_year_skip_button);
    in_fresh_child(test_cross_on_end_turn_ends_one_year_per_press);
    in_fresh_child(test_cross_held_on_arrival_ends_no_year);
    in_fresh_child(test_a_new_game_starts_on_the_map);
    in_fresh_child(test_network_mode_survives_a_tab_visit);
    in_fresh_child(test_free_mode_survives_a_tab_visit_and_pans_from_slow);
    in_fresh_child(test_shoulders_close_a_planet_menu_then_switch);
    in_fresh_child(test_shoulders_wait_under_the_pause_menu);
    in_fresh_child(test_final_score_ignores_shoulders_circle_and_start);
    in_fresh_child(test_shoulders_return_to_the_starting_planet_then_switch);
    in_fresh_child(test_shoulders_wait_on_the_triangle_release_frame);
    in_fresh_child(test_shoulders_drop_the_drag_return_to_the_starting_planet_then_switch);
    in_fresh_child(test_shoulders_switch_and_free_mode_pans_under_the_year_box);
    in_fresh_child(test_start_opens_pause_from_free_mode);
    in_fresh_child(test_start_and_circle_close_pause_pressing_nothing_on_the_map);
    in_fresh_child(test_pause_from_network_mode_gives_back_the_planet_slipway_and_camera);
    in_fresh_child(test_pause_from_end_turn_and_build_keeps_the_tab_and_its_cursor);
    in_fresh_child(test_pause_keeps_the_starting_planet_for_the_next_circle);
    in_fresh_child(test_cross_on_each_row_but_restart_closes_pause);
    in_fresh_child(test_cross_on_restart_gives_the_title_then_a_new_game_on_the_map);
    in_fresh_child(test_the_d_pad_on_pause_moves_among_its_rows_and_no_camera);
    in_fresh_child(test_cross_held_into_pause_presses_no_row_until_pressed_again);
    in_fresh_child(test_the_cross_that_closes_pause_on_end_turn_ends_no_year);
    in_fresh_child(test_start_is_ignored_on_the_title);
    in_fresh_child(test_start_is_ignored_under_a_planet_menu_and_on_tech_and_score);
    in_fresh_child(test_start_is_ignored_while_cross_holds_a_fold);
    in_fresh_child(test_start_is_ignored_on_the_triangle_release_frame);
    in_fresh_child(test_start_and_shoulders_wait_on_the_landing_frame);
    in_fresh_child(test_start_while_dragging_drops_the_slipway_opens_pause_and_drags_again_after);
    in_fresh_child(test_start_is_ignored_on_the_frame_tech_returns_to_the_map);
    in_fresh_child(test_start_with_r1_opens_pause_and_keeps_the_tab);
    in_fresh_child(test_start_opens_pause_over_the_year_box);
    in_fresh_child(test_start_while_carrying_suspends_the_structure_under_pause);
    in_fresh_child(test_closing_pause_resumes_the_carry_where_it_was);
    in_fresh_child(test_restart_under_pause_drops_the_suspended_structure);
    in_fresh_child(test_pause_while_carrying_from_network_mode_keeps_the_starting_planet);
    in_fresh_child(test_circle_on_the_release_frame_of_the_pause_click_is_dropped);
    in_fresh_child(test_the_ui_pad_tab_is_the_tab_hud_button);
    in_fresh_child(test_no_pad_selection_is_shown_behind_a_tab);
    in_fresh_child(test_the_tech_tab_lights_the_tech_button_under_its_panel);
    in_fresh_child(test_landing_on_tech_opens_its_panel_and_an_invention_returns_to_the_map);
    in_fresh_child(test_cross_on_the_tech_header_closes_the_box_and_returns_to_the_map);
    in_fresh_child(test_circle_on_tech_closes_the_box_and_returns_to_the_map_in_one_press);
    in_fresh_child(test_landing_on_score_shows_its_panel_and_cross_presses_nothing);
    in_fresh_child(test_l1_on_score_hands_over_to_tech_on_the_third_frame);
    in_fresh_child(test_r1_on_tech_closes_the_box_and_shows_score_on_the_same_frame);
    in_fresh_child(test_r1_on_score_hides_it_and_returns_to_the_map);
    in_fresh_child(test_a_tab_whose_button_is_gone_returns_to_the_map);
    in_fresh_child(test_triangle_on_the_release_frame_of_leaving_score_does_nothing);
    in_fresh_child(test_circle_leaving_score_is_back_in_free_mode_before_the_next_frame);
    in_fresh_child(test_the_landing_after_a_shoulder_cancel_waits_for_its_release);
    in_fresh_child(test_pad_pause_draws_no_crosshair);
    in_fresh_child(test_a_carried_structure_draws_no_crosshair);
    in_fresh_child(test_network_mode_draws_no_crosshair);
    in_fresh_child(test_a_planet_menu_draws_no_crosshair);
    in_fresh_child(test_pad_free_mode_draws_a_crosshair_at_the_centre);
    in_fresh_child(test_a_drag_draws_the_partial_slipway_and_its_cost);
    in_fresh_child(test_a_drag_across_a_planet_draws_the_refused_feedback);
    printf("test_pad_nav: ok\n");
    return 0;
}
