#define _POSIX_C_SOURCE 200809L

#include "game/pad_nav.h"

#include "engine/dispatch_context.h"
#include "engine/entity_id.h"
#include "engine/entity_registry.h"
#include "engine/frame_zone.h"
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
#include "game/pad_select.h"
#include "game/pad_target.h"
#include "game/planet.h"
#include "game/planet_family_trade.h"
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

static int cheats_fired;

static void run_frame(input_step_t *step, int pad_buttons) {
    p8_input_frame_t raw = { 0, 0, 0, pad_buttons };
    cheats_fired += input_step_run(step, &raw);
    cart_update60();
}

/* Press on one frame, release on the next: the focus moves on the press. */
static void tap(input_step_t *step, int pad_buttons) {
    run_frame(step, pad_buttons);
    run_frame(step, 0);
}

/* The bar closed and no cycle panel open: what every map mode needs. */
static int on_the_map(const pad_nav_t *nav) {
    return !pad_nav_build_bar_open(nav) && !pad_nav_tech_open(nav) && !pad_nav_score_open(nav);
}

/* R1 from the map or from the bar. */
static void open_tech(input_step_t *step) {
    assert(!pad_nav_tech_open(&step->pad_nav) && !pad_nav_score_open(&step->pad_nav));
    tap(step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step->pad_nav));
}

/* □ from jump mode: the plain aim mode every game started in before jump mode. */
static void leave_jump_mode(input_step_t *step) {
    tap(step, P8_PAD_SQUARE);
    assert(pad_nav_aim_mode(&step->pad_nav));
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

/* The × that starts the game is not a press edge in aim mode: it places no probe. */
static void test_cross_held_across_the_click_leaves_nothing_held(void) {
    input_step_t step;
    start_title(&step);
    for (int i = 0; i < 2 * SETTLE_FRAMES; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == (i == 0 ? P8_MOUSE_LEFT : 0));
    }
    assert(strcmp(economy_score_difficulty()->name, "forgiving") == 0);
    assert(pad_nav_jump_mode(&step.pad_nav));
    run_frame(&step, P8_PAD_CROSS | P8_PAD_SQUARE);
    for (int i = 0; i < SETTLE_FRAMES; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert(pad_nav_aim_mode(&step.pad_nav));
    assert(ui_left_held(main_ui()) == NULL);
    run_frame(&step, 0);
    assert(ui_left_held(main_ui()) == NULL);
    assert(tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0);
}

static void test_aim_mode_holds_the_cursor_at_the_centre(void) {
    input_step_t step;
    start_title(&step);
    assert(!pad_nav_aim_mode(&step.pad_nav));
    run_frame(&step, P8_PAD_CROSS);
    assert(!pad_nav_panel_open());
    leave_jump_mode(&step);
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

static void start_game_in_jump_mode(input_step_t *step) {
    start_title(step);
    cart_start_game((button_t *)title_button("forgiving"));
    settle(step);
    assert(main_economy_typed() != NULL && !pad_nav_panel_open());
    assert(pad_nav_jump_mode(&step->pad_nav));
}

static void start_game(input_step_t *step) {
    start_game_in_jump_mode(step);
    leave_jump_mode(step);
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
static void test_aim_pans_with_the_speed_curve(void) {
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

static void test_aim_mode_diagonal_cancel_and_reset(void) {
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
static void test_aim_mode_bound_brings_every_planet_under_the_centre(void) {
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

static void test_aim_mode_triangle_on_empty_space_does_nothing(void) {
    input_step_t step;
    start_game(&step);
    pan_to_empty_space(&step);
    vec_t from = camera_at();
    int live = live_entities()->count;
    int boxes = tagged_count(TAG_MESSAGE_BOX);
    for (int i = 0; i < 3; i++) {
        run_frame(&step, i == 1 ? 0 : P8_PAD_TRIANGLE);
        const p8_input_frame_t *played = input_step_played_frame(&step);
        assert(played->mouse_x == P8_SCREEN_WIDTH / 2 && played->mouse_y == P8_SCREEN_HEIGHT / 2);
        assert(played->mouse_buttons == 0);
        assert(pad_nav_aim_mode(&step.pad_nav));
    }
    assert_moved(from, 0, 0);
    assert(live_entities()->count == live);
    assert(tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0);
    assert(tagged_count(TAG_MESSAGE_BOX) == boxes && main_ui()->ephemerals.count == 0);
}

static void test_aim_mode_first_cross_places_the_probe(void) {
    input_step_t step;
    start_game(&step);
    pan_to_empty_space(&step);
    tap(&step, P8_PAD_CROSS);
    assert(tagged_count(TAG_PROBE) == 1 && tagged_count(TAG_WORMHOLE) == 0);
    assert(pad_nav_aim_mode(&step.pad_nav));
    const p8_input_frame_t *played = input_step_played_frame(&step);
    assert(played->mouse_x == P8_SCREEN_WIDTH / 2 && played->mouse_y == P8_SCREEN_HEIGHT / 2);
    assert(played->mouse_buttons == 0);
}

static void test_aim_mode_short_cross_hold_abandons_the_fold(void) {
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
    assert(pad_nav_aim_mode(&step.pad_nav));
}

/* The test puts the planet under the centre through the pan operation, as pad navigation itself moves the camera. */
static void test_aim_mode_cross_on_a_planet_presses_nothing(void) {
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
            assert(pad_nav_aim_mode(&step.pad_nav));
            end_forked_copy();
        }
    }
}

static void test_aim_pans_while_cross_is_held(void) {
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

/* × engaged when the panel opens, then held again through the panel's close: back in aim mode it is no press edge. */
static void test_aim_mode_cross_engagement_ends_under_a_panel(void) {
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
    assert(pad_nav_aim_mode(&step.pad_nav));
    for (int i = 0; i < 5; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
}

/* A held direction pans nothing under a panel, and its speed starts over once aim mode resumes. */
static void test_panel_takes_precedence_then_the_cursor_returns_to_the_centre(void) {
    entity_t *entries[ITEMS_CAP];
    input_step_t step;
    start_game(&step);
    hold(&step, P8_PAD_RIGHT, 20);
    galaxy_menu_on_click(4)(NULL);
    vec_t from = camera_at();
    hold(&step, P8_PAD_RIGHT, SETTLE_FRAMES);
    assert(pad_nav_panel_open() && !pad_nav_aim_mode(&step.pad_nav));
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
    assert(pad_nav_aim_mode(&step.pad_nav));
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

/* The final score panel opens on the map by itself: a × held into it, from jump mode with nothing hovered, presses
 * nothing until pressed again, even after a title × armed an earlier box. */
static void test_cross_held_into_the_final_score_panel_on_the_map_presses_nothing(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    assert(pad_nav_hovered(&step.pad_nav) == NULL);
    hold(&step, P8_PAD_CROSS, 2);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    main_economy_typed()->end_status = "final score";
    main_economy_typed()->has_end_status = 1;
    entity_t *panel = &main_score_panel()->header;
    panel->class->states[ENTITY_STATE_NORMAL](panel, p8_fixed_from_int(0));
    for (int i = 0; i < 40; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert(main_economy_typed() != NULL && pad_nav_panel_open());
    hold(&step, 0, 2);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() == NULL);
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

static int is_button_of(const entity_t *e, const planet_t *planet) {
    return e->class == button_class() && entity_get(((const button_t *)e)->project_planet) == &planet->header;
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
    assert(pad_nav_aim_mode(&step->pad_nav));
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
    assert(pad_nav_aim_mode(&step.pad_nav));
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
    assert(pad_nav_aim_mode(&step.pad_nav));
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

/* The planet menu is jump mode's △ on the hovered object: aim mode presses no planet under the crosshair. */
static void test_aim_mode_triangle_over_a_known_planet_or_a_colony_does_nothing(void) {
    for (int settled = 0; settled < 2; settled++) {
        if (in_forked_copy()) {
            input_step_t step;
            start_game(&step);
            planet_t *planet = known_planet_under_centre(&step);
            if (settled) {
                settle_planet(&step, planet);
                pan_planet_under_centre(&step, planet, 0);
            }
            vec_t from = camera_at();
            world_count_t before = world_count();
            for (int i = 0; i < 4; i++) {
                run_frame(&step, i % 2 ? 0 : P8_PAD_TRIANGLE);
                assert(input_step_played_frame(&step)->mouse_buttons == 0);
                assert_cursor_at_centre(&step);
                assert(pad_nav_aim_mode(&step.pad_nav));
            }
            settle(&step);
            assert(main_ui()->ephemerals.count == 0 && pad_nav_aim_mode(&step.pad_nav));
            assert_moved(from, 0, 0);
            assert_world_unchanged(before);
            end_forked_copy();
        }
    }
}

/* A slipway pressed out of a colony under the crosshair would follow it: aim mode holds none. */
static void test_cross_in_aim_mode_on_a_colony_presses_nothing(void) {
    input_step_t step;
    start_game(&step);
    planet_t *colony = known_planet_closest_to_centre();
    settle_planet(&step, colony);
    pan_planet_under_centre(&step, colony, 0);
    world_count_t before = world_count();
    for (int i = 0; i < 20; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
    }
    run_frame(&step, 0);
    assert_world_unchanged(before);
    assert(pad_nav_aim_mode(&step.pad_nav) && main_ui()->ephemerals.count == 0);
}

static int off_screen(int x, int y) {
    return x < 0 || x >= P8_SCREEN_WIDTH || y < 0 || y >= P8_SCREEN_HEIGHT;
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

/* Moves the icon so its hit box centre sits on the planet's screen point. */
static void cover_with_icon(entity_t *icon, const planet_t *planet) {
    vec_t at = vec_sub(entity_pos(&planet->header), entity_pos(&main_camera_typed()->header));
    vec_t middle = vec_make(p8_fixed_div(p8_fixed_add(icon->hit_box.left, icon->hit_box.right), p8_fixed_from_int(2)),
                            p8_fixed_div(p8_fixed_add(icon->hit_box.top, icon->hit_box.bottom), p8_fixed_from_int(2)));
    entity_set_pos(icon, vec_sub(vec_make(p8_fixed_flr(at.x), p8_fixed_flr(at.y)), middle));
}

static planet_t *full_slipgate_under_centre(input_step_t *step) {
    planet_keys_t keys = {0};
    keys.pos = centre_world();
    keys.has_pos = 1;
    keys.map_pos = vec_div(keys.pos, p8_fixed_from_int(128));
    keys.has_map_pos = 1;
    planet_t *gate = slipgate_new(&keys);
    entity_set_state(&gate->header, ENTITY_STATE_SETTLED);
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    for (int i = 0; i < 3; i++) {
        slipway_keys_t slipway_keys = { .start = entity_id_link(&gate->header) };
        slipway_t *slipway = slipway_new(&slipway_keys);
        slipway_connect(slipway, planets->items[i]);
        gate->header.class->attach_slipway(&gate->header, &slipway->header);
        planets->items[i]->class->attach_slipway(planets->items[i], &slipway->header);
    }
    assert(gate->slipways.count == 3);
    pan_planet_under_centre(step, gate, 0);
    return gate;
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

/* The fold holds the left button: □ must not leave aim mode under it. */
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
        assert(pad_nav_aim_mode(&step.pad_nav));
    }
}

enum { PAD_CIRCLE_RADIUS = 9, PAD_CIRCLE_COLOUR = 7 };

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

static void test_shoulders_switch_and_aim_pans_under_the_year_box(void) {
    input_step_t step;
    start_game(&step);
    score_panel_new_year(p8_fixed_from_int(3421));
    entity_t *year_box = &shown_message_box()->header;
    run_frame(&step, 0);
    tap(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav) && main_ui()->ephemerals.count > 0);
    assert(entity_list_has(live_entities(), year_box) && shown_message_box() != NULL);
    tap(&step, P8_PAD_CIRCLE);
    assert(on_the_map(&step.pad_nav) && pad_nav_aim_mode(&step.pad_nav));
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

static void test_a_held_shoulder_moves_one_tab_and_both_together_move_none(void) {
    input_step_t step;
    start_game(&step);
    hold(&step, P8_PAD_R1, 10);
    assert(pad_nav_tech_open(&step.pad_nav));
    run_frame(&step, 0);
    tap(&step, P8_PAD_L1 | P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav));
}

static void open_build_bar(input_step_t *step) {
    assert(!pad_nav_build_bar_open(&step->pad_nav));
    tap(step, P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step->pad_nav));
}

typedef enum { THE_BAR, THE_TECH_PANEL, THE_SCORE_PANEL } off_map_t;

static int is_open(const pad_nav_t *nav, off_map_t where) {
    switch (where) {
    case THE_BAR:
        return pad_nav_build_bar_open(nav);
    case THE_TECH_PANEL:
        return pad_nav_tech_open(nav);
    case THE_SCORE_PANEL:
        return pad_nav_score_open(nav);
    }
    return 0;
}

/* From the map: L1 for the bar, R1 for tech, R1 R1 for score. */
static void open_from_the_map(input_step_t *step, off_map_t where) {
    assert(on_the_map(&step->pad_nav));
    if (where == THE_BAR) {
        open_build_bar(step);
        return;
    }
    open_tech(step);
    if (where == THE_SCORE_PANEL) {
        tap(step, P8_PAD_R1);
    }
    assert(is_open(&step->pad_nav, where));
}

static void test_r1_cycles_tech_score_closed_from_the_map_and_opens_tech_from_the_bar(void) {
    input_step_t step;
    start_game(&step);
    assert(on_the_map(&step.pad_nav));
    tap(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav));
    tap(&step, P8_PAD_R1);
    assert(pad_nav_score_open(&step.pad_nav));
    tap(&step, P8_PAD_R1);
    assert(on_the_map(&step.pad_nav));
    open_build_bar(&step);
    tap(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav) && !pad_nav_build_bar_open(&step.pad_nav));
    tap(&step, P8_PAD_R1);
    assert(pad_nav_score_open(&step.pad_nav));
}

static void assert_played(const input_step_t *step, int x, int y, int buttons) {
    const p8_input_frame_t *played = input_step_played_frame(step);
    assert(played->mouse_x == x && played->mouse_y == y && played->mouse_buttons == buttons);
}

static void assert_clicked(const input_step_t *step, int index) {
    int x, y;
    pad_panel_focus_point(&galaxy_menu_find_button(index)->header, &x, &y);
    assert_played(step, x, y, P8_MOUSE_LEFT);
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
    run_frame(&step, P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    assert_build_focus(&step, basic_structure_icon());
    assert(ui_pad_tab(main_ui()) == NULL);
    run_frame(&step, 0);
    assert_build_focus(&step, basic_structure_icon());
    assert(ui_pad_tab(main_ui()) == NULL);
}

static void test_build_left_and_right_move_between_the_icons_without_wrapping(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
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
    open_build_bar(&step);
    int rest_x = p8_fixed_flr_int(structure_icon_rest_cursor()->x);
    for (int i = 0; i < revealed; i++) {
        assert_build_focus(&step, revealed_icon_at(rest_x - 10 * i));
        tap(&step, P8_PAD_LEFT);
    }
    assert_build_focus(&step, revealed_icon_at(rest_x - 10 * (revealed - 1)));
}

static void test_the_bar_remembers_its_icon_across_a_close_until_a_new_game(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    tap(&step, P8_PAD_LEFT);
    tap(&step, P8_PAD_L1);
    assert(!pad_nav_build_bar_open(&step.pad_nav) && pad_nav_build_icon(&step.pad_nav) == NULL);
    run_frame(&step, P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    assert_build_focus(&step, processor_icon());
    cart_init();
    settle(&step);
    cart_start_game((button_t *)title_button("forgiving"));
    settle(&step);
    open_build_bar(&step);
    assert_build_focus(&step, basic_structure_icon());
}

static void test_build_up_down_square_and_triangle_press_nothing(void) {
    static const int buttons[4] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_SQUARE, P8_PAD_TRIANGLE };
    input_step_t step;
    start_game(&step);
    vec_t from = camera_at();
    int live = live_entities()->count;
    open_build_bar(&step);
    for (int b = 0; b < 4; b++) {
        hold(&step, buttons[b], 3);
        assert_build_focus(&step, basic_structure_icon());
        run_frame(&step, 0);
    }
    assert(pad_nav_build_bar_open(&step.pad_nav));
    assert_moved(from, 0, 0);
    assert(live_entities()->count == live && main_ui()->ephemerals.count == 0);
    assert(tagged_count(TAG_PROBE) == 0);
}

/* score leaves through a click on its backdrop: its own test covers it. */
static void test_circle_on_a_tab_returns_to_the_map(void) {
    static const off_map_t places[2] = { THE_BAR, THE_TECH_PANEL };
    input_step_t step;
    start_game(&step);
    for (int t = 0; t < 2; t++) {
        if (in_forked_copy()) {
            open_from_the_map(&step, places[t]);
            run_frame(&step, P8_PAD_CIRCLE | P8_PAD_CROSS);
            assert(is_open(&step.pad_nav, places[t]) && input_step_played_frame(&step)->mouse_buttons == 0);
            run_frame(&step, 0);
            run_frame(&step, P8_PAD_CIRCLE);
            assert(on_the_map(&step.pad_nav));
            assert_played(&step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
            run_frame(&step, P8_PAD_CIRCLE);
            assert(pad_nav_aim_mode(&step.pad_nav));
            assert_played(&step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
            end_forked_copy();
        }
    }
}

static void assert_carrying(const input_step_t *step, const entity_t *structure) {
    assert(on_the_map(&step->pad_nav));
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
    assert(pad_nav_aim_mode(&step->pad_nav));
}

static void test_cross_takes_the_structure_and_a_second_cross_places_it(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
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
    open_build_bar(&step);
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
    open_build_bar(&step);
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
    open_build_bar(&step);
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
    assert_moved(from, 0, 0);
    assert_played(&step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
}

/* Start suspends the carry before pause opens, so the pause menu is opened directly. */
static void test_a_panel_opened_while_carrying_releases_at_the_icon(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    int x, y;
    pad_panel_focus_point(basic_structure_icon(), &x, &y);
    (void)take_structure(&step);
    galaxy_menu_on_click(4)(NULL);
    assert(pad_nav_panel_open());
    run_frame(&step, 0);
    assert_played(&step, x, y, 0);
}

static void test_circle_cancels_a_structure_taken_from_jump_mode_then_leaves_aim_mode(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *planet = known_planet_closest_to_centre();
    hold(&step, 0, 2);
    open_build_bar(&step);
    entity_t *structure = take_structure(&step);
    assert(pad_nav_placing(&step.pad_nav) == structure);
    world_count_t before = world_count();
    tap(&step, P8_PAD_CIRCLE);
    assert(structure->is_dead && pad_nav_placing(&step.pad_nav) == NULL);
    assert_world_unchanged(before);
    assert(pad_nav_aim_mode(&step.pad_nav));
    tap(&step, P8_PAD_CIRCLE);
    assert(pad_nav_jump_mode(&step.pad_nav) && pad_nav_hovered(&step.pad_nav) == &planet->header);
}

static void test_triangle_and_square_wait_while_carrying(void) {
    static const int buttons[2] = { P8_PAD_TRIANGLE, P8_PAD_SQUARE };
    input_step_t step;
    start_game(&step);
    planet_t *planet = known_planet_under_centre(&step);
    open_build_bar(&step);
    entity_t *structure = take_structure(&step);
    for (int b = 0; b < 2; b++) {
        run_frame(&step, buttons[b]);
        assert_carrying(&step, structure);
        run_frame(&step, 0);
        assert_carrying(&step, structure);
    }
    assert(main_ui()->ephemerals.count == 0);
    assert(planet->header.state == ENTITY_STATE_KNOWN);
}

/* The release lands on the icon, as ○'s does: the cart refuses the drop. */
static void test_r1_cancels_the_carried_structure_then_lands_on_tech_once_the_cancel_is_past(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    int x, y;
    pad_panel_focus_point(basic_structure_icon(), &x, &y);
    entity_t *structure = take_structure(&step);
    pan_clear_of_every_node(&step, structure);
    world_count_t before = world_count();
    p8_fixed_t credits = main_economy_typed()->credits;
    run_frame(&step, P8_PAD_R1);
    assert_played(&step, x, y, 0);
    assert(pad_nav_tech_open(&step.pad_nav) && main_ui()->ephemerals.count == 0);
    assert(structure->is_dead && ui_left_held(main_ui()) == NULL && pad_nav_placing(&step.pad_nav) == NULL);
    run_frame(&step, 0);
    assert_clicked(&step, 2);
    assert(pad_nav_tech_open(&step.pad_nav) && main_ui()->ephemerals.count > 0);
    assert_world_unchanged(before);
    assert(main_economy_typed()->credits == credits);
}

static void test_cross_held_on_arrival_on_build_takes_nothing(void) {
    input_step_t step;
    start_game(&step);
    int live = live_entities()->count;
    run_frame(&step, P8_PAD_CROSS | P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    for (int i = 0; i < 10; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert(live_entities()->count == live && ui_left_held(main_ui()) == NULL);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    run_frame(&step, 0);
    (void)take_structure(&step);
}

/* The icon leaves the registry between the press and the ui's dispatch: the press reaches the background, which
 * hands over a probe, no structure. */
static void test_a_press_that_hands_over_no_structure_stays_on_build(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    p8_input_frame_t raw = { 0, 0, 0, P8_PAD_CROSS };
    input_step_run(&step, &raw);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    entity_unregister(pad_nav_build_icon(&step.pad_nav));
    cart_update60();
    assert(ui_left_held(main_ui()) != NULL && !planet_is_family(ui_left_held(main_ui())));
    for (int i = 0; i < 3; i++) {
        run_frame(&step, 0);
        assert(pad_nav_build_bar_open(&step.pad_nav) && pad_nav_placing(&step.pad_nav) == NULL);
        assert(input_step_played_frame(&step)->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
    }
    tap(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav) && !pad_nav_build_bar_open(&step.pad_nav));
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

/* Select held for frames, each of which must press nothing. */
static void hold_select_pressing_nothing(input_step_t *step, int frames) {
    for (int i = 0; i < frames; i++) {
        run_frame(step, P8_PAD_SELECT);
        assert(input_step_played_frame(step)->mouse_buttons == 0);
    }
}

/* Holds Select up to its 30th frame, which presses the year-skip button, then one more frame, its release. */
static void hold_select_to_the_year(input_step_t *step) {
    int x, y;
    year_skip_point(&x, &y);
    hold_select_pressing_nothing(step, PAD_SELECT_HOLD_FRAMES - 1);
    run_frame(step, P8_PAD_SELECT);
    assert_played(step, x, y, P8_MOUSE_LEFT);
    assert(pad_nav_year_chain_open(&step->pad_nav));
    run_frame(step, P8_PAD_SELECT);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
}

static void test_a_new_game_starts_on_the_map(void) {
    input_step_t step;
    start_game(&step);
    open_tech(&step);
    cart_init();
    settle(&step);
    cart_start_game((button_t *)title_button("forgiving"));
    run_frame(&step, 0);
    assert(main_economy_typed() != NULL);
    assert(on_the_map(&step.pad_nav) && pad_nav_jump_mode(&step.pad_nav));
    assert_cursor_at_centre(&step);
}

static void test_aim_mode_survives_a_tab_visit_and_pans_from_slow(void) {
    input_step_t step;
    start_game(&step);
    hold(&step, P8_PAD_RIGHT, 30);
    vec_t from = camera_at();
    run_frame(&step, P8_PAD_RIGHT | P8_PAD_R1);
    run_frame(&step, P8_PAD_RIGHT);
    tap(&step, P8_PAD_R1);
    run_frame(&step, P8_PAD_RIGHT | P8_PAD_CIRCLE);
    assert(on_the_map(&step.pad_nav));
    assert_moved(from, 0, 0);
    run_frame(&step, P8_PAD_RIGHT);
    assert(pad_nav_aim_mode(&step.pad_nav));
    assert_moved(from, 1, 0);
}

static void assert_shoulders_wait(input_step_t *step) {
    int bar = pad_nav_build_bar_open(&step->pad_nav), tech = pad_nav_tech_open(&step->pad_nav),
        score = pad_nav_score_open(&step->pad_nav);
    tap(step, P8_PAD_R1);
    assert(pad_nav_build_bar_open(&step->pad_nav) == bar && pad_nav_tech_open(&step->pad_nav) == tech &&
           pad_nav_score_open(&step->pad_nav) == score);
    tap(step, P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step->pad_nav) == bar && pad_nav_tech_open(&step->pad_nav) == tech &&
           pad_nav_score_open(&step->pad_nav) == score);
}

static void assert_final_score_unchanged(const input_step_t *step, entity_t *again) {
    entity_t *buttons[ITEMS_CAP];
    assert(on_the_map(&step->pad_nav) && pad_nav_panel_open());
    assert(end_buttons(buttons) == 2 && !again->is_dead && pad_nav_focus(&step->pad_nav) == again);
    assert(!pause_menu_shown() && main_economy_typed() != NULL);
}

static void test_final_score_ignores_shoulders_circle_and_start(void) {
    static const int buttons[6] = { P8_PAD_L1, P8_PAD_R1, P8_PAD_CIRCLE, P8_PAD_START, P8_PAD_TRIANGLE, P8_PAD_SQUARE };
    input_step_t step;
    start_game(&step);
    main_economy_typed()->year = p8_fixed_from_int(3425);
    hold_select_to_the_year(&step);
    run_frame(&step, 0);
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

static void test_shoulders_wait_under_the_pause_menu(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    galaxy_menu_on_click(4)(NULL);
    settle(&step);
    assert(pause_menu_shown());
    assert_shoulders_wait(&step);
    assert(pause_menu_shown());
    tap(&step, P8_PAD_CIRCLE);
    assert(pad_nav_build_bar_open(&step.pad_nav) && !pause_menu_shown());
}

/* Lit from the R1 frame that opens the panel, through its landing, until it closes; the year skip never. */
static void test_the_ui_pad_tab_is_the_open_cycle_panels_hud_button(void) {
    input_step_t step;
    start_game(&step);
    assert(ui_pad_tab(main_ui()) == NULL);
    static const int shoulders[5] = { P8_PAD_R1, P8_PAD_R1, P8_PAD_R1, P8_PAD_L1, P8_PAD_R1 };
    const entity_t *lit[5] = { &galaxy_menu_find_button(2)->header, &galaxy_menu_find_button(3)->header, NULL, NULL,
                               &galaxy_menu_find_button(2)->header };
    for (int i = 0; i < 5; i++) {
        run_frame(&step, shoulders[i]);
        assert(ui_pad_tab(main_ui()) == lit[i]);
        run_frame(&step, 0);
        assert(pad_nav_tech_open(&step.pad_nav) == (i == 0 || i == 4));
        assert(pad_nav_score_open(&step.pad_nav) == (i == 1));
        assert(pad_nav_build_bar_open(&step.pad_nav) == (i == 3));
        assert(ui_pad_tab(main_ui()) == lit[i]);
        assert(ui_pad_tab(main_ui()) != &galaxy_menu_find_button(1)->header);
    }
    tap(&step, P8_PAD_R1);
    tap(&step, P8_PAD_R1);
    open_tech(&step);
    cart_init();
    settle(&step);
    cart_start_game((button_t *)title_button("forgiving"));
    run_frame(&step, 0);
    assert(ui_pad_tab(main_ui()) == NULL);
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

static void test_the_tech_panel_lights_the_tech_button_under_it(void) {
    input_step_t step;
    start_game(&step);
    open_tech(&step);
    settle(&step);
    assert_tech_panel_up_and_its_button_not_hovered();
    cart_draw();
    assert(filled_with(2, HOVER_FILL) && filled_with(1, IDLE_FILL));
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
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav));
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
    assert(main_ui()->ephemerals.count == 0 && pad_nav_tech_open(&step.pad_nav));
    run_frame(&step, 0);
    assert(on_the_map(&step.pad_nav));
}

static void test_cross_on_the_tech_header_closes_the_box_and_returns_to_the_map(void) {
    input_step_t step;
    start_game(&step);
    open_tech(&step);
    assert(main_ui()->ephemerals.count > 0 && ((const button_t *)pad_nav_focus(&step.pad_nav))->on_click == NULL);
    tap(&step, P8_PAD_CROSS);
    assert(invented_count() == 0 && main_ui()->ephemerals.count == 0);
    assert(on_the_map(&step.pad_nav));
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
    open_tech(step);
    run_frame(step, P8_PAD_R1);
    assert(pad_nav_score_open(&step->pad_nav));
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
    assert(pad_nav_score_open(&step.pad_nav) && score_panel_shown() && backdrop() == shown_backdrop);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(on_the_map(&step.pad_nav) && !score_panel_shown());
    run_frame(&step, 0);
    assert(backdrop() == NULL && on_the_map(&step.pad_nav) && pad_nav_aim_mode(&step.pad_nav));
}

static void test_r1_on_tech_closes_the_box_and_shows_score_on_the_same_frame(void) {
    input_step_t step;
    start_game(&step);
    open_tech(&step);
    assert(main_ui()->ephemerals.count > 0 && !score_panel_shown());
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_score_open(&step.pad_nav));
    assert_clicked(&step, 3);
    assert(main_ui()->ephemerals.count == 0 && score_panel_shown() && backdrop() != NULL);
}

static void test_r1_on_score_hides_it_and_returns_to_the_map(void) {
    input_step_t step;
    start_game(&step);
    open_score_from_the_map(&step);
    run_frame(&step, P8_PAD_R1);
    assert(on_the_map(&step.pad_nav) && !score_panel_shown());
    run_frame(&step, 0);
    assert(backdrop() == NULL && on_the_map(&step.pad_nav));
}

static void test_tech_and_score_read_open_from_the_tab_change_frame_and_closed_without_a_game(void) {
    input_step_t step;
    start_game(&step);
    assert(!pad_nav_tech_open(&step.pad_nav) && !pad_nav_score_open(&step.pad_nav));
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav) && !pad_nav_score_open(&step.pad_nav));
    run_frame(&step, 0);
    assert(pad_nav_tech_open(&step.pad_nav) && main_ui()->ephemerals.count > 0);
    run_frame(&step, P8_PAD_R1);
    assert(!pad_nav_tech_open(&step.pad_nav) && pad_nav_score_open(&step.pad_nav));
    run_frame(&step, 0);
    assert(pad_nav_score_open(&step.pad_nav) && score_panel_shown());
    run_frame(&step, P8_PAD_R1);
    assert(!pad_nav_tech_open(&step.pad_nav) && !pad_nav_score_open(&step.pad_nav));
    run_frame(&step, 0);
    open_build_bar(&step);
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav) && !pad_nav_score_open(&step.pad_nav));
    run_frame(&step, 0);
    assert(pad_nav_tech_open(&step.pad_nav) && main_ui()->ephemerals.count > 0);
    tap(&step, P8_PAD_CIRCLE);
    assert(on_the_map(&step.pad_nav));
    if (in_forked_copy()) {
        open_tech(&step);
        cart_init();
        assert(main_economy_typed() == NULL && !pad_nav_tech_open(&step.pad_nav));
        end_forked_copy();
    }
    open_score_from_the_map(&step);
    cart_init();
    assert(main_economy_typed() == NULL && !pad_nav_score_open(&step.pad_nav));
}

static void test_r1_does_nothing_with_no_game_or_on_the_final_score_panel(void) {
    input_step_t step;
    end_game(&step);
    assert(pad_nav_panel_open());
    entity_t *focus = pad_nav_focus(&step.pad_nav);
    for (int i = 0; i < 3; i++) {
        run_frame(&step, P8_PAD_R1);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        run_frame(&step, 0);
    }
    assert(on_the_map(&step.pad_nav) && !pad_nav_tech_open(&step.pad_nav));
    assert(pad_nav_panel_open() && pad_nav_focus(&step.pad_nav) == focus);
    cart_init();
    settle(&step);
    tap(&step, P8_PAD_R1);
    assert(main_economy_typed() == NULL && on_the_map(&step.pad_nav));
    assert(!pad_nav_tech_open(&step.pad_nav) && input_step_played_frame(&step)->mouse_buttons == 0);
}

static void test_cross_held_through_r1_presses_nothing_on_tech_until_pressed_again(void) {
    input_step_t step;
    start_game(&step);
    main_economy_typed()->research_points = p8_fixed_from_int(5);
    run_frame(&step, P8_PAD_CROSS | P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav));
    assert_clicked(&step, 2);
    run_frame(&step, P8_PAD_CROSS);
    assert(main_ui()->ephemerals.count > 0 && input_step_played_frame(&step)->mouse_buttons == 0);
    entity_t *header = pad_nav_focus(&step.pad_nav);
    assert(header == pad_nav_first_item() && ((const button_t *)header)->on_click == NULL);
    for (int i = 0; i < 10; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert(main_ui()->ephemerals.count > 0 && pad_nav_tech_open(&step.pad_nav) && invented_count() == 0);
    run_frame(&step, 0);
    tap(&step, P8_PAD_DOWN);
    (void)focus_affordable_row(&step);
    run_frame(&step, P8_PAD_CROSS);
    assert(invented_count() == 1);
}

static void test_cross_held_through_r1_r1_presses_nothing_on_score(void) {
    input_step_t step;
    start_game(&step);
    run_frame(&step, P8_PAD_CROSS | P8_PAD_R1);
    hold(&step, P8_PAD_CROSS, 3);
    run_frame(&step, P8_PAD_CROSS | P8_PAD_R1);
    assert(pad_nav_score_open(&step.pad_nav) && score_panel_shown());
    for (int i = 0; i < 10; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert(pad_nav_score_open(&step.pad_nav) && score_panel_shown());
}

static void r1_round(input_step_t *step) {
    tap(step, P8_PAD_R1);
    tap(step, P8_PAD_DOWN);
    tap(step, P8_PAD_CIRCLE);
    tap(step, P8_PAD_LEFT);
    tap(step, P8_PAD_R1);
    tap(step, P8_PAD_R1);
    tap(step, P8_PAD_CIRCLE);
    settle(step);
    tap(step, P8_PAD_R1);
    tap(step, P8_PAD_R1);
    tap(step, P8_PAD_R1);
    settle(step);
    tap(step, P8_PAD_L1);
    tap(step, P8_PAD_RIGHT);
    tap(step, P8_PAD_L1);
    tap(step, P8_PAD_R1);
    tap(step, P8_PAD_DOWN);
    (void)focus_affordable_row(step);
    tap(step, P8_PAD_CROSS);
    settle(step);
    assert(on_the_map(&step->pad_nav));
    tap(step, P8_PAD_L1);
    tap(step, P8_PAD_LEFT);
    tap(step, P8_PAD_R1);
    tap(step, P8_PAD_DOWN);
    tap(step, P8_PAD_R1);
    tap(step, P8_PAD_RIGHT);
    tap(step, P8_PAD_R1);
    settle(step);
    assert(on_the_map(&step->pad_nav));
}

static void test_r1_cycles_circle_exits_inventions_and_bar_visits_never_type_the_cheat(void) {
    input_step_t step;
    start_game(&step);
    main_economy_typed()->research_points = p8_fixed_from_int(50);
    cheats_fired = 0;
    for (int i = 0; i < 3; i++) {
        r1_round(&step);
        tap(&step, P8_PAD_UP);
    }
    assert(cheats_fired == 0 && invented_count() == 3);
}

static void test_r1_cycles_leave_the_game_stream_unchanged(void) {
    input_step_t step;
    start_game(&step);
    uint32_t game = p8_fixed_rnd_state();
    for (int i = 0; i < 3; i++) {
        tap(&step, P8_PAD_R1);
        tap(&step, P8_PAD_R1);
        tap(&step, P8_PAD_R1);
        settle(&step);
        tap(&step, P8_PAD_R1);
        tap(&step, P8_PAD_CIRCLE);
        tap(&step, P8_PAD_R1);
        tap(&step, P8_PAD_R1);
        tap(&step, P8_PAD_CIRCLE);
        settle(&step);
        tap(&step, P8_PAD_L1);
        tap(&step, P8_PAD_R1);
        tap(&step, P8_PAD_R1);
        tap(&step, P8_PAD_R1);
        settle(&step);
    }
    assert(p8_fixed_rnd_state() == game && on_the_map(&step.pad_nav));
}

static void test_a_cycle_panel_whose_button_is_gone_returns_to_the_map(void) {
    input_step_t step;
    start_game(&step);
    galaxy_menu_find_button(2)->header.is_dead = 1;
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav) && input_step_played_frame(&step)->mouse_buttons == 0);
    run_frame(&step, 0);
    assert(on_the_map(&step.pad_nav) && main_ui()->ephemerals.count == 0);
}

static void test_circle_leaving_score_is_back_in_aim_mode_before_the_next_frame(void) {
    input_step_t step;
    start_game(&step);
    open_score_from_the_map(&step);
    run_frame(&step, P8_PAD_CIRCLE);
    entity_t *dead = backdrop();
    assert(dead != NULL && dead->is_dead);
    assert(!pad_nav_panel_open() && pad_nav_aim_mode(&step.pad_nav));
}

/* ○ leaves score by a backdrop click; its release frame on the map must emit no press. */
static void leave_score_then_triangle_on_the_release_frame(input_step_t *step) {
    open_score_from_the_map(step);
    run_frame(step, P8_PAD_CIRCLE);
    assert(on_the_map(&step->pad_nav));
    assert(!pad_nav_panel_open());
    run_frame(step, P8_PAD_TRIANGLE | P8_PAD_CROSS);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    run_frame(step, 0);
    assert(input_step_played_frame(step)->mouse_buttons == 0 && main_ui()->ephemerals.count == 0);
    assert(backdrop() == NULL && tagged_count(TAG_PROBE) == 0);
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

static void test_start_opens_pause_from_aim_mode(void) {
    input_step_t step;
    start_game(&step);
    vec_t from = camera_at();
    open_pause(&step);
    settle(&step);
    assert_restart_focused_and_hovered(&step);
    assert_moved(from, 0, 0);
    assert(tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0 && ui_left_held(main_ui()) == NULL);
    assert(on_the_map(&step.pad_nav));
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
            assert(pad_nav_aim_mode(&step.pad_nav));
            assert_cursor_at_centre(&step);
            assert_moved(from, 0, 0);
            end_forked_copy();
        }
    }
}

static void test_pause_from_the_bar_keeps_the_bar_and_its_icon(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    tap(&step, P8_PAD_LEFT);
    entity_t *icon = processor_icon();
    assert(pad_nav_build_icon(&step.pad_nav) == icon);
    int live = live_entities()->count;
    open_pause(&step);
    close_pause(&step, P8_PAD_START);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    run_frame(&step, 0);
    assert_build_focus(&step, icon);
    assert(live_entities()->count == live && pad_nav_placing(&step.pad_nav) == NULL);
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
            assert(pad_nav_aim_mode(&step.pad_nav));
            end_forked_copy();
        }
    }
}

static void test_cross_on_restart_gives_the_title_then_a_new_game_on_the_map(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    open_pause(&step);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() == NULL && pad_nav_panel_open());
    assert(on_the_map(&step.pad_nav));
    assert_focused_and_hovered(&step, "forgiving");
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() != NULL && !pad_nav_panel_open());
    assert(on_the_map(&step.pad_nav) && pad_nav_jump_mode(&step.pad_nav));
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

/* Once an invention closes the tech box, the tab returns to the map on the next frame: Start there opens nothing. */
static void test_start_is_ignored_on_the_frame_tech_returns_to_the_map(void) {
    input_step_t step;
    start_game(&step);
    main_economy_typed()->research_points = p8_fixed_from_int(5);
    open_tech(&step);
    tap(&step, P8_PAD_DOWN);
    (void)focus_affordable_row(&step);
    run_frame(&step, P8_PAD_CROSS);
    assert(invented_count() == 1 && main_ui()->ephemerals.count == 0 && pad_nav_tech_open(&step.pad_nav));
    assert_start_ignored(&step, 0);
    assert(on_the_map(&step.pad_nav) && pad_nav_aim_mode(&step.pad_nav));
}

static void test_start_with_r1_opens_pause_and_keeps_the_tab(void) {
    input_step_t step;
    start_game(&step);
    int x, y;
    pause_button_point(&x, &y);
    run_frame(&step, P8_PAD_START | P8_PAD_R1);
    assert_played(&step, x, y, P8_MOUSE_LEFT);
    assert(on_the_map(&step.pad_nav));
    run_frame(&step, 0);
    run_frame(&step, 0);
    assert_restart_focused_and_hovered(&step);
    assert(on_the_map(&step.pad_nav));
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
    assert(pad_nav_aim_mode(&step.pad_nav));
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

static void test_the_score_panel_lights_the_score_button_from_the_r1_on_tech(void) {
    input_step_t step;
    start_game(&step);
    open_tech(&step);
    settle(&step);
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_score_open(&step.pad_nav) && ui_pad_tab(main_ui()) == &galaxy_menu_find_button(3)->header);
    settle(&step);
    assert(score_panel_shown() && ui_pad_tab(main_ui()) == &galaxy_menu_find_button(3)->header);
    cart_draw();
    assert(filled_with(3, HOVER_FILL) && filled_with(2, IDLE_FILL) && filled_with(1, IDLE_FILL));
}

static void test_nothing_is_lit_on_the_bar_while_carrying_under_pause_or_on_the_final_score_panel(void) {
    input_step_t step;
    if (in_forked_copy()) {
        end_game(&step);
        assert(pad_nav_panel_open() && ui_pad_tab(main_ui()) == NULL);
        end_forked_copy();
    }
    start_game(&step);
    open_pause(&step);
    assert(ui_pad_tab(main_ui()) == NULL);
    close_pause(&step, P8_PAD_START);
    open_build_bar(&step);
    assert(ui_pad_tab(main_ui()) == NULL);
    open_pause(&step);
    assert(ui_pad_tab(main_ui()) == NULL);
    close_pause(&step, P8_PAD_START);
    entity_t *structure = take_structure(&step);
    for (int i = 0; i < 3; i++) {
        run_frame(&step, 0);
        assert(pad_nav_placing(&step.pad_nav) == structure && ui_pad_tab(main_ui()) == NULL);
    }
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
    open_build_bar(&step);
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
    open_build_bar(&step);
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
    open_build_bar(&step);
    entity_t *structure = take_structure(&step);
    suspend_into_pause(&step, structure);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() == NULL && pad_nav_placing(&step.pad_nav) == NULL && ui_left_held(main_ui()) == NULL);
    assert_focused_and_hovered(&step, "forgiving");
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() != NULL && pad_nav_jump_mode(&step.pad_nav));
    assert(pad_nav_placing(&step.pad_nav) == NULL && ui_left_held(main_ui()) == NULL);
    tap(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav));
}

/* ui_clear_ephemerals would close pause there with the left button up, so the resume's held button would be a press on the map. */
static void test_circle_on_the_release_frame_of_the_pause_click_is_dropped(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
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

static void test_l1_while_carrying_cancels_at_the_icon_then_opens_the_bar_pressing_nothing(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    int x, y;
    pad_panel_focus_point(basic_structure_icon(), &x, &y);
    entity_t *structure = take_structure(&step);
    world_count_t before = world_count();
    int live = live_entities()->count;
    run_frame(&step, P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    assert_played(&step, x, y, 0);
    run_frame(&step, 0);
    assert(structure->is_dead && pad_nav_placing(&step.pad_nav) == NULL && ui_left_held(main_ui()) == NULL);
    assert_world_unchanged(before);
    assert(live_entities()->count == live - 1);
    assert_played(&step, x, y, 0);
    assert(pad_nav_build_bar_open(&step.pad_nav) && pad_nav_build_icon(&step.pad_nav) == basic_structure_icon());
}

static void test_pad_aim_mode_draws_a_crosshair_at_the_centre(void) {
    input_step_t step;
    start_game(&step);
    assert(pad_nav_aim_mode(&step.pad_nav));
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
    open_build_bar(&step);
    entity_t *structure = take_structure(&step);
    pan_clear_of_every_node(&step, structure);
    assert(pad_nav_aim_mode(&step.pad_nav));
    assert_no_crosshair_drawn();
}

static void assert_jump_mode_rests_at_the_centre(input_step_t *step) {
    assert(pad_nav_jump_mode(&step->pad_nav) && !pad_nav_aim_mode(&step->pad_nav));
    assert(pad_nav_hovered(&step->pad_nav) == NULL);
    assert_played(step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
    assert(ui_pad_selection(main_ui()) == NULL);
}

static void test_a_new_game_starts_in_jump_mode_with_nothing_hovered(void) {
    static const int presses[] = { 0, P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT, P8_PAD_TRIANGLE };
    input_step_t step;
    start_game_in_jump_mode(&step);
    assert_jump_mode_rests_at_the_centre(&step);
    world_count_t before = world_count();
    vec_t from = camera_at();
    for (int i = 0; i < 30; i++) {
        run_frame(&step, presses[i % 6]);
        assert_jump_mode_rests_at_the_centre(&step);
    }
    assert_moved(from, 0, 0);
    assert_world_unchanged(before);
    assert_no_crosshair_drawn();
}

static void restart_into_a_new_game(input_step_t *step) {
    cart_init();
    for (int i = 0; i < SETTLE_FRAMES; i++) {
        cart_update60();
    }
    cart_start_game((button_t *)title_button("forgiving"));
    run_frame(step, 0);
    assert(main_economy_typed() != NULL && !pad_nav_panel_open());
}

static void test_a_new_game_after_aim_mode_starts_in_jump_mode(void) {
    input_step_t step;
    start_game(&step);
    hold(&step, P8_PAD_RIGHT, 5);
    restart_into_a_new_game(&step);
    assert_jump_mode_rests_at_the_centre(&step);
}

static void assert_hovering(const input_step_t *step, const entity_t *e) {
    assert(pad_nav_jump_mode(&step->pad_nav));
    assert(pad_nav_hovered(&step->pad_nav) == e);
    assert_cursor_on(step, e);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    assert(ui_pad_selection(main_ui()) == e);
}

/* The rest point minus the camera before shake, floored: what the edge scroll reads. */
static void rest_screen_point(const entity_t *e, int *x, int *y) {
    vec_t camera = camera_at();
    pad_jump_world_point(e, x, y);
    *x -= p8_fixed_flr_int(camera.x);
    *y -= p8_fixed_flr_int(camera.y);
}

static void test_jump_mode_seeds_on_the_first_object_that_appears(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    hold(&step, 0, 5);
    planet_t *planet = known_planet_closest_to_centre();
    int x, y;
    rest_screen_point(&planet->header, &x, &y);
    assert(pad_jump_inside(x, y));
    vec_t from = camera_at();
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
    hold(&step, 0, 10);
    assert_hovering(&step, &planet->header);
    assert_moved(from, 0, 0);
}

/* NULL when no planet's rest point lies outside the inner rectangle. */
static planet_t *planet_farthest_outside(void) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    planet_t *chosen = NULL;
    int best = 0;
    for (int i = 0; i < planets->count; i++) {
        int x, y, dx, dy;
        rest_screen_point(planets->items[i], &x, &y);
        pad_jump_scroll(x, y, &dx, &dy);
        int outside = absolute_int(dx) + absolute_int(dy);
        if (outside > best) {
            chosen = (planet_t *)planets->items[i];
            best = outside;
        }
    }
    return chosen;
}

static void make_known(planet_t *planet) {
    for (int i = 0; i < 64 && planet->header.state != ENTITY_STATE_KNOWN; i++) {
        planet_roll_type(planet);
    }
    assert(planet->header.state == ENTITY_STATE_KNOWN);
}

static int edge_of(int value, int low, int high) {
    return value < low ? low : value > high ? high : value;
}

static void test_the_edge_scroll_brings_the_hovered_object_just_inside_at_8_px_a_frame(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *planet = planet_farthest_outside();
    assert(planet != NULL);
    make_known(planet);
    int x, y;
    rest_screen_point(&planet->header, &x, &y);
    int want_x = edge_of(x, PAD_JUMP_SCROLL_MARGIN, P8_SCREEN_WIDTH - 1 - PAD_JUMP_SCROLL_MARGIN);
    int want_y = edge_of(y, PAD_JUMP_SCROLL_MARGIN, P8_SCREEN_HEIGHT - 1 - PAD_JUMP_SCROLL_MARGIN_BOTTOM);
    assert(absolute_int(want_x - x) + absolute_int(want_y - y) > PAD_JUMP_SCROLL_SPEED);
    int moving_frames = 0;
    for (int i = 0; i < 60; i++) {
        vec_t from = camera_at();
        run_frame(&step, 0);
        assert(pad_nav_hovered(&step.pad_nav) == &planet->header);
        assert_cursor_on(&step, &planet->header);
        vec_t moved = vec_sub(camera_at(), from);
        assert(absolute_int(p8_fixed_flr_int(moved.x)) <= PAD_JUMP_SCROLL_SPEED);
        assert(absolute_int(p8_fixed_flr_int(moved.y)) <= PAD_JUMP_SCROLL_SPEED);
        moving_frames += moved.x != 0 || moved.y != 0;
    }
    assert(moving_frames > 1);
    rest_screen_point(&planet->header, &x, &y);
    assert(pad_jump_inside(x, y));
    assert(x == want_x && y == want_y);
}

/* The colony is made before any frame sees the known planet, so the seed chooses between both. */
static void test_jump_mode_seeds_on_the_colony_over_a_nearer_known_planet(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *colony = planet_farthest_outside();
    assert(colony != NULL);
    make_known(colony);
    (void)planet_left_press(colony);
    assert(click_colonise_row());
    ui_clear_ephemerals(main_ui());
    assert(colony->header.state == ENTITY_STATE_SETTLED);
    planet_t *near = known_planet_closest_to_centre();
    assert(near != colony);
    run_frame(&step, 0);
    assert(pad_nav_hovered(&step.pad_nav) == &colony->header);
}

static void test_the_edge_scroll_reads_the_rest_point_not_the_bob(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *planet = known_planet_closest_to_centre();
    run_frame(&step, 0);
    assert(pad_nav_hovered(&step.pad_nav) == &planet->header);
    vec_t rest = entity_pos(&planet->header);
    vec_t from = camera_at();
    for (int i = 0; i < 60; i++) {
        entity_set_pos(&planet->header, vec_add(rest, vec_make(p8_fixed_from_int(200), p8_fixed_from_int(200))));
        run_frame(&step, 0);
        entity_set_pos(&planet->header, rest);
    }
    assert_moved(from, 0, 0);
}

static void test_square_in_jump_mode_enters_aim_mode_where_the_camera_is(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *planet = known_planet_closest_to_centre();
    hold(&step, 0, 2);
    assert(pad_nav_hovered(&step.pad_nav) == &planet->header);
    vec_t from = camera_at();
    run_frame(&step, P8_PAD_SQUARE);
    assert(pad_nav_aim_mode(&step.pad_nav) && !pad_nav_jump_mode(&step.pad_nav));
    assert(pad_nav_hovered(&step.pad_nav) == NULL);
    assert_played(&step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
    assert_moved(from, 0, 0);
    cart_draw();
    assert(crosshair_pixels_lit() == 4 * CROSSHAIR_ARM);
    run_frame(&step, 0);
    hold(&step, P8_PAD_RIGHT, 2);
    assert_moved(from, 2, 0);
}

static void make_every_planet_known(void) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    for (int i = 0; i < planets->count; i++) {
        make_known((planet_t *)planets->items[i]);
    }
}

static void assert_leaving_aim_mode_hovers_the_nearest(int button) {
    input_step_t step;
    start_game(&step);
    planet_t *far = planet_farthest_outside();
    assert(far != NULL);
    make_known(far);
    planet_t *near = known_planet_closest_to_centre();
    assert(near != far);
    run_frame(&step, 0);
    assert(pad_nav_aim_mode(&step.pad_nav));
    vec_t from = camera_at();
    run_frame(&step, button);
    assert(pad_nav_jump_mode(&step.pad_nav) && !pad_nav_aim_mode(&step.pad_nav));
    assert(pad_nav_hovered(&step.pad_nav) == &near->header);
    assert_played(&step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
    assert_moved(from, 0, 0);
    run_frame(&step, 0);
    assert_hovering(&step, &near->header);
}

static void test_square_in_aim_mode_goes_back_to_jump_mode_on_the_nearest_target(void) {
    assert_leaving_aim_mode_hovers_the_nearest(P8_PAD_SQUARE);
}

static void test_circle_in_aim_mode_goes_back_to_jump_mode_on_the_nearest_target(void) {
    assert_leaving_aim_mode_hovers_the_nearest(P8_PAD_CIRCLE);
}

static void test_square_in_aim_mode_over_a_colony_hovers_it_and_presses_nothing(void) {
    input_step_t step;
    start_game(&step);
    planet_t *colony = planet_closest_to_centre();
    settle_planet(&step, colony);
    pan_planet_under_centre(&step, colony, 0);
    assert(pad_nav_aim_mode(&step.pad_nav));
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_SQUARE);
    for (int i = 0; i <= 10; i++) {
        assert(pad_nav_jump_mode(&step.pad_nav) && pad_nav_hovered(&step.pad_nav) == &colony->header);
        assert(ui_left_held(main_ui()) == NULL);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        run_frame(&step, 0);
    }
    assert_world_unchanged(before);
}

static void assert_leaving_aim_mode_with_no_target_hovers_nothing(int button) {
    input_step_t step;
    start_game(&step);
    run_frame(&step, 0);
    run_frame(&step, button);
    for (int i = 0; i < 3; i++) {
        assert(pad_nav_jump_mode(&step.pad_nav) && pad_nav_hovered(&step.pad_nav) == NULL);
        assert_played(&step, P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2, 0);
        run_frame(&step, 0);
    }
    planet_t *planet = known_planet_closest_to_centre();
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
}

static void test_square_in_aim_mode_with_no_target_hovers_nothing_until_one_appears(void) {
    assert_leaving_aim_mode_with_no_target_hovers_nothing(P8_PAD_SQUARE);
}

static void test_circle_in_aim_mode_with_no_target_hovers_nothing_until_one_appears(void) {
    assert_leaving_aim_mode_with_no_target_hovers_nothing(P8_PAD_CIRCLE);
}

/* No aim-mode input opens a box ○ closes: the planet menu is opened by the left press a click would reach, so the pad
 * stays in plain aim mode under it. */
static void test_circle_in_aim_mode_closes_a_box_before_leaving(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = known_planet_under_centre(&step);
    (void)planet_left_press(planet);
    run_frame(&step, 0);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(main_ui()->ephemerals.count == 0);
    run_frame(&step, 0);
    assert(pad_nav_aim_mode(&step.pad_nav));
    settle(&step);
    assert(pad_nav_aim_mode(&step.pad_nav));
    run_frame(&step, P8_PAD_CIRCLE);
    assert(pad_nav_jump_mode(&step.pad_nav) && pad_nav_hovered(&step.pad_nav) == &planet->header);
}

/* The fold's release must not go to jump mode, which never holds the left button. */
static void test_square_and_circle_wait_while_cross_holds_a_fold(void) {
    input_step_t step;
    start_game(&step);
    pan_to_empty_space(&step);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    hold(&step, P8_PAD_RIGHT, 10);
    run_frame(&step, 0);
    p8_fixed_t credits = main_economy_typed()->credits;
    for (int i = 0; i < 60; i++) {
        run_frame(&step, P8_PAD_CROSS | (i % 4 == 1 ? P8_PAD_SQUARE : i % 4 == 3 ? P8_PAD_CIRCLE : 0));
        assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
        assert(pad_nav_aim_mode(&step.pad_nav));
    }
    run_frame(&step, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0 && pad_nav_aim_mode(&step.pad_nav));
    settle(&step);
    assert(main_economy_typed()->credits < credits);
    assert(pad_nav_aim_mode(&step.pad_nav));
    run_frame(&step, P8_PAD_SQUARE);
    assert(pad_nav_jump_mode(&step.pad_nav));
}

static void test_aim_pans_and_toggles_leave_the_game_stream_unchanged(void) {
    static const int presses[] = { P8_PAD_SQUARE, P8_PAD_RIGHT, P8_PAD_SQUARE, P8_PAD_DOWN,
                                   P8_PAD_SQUARE, P8_PAD_LEFT,  P8_PAD_CIRCLE, P8_PAD_UP };
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    run_frame(&step, 0);
    uint32_t game = p8_fixed_rnd_state();
    int aim_frames = 0, jump_frames = 0;
    for (int i = 0; i < (int)(sizeof presses / sizeof presses[0]); i++) {
        int frames = presses[i] == P8_PAD_SQUARE || presses[i] == P8_PAD_CIRCLE ? 1 : 25;
        hold(&step, presses[i], frames);
        run_frame(&step, 0);
        aim_frames += pad_nav_aim_mode(&step.pad_nav) ? frames : 0;
        jump_frames += pad_nav_jump_mode(&step.pad_nav) ? frames : 0;
    }
    assert(aim_frames >= 50 && jump_frames >= 50 && pad_nav_jump_mode(&step.pad_nav));
    assert(p8_fixed_rnd_state() == game && "aim mode draws no game number until × presses");
}

static void test_crosshair_is_drawn_in_aim_mode_only(void) {
    static const int buttons[2] = { P8_PAD_SQUARE, P8_PAD_CIRCLE };
    input_step_t step;
    start_game(&step);
    for (int b = 0; b < 2; b++) {
        run_frame(&step, 0);
        cart_draw();
        assert(main_ui()->pad_crosshair && crosshair_pixels_lit() == 4 * CROSSHAIR_ARM);
        run_frame(&step, buttons[b]);
        cart_draw();
        assert(!main_ui()->pad_crosshair && crosshair_pixels_lit() < 4 * CROSSHAIR_ARM);
        run_frame(&step, 0);
        tap(&step, P8_PAD_SQUARE);
        assert(pad_nav_aim_mode(&step.pad_nav));
    }
}

/* ○ before × has acted on anything, and × with nothing hovered. */
static void test_circle_with_nothing_acted_on_and_cross_on_nothing_change_nothing(void) {
    static const int presses[] = { P8_PAD_CROSS, 0, P8_PAD_CIRCLE, 0, P8_PAD_CROSS, P8_PAD_CROSS, P8_PAD_CIRCLE };
    input_step_t step;
    start_game_in_jump_mode(&step);
    run_frame(&step, 0);
    assert(pad_nav_hovered(&step.pad_nav) == NULL);
    world_count_t before = world_count();
    vec_t from = camera_at();
    for (int i = 0; i < (int)(sizeof presses / sizeof presses[0]); i++) {
        run_frame(&step, presses[i]);
        assert(pad_nav_jump_mode(&step.pad_nav) && pad_nav_hovered(&step.pad_nav) == NULL);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    planet_t *planet = known_planet_closest_to_centre();
    hold(&step, 0, 2);
    for (int i = 0; i < 4; i++) {
        run_frame(&step, i % 2 == 0 ? P8_PAD_CIRCLE : 0);
        assert(pad_nav_hovered(&step.pad_nav) == &planet->header);
        assert_cursor_on(&step, &planet->header);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert(pad_nav_last_acted(&step.pad_nav) == NULL && !pad_nav_panel_open());
    assert_moved(from, 0, 0);
    assert_world_unchanged(before);
    assert(tagged_count(TAG_PROBE) == 0 && tagged_count(TAG_WORMHOLE) == 0);
}

static void test_pad_nav_hovered_is_null_outside_jump_mode(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *planet = known_planet_closest_to_centre();
    run_frame(&step, 0);
    assert(pad_nav_hovered(&step.pad_nav) == &planet->header);
    tap(&step, P8_PAD_SQUARE);
    assert(pad_nav_hovered(&step.pad_nav) == NULL);
}

static void test_r1_cycles_tech_score_closed_from_jump_mode_back_on_the_hovered_object(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *planet = known_planet_closest_to_centre();
    hold(&step, 0, 2);
    vec_t from = camera_at();
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav) && !pad_nav_build_bar_open(&step.pad_nav));
    assert_clicked(&step, 2);
    run_frame(&step, 0);
    assert(pad_nav_tech_open(&step.pad_nav) && main_ui()->ephemerals.count > 0);
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_score_open(&step.pad_nav));
    assert_clicked(&step, 3);
    run_frame(&step, 0);
    assert(score_panel_shown() && backdrop() != NULL);
    run_frame(&step, P8_PAD_R1);
    assert(on_the_map(&step.pad_nav) && !score_panel_shown());
    run_frame(&step, 0);
    assert(backdrop() == NULL && pad_nav_jump_mode(&step.pad_nav));
    assert_cursor_on(&step, &planet->header);
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
    assert_moved(from, 0, 0);
}

static void test_circle_on_tech_and_on_score_returns_to_jump_mode_on_the_hovered_object(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *planet = known_planet_closest_to_centre();
    hold(&step, 0, 2);
    vec_t from = camera_at();
    tap(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav) && main_ui()->ephemerals.count > 0);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(on_the_map(&step.pad_nav) && main_ui()->ephemerals.count == 0);
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
    tap(&step, P8_PAD_R1);
    tap(&step, P8_PAD_R1);
    assert(pad_nav_score_open(&step.pad_nav) && score_panel_shown());
    run_frame(&step, P8_PAD_CIRCLE);
    assert(on_the_map(&step.pad_nav) && !score_panel_shown());
    run_frame(&step, 0);
    assert(backdrop() == NULL);
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
    assert_moved(from, 0, 0);
}

static void test_taking_a_structure_from_jump_mode_enters_plain_aim_mode_where_the_camera_is(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    vec_t from = camera_at();
    open_build_bar(&step);
    (void)take_structure(&step);
    assert(pad_nav_aim_mode(&step.pad_nav));
    assert_moved(from, 0, 0);
}

/* The live set jump mode searches: the TAG_NODE bucket filtered by pad_target_is_candidate. */
static entity_t *expected_jump(const entity_t *origin, int direction) {
    frame_zone_mark_t mark = frame_zone_mark();
    entity_list_t candidates = pad_jump_candidates(frame_zone(), entities_by_tag(TAG_NODE));
    entity_t *picked = pad_target_pick(origin, direction, &candidates);
    frame_zone_rewind(mark);
    return picked;
}

/* Lets the edge scroll finish, so a still camera afterwards means the frame moved nothing. */
static entity_t *hover_settled(input_step_t *step) {
    hold(step, 0, 40);
    entity_t *hovered = pad_nav_hovered(&step->pad_nav);
    assert(hovered != NULL);
    return hovered;
}

static void test_a_d_pad_press_jumps_to_the_picked_target(void) {
    static const int directions[] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT };
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    entity_t *origin = hover_settled(&step);
    int jumps = 0;
    for (int d = 0; d < 4; d++) {
        entity_t *expected = expected_jump(origin, directions[d]);
        jumps += expected != NULL;
        if (in_forked_copy()) {
            run_frame(&step, directions[d]);
            assert_hovering(&step, expected != NULL ? expected : origin);
            end_forked_copy();
        }
    }
    assert(jumps >= 2);
}

static entity_t *hover_the_leftmost(input_step_t *step) {
    entity_t *hovered = hover_settled(step);
    for (int i = 0; i < 64 && expected_jump(hovered, P8_PAD_LEFT) != NULL; i++) {
        tap(step, P8_PAD_LEFT);
        assert(pad_nav_hovered(&step->pad_nav) != hovered);
        hovered = pad_nav_hovered(&step->pad_nav);
    }
    assert(expected_jump(hovered, P8_PAD_LEFT) == NULL);
    return hover_settled(step);
}

static void test_holding_a_direction_repeats_the_jump_after_18_then_every_8_frames(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    entity_t *hovered = hover_the_leftmost(&step);
    for (int frame = 0; frame <= 30; frame++) {
        entity_t *expected = frame == 0 || frame == 18 || frame == 26 ? expected_jump(hovered, P8_PAD_RIGHT) : hovered;
        assert(expected != NULL);
        run_frame(&step, P8_PAD_RIGHT);
        assert_hovering(&step, expected);
        assert((expected != hovered) == (frame == 0 || frame == 18 || frame == 26));
        hovered = expected;
    }
}

static void test_a_d_pad_press_with_an_empty_cone_changes_nothing(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    entity_t *hovered = hover_the_leftmost(&step);
    vec_t from = camera_at();
    for (int i = 0; i < 10; i++) {
        run_frame(&step, i % 2 == 0 ? P8_PAD_LEFT : 0);
        assert_hovering(&step, hovered);
    }
    assert_moved(from, 0, 0);
}

static void test_a_hold_through_pause_repeats_no_jump_until_pressed_again(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    entity_t *hovered = hover_the_leftmost(&step);
    run_frame(&step, P8_PAD_RIGHT);
    entity_t *landed = pad_nav_hovered(&step.pad_nav);
    assert(landed != hovered);
    hold(&step, P8_PAD_RIGHT, 9);
    run_frame(&step, P8_PAD_RIGHT | P8_PAD_START);
    hold(&step, P8_PAD_RIGHT, 3);
    assert(pause_menu_shown());
    run_frame(&step, P8_PAD_RIGHT | P8_PAD_START);
    for (int i = 0; i < 40; i++) {
        run_frame(&step, P8_PAD_RIGHT);
        assert(!pause_menu_shown() && pad_nav_hovered(&step.pad_nav) == landed);
    }
    run_frame(&step, 0);
    run_frame(&step, P8_PAD_RIGHT);
    assert(pad_nav_hovered(&step.pad_nav) == expected_jump(landed, P8_PAD_RIGHT));
}

static void idle_scrolling_at_most_8_px_a_frame(input_step_t *step, int frames) {
    for (int i = 0; i < frames; i++) {
        vec_t from = camera_at();
        run_frame(step, 0);
        vec_t moved = vec_sub(camera_at(), from);
        assert(absolute_int(p8_fixed_flr_int(moved.x)) <= PAD_JUMP_SCROLL_SPEED);
        assert(absolute_int(p8_fixed_flr_int(moved.y)) <= PAD_JUMP_SCROLL_SPEED);
    }
}

static void test_a_jump_scrolls_only_to_bring_the_target_just_inside(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    entity_t *hovered = hover_the_leftmost(&step);
    int inside_jumps = 0, outside_jumps = 0;
    for (int i = 0; i < 64 && expected_jump(hovered, P8_PAD_RIGHT) != NULL; i++) {
        entity_t *target = expected_jump(hovered, P8_PAD_RIGHT);
        int x, y;
        rest_screen_point(target, &x, &y);
        int want_x = edge_of(x, PAD_JUMP_SCROLL_MARGIN, P8_SCREEN_WIDTH - 1 - PAD_JUMP_SCROLL_MARGIN);
        int want_y = edge_of(y, PAD_JUMP_SCROLL_MARGIN, P8_SCREEN_HEIGHT - 1 - PAD_JUMP_SCROLL_MARGIN_BOTTOM);
        vec_t from = camera_at();
        run_frame(&step, P8_PAD_RIGHT);
        assert_hovering(&step, target);
        idle_scrolling_at_most_8_px_a_frame(&step, 40);
        if (pad_jump_inside(x, y)) {
            assert_moved(from, 0, 0);
            inside_jumps++;
        } else {
            outside_jumps++;
        }
        rest_screen_point(target, &x, &y);
        assert(x == want_x && y == want_y);
        hovered = target;
    }
    assert(inside_jumps > 0 && outside_jumps > 0);
}

/* No fold or probe in the sequence: reveals are the only game draws after the title. */
static void test_jumps_repeats_and_edge_scrolls_leave_the_game_stream_unchanged(void) {
    static const int directions[] = { P8_PAD_RIGHT, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_UP };
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    run_frame(&step, 0);
    uint32_t game = p8_fixed_rnd_state();
    int frames = 0, jumps = 0, scrolls = 0;
    for (int d = 0; d < 4; d++) {
        for (int i = 0; i < 46; i++, frames++) {
            int pad = i < 40 ? directions[d] : i % 2 == 0 ? directions[(d + 1) % 4] : 0;
            entity_t *before = pad_nav_hovered(&step.pad_nav);
            vec_t from = camera_at();
            run_frame(&step, pad);
            jumps += pad_nav_hovered(&step.pad_nav) != before;
            scrolls += camera_at().x != from.x || camera_at().y != from.y;
        }
    }
    assert(frames >= 120 && jumps >= 8 && scrolls > 0);
    assert(p8_fixed_rnd_state() == game && "jump mode draws no game number");
}

static void test_triangle_on_a_hovered_known_planet_opens_its_menu_and_circle_comes_back(void) {
    entity_t *rows[ITEMS_CAP];
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *planet = known_planet_closest_to_centre();
    hold(&step, 0, 2);
    assert_hovering(&step, &planet->header);
    run_frame(&step, P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    assert_cursor_on(&step, &planet->header);
    run_frame(&step, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    int count = box_entries(rows);
    assert(count > 0 && is_button_of(rows[count - 1], planet));
    assert(pad_nav_panel_open() && pad_nav_hovered(&step.pad_nav) == &planet->header);
    tap(&step, P8_PAD_CIRCLE);
    assert(!pad_nav_panel_open());
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
}

/* △ then a release frame; returns the hovered object after both. */
static entity_t *triangle_click(input_step_t *step, int with_press, int with_release) {
    run_frame(step, P8_PAD_TRIANGLE | with_press);
    assert(input_step_played_frame(step)->mouse_buttons == P8_MOUSE_LEFT);
    assert(!pad_nav_panel_open());
    run_frame(step, with_release);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    return pad_nav_hovered(&step->pad_nav);
}

/* A direction pressed with △, or on its release frame, jumps nowhere: the cursor stays on the clicked colony. */
static void test_triangle_on_a_hovered_colony_opens_its_panel_and_no_jump_shares_the_click(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    planet_t *colony = planet_closest_to_centre();
    settle_planet(&step, colony);
    hold(&step, 0, 40);
    assert_hovering(&step, &colony->header);
    int direction = expected_jump(&colony->header, P8_PAD_RIGHT) != NULL ? P8_PAD_RIGHT : P8_PAD_LEFT;
    assert(expected_jump(&colony->header, direction) != NULL);
    if (in_forked_copy()) {
        assert(triangle_click(&step, direction, direction) == &colony->header);
        end_forked_copy();
    }
    if (in_forked_copy()) {
        assert(triangle_click(&step, 0, direction) == &colony->header);
        assert_cursor_on(&step, &colony->header);
        end_forked_copy();
    }
    assert(triangle_click(&step, 0, 0) == &colony->header);
    settle(&step);
    assert(pad_nav_panel_open() && main_ui()->ephemerals.count > 0);
    assert(pad_nav_hovered(&step.pad_nav) == &colony->header);
}

static void test_triangle_on_a_hovered_object_still_scrolling_in_presses_nothing(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *planet = planet_farthest_outside();
    assert(planet != NULL);
    make_known(planet);
    run_frame(&step, 0);
    assert(pad_nav_hovered(&step.pad_nav) == &planet->header);
    int x, y;
    rest_screen_point(&planet->header, &x, &y);
    assert(!pad_jump_inside(x, y));
    world_count_t before = world_count();
    for (int i = 0; i < 2; i++) {
        run_frame(&step, i == 0 ? P8_PAD_TRIANGLE : 0);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        assert_cursor_on(&step, &planet->header);
    }
    assert(!pad_nav_panel_open() && main_ui()->ephemerals.count == 0);
    assert_world_unchanged(before);
}

static planet_t *jump_on_known_planet(input_step_t *step) {
    start_game_in_jump_mode(step);
    planet_t *planet = known_planet_closest_to_centre();
    hold(step, 0, 40);
    assert_hovering(step, &planet->header);
    return planet;
}

static planet_t *jump_on_colony(input_step_t *step) {
    start_game_in_jump_mode(step);
    planet_t *colony = planet_closest_to_centre();
    settle_planet(step, colony);
    hold(step, 0, 40);
    assert_hovering(step, &colony->header);
    return colony;
}

/* In slipway mode × is the held left button: Start lets it up first, and the click on the pause button follows a frame
 * later. */
static void test_cross_held_into_pause_presses_no_row_until_pressed_again(void) {
    input_step_t step;
    entity_t *colony = &jump_on_colony(&step)->header;
    year_facts_t before = year_facts();
    hold(&step, P8_PAD_CROSS, 5);
    assert(pad_nav_slipway_start(&step.pad_nav) == colony && ui_left_held(main_ui()) != NULL);
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
    assert_years_ended(before, 0);
    assert(pad_nav_slipway_start(&step.pad_nav) == NULL && ui_left_held(main_ui()) == NULL);
    run_frame(&step, 0);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() == NULL);
}

static void test_cross_on_a_hovered_known_planet_opens_its_menu_and_records_it(void) {
    entity_t *rows[ITEMS_CAP];
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *planet = known_planet_closest_to_centre();
    hold(&step, 0, 2);
    assert_hovering(&step, &planet->header);
    assert(pad_nav_last_acted(&step.pad_nav) == NULL);
    run_frame(&step, P8_PAD_CROSS);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    assert_cursor_on(&step, &planet->header);
    run_frame(&step, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    int count = box_entries(rows);
    assert(count > 0 && is_button_of(rows[count - 1], planet));
    assert(pad_nav_panel_open() && pad_nav_hovered(&step.pad_nav) == &planet->header);
    assert(pad_nav_last_acted(&step.pad_nav) == &planet->header);
    tap(&step, P8_PAD_CIRCLE);
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
    assert(pad_nav_last_acted(&step.pad_nav) == &planet->header);
}

/* pad_panel_step presses the focused row while × is held: the opening × must be pressed again. */
static void test_cross_held_from_the_opening_click_presses_no_row_until_pressed_again(void) {
    entity_t *rows[ITEMS_CAP];
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
    p8_fixed_t credits = main_economy_typed()->credits;
    for (int i = 0; i < 12; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(i == 0 || input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert(pad_nav_panel_open() && planet->header.state == ENTITY_STATE_KNOWN);
    assert(main_economy_typed()->credits == credits);
    run_frame(&step, 0);
    int count = box_entries(rows);
    assert(count >= 2 && is_button_of(rows[1], planet));
    tap(&step, P8_PAD_DOWN);
    assert(pad_nav_focus(&step.pad_nav) == rows[1]);
    p8_fixed_t cost = ((button_t *)rows[1])->project_cost;
    for (int i = 0; i < 4 && main_ui()->ephemerals.count > 0; i++) {
        run_frame(&step, P8_PAD_CROSS);
    }
    assert(main_ui()->ephemerals.count == 0 && planet->header.state == ENTITY_STATE_SETTLED);
    assert(main_economy_typed()->credits == p8_fixed_sub(credits, cost));
    run_frame(&step, 0);
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
    assert(pad_nav_last_acted(&step.pad_nav) == &planet->header);
}

/* No button on the × frame or the next 10, no box, nothing recorded. */
static void assert_cross_opens_nothing(input_step_t *step, const planet_t *planet) {
    world_count_t before = world_count();
    for (int i = 0; i < 11; i++) {
        run_frame(step, i == 0 ? P8_PAD_CROSS : 0);
        assert(input_step_played_frame(step)->mouse_buttons == 0);
        assert(pad_nav_hovered(&step->pad_nav) == &planet->header);
    }
    assert(!pad_nav_panel_open() && main_ui()->ephemerals.count == 0);
    assert(pad_nav_last_acted(&step->pad_nav) == NULL);
    assert_world_unchanged(before);
}

static void test_cross_on_a_known_planet_scrolling_in_or_under_the_hud_presses_nothing(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    if (in_forked_copy()) {
        planet_t *planet = known_planet_closest_to_centre();
        hold(&step, 0, 2);
        cover_with_icon(live_of_class(structure_icon_class()), planet);
        assert_cross_opens_nothing(&step, planet);
        end_forked_copy();
    }
    planet_t *planet = planet_farthest_outside();
    assert(planet != NULL);
    make_known(planet);
    run_frame(&step, 0);
    assert(pad_nav_hovered(&step.pad_nav) == &planet->header);
    int x, y;
    rest_screen_point(&planet->header, &x, &y);
    assert(!pad_jump_inside(x, y));
    assert_cross_opens_nothing(&step, planet);
}

static void test_cross_and_triangle_on_one_frame_give_one_click(void) {
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
    run_frame(&step, P8_PAD_CROSS | P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    for (int i = 0; i < 10; i++) {
        run_frame(&step, 0);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    entity_t *rows[ITEMS_CAP];
    int count = box_entries(rows);
    assert(count > 0 && is_button_of(rows[count - 1], planet) && pad_nav_panel_open());
    assert(pad_nav_hovered(&step.pad_nav) == &planet->header);
}

/* The closing frame presses nothing, and the marker is back on the next. */
static void assert_triangle_closes_the_menu_of(input_step_t *step, const planet_t *planet) {
    assert(pad_nav_panel_open() && main_ui()->ephemerals.count > 0);
    world_count_t before = world_count();
    state_t state = planet->header.state;
    run_frame(step, P8_PAD_TRIANGLE);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    assert(!pad_nav_panel_open() && main_ui()->ephemerals.count == 0);
    assert(pad_nav_jump_mode(&step->pad_nav) && pad_nav_hovered(&step->pad_nav) == &planet->header);
    run_frame(step, 0);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    assert_hovering(step, &planet->header);
    assert(ui_pad_selection(main_ui()) == &planet->header);
    assert(planet->header.state == state);
    assert_world_unchanged(before);
}

static void test_triangle_again_closes_a_known_planets_menu(void) {
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
    tap(&step, P8_PAD_TRIANGLE);
    settle(&step);
    assert_triangle_closes_the_menu_of(&step, planet);
}

static void test_triangle_again_closes_a_colonys_inspection_panel(void) {
    input_step_t step;
    planet_t *colony = jump_on_colony(&step);
    tap(&step, P8_PAD_TRIANGLE);
    settle(&step);
    assert_triangle_closes_the_menu_of(&step, colony);
}

/* △ on the release frame of ×'s click leaves the menu up; the next △ closes it. */
static void test_triangle_on_the_release_frame_closes_nothing_and_closes_a_menu_cross_opened_after(void) {
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
    run_frame(&step, P8_PAD_CROSS);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    if (in_forked_copy()) {
        run_frame(&step, P8_PAD_TRIANGLE);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        assert(pad_nav_panel_open() && main_ui()->ephemerals.count > 0);
        end_forked_copy();
    }
    run_frame(&step, 0);
    run_frame(&step, 0);
    assert_triangle_closes_the_menu_of(&step, planet);
}

/* △ means only the hovered object's menu: it never closes the year box. */
static void test_triangle_under_the_year_box_closes_only_a_planet_menu(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    score_panel_new_year(p8_fixed_from_int(3421));
    entity_t *year_box = &shown_message_box()->header;
    run_frame(&step, 0);
    assert(pad_nav_hovered(&step.pad_nav) == NULL);
    for (int i = 0; i < 2; i++) {
        run_frame(&step, i == 0 ? P8_PAD_TRIANGLE : 0);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert(!pad_nav_panel_open() && main_ui()->ephemerals.count == 0);
    assert(shown_message_box() == (message_box_t *)year_box);
    planet_t *planet = known_planet_closest_to_centre();
    hold(&step, 0, 2);
    assert_hovering(&step, &planet->header);
    tap(&step, P8_PAD_TRIANGLE);
    run_frame(&step, 0);
    assert_triangle_closes_the_menu_of(&step, planet);
    assert(shown_message_box() == (message_box_t *)year_box);
}

#define CHORD (P8_PAD_CROSS | P8_PAD_CIRCLE)

/* Four chords, one every 2 frames; returns how often the cheat fired and asserts the mouse buttons played. */
static int type_chords_playing(input_step_t *step, int mouse_buttons) {
    int fired = cheats_fired;
    for (int i = 0; i < 2 * CHEAT_CHORDS; i++) {
        run_frame(step, i % 2 == 0 ? CHORD : 0);
        assert(input_step_played_frame(step)->mouse_buttons == mouse_buttons);
    }
    return cheats_fired - fired;
}

static int type_chords(input_step_t *step) {
    return type_chords_playing(step, 0);
}

static void test_four_chords_on_a_hovered_colony_cheat_and_change_nothing_else(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    planet_t *colony = planet_closest_to_centre();
    settle_planet(&step, colony);
    hold(&step, 0, 40);
    assert_hovering(&step, &colony->header);
    world_count_t before = world_count();
    vec_t from = camera_at();
    uint32_t game = p8_fixed_rnd_state();
    int mouse_x = input_step_played_frame(&step)->mouse_x, mouse_y = input_step_played_frame(&step)->mouse_y;
    assert(type_chords(&step) == 1);
    assert(main_economy_typed()->credits == p8_fixed_from_int(200));
    assert(main_economy_typed()->research_points == p8_fixed_from_int(99));
    before.credits = main_economy_typed()->credits;
    assert_world_unchanged(before);
    assert_hovering(&step, &colony->header);
    assert(input_step_played_frame(&step)->mouse_x == mouse_x && input_step_played_frame(&step)->mouse_y == mouse_y);
    assert_moved(from, 0, 0);
    assert(p8_fixed_rnd_state() == game);
}

/* The chord's × would click the focused row and its ○ would close the menu. */
static void test_a_chord_on_a_planet_menu_presses_no_row_and_closes_nothing(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *planet = known_planet_closest_to_centre();
    hold(&step, 0, 2);
    tap(&step, P8_PAD_TRIANGLE);
    settle(&step);
    assert(pad_nav_panel_open());
    world_count_t before = world_count();
    assert(type_chords(&step) == 1);
    before.credits = main_economy_typed()->credits;
    assert(pad_nav_panel_open());
    assert_world_unchanged(before);
    tap(&step, P8_PAD_CIRCLE);
    assert(!pad_nav_panel_open());
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
}

/* × held after the chord's ○ goes up, and ○ pressed again under it, still press nothing until both are up. */
static void test_buttons_held_from_a_chord_act_on_nothing_until_both_are_up(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    known_planet_closest_to_centre();
    hold(&step, 0, 2);
    tap(&step, P8_PAD_TRIANGLE);
    settle(&step);
    assert(pad_nav_panel_open());
    world_count_t before = world_count();
    static const int frames[] = { CHORD, P8_PAD_CROSS, P8_PAD_CROSS, CHORD, CHORD, P8_PAD_CIRCLE, P8_PAD_CIRCLE };
    for (int i = 0; i < (int)(sizeof frames / sizeof frames[0]); i++) {
        run_frame(&step, frames[i]);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        assert(pad_nav_panel_open());
    }
    run_frame(&step, 0);
    assert_world_unchanged(before);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(!pad_nav_panel_open());
}

/* The pad hides × and ○ from each chord cheat_step counts until both are up, and nowhere else: unless hidden, × down
 * in jump mode starts a slipway and ○ down in slipway mode cancels it. */
static void test_the_pad_hides_cross_and_circle_exactly_from_the_chords_cheat_step_counts(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *colony = planet_closest_to_centre();
    settle_planet(&step, colony);
    hold(&step, 0, 40);
    assert_hovering(&step, &colony->header);
    int held = 0;
    static const int directions[] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT };
    for (int i = 0; i < 4 && held == 0; i++) {
        if (expected_jump(&colony->header, directions[i]) == NULL) {
            held = directions[i];
        }
    }
    assert(held != 0 && "the colony needs one direction with an empty cone, so a held D-pad keeps it hovered");
    const int x = P8_PAD_CROSS, o = P8_PAD_CIRCLE, d = held;
    const int frames[] = {
        CHORD, 0, 0, CHORD, 0, CHORD, x, 0, CHORD, 0,
        x, o, 0,
        o, o | x, 0, o, 0,
        x, 0, CHORD, x, x | o, 0, o, 0,
        d, d | CHORD, d, d | x, d, d | o, 0,
    };
    const int count = (int)(sizeof frames / sizeof frames[0]);
    cheat_t cheat;
    cheat_init(&cheat);
    int previous = 0, hidden = 0, chords = 0, fired = cheats_fired, own_fired = 0;
    for (int i = 0; i < count; i++) {
        int raw = frames[i];
        int chords_before = cheat.chords;
        int fires = cheat_step(&cheat, raw);
        own_fired += fires;
        int counted = fires || cheat.chords > chords_before;
        chords += counted;
        if (counted) {
            hidden = 1;
        } else if (!(raw & CHORD)) {
            hidden = 0;
        }
        int cross_down = (raw & ~previous & x) != 0, circle_down = (raw & ~previous & o) != 0;
        int was_slipway = pad_nav_slipway_start(&step.pad_nav) != NULL;
        assert(was_slipway || pad_nav_hovered(&step.pad_nav) == &colony->header);
        run_frame(&step, raw);
        int is_slipway = pad_nav_slipway_start(&step.pad_nav) != NULL;
        if (cross_down && !was_slipway) {
            assert(is_slipway == !hidden);
        } else if (circle_down && was_slipway) {
            assert(is_slipway == hidden);
        } else {
            assert(is_slipway == was_slipway);
        }
        if (is_slipway) {
            assert(pad_nav_slipway_start(&step.pad_nav) == &colony->header);
        } else {
            assert(pad_nav_jump_mode(&step.pad_nav) && pad_nav_hovered(&step.pad_nav) == &colony->header);
        }
        previous = raw;
    }
    assert(chords == 6 && own_fired == 1 && cheats_fired - fired == 1);
}

/* The chord's ○ would leave aim mode and its × would place a probe. */
static void test_four_chords_in_aim_mode_cheat_and_change_nothing_else(void) {
    input_step_t step;
    start_game(&step);
    pan_to_empty_space(&step);
    make_every_planet_known();
    run_frame(&step, 0);
    world_count_t before = world_count();
    vec_t from = camera_at();
    uint32_t game = p8_fixed_rnd_state();
    assert(type_chords(&step) == 1);
    hold(&step, 0, 10);
    before.credits = main_economy_typed()->credits;
    assert_world_unchanged(before);
    assert(pad_nav_aim_mode(&step.pad_nav));
    assert_cursor_at_centre(&step);
    assert_moved(from, 0, 0);
    assert(p8_fixed_rnd_state() == game);
}

static void test_probing_and_leaving_aim_mode_never_types_the_cheat(void) {
    input_step_t step;
    start_game(&step);
    pan_to_empty_space(&step);
    cheats_fired = 0;
    for (int round = 0; round < 2; round++) {
        tap(&step, P8_PAD_CROSS);
        tap(&step, P8_PAD_CIRCLE);
        assert(pad_nav_jump_mode(&step.pad_nav));
        tap(&step, P8_PAD_SQUARE);
        assert(pad_nav_aim_mode(&step.pad_nav));
    }
    hold(&step, P8_PAD_CROSS, 40);
    run_frame(&step, 0);
    tap(&step, P8_PAD_CIRCLE);
    assert(pad_nav_jump_mode(&step.pad_nav));
    assert(cheats_fired == 0 && tagged_count(TAG_PROBE) == 1);
    assert(main_economy_typed()->credits != p8_fixed_from_int(200));
}

/* The slipway mode's targets: the jump candidates minus the start. */
static entity_t *slipway_pick(const entity_t *start, const entity_t *origin, int direction) {
    frame_zone_mark_t mark = frame_zone_mark();
    entity_list_t candidates = pad_jump_candidates(frame_zone(), entities_by_tag(TAG_NODE));
    entity_list_t targets = pad_slipway_candidates(frame_zone(), &candidates, start);
    entity_t *picked = pad_target_pick(origin, direction, &targets);
    frame_zone_rewind(mark);
    return picked;
}

enum { JUMP_PATH_CAP = 128 };

/* The shortest D-pad path from origin to target, through slipway_pick from start, or expected_jump when start is NULL;
 * returns its length, -1 when none exists. */
static int path_to(const entity_t *start, const entity_t *origin, const entity_t *target, int directions[JUMP_PATH_CAP]) {
    static const int all[] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT };
    const entity_t *seen[JUMP_PATH_CAP];
    int parent[JUMP_PATH_CAP], via[JUMP_PATH_CAP];
    int count = 1, head = 0;
    seen[0] = origin;
    parent[0] = -1;
    while (head < count && seen[head] != target) {
        for (int d = 0; d < 4; d++) {
            const entity_t *next = start != NULL ? slipway_pick(start, seen[head], all[d]) : expected_jump(seen[head], all[d]);
            int known = next == NULL;
            for (int i = 0; i < count && !known; i++) {
                known = seen[i] == next;
            }
            if (!known && count < JUMP_PATH_CAP) {
                seen[count] = next;
                parent[count] = head;
                via[count++] = all[d];
            }
        }
        head++;
    }
    if (head == count) {
        return -1;
    }
    int length = 0;
    for (int i = head; parent[i] >= 0; i = parent[i]) {
        length++;
    }
    for (int i = head, k = length - 1; parent[i] >= 0; i = parent[i], k--) {
        directions[k] = via[i];
    }
    return length;
}

/* Taps the path, then lets the edge scroll finish. */
static void tap_path(input_step_t *step, const entity_t *start, const entity_t *origin, const entity_t *target) {
    int directions[JUMP_PATH_CAP];
    int length = path_to(start, origin, target, directions);
    assert(length >= 0 && "tap_path: the D-pad reaches the target");
    for (int i = 0; i < length; i++) {
        tap(step, directions[i]);
    }
    hold(step, 0, 40);
}

static void hover_object(input_step_t *step, const entity_t *e) {
    run_frame(step, 0);
    entity_t *hovered = pad_nav_hovered(&step->pad_nav);
    assert(hovered != NULL);
    tap_path(step, NULL, hovered, e);
    assert_hovering(step, e);
}

static void aim_at(input_step_t *step, const entity_t *target) {
    entity_t *start = pad_nav_slipway_start(&step->pad_nav);
    entity_t *aimed = pad_nav_slipway_target(&step->pad_nav);
    assert(start != NULL);
    tap_path(step, start, aimed != NULL ? aimed : start, target);
    assert(pad_nav_slipway_target(&step->pad_nav) == target);
}

/* The ui holds a slipway out of start, the pad holds the left button on the aimed object. */
static void assert_slipway_mode(const input_step_t *step, const entity_t *start, const entity_t *target) {
    assert(!pad_nav_jump_mode(&step->pad_nav) && pad_nav_hovered(&step->pad_nav) == NULL);
    assert(pad_nav_slipway_start(&step->pad_nav) == start && pad_nav_slipway_target(&step->pad_nav) == target);
    entity_t *held = ui_left_held(main_ui());
    assert(held != NULL && held->class == slipway_class() && slipway_start((const slipway_t *)held) == start);
    assert(input_step_played_frame(step)->mouse_buttons == P8_MOUSE_LEFT);
    assert_cursor_on(step, target != NULL ? target : start);
    assert(!pad_nav_aim_mode(&step->pad_nav));
}

/* × on the hovered object: its press on this frame, the slipway held from the next. */
static void start_slipway(input_step_t *step, const entity_t *start) {
    assert_hovering(step, start);
    run_frame(step, P8_PAD_CROSS);
    assert(input_step_played_frame(step)->mouse_buttons == P8_MOUSE_LEFT);
    assert_cursor_on(step, start);
    run_frame(step, 0);
    assert_slipway_mode(step, start, NULL);
}

/* × on the aimed target: the release there, then the re-press of the start the outcome leaves, then its hold. */
static void cross_on_target(input_step_t *step) {
    entity_t *target = pad_nav_slipway_target(&step->pad_nav);
    assert(target != NULL);
    run_frame(step, P8_PAD_CROSS);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    assert_cursor_on(step, target);
    run_frame(step, 0);
    run_frame(step, 0);
}

static p8_fixed_t slipway_cost(const entity_t *start, const entity_t *end) {
    entity_t *slipway = slipway_between(start, end);
    assert(slipway != NULL);
    return ((const slipway_t *)slipway)->targeting.cost;
}

/* a, b and c settled on one row, 10 px apart: Right goes from a to b, then from b to c. */
static void settled_row(input_step_t *step, planet_t **a, planet_t **b, planet_t **c) {
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    *a = most_isolated_planet();
    planet_t *others[2];
    for (int i = 0, n = 0; n < 2; i++) {
        if (planets->items[i] != &(*a)->header) {
            others[n++] = (planet_t *)planets->items[i];
        }
    }
    *b = others[0];
    *c = others[1];
    place_near(*b, *a, 10, 0);
    place_near(*c, *a, 20, 0);
    settle_planet(step, *a);
    settle_planet(step, *b);
    settle_planet(step, *c);
    main_economy_typed()->credits = p8_fixed_from_int(999);
    assert(slipway_pick(&(*a)->header, &(*a)->header, P8_PAD_RIGHT) == &(*b)->header);
    assert(slipway_pick(&(*b)->header, &(*b)->header, P8_PAD_RIGHT) == &(*c)->header);
}

static void test_cross_dpad_cross_dpad_cross_builds_a_chain_of_two_slipways(void) {
    input_step_t step;
    planet_t *a, *b, *c;
    start_game_in_jump_mode(&step);
    settled_row(&step, &a, &b, &c);
    hover_object(&step, &a->header);
    world_count_t before = world_count();
    start_slipway(&step, &a->header);
    tap(&step, P8_PAD_RIGHT);
    assert_slipway_mode(&step, &a->header, &b->header);
    cross_on_target(&step);
    assert_slipway_mode(&step, &b->header, NULL);
    tap(&step, P8_PAD_RIGHT);
    assert_slipway_mode(&step, &b->header, &c->header);
    cross_on_target(&step);
    assert_slipway_mode(&step, &c->header, NULL);
    world_count_t now = world_count();
    assert(now.slipways == before.slipways + 2);
    p8_fixed_t spent = p8_fixed_add(slipway_cost(&a->header, &b->header), slipway_cost(&b->header, &c->header));
    assert(spent > 0 && now.credits == p8_fixed_sub(before.credits, spent));
    run_frame(&step, P8_PAD_CIRCLE);
    assert(slipways_left_placing() == 0 && ui_left_held(main_ui()) == NULL);
    run_frame(&step, 0);
    assert_hovering(&step, &c->header);
    assert(world_count().slipways == now.slipways && world_count().credits == now.credits);
}

static void test_a_built_slipway_records_its_target_and_a_refused_one_keeps_the_start(void) {
    input_step_t step;
    planet_t *a, *b, *c;
    start_game_in_jump_mode(&step);
    settled_row(&step, &a, &b, &c);
    hover_object(&step, &a->header);
    start_slipway(&step, &a->header);
    assert(pad_nav_last_acted(&step.pad_nav) == &a->header);
    tap(&step, P8_PAD_RIGHT);
    assert_slipway_mode(&step, &a->header, &b->header);
    assert(pad_nav_last_acted(&step.pad_nav) == &a->header);
    cross_on_target(&step);
    assert_slipway_mode(&step, &b->header, NULL);
    assert(pad_nav_last_acted(&step.pad_nav) == &b->header);
    tap(&step, P8_PAD_LEFT);
    assert_slipway_mode(&step, &b->header, &a->header);
    cross_on_target(&step);
    assert_slipway_mode(&step, &b->header, NULL);
    assert(pad_nav_last_acted(&step.pad_nav) == &b->header);
    tap(&step, P8_PAD_CIRCLE);
    run_frame(&step, 0);
    assert_hovering(&step, &b->header);
    assert(pad_nav_last_acted(&step.pad_nav) == &b->header);
}

/* The chords' × would open the known planet's menu. */
static void test_four_chords_on_a_hovered_known_planet_cheat_and_open_nothing(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *colony = most_isolated_planet();
    planet_t *known = NULL;
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    for (int i = 0; known == NULL; i++) {
        if (planets->items[i] != &colony->header) {
            known = (planet_t *)planets->items[i];
        }
    }
    place_near(known, colony, 10, 0);
    settle_planet(&step, colony);
    make_known(known);
    hover_object(&step, &colony->header);
    start_slipway(&step, &colony->header);
    tap(&step, P8_PAD_CIRCLE);
    run_frame(&step, 0);
    assert_hovering(&step, &colony->header);
    assert(expected_jump(&colony->header, P8_PAD_RIGHT) == &known->header);
    tap(&step, P8_PAD_RIGHT);
    hold(&step, 0, 40);
    assert_hovering(&step, &known->header);
    world_count_t before = world_count();
    assert(type_chords(&step) == 1);
    hold(&step, 0, 10);
    before.credits = main_economy_typed()->credits;
    assert_world_unchanged(before);
    assert(!pad_nav_panel_open() && main_ui()->ephemerals.count == 0);
    assert_hovering(&step, &known->header);
    assert(pad_nav_last_acted(&step.pad_nav) == &colony->header);
}

/* a is the last object × acted on (a slipway started and cancelled), b is hovered, the year box is up. */
static entity_t *year_box_with_the_last_object_elsewhere(input_step_t *step, planet_t **a, planet_t **b) {
    planet_t *c;
    start_game_in_jump_mode(step);
    settled_row(step, a, b, &c);
    hover_object(step, &(*a)->header);
    start_slipway(step, &(*a)->header);
    tap(step, P8_PAD_CIRCLE);
    tap(step, P8_PAD_RIGHT);
    hold(step, 0, 40);
    assert_hovering(step, &(*b)->header);
    assert(pad_nav_last_acted(&step->pad_nav) == &(*a)->header);
    score_panel_new_year(p8_fixed_from_int(3421));
    entity_t *year_box = &shown_message_box()->header;
    run_frame(step, 0);
    return year_box;
}

/* ○ closes the year box; on that frame and the next the buttons are only the held ones. */
static void circle_closes_the_year_box(input_step_t *step, entity_t *year_box, int held_buttons) {
    assert(shown_message_box() == (message_box_t *)year_box);
    run_frame(step, P8_PAD_CIRCLE);
    assert(input_step_played_frame(step)->mouse_buttons == held_buttons);
    assert(shown_message_box() == NULL && year_box->is_dead);
    run_frame(step, 0);
    assert(input_step_played_frame(step)->mouse_buttons == held_buttons);
}

static void test_circle_closes_the_year_box_over_jump_mode_and_moves_nothing(void) {
    input_step_t step;
    planet_t *a, *b;
    entity_t *year_box = year_box_with_the_last_object_elsewhere(&step, &a, &b);
    vec_t from = camera_at();
    world_count_t before = world_count();
    circle_closes_the_year_box(&step, year_box, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert_hovering(&step, &b->header);
    assert_moved(from, 0, 0);
    assert_world_unchanged(before);
    assert(pad_nav_last_acted(&step.pad_nav) == &a->header);
}

/* The year box's companion of test_circle_in_aim_mode_closes_a_box_before_leaving. */
static void test_circle_in_aim_mode_closes_the_year_box_before_leaving(void) {
    input_step_t step;
    planet_t *a, *b;
    entity_t *year_box = year_box_with_the_last_object_elsewhere(&step, &a, &b);
    tap(&step, P8_PAD_SQUARE);
    assert(pad_nav_aim_mode(&step.pad_nav));
    circle_closes_the_year_box(&step, year_box, 0);
    assert(pad_nav_aim_mode(&step.pad_nav));
    assert_cursor_at_centre(&step);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(pad_nav_jump_mode(&step.pad_nav));
}

static void test_circle_closes_the_year_box_over_a_slipway_then_cancels_it(void) {
    input_step_t step;
    planet_t *a, *b;
    entity_t *year_box = year_box_with_the_last_object_elsewhere(&step, &a, &b);
    start_slipway(&step, &b->header);
    world_count_t before = world_count();
    circle_closes_the_year_box(&step, year_box, P8_MOUSE_LEFT);
    assert_slipway_mode(&step, &b->header, NULL);
    run_frame(&step, P8_PAD_CIRCLE);
    run_frame(&step, 0);
    assert_hovering(&step, &b->header);
    assert(ui_left_held(main_ui()) == NULL);
    assert_world_unchanged(before);
}

static void test_circle_closes_the_year_box_over_a_carried_structure_and_keeps_it(void) {
    input_step_t step;
    planet_t *a, *b;
    entity_t *year_box = year_box_with_the_last_object_elsewhere(&step, &a, &b);
    open_build_bar(&step);
    entity_t *structure = take_structure(&step);
    circle_closes_the_year_box(&step, year_box, P8_MOUSE_LEFT);
    assert_carrying(&step, structure);
    assert(!structure->is_dead);
}

/* One box per ○: the planet menu first, the year box next. */
static void test_circle_closes_a_planet_menu_then_the_year_box(void) {
    input_step_t step;
    planet_t *a, *b;
    entity_t *year_box = year_box_with_the_last_object_elsewhere(&step, &a, &b);
    tap(&step, P8_PAD_TRIANGLE);
    settle(&step);
    assert(pad_nav_panel_open() && main_ui()->ephemerals.count > 0);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert(!pad_nav_panel_open() && shown_message_box() == (message_box_t *)year_box);
    run_frame(&step, 0);
    assert_hovering(&step, &b->header);
    circle_closes_the_year_box(&step, year_box, 0);
    assert_hovering(&step, &b->header);
}

/* On the bar or on tech ○ returns to the map and the year box stays. */
static void test_circle_on_the_bar_or_tech_returns_to_the_map_under_the_year_box(void) {
    static const off_map_t places[2] = { THE_BAR, THE_TECH_PANEL };
    input_step_t step;
    planet_t *a, *b;
    entity_t *year_box = year_box_with_the_last_object_elsewhere(&step, &a, &b);
    for (int t = 0; t < 2; t++) {
        if (in_forked_copy()) {
            open_from_the_map(&step, places[t]);
            tap(&step, P8_PAD_CIRCLE);
            assert(on_the_map(&step.pad_nav));
            assert(shown_message_box() == (message_box_t *)year_box && !year_box->is_dead);
            end_forked_copy();
        }
    }
}

static planet_t *hovered_colony_near_centre(input_step_t *step) {
    make_every_planet_known();
    planet_t *colony = planet_closest_to_centre();
    settle_planet(step, colony);
    hold(step, 0, 40);
    assert_hovering(step, &colony->header);
    return colony;
}

static void assert_camera_step_at_most_8(vec_t from) {
    vec_t moved = vec_sub(camera_at(), from);
    assert(absolute_int(p8_fixed_flr_int(moved.x)) <= 8 && absolute_int(p8_fixed_flr_int(moved.y)) <= 8);
}

/* Each press and each hold repeat aims at slipway_pick from the aimed object, never at the start. */
static void test_cross_on_a_hovered_colony_holds_its_slipway_and_the_d_pad_aims_past_it(void) {
    static const int directions[] = { P8_PAD_RIGHT, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_UP };
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *colony = hovered_colony_near_centre(&step);
    start_slipway(&step, &colony->header);
    int aimed_away = 0;
    for (int t = 0; t < 12; t++) {
        int direction = directions[t % 4];
        entity_t *before = pad_nav_slipway_target(&step.pad_nav);
        entity_t *expected = slipway_pick(&colony->header, before != NULL ? before : &colony->header, direction);
        vec_t from = camera_at();
        run_frame(&step, direction);
        assert_camera_step_at_most_8(from);
        assert(pad_nav_slipway_target(&step.pad_nav) == (expected != NULL ? expected : before));
        assert(pad_nav_slipway_target(&step.pad_nav) != &colony->header);
        aimed_away += pad_nav_slipway_target(&step.pad_nav) != NULL;
        for (int i = 0; i < 12; i++) {
            from = camera_at();
            run_frame(&step, 0);
            assert_camera_step_at_most_8(from);
        }
        assert_slipway_mode(&step, &colony->header, pad_nav_slipway_target(&step.pad_nav));
    }
    assert(aimed_away > 0);
    for (int i = 0; i <= 34; i++) {
        entity_t *before = pad_nav_slipway_target(&step.pad_nav);
        entity_t *expected = before;
        if (i == 0 || i == 18 || i == 26 || i == 34) {
            entity_t *picked = slipway_pick(&colony->header, before != NULL ? before : &colony->header, P8_PAD_RIGHT);
            expected = picked != NULL ? picked : before;
        }
        run_frame(&step, P8_PAD_RIGHT);
        assert(pad_nav_slipway_target(&step.pad_nav) == expected);
    }
    hold(&step, 0, 40);
    entity_t *target = pad_nav_slipway_target(&step.pad_nav);
    assert(target != NULL);
    int x, y;
    rest_screen_point(target, &x, &y);
    assert(pad_jump_inside(x, y));
    assert_slipway_mode(&step, &colony->header, target);
}

/* From b, Left would aim back at the start a without the exclusion: no other object lies left of b. */
static void test_no_direction_aims_at_the_start(void) {
    input_step_t step;
    planet_t *a, *b, *c;
    start_game_in_jump_mode(&step);
    settled_row(&step, &a, &b, &c);
    hover_object(&step, &a->header);
    start_slipway(&step, &a->header);
    tap(&step, P8_PAD_RIGHT);
    assert_slipway_mode(&step, &a->header, &b->header);
    assert(expected_jump(&b->header, P8_PAD_LEFT) == &a->header);
    tap(&step, P8_PAD_LEFT);
    assert_slipway_mode(&step, &a->header, &b->header);
}

static planet_t *gate_hovered_full(input_step_t *step) {
    planet_t *gate = full_slipgate_under_centre(step);
    hover_object(step, &gate->header);
    return gate;
}

/* No button on the × frame or the next 10, nothing held, the mode still jump. */
static void assert_cross_starts_nothing(input_step_t *step, const entity_t *hovered) {
    world_count_t before = world_count();
    for (int i = 0; i < 11; i++) {
        run_frame(step, i == 0 ? P8_PAD_CROSS : 0);
        assert(input_step_played_frame(step)->mouse_buttons == 0);
        assert(pad_nav_jump_mode(&step->pad_nav) && pad_nav_slipway_start(&step->pad_nav) == NULL);
        assert(hovered == NULL || pad_nav_hovered(&step->pad_nav) == hovered);
    }
    assert(ui_left_held(main_ui()) == NULL);
    assert_world_unchanged(before);
}

static void test_a_refused_start_holds_nothing(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    if (in_forked_copy()) {
        planet_t *gate = gate_hovered_full(&step);
        assert_cross_starts_nothing(&step, &gate->header);
        end_forked_copy();
    }
    if (in_forked_copy()) {
        planet_t *colony = hovered_colony_near_centre(&step);
        cover_with_icon(live_of_class(structure_icon_class()), colony);
        assert_cross_starts_nothing(&step, &colony->header);
        end_forked_copy();
    }
    planet_t *far = planet_farthest_outside();
    settle_planet(&step, far);
    int x, y;
    rest_screen_point(&far->header, &x, &y);
    assert(pad_nav_hovered(&step.pad_nav) == &far->header && !pad_jump_inside(x, y));
    assert_cross_starts_nothing(&step, &far->header);
}

static void test_cross_with_nothing_hovered_presses_nothing(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    assert(pad_nav_hovered(&step.pad_nav) == NULL);
    assert_cross_starts_nothing(&step, NULL);
}

/* start settled and hovered, then × on it. */
static void slipway_from(input_step_t *step, planet_t *start) {
    settle_planet(step, start);
    hover_object(step, &start->header);
    start_slipway(step, &start->header);
}

static void assert_target_refused(input_step_t *step, const entity_t *start, const entity_t *target) {
    aim_at(step, target);
    world_count_t before = world_count();
    cross_on_target(step);
    assert_slipway_mode(step, start, NULL);
    assert_world_unchanged(before);
    hold(step, 0, 10);
    assert_slipway_mode(step, start, NULL);
}

static void test_a_refused_target_aims_again_from_the_same_start(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game_in_jump_mode(&step);
    if (in_forked_copy()) {
        find_pair(near_pair, &start, &target);
        make_known(target);
        slipway_from(&step, start);
        assert_target_refused(&step, &start->header, &target->header);
        end_forked_copy();
    }
    if (in_forked_copy()) {
        find_pair(too_far_pair, &start, &target);
        settle_planet(&step, target);
        slipway_from(&step, start);
        assert_target_refused(&step, &start->header, &target->header);
        end_forked_copy();
    }
    if (in_forked_copy()) {
        find_pair(near_pair, &start, &target);
        settle_planet(&step, target);
        slipway_from(&step, start);
        main_economy_typed()->credits = p8_fixed_from_int(0);
        assert_target_refused(&step, &start->header, &target->header);
        end_forked_copy();
    }
    find_pair(near_pair, &start, &target);
    settle_planet(&step, target);
    slipway_from(&step, start);
    aim_at(&step, &target->header);
    cross_on_target(&step);
    assert(slipway_between(&start->header, &target->header) != NULL);
    assert_slipway_mode(&step, &target->header, NULL);
    assert_target_refused(&step, &target->header, &start->header);
}

static void test_circle_cancels_the_slipway_back_on_its_start(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game_in_jump_mode(&step);
    find_pair(near_pair, &start, &target);
    settle_planet(&step, target);
    slipway_from(&step, start);
    world_count_t before = world_count();
    for (int aimed = 0; aimed < 2; aimed++) {
        if (aimed) {
            start_slipway(&step, &start->header);
            aim_at(&step, &target->header);
        }
        run_frame(&step, P8_PAD_CIRCLE);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        assert(ui_left_held(main_ui()) == NULL && slipways_left_placing() == 0);
        run_frame(&step, 0);
        assert_hovering(&step, &start->header);
        assert_world_unchanged(before);
        hold(&step, 0, 40);
    }
}

/* The edge scroll keeps the target just inside; a pan of the camera then puts the start off screen. */
static void test_an_off_screen_start_still_builds(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game_in_jump_mode(&step);
    find_pair(off_screen_pair, &start, &target);
    settle_planet(&step, target);
    slipway_from(&step, start);
    aim_at(&step, &target->header);
    int x, y, sx, sy;
    rest_screen_point(&target->header, &x, &y);
    rest_screen_point(&start->header, &sx, &sy);
    camera_pan(main_camera_typed(), 0, sy < y ? y - 20 : y - 88);
    run_frame(&step, 0);
    vec_t at = vec_sub(entity_pos(&start->header), camera_at());
    assert(off_screen(p8_fixed_flr_int(at.x), p8_fixed_flr_int(at.y)));
    world_count_t before = world_count();
    cross_on_target(&step);
    assert(slipway_between(&start->header, &target->header) != NULL);
    assert(world_count().slipways == before.slipways + 1);
    assert_slipway_mode(&step, &target->header, NULL);
}

/* The frame after the release judges the drop: Start and the shoulders wait for it. */
static void test_start_and_shoulders_wait_on_the_frame_after_a_slipway_release(void) {
    static const int buttons[2] = { P8_PAD_START, P8_PAD_R1 };
    input_step_t step;
    planet_t *a, *b, *c;
    start_game_in_jump_mode(&step);
    settled_row(&step, &a, &b, &c);
    hover_object(&step, &a->header);
    start_slipway(&step, &a->header);
    tap(&step, P8_PAD_RIGHT);
    assert_slipway_mode(&step, &a->header, &b->header);
    for (int i = 0; i < 2; i++) {
        if (in_forked_copy()) {
            world_count_t before = world_count();
            run_frame(&step, P8_PAD_CROSS);
            run_frame(&step, buttons[i]);
            assert(!pause_menu_shown() && on_the_map(&step.pad_nav));
            run_frame(&step, 0);
            assert(!pause_menu_shown() && on_the_map(&step.pad_nav));
            assert(slipway_between(&a->header, &b->header) != NULL && world_count().slipways == before.slipways + 1);
            assert_slipway_mode(&step, &b->header, NULL);
            end_forked_copy();
        }
    }
}

/* A HUD item over the new start would take the re-press: none goes out, and jump mode hovers that start. */
static void test_no_re_press_while_the_hud_covers_the_new_start(void) {
    input_step_t step;
    planet_t *a, *b, *c;
    start_game_in_jump_mode(&step);
    settled_row(&step, &a, &b, &c);
    hover_object(&step, &a->header);
    start_slipway(&step, &a->header);
    tap(&step, P8_PAD_RIGHT);
    assert_slipway_mode(&step, &a->header, &b->header);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_CROSS);
    int x, y;
    pad_panel_focus_point(basic_structure_icon(), &x, &y);
    vec_t at = vec_sub(entity_pos(&b->header), camera_at());
    camera_pan(main_camera_typed(), p8_fixed_flr_int(at.x) - x, p8_fixed_flr_int(at.y) - y);
    run_frame(&step, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
    assert(pad_nav_jump_mode(&step.pad_nav) && pad_nav_hovered(&step.pad_nav) == &b->header);
    assert(slipway_between(&a->header, &b->header) != NULL && world_count().slipways == before.slipways + 1);
    hold(&step, 0, 10);
    assert(ui_left_held(main_ui()) == NULL && slipways_left_placing() == 0);
}

/* After pause, a tab visit or a box, the cancelled slipway left nothing held or built and jump mode hovers its start. */
static void test_pause_a_tab_change_and_a_box_cancel_the_slipway(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game_in_jump_mode(&step);
    find_pair(near_pair, &start, &target);
    make_known(target);
    slipway_from(&step, start);
    aim_at(&step, &target->header);
    world_count_t before = world_count();
    if (in_forked_copy()) {
        tap(&step, P8_PAD_START);
        settle(&step);
        assert(pause_menu_shown() && ui_left_held(main_ui()) == NULL && slipways_left_placing() == 0);
        tap(&step, P8_PAD_START);
        settle(&step);
        assert(!pause_menu_shown());
        assert_hovering(&step, &start->header);
        assert_world_unchanged(before);
        end_forked_copy();
    }
    if (in_forked_copy()) {
        tap(&step, P8_PAD_R1);
        assert(pad_nav_tech_open(&step.pad_nav) && main_ui()->ephemerals.count > 0);
        assert(ui_left_held(main_ui()) == NULL && slipways_left_placing() == 0);
        tap(&step, P8_PAD_CIRCLE);
        assert(on_the_map(&step.pad_nav));
        run_frame(&step, 0);
        assert_hovering(&step, &start->header);
        assert_world_unchanged(before);
        end_forked_copy();
    }
    (void)planet_left_press(target);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, 0);
    assert(ui_left_held(main_ui()) == NULL && slipways_left_placing() == 0);
    tap(&step, P8_PAD_CIRCLE);
    assert(main_ui()->ephemerals.count == 0);
    run_frame(&step, 0);
    assert_hovering(&step, &start->header);
    assert_world_unchanged(before);
}

/* The target is a colony, so a plain release on it would build: the drop has no target. */
static void test_a_start_that_dies_drops_the_slipway(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game_in_jump_mode(&step);
    find_pair(near_pair, &start, &target);
    settle_planet(&step, target);
    slipway_from(&step, start);
    aim_at(&step, &target->header);
    start->header.is_dead = 1;
    assert(pad_nav_slipway_start(&step.pad_nav) == NULL);
    run_frame(&step, 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0 && ui_left_held(main_ui()) == NULL);
    run_frame(&step, 0);
    assert_hovering(&step, &target->header);
    assert(world_count().slipways == 0 && slipways_left_placing() == 0);
}

static void test_triangle_and_square_do_nothing_in_slipway_mode(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game_in_jump_mode(&step);
    find_pair(near_pair, &start, &target);
    make_known(target);
    slipway_from(&step, start);
    aim_at(&step, &target->header);
    world_count_t before = world_count();
    vec_t from = camera_at();
    static const int presses[] = { P8_PAD_TRIANGLE, 0, P8_PAD_SQUARE, 0, P8_PAD_TRIANGLE | P8_PAD_SQUARE, 0 };
    for (int i = 0; i < 6; i++) {
        run_frame(&step, presses[i]);
        assert_slipway_mode(&step, &start->header, &target->header);
    }
    assert(main_ui()->ephemerals.count == 0);
    assert_moved(from, 0, 0);
    assert_world_unchanged(before);
}

static void test_chords_in_slipway_mode_neither_build_nor_cancel(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game_in_jump_mode(&step);
    find_pair(near_pair, &start, &target);
    settle_planet(&step, target);
    slipway_from(&step, start);
    aim_at(&step, &target->header);
    world_count_t before = world_count();
    assert(type_chords_playing(&step, P8_MOUSE_LEFT) == 1);
    hold(&step, 0, 4);
    assert_slipway_mode(&step, &start->header, &target->header);
    assert(world_count().slipways == before.slipways && slipway_between(&start->header, &target->header) == NULL);
    assert(main_economy_typed()->credits == p8_fixed_from_int(200));
}

/* × D-pad × ○ ○, twice: build a slipway, back out, and again; the cheat never fires. */
static void test_building_and_backing_out_never_types_the_cheat(void) {
    input_step_t step;
    planet_t *a, *b, *c;
    start_game_in_jump_mode(&step);
    settled_row(&step, &a, &b, &c);
    hover_object(&step, &a->header);
    world_count_t before = world_count();
    cheats_fired = 0;
    planet_t *starts[2] = { a, b };
    planet_t *ends[2] = { b, c };
    for (int round = 0; round < 2; round++) {
        start_slipway(&step, &starts[round]->header);
        tap(&step, P8_PAD_RIGHT);
        cross_on_target(&step);
        assert_slipway_mode(&step, &ends[round]->header, NULL);
        tap(&step, P8_PAD_CIRCLE);
        assert_hovering(&step, &ends[round]->header);
        tap(&step, P8_PAD_CIRCLE);
        assert_hovering(&step, &ends[round]->header);
    }
    assert(cheats_fired == 0 && world_count().slipways == before.slipways + 2);
    assert(main_economy_typed()->credits != p8_fixed_from_int(200));
}

/* a is the last object × acted on (a slipway started from it and cancelled), b hovered beside it. */
static void acted_on_a_then_on_b(input_step_t *step, planet_t **a, planet_t **b) {
    planet_t *c;
    start_game_in_jump_mode(step);
    settled_row(step, a, b, &c);
    hover_object(step, &(*a)->header);
    start_slipway(step, &(*a)->header);
    tap(step, P8_PAD_CIRCLE);
    tap(step, P8_PAD_RIGHT);
    hold(step, 0, 40);
    assert_hovering(step, &(*b)->header);
    assert(pad_nav_last_acted(&step->pad_nav) == &(*a)->header);
}

/* ○ at the root: the hovered object becomes target on this frame, with no button. */
static void circle_jumps_back_to(input_step_t *step, const entity_t *target) {
    run_frame(step, P8_PAD_CIRCLE);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    assert(pad_nav_jump_mode(&step->pad_nav) && pad_nav_hovered(&step->pad_nav) == target);
    hold(step, 0, 40);
    assert_hovering(step, target);
}

static void test_circle_cancels_a_slipway_onto_its_start_and_the_next_does_nothing(void) {
    input_step_t step;
    planet_t *a, *b, *c;
    start_game_in_jump_mode(&step);
    settled_row(&step, &a, &b, &c);
    hover_object(&step, &a->header);
    world_count_t before = world_count();
    start_slipway(&step, &a->header);
    tap(&step, P8_PAD_RIGHT);
    assert_slipway_mode(&step, &a->header, &b->header);
    tap(&step, P8_PAD_CIRCLE);
    assert_hovering(&step, &a->header);
    vec_t from = camera_at();
    tap(&step, P8_PAD_CIRCLE);
    assert_hovering(&step, &a->header);
    assert_moved(from, 0, 0);
    assert_world_unchanged(before);
}

/* Each ○ does one rung: cancel the structure, leave aim mode, then jump back to a. */
static void test_circle_cancels_a_structure_then_leaves_aim_mode_then_jumps_back(void) {
    input_step_t step;
    planet_t *a, *b;
    acted_on_a_then_on_b(&step, &a, &b);
    open_build_bar(&step);
    entity_t *structure = take_structure(&step);
    tap(&step, P8_PAD_CIRCLE);
    assert(structure->is_dead && pad_nav_placing(&step.pad_nav) == NULL && pad_nav_aim_mode(&step.pad_nav));
    tap(&step, P8_PAD_CIRCLE);
    assert(pad_nav_jump_mode(&step.pad_nav));
    entity_t *nearest = pad_nav_hovered(&step.pad_nav);
    assert(nearest != NULL);
    if (nearest != &a->header) {
        circle_jumps_back_to(&step, &a->header);
    }
}

static void test_circle_at_the_root_jumps_back_and_edge_scrolls_at_8_px_a_frame(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    planet_t *colony = planet_closest_to_centre();
    settle_planet(&step, colony);
    hover_object(&step, &colony->header);
    start_slipway(&step, &colony->header);
    tap(&step, P8_PAD_CIRCLE);
    planet_t *far = planet_farthest_outside();
    assert(far != NULL && far != colony);
    hover_object(&step, &far->header);
    int x, y;
    rest_screen_point(&colony->header, &x, &y);
    assert(!pad_jump_inside(x, y) && "the colony is out of the inner rectangle once the far planet is hovered");
    run_frame(&step, P8_PAD_CIRCLE);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert(pad_nav_hovered(&step.pad_nav) == &colony->header);
    for (int i = 0; i < 60 && !pad_jump_inside(x, y); i++) {
        vec_t from = camera_at();
        run_frame(&step, 0);
        vec_t moved = vec_sub(camera_at(), from);
        assert(absolute_int(p8_fixed_flr_int(moved.x)) <= PAD_JUMP_SCROLL_SPEED);
        assert(absolute_int(p8_fixed_flr_int(moved.y)) <= PAD_JUMP_SCROLL_SPEED);
        rest_screen_point(&colony->header, &x, &y);
    }
    assert(pad_jump_inside(x, y));
    run_frame(&step, 0);
    assert_hovering(&step, &colony->header);
    assert(ui_pad_selection(main_ui()) == &colony->header);
}

/* A resumed game starts with nothing to go back to. */
static void test_circle_after_a_resume_does_nothing(void) {
    input_step_t step;
    planet_t *a, *b;
    acted_on_a_then_on_b(&step, &a, &b);
    economy_new_year(main_economy_typed());
    run_frame(&step, 0);
    open_pause(&step);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() == NULL && title_button("resume") != NULL);
    tap(&step, P8_PAD_UP);
    tap(&step, P8_PAD_CROSS);
    settle(&step);
    assert(main_economy_typed() != NULL && pad_nav_last_acted(&step.pad_nav) == NULL);
    entity_t *hovered = pad_nav_hovered(&step.pad_nav);
    vec_t from = camera_at();
    world_count_t before = world_count();
    for (int i = 0; i < 4; i++) {
        run_frame(&step, i % 2 == 0 ? P8_PAD_CIRCLE : 0);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
        assert(pad_nav_hovered(&step.pad_nav) == hovered);
    }
    assert_moved(from, 0, 0);
    assert_world_unchanged(before);
}

static void test_circle_with_the_last_object_gone_or_in_a_new_game_does_nothing(void) {
    input_step_t step;
    planet_t *a, *b;
    acted_on_a_then_on_b(&step, &a, &b);
    if (in_forked_copy()) {
        a->header.is_dead = 1;
        run_frame(&step, 0);
        assert(pad_nav_last_acted(&step.pad_nav) == NULL);
        vec_t from = camera_at();
        tap(&step, P8_PAD_CIRCLE);
        assert_hovering(&step, &b->header);
        assert_moved(from, 0, 0);
        end_forked_copy();
    }
    restart_into_a_new_game(&step);
    settle(&step);
    assert(pad_nav_last_acted(&step.pad_nav) == NULL);
    entity_t *hovered = pad_nav_hovered(&step.pad_nav);
    vec_t from = camera_at();
    tap(&step, P8_PAD_CIRCLE);
    assert(pad_nav_jump_mode(&step.pad_nav) && pad_nav_hovered(&step.pad_nav) == hovered);
    assert_moved(from, 0, 0);
}

/* Ordinary ×, △ and ○ presses with D-pad moves between them: no cheat and no game number drawn. */
static void test_ordinary_cross_triangle_and_circle_play_never_types_the_cheat(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *colony = most_isolated_planet();
    planet_t *known = NULL;
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    for (int i = 0; known == NULL; i++) {
        if (planets->items[i] != &colony->header) {
            known = (planet_t *)planets->items[i];
        }
    }
    place_near(known, colony, 10, 0);
    settle_planet(&step, colony);
    make_known(known);
    hover_object(&step, &known->header);
    cheats_fired = 0;
    uint32_t game = p8_fixed_rnd_state();
    world_count_t before = world_count();
    tap(&step, P8_PAD_CROSS);
    assert(pad_nav_panel_open());
    tap(&step, P8_PAD_CIRCLE);
    assert(!pad_nav_panel_open());
    tap(&step, P8_PAD_CROSS);
    assert(pad_nav_panel_open());
    tap(&step, P8_PAD_TRIANGLE);
    assert(!pad_nav_panel_open());
    tap(&step, P8_PAD_TRIANGLE);
    settle(&step);
    assert(pad_nav_panel_open());
    tap(&step, P8_PAD_TRIANGLE);
    assert(!pad_nav_panel_open());
    tap(&step, P8_PAD_LEFT);
    hold(&step, 0, 40);
    assert_hovering(&step, &colony->header);
    start_slipway(&step, &colony->header);
    tap(&step, P8_PAD_CIRCLE);
    tap(&step, P8_PAD_CIRCLE);
    tap(&step, P8_PAD_CIRCLE);
    assert_hovering(&step, &colony->header);
    tap(&step, P8_PAD_RIGHT);
    hold(&step, 0, 40);
    assert_hovering(&step, &known->header);
    circle_jumps_back_to(&step, &colony->header);
    assert(cheats_fired == 0);
    assert(p8_fixed_rnd_state() == game && "jumps, menus and toggles draw no game number");
    assert_world_unchanged(before);
}

/* Pause, a tab visit and slipway mode leave the last object to ○. */
static void test_pause_a_tab_and_a_slipway_keep_the_last_object(void) {
    input_step_t step;
    planet_t *a, *b, *c;
    acted_on_a_then_on_b(&step, &a, &b);
    open_pause(&step);
    close_pause(&step, P8_PAD_START);
    circle_jumps_back_to(&step, &a->header);
    tap(&step, P8_PAD_RIGHT);
    hold(&step, 0, 40);
    assert_hovering(&step, &b->header);
    open_build_bar(&step);
    tap(&step, P8_PAD_CIRCLE);
    assert(on_the_map(&step.pad_nav));
    settle(&step);
    if (pad_nav_aim_mode(&step.pad_nav)) {
        tap(&step, P8_PAD_SQUARE);
    }
    assert(pad_nav_jump_mode(&step.pad_nav) && pad_nav_hovered(&step.pad_nav) != &a->header);
    circle_jumps_back_to(&step, &a->header);
    start_slipway(&step, &a->header);
    tap(&step, P8_PAD_RIGHT);
    cross_on_target(&step);
    assert_slipway_mode(&step, &b->header, NULL);
    tap(&step, P8_PAD_CIRCLE);
    assert_hovering(&step, &b->header);
    c = (planet_t *)expected_jump(&b->header, P8_PAD_RIGHT);
    assert(c != NULL);
    tap(&step, P8_PAD_RIGHT);
    hold(&step, 0, 40);
    assert_hovering(&step, &c->header);
    circle_jumps_back_to(&step, &b->header);
}

static void test_building_refusing_and_cancelling_leave_the_game_stream_unchanged(void) {
    input_step_t step;
    planet_t *a, *b, *c;
    start_game_in_jump_mode(&step);
    settled_row(&step, &a, &b, &c);
    hover_object(&step, &a->header);
    uint32_t game = p8_fixed_rnd_state();
    start_slipway(&step, &a->header);
    aim_at(&step, &b->header);
    cross_on_target(&step);
    aim_at(&step, &a->header);
    cross_on_target(&step);
    aim_at(&step, &c->header);
    hold(&step, P8_PAD_LEFT, 40);
    tap(&step, P8_PAD_CIRCLE);
    hold(&step, P8_PAD_RIGHT, 40);
    assert(slipway_between(&a->header, &b->header) != NULL && slipway_between(&b->header, &c->header) == NULL);
    assert(p8_fixed_rnd_state() == game && "slipway mode draws no game number");
}

static void test_colonising_through_the_menu_ends_in_jump_mode_on_the_colony(void) {
    entity_t *rows[ITEMS_CAP];
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
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
    run_frame(&step, 0);
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
    assert(ui_left_held(main_ui()) == NULL && tagged_count(TAG_SLIPWAY) == 0);
}

/* × held across the △: the click still releases on the next frame. */
static void test_triangle_click_releases_even_with_cross_held(void) {
    input_step_t step;
    (void)jump_on_known_planet(&step);
    run_frame(&step, P8_PAD_CROSS | P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    run_frame(&step, P8_PAD_CROSS);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
}

/* A colony's △ opens its panel on the release: × on that frame must start no slipway out of it. */
static void test_cross_on_the_triangle_release_frame_is_ignored(void) {
    input_step_t step;
    planet_t *colony = jump_on_colony(&step);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_TRIANGLE);
    run_frame(&step, P8_PAD_CROSS);
    assert(pad_nav_slipway_start(&step.pad_nav) == NULL && ui_left_held(main_ui()) == NULL);
    assert_cursor_on(&step, &colony->header);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, 0);
    assert_world_unchanged(before);
}

static void test_r1_cycles_tech_score_closed_from_aim_mode_back_in_it_panning_from_slow(void) {
    input_step_t step;
    start_game(&step);
    hold(&step, P8_PAD_RIGHT, 20);
    run_frame(&step, 0);
    vec_t from = camera_at();
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav));
    run_frame(&step, 0);
    tap(&step, P8_PAD_R1);
    assert(pad_nav_score_open(&step.pad_nav));
    run_frame(&step, P8_PAD_R1);
    assert(on_the_map(&step.pad_nav) && !score_panel_shown());
    run_frame(&step, 0);
    assert(backdrop() == NULL);
    assert(pad_nav_aim_mode(&step.pad_nav));
    assert_moved(from, 0, 0);
    assert_world_unchanged(before);
    run_frame(&step, P8_PAD_RIGHT);
    assert_moved(from, 1, 0);
}

static void test_r1_closes_a_planet_menu_and_opens_tech(void) {
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
    tap(&step, P8_PAD_TRIANGLE);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav));
    assert_clicked(&step, 2);
    run_frame(&step, 0);
    assert(main_ui()->ephemerals.count > 0 && pad_nav_focus(&step.pad_nav) == pad_nav_first_item());
    assert(((const button_t *)pad_nav_focus(&step.pad_nav))->on_click == NULL);
    tap(&step, P8_PAD_CIRCLE);
    assert(on_the_map(&step.pad_nav));
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
}

/* A settled planet opens its panel on the release, so no panel is up yet on that frame. */
static void test_shoulders_wait_on_the_triangle_release_frame(void) {
    input_step_t step;
    (void)jump_on_colony(&step);
    run_frame(&step, P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT && main_ui()->ephemerals.count == 0);
    run_frame(&step, P8_PAD_R1);
    assert(on_the_map(&step.pad_nav));
    assert(input_step_played_frame(&step)->mouse_buttons == 0 && main_ui()->ephemerals.count > 0);
}

static void test_start_is_ignored_under_a_planet_menu_and_on_tech_and_score(void) {
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
    if (in_forked_copy()) {
        tap(&step, P8_PAD_TRIANGLE);
        int rows = main_ui()->ephemerals.count;
        assert(rows > 0);
        assert_start_ignored(&step, 0);
        settle(&step);
        assert(main_ui()->ephemerals.count == rows && !pause_menu_shown());
        assert(pad_nav_hovered(&step.pad_nav) == &planet->header);
        end_forked_copy();
    }
    if (in_forked_copy()) {
        open_tech(&step);
        int rows = main_ui()->ephemerals.count;
        assert(rows > 0);
        assert_start_ignored(&step, 0);
        settle(&step);
        assert(main_ui()->ephemerals.count == rows && !pause_menu_shown());
        assert(pad_nav_tech_open(&step.pad_nav));
        end_forked_copy();
    }
    open_score_from_the_map(&step);
    assert_start_ignored(&step, 0);
    settle(&step);
    assert(score_panel_shown() && !pause_menu_shown() && pad_nav_score_open(&step.pad_nav));
}

static void test_start_is_ignored_on_the_triangle_release_frame(void) {
    input_step_t step;
    planet_t *planet = jump_on_colony(&step);
    run_frame(&step, P8_PAD_TRIANGLE);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT && main_ui()->ephemerals.count == 0);
    run_frame(&step, P8_PAD_START);
    assert_cursor_on(&step, &planet->header);
    run_frame(&step, 0);
    assert(!pause_menu_shown() && main_ui()->ephemerals.count > 0);
    assert(pad_nav_hovered(&step.pad_nav) == &planet->header);
}

static void test_no_pad_selection_is_shown_behind_the_bar_or_a_cycle_panel(void) {
    input_step_t step;
    planet_t *colony = jump_on_colony(&step);
    assert(ui_pad_selection(main_ui()) == &colony->header);
    tap(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav) && ui_pad_selection(main_ui()) == NULL);
    tap(&step, P8_PAD_R1);
    assert(pad_nav_score_open(&step.pad_nav) && ui_pad_selection(main_ui()) == NULL);
    /* ○ leaves score through a backdrop click: its release frame shows no selection either. */
    run_frame(&step, P8_PAD_CIRCLE);
    assert(on_the_map(&step.pad_nav) && ui_pad_selection(main_ui()) == NULL);
    run_frame(&step, 0);
    assert(ui_pad_selection(main_ui()) == NULL);
    run_frame(&step, 0);
    assert(ui_pad_selection(main_ui()) == &colony->header);
    tap(&step, P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step.pad_nav) && ui_pad_selection(main_ui()) == NULL);
    tap(&step, P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav) && ui_pad_selection(main_ui()) == NULL);
}

static void test_circle_on_tech_closes_the_box_and_returns_to_the_map_in_one_press(void) {
    input_step_t step;
    planet_t *colony = jump_on_colony(&step);
    vec_t from = camera_at();
    open_tech(&step);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, P8_PAD_CIRCLE);
    assert(on_the_map(&step.pad_nav) && main_ui()->ephemerals.count == 0);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    run_frame(&step, P8_PAD_CIRCLE);
    run_frame(&step, 0);
    assert(main_ui()->ephemerals.count == 0);
    assert_hovering(&step, &colony->header);
    assert_moved(from, 0, 0);
}

static void test_triangle_on_the_release_frame_of_leaving_score_does_nothing(void) {
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
    leave_score_then_triangle_on_the_release_frame(&step);
    assert(pad_nav_hovered(&step.pad_nav) == &planet->header);
    tap(&step, P8_PAD_TRIANGLE);
    assert(main_ui()->ephemerals.count > 0);
}

static void test_a_planet_menu_draws_no_crosshair(void) {
    input_step_t step;
    (void)jump_on_known_planet(&step);
    tap(&step, P8_PAD_TRIANGLE);
    run_frame(&step, 0);
    assert(pad_nav_panel_open() && ui_hovered(main_ui()) == pad_nav_first_item());
    assert_no_crosshair_drawn();
}

static void test_pause_keeps_aim_mode_for_the_next_circle(void) {
    input_step_t step;
    start_game(&step);
    planet_t *planet = known_planet_under_centre(&step);
    hold(&step, P8_PAD_RIGHT, 20);
    run_frame(&step, 0);
    vec_t from = camera_at();
    open_pause(&step);
    close_pause(&step, P8_PAD_CIRCLE);
    assert(pad_nav_aim_mode(&step.pad_nav));
    assert_moved(from, 0, 0);
    tap(&step, P8_PAD_CIRCLE);
    assert(pad_nav_jump_mode(&step.pad_nav) && pad_nav_hovered(&step.pad_nav) == &planet->header);
}

static void test_pause_from_jump_mode_gives_back_the_hovered_object_and_camera(void) {
    input_step_t step;
    planet_t *colony = jump_on_colony(&step);
    vec_t from = camera_at();
    open_pause(&step);
    tap(&step, P8_PAD_UP);
    close_pause(&step, P8_PAD_START);
    run_frame(&step, 0);
    assert_hovering(&step, &colony->header);
    assert_moved(from, 0, 0);
}

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

/* Over one blink period the circle round the hovered object is drawn whole on some frames and not at all on others;
 * a planet menu hides it. */
static void test_the_hovered_object_blinks_a_circle(void) {
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
    int whole = 0, none = 0;
    for (int i = 0; i < UI_PAD_BLINK_PERIOD; i++) {
        run_frame(&step, 0);
        assert_hovering(&step, &planet->header);
        int total;
        cart_draw();
        int lit = circle_pixels_lit_near(input_step_played_frame(&step)->mouse_x, input_step_played_frame(&step)->mouse_y,
                                         &total);
        whole += lit == total;
        none += lit == 0;
    }
    assert(whole > 0 && none > 0 && whole + none == UI_PAD_BLINK_PERIOD);
    tap(&step, P8_PAD_TRIANGLE);
    settle(&step);
    assert(pad_nav_panel_open() && ui_pad_selection(main_ui()) == NULL);
}

/* A colony far off and two known planets near the centre: once the hovered one dies, the reseed takes the object
 * nearest the centre, not the first colony; with every candidate dead, nothing is hovered. */
static void test_a_dead_hovered_object_reseeds_on_the_object_nearest_the_centre(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    planet_t *colony = planet_farthest_outside();
    make_known(colony);
    (void)planet_left_press(colony);
    assert(click_colonise_row());
    ui_clear_ephemerals(main_ui());
    planet_t *near = known_planet_closest_to_centre();
    hover_object(&step, &near->header);
    const entity_list_t *planets = entities_by_tag(TAG_PLANET);
    planet_t *second = NULL;
    vec_t centre = centre_world();
    for (int i = 0; i < planets->count; i++) {
        planet_t *p = (planet_t *)planets->items[i];
        if (p == near || p == colony) {
            continue;
        }
        vec_t gap = vec_sub(entity_pos(&p->header), centre);
        vec_t best = second != NULL ? vec_sub(entity_pos(&second->header), centre) : gap;
        if (second == NULL || vec_length_squared(gap) < vec_length_squared(best)) {
            second = p;
        }
    }
    make_known(second);
    run_frame(&step, 0);
    assert(pad_nav_hovered(&step.pad_nav) == &near->header);
    near->header.is_dead = 1;
    run_frame(&step, 0);
    assert(pad_nav_hovered(&step.pad_nav) == &second->header && "the nearest, not the first colony");
    second->header.is_dead = 1;
    colony->header.is_dead = 1;
    run_frame(&step, 0);
    assert(pad_nav_jump_mode(&step.pad_nav) && pad_nav_hovered(&step.pad_nav) == NULL);
    assert_cursor_at_centre(&step);
}

static int node_index(const entity_t *e) {
    const entity_list_t *nodes = entities_by_tag(TAG_NODE);
    for (int i = 0; i < nodes->count; i++) {
        if (nodes->items[i] == e) {
            return i;
        }
    }
    assert(0 && "node_index: the entity is a node");
    return -1;
}

static void satisfy_every_need(planet_t *planet) {
    for (int i = 0; i < planet->needs.count; i++) {
        need_t *need = planet->needs.items[i];
        need->has_satisfied = 1;
        need->satisfied = p8_fixed_from_int(1);
    }
    assert(!pad_select_needs_attention(&planet->header));
}

static int attention_count(void) {
    const entity_list_t *nodes = entities_by_tag(TAG_NODE);
    int count = 0;
    for (int i = 0; i < nodes->count; i++) {
        count += pad_select_needs_attention(nodes->items[i]);
    }
    return count;
}

/* settled_row's three colonies in registration order, the middle one with every need met: the only two needing
 * attention are *first and *last. */
static void attention_row(input_step_t *step, planet_t **first, planet_t **middle, planet_t **last) {
    planet_t *row[3];
    settled_row(step, &row[0], &row[1], &row[2]);
    for (int i = 1; i < 3; i++) {
        for (int j = i; j > 0 && node_index(&row[j]->header) < node_index(&row[j - 1]->header); j--) {
            planet_t *swap = row[j];
            row[j] = row[j - 1];
            row[j - 1] = swap;
        }
    }
    *first = row[0];
    *middle = row[1];
    *last = row[2];
    satisfy_every_need(*middle);
    assert(pad_select_needs_attention(&(*first)->header) && pad_select_needs_attention(&(*last)->header));
    assert(attention_count() == 2);
}

/* A Select tap: its press, then its release frame, where the tap acts. */
static void select_tap(input_step_t *step) {
    run_frame(step, P8_PAD_SELECT);
    assert(input_step_played_frame(step)->mouse_buttons == 0);
    run_frame(step, 0);
}

static void assert_select_tap_lands_on(input_step_t *step, const entity_t *target) {
    vec_t from = camera_at();
    select_tap(step);
    vec_t moved = vec_sub(camera_at(), from);
    assert(absolute_int(p8_fixed_flr_int(moved.x)) <= 8 && absolute_int(p8_fixed_flr_int(moved.y)) <= 8);
    assert_hovering(step, target);
}

static void test_select_taps_cycle_the_colonies_needing_attention_in_order(void) {
    input_step_t step;
    planet_t *first, *middle, *last;
    start_game_in_jump_mode(&step);
    attention_row(&step, &first, &middle, &last);
    hover_object(&step, &first->header);
    entity_t *last_acted = pad_nav_last_acted(&step.pad_nav);
    uint32_t game = p8_fixed_rnd_state();
    assert_select_tap_lands_on(&step, &last->header);
    assert_select_tap_lands_on(&step, &first->header);
    hover_object(&step, &middle->header);
    assert_select_tap_lands_on(&step, &last->header);
    assert(pad_nav_last_acted(&step.pad_nav) == last_acted);
    assert(p8_fixed_rnd_state() == game && "a Select tap draws no game number");
}

/* The tap's own frame scrolls, as a D-pad jump's does, then each frame at most 8 px per axis until the colony is inside. */
static void test_a_select_tap_to_a_colony_outside_edge_scrolls_at_8_px_a_frame(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    planet_t *near = planet_closest_to_centre();
    planet_t *far = planet_farthest_outside();
    assert(near != far);
    settle_planet(&step, near);
    settle_planet(&step, far);
    assert(attention_count() == 2);
    hover_object(&step, &near->header);
    int x, y;
    rest_screen_point(&far->header, &x, &y);
    assert(!pad_jump_inside(x, y));
    int frames = 0;
    vec_t from = camera_at();
    run_frame(&step, P8_PAD_SELECT);
    run_frame(&step, 0);
    assert(pad_nav_hovered(&step.pad_nav) == &far->header);
    for (;;) {
        vec_t moved = vec_sub(camera_at(), from);
        int dx = absolute_int(p8_fixed_flr_int(moved.x)), dy = absolute_int(p8_fixed_flr_int(moved.y));
        assert(dx <= 8 && dy <= 8);
        assert((frames > 0 || dx + dy > 0) && "the tap's release frame scrolls");
        rest_screen_point(&far->header, &x, &y);
        if (pad_jump_inside(x, y)) {
            break;
        }
        assert(++frames < 200);
        from = camera_at();
        run_frame(&step, 0);
    }
    assert_hovering(&step, &far->header);
}


static void test_a_select_hold_under_30_frames_ends_nothing(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    entity_t *hovered = hover_settled(&step);
    year_facts_t before = year_facts();
    hold_select_pressing_nothing(&step, PAD_SELECT_HOLD_FRAMES - 1);
    assert(pad_nav_select_held(&step.pad_nav) == PAD_SELECT_HOLD_FRAMES - 1);
    hold(&step, 0, 2);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    assert_years_ended(before, 0);
    assert_hovering(&step, hovered);
    assert(pad_nav_select_held(&step.pad_nav) == 0 && !pad_nav_year_chain_open(&step.pad_nav));
}

static void test_a_select_hold_of_30_frames_presses_the_year_skip_button_once(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    entity_t *hovered = hover_settled(&step);
    vec_t from = camera_at();
    year_facts_t before = year_facts();
    entity_t *last_acted = pad_nav_last_acted(&step.pad_nav);
    hold_select_to_the_year(&step);
    assert_cursor_on(&step, hovered);
    assert(pad_nav_hovered(&step.pad_nav) == hovered);
    assert_years_ended(before, 1);
    hold(&step, 0, 1);
    assert_hovering(&step, hovered);
    assert_moved(from, 0, 0);
    assert(pad_nav_last_acted(&step.pad_nav) == last_acted);
}

static void test_a_select_hold_of_90_frames_ends_one_year(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    year_facts_t before = year_facts();
    hold_select_to_the_year(&step);
    hold_select_pressing_nothing(&step, 90 - PAD_SELECT_HOLD_FRAMES - 1);
    run_frame(&step, 0);
    assert_years_ended(before, 1);
}

static void test_fresh_select_presses_after_a_year_chain_more_years(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    year_facts_t before = year_facts();
    uint32_t game = p8_fixed_rnd_state();
    int x, y;
    year_skip_point(&x, &y);
    hold_select_to_the_year(&step);
    run_frame(&step, 0);
    for (int year = 2; year <= 7; year++) {
        run_frame(&step, P8_PAD_SELECT);
        assert_played(&step, x, y, P8_MOUSE_LEFT);
        run_frame(&step, 0);
        assert_years_ended(before, year);
    }
    assert(pad_nav_year_chain_open(&step.pad_nav));
    hold(&step, 0, PAD_SELECT_CHAIN_FRAMES - 1);
    assert(!pad_nav_year_chain_open(&step.pad_nav));
    hold_select_pressing_nothing(&step, 1);
    assert(pad_nav_select_held(&step.pad_nav) == 1);
    run_frame(&step, 0);
    assert_years_ended(before, 7);
    assert(p8_fixed_rnd_state() == game && "a year end draws no game number");
}

static void test_a_select_hold_ends_the_year_under_the_year_box(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    score_panel_new_year(p8_fixed_from_int(3421));
    assert(shown_message_box() != NULL);
    run_frame(&step, 0);
    year_facts_t before = year_facts();
    hold_select_to_the_year(&step);
    assert_years_ended(before, 1);
}

static void test_select_in_aim_mode_ends_the_year_and_taps_back_to_jump_mode(void) {
    input_step_t step;
    start_game(&step);
    year_facts_t before = year_facts();
    hold_select_to_the_year(&step);
    assert_cursor_at_centre(&step);
    assert(pad_nav_aim_mode(&step.pad_nav));
    assert_years_ended(before, 1);
    planet_t *colony = planet_closest_to_centre();
    make_known(colony);
    settle_planet(&step, colony);
    settle(&step);
    assert(pad_select_needs_attention(&colony->header) && attention_count() == 1);
    assert(pad_nav_aim_mode(&step.pad_nav));
    select_tap(&step);
    assert_hovering(&step, &colony->header);
}

static void test_a_select_tap_with_nothing_needing_attention_changes_nothing(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    entity_t *hovered = hover_settled(&step);
    assert(attention_count() == 0);
    vec_t from = camera_at();
    select_tap(&step);
    assert_hovering(&step, hovered);
    assert_moved(from, 0, 0);
    leave_jump_mode(&step);
    select_tap(&step);
    assert(pad_nav_aim_mode(&step.pad_nav));
    assert_cursor_at_centre(&step);
}

/* A 40-frame Select hold, then a tap, with held kept down: each frame plays mouse_buttons, and no year ends. */
static void assert_select_inert(input_step_t *step, int held, int mouse_buttons) {
    p8_fixed_t year = main_economy_typed()->year;
    for (int i = 0; i < 40; i++) {
        run_frame(step, held | P8_PAD_SELECT);
        assert(input_step_played_frame(step)->mouse_buttons == mouse_buttons);
    }
    run_frame(step, held);
    run_frame(step, held | P8_PAD_SELECT);
    assert(input_step_played_frame(step)->mouse_buttons == mouse_buttons);
    run_frame(step, held);
    assert(input_step_played_frame(step)->mouse_buttons == mouse_buttons);
    assert(main_economy_typed()->year == year && !pad_nav_year_chain_open(&step->pad_nav));
}

static void test_select_does_nothing_while_something_is_held_or_open(void) {
    static const off_map_t places[] = { THE_BAR, THE_TECH_PANEL, THE_SCORE_PANEL };
    input_step_t step;
    planet_t *first, *middle, *last;
    start_game_in_jump_mode(&step);
    attention_row(&step, &first, &middle, &last);
    hover_object(&step, &first->header);
    if (in_forked_copy()) {
        start_slipway(&step, &first->header);
        assert_select_inert(&step, 0, P8_MOUSE_LEFT);
        assert_slipway_mode(&step, &first->header, NULL);
        end_forked_copy();
    }
    if (in_forked_copy()) {
        open_build_bar(&step);
        entity_t *structure = take_structure(&step);
        assert_select_inert(&step, 0, P8_MOUSE_LEFT);
        assert_carrying(&step, structure);
        end_forked_copy();
    }
    if (in_forked_copy()) {
        leave_jump_mode(&step);
        pan_to_empty_space(&step);
        tap(&step, P8_PAD_CROSS);
        settle(&step);
        run_frame(&step, P8_PAD_CROSS);
        assert_select_inert(&step, P8_PAD_CROSS, P8_MOUSE_LEFT);
        end_forked_copy();
    }
    if (in_forked_copy()) {
        tap(&step, P8_PAD_TRIANGLE);
        settle(&step);
        assert(pad_nav_panel_open() && main_ui()->ephemerals.count > 0);
        assert_select_inert(&step, 0, 0);
        assert(pad_nav_panel_open());
        end_forked_copy();
    }
    if (in_forked_copy()) {
        tap(&step, P8_PAD_START);
        settle(&step);
        assert(pause_menu_shown());
        assert_select_inert(&step, 0, 0);
        assert(pause_menu_shown());
        end_forked_copy();
    }
    for (int i = 0; i < (int)(sizeof places / sizeof places[0]); i++) {
        if (in_forked_copy()) {
            open_from_the_map(&step, places[i]);
            settle(&step);
            assert_select_inert(&step, 0, 0);
            assert(is_open(&step.pad_nav, places[i]));
            end_forked_copy();
        }
    }
}

/* × picking a pause row presses the left button with no release frame after it, and pause closes on that press: the
 * next frame's Select press sees the left button down. */
static void test_a_refused_year_press_presses_nothing_and_is_not_retried(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    hover_settled(&step);
    year_facts_t before = year_facts();
    hold_select_to_the_year(&step);
    run_frame(&step, 0);
    open_pause(&step);
    tap(&step, P8_PAD_UP);
    run_frame(&step, P8_PAD_CROSS);
    assert(!pause_menu_shown() && pad_nav_year_chain_open(&step.pad_nav));
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    if (in_forked_copy()) {
        run_frame(&step, P8_PAD_CROSS);
        run_frame(&step, P8_PAD_CROSS | P8_PAD_SELECT);
        assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
        run_frame(&step, 0);
        assert_years_ended(before, 2);
        end_forked_copy();
    }
    run_frame(&step, P8_PAD_CROSS | P8_PAD_SELECT);
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    hold_select_pressing_nothing(&step, 60);
    hold(&step, 0, 2);
    assert_years_ended(before, 1);
    assert(!pad_nav_year_chain_open(&step.pad_nav));
}

static void test_a_new_game_restarts_the_select_count(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    hold_select_pressing_nothing(&step, PAD_SELECT_HOLD_FRAMES - 1);
    cart_start_game(NULL);
    year_facts_t before = year_facts();
    hold_select_pressing_nothing(&step, PAD_SELECT_HOLD_FRAMES - 1);
    assert_years_ended(before, 0);
    run_frame(&step, P8_PAD_SELECT);
    assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
    run_frame(&step, 0);
    assert_years_ended(before, 1);
}

static void test_select_is_no_part_of_the_cheat_chord(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    make_every_planet_known();
    planet_t *colony = planet_closest_to_centre();
    settle_planet(&step, colony);
    hold(&step, 0, 40);
    assert_hovering(&step, &colony->header);
    world_count_t before = world_count();
    p8_fixed_t year = main_economy_typed()->year;
    hold_select_pressing_nothing(&step, 10);
    int fired = cheats_fired;
    for (int i = 0; i < 2 * CHEAT_CHORDS; i++) {
        run_frame(&step, P8_PAD_SELECT | (i % 2 == 0 ? CHORD : 0));
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    hold_select_pressing_nothing(&step, PAD_SELECT_HOLD_FRAMES - 1 - 10 - 2 * CHEAT_CHORDS);
    run_frame(&step, 0);
    assert(cheats_fired == fired + 1);
    before.credits = main_economy_typed()->credits;
    assert_world_unchanged(before);
    assert(main_economy_typed()->year == year);
    assert_hovering(&step, &colony->header);
    static const int ordinary[] = { P8_PAD_CROSS, P8_PAD_CROSS, P8_PAD_CIRCLE, P8_PAD_CIRCLE,
                                    P8_PAD_CROSS, P8_PAD_CROSS, P8_PAD_CIRCLE, P8_PAD_CIRCLE };
    fired = cheats_fired;
    for (int i = 0; i < (int)(sizeof ordinary / sizeof ordinary[0]); i++) {
        tap(&step, ordinary[i] | P8_PAD_SELECT);
        tap(&step, P8_PAD_SELECT);
    }
    assert(cheats_fired == fired);
}

static void test_l1_opens_the_bar_from_jump_mode_and_l1_closes_it_on_the_same_hovered_object(void) {
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
    vec_t from = camera_at();
    run_frame(&step, P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    assert_build_focus(&step, basic_structure_icon());
    run_frame(&step, 0);
    assert_build_focus(&step, basic_structure_icon());
    tap(&step, P8_PAD_L1);
    assert(!pad_nav_build_bar_open(&step.pad_nav) && on_the_map(&step.pad_nav));
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
    assert_moved(from, 0, 0);
}

static void test_circle_closes_the_bar_on_the_same_hovered_object(void) {
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
    vec_t from = camera_at();
    open_build_bar(&step);
    assert_build_focus(&step, basic_structure_icon());
    tap(&step, P8_PAD_CIRCLE);
    assert(!pad_nav_build_bar_open(&step.pad_nav));
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
    assert_moved(from, 0, 0);
}

static void test_l1_twice_in_aim_mode_comes_back_to_aim_mode_with_the_pan_slow_again(void) {
    input_step_t step;
    start_game(&step);
    hold(&step, P8_PAD_RIGHT, 20);
    run_frame(&step, 0);
    vec_t from = camera_at();
    open_build_bar(&step);
    tap(&step, P8_PAD_L1);
    assert(!pad_nav_build_bar_open(&step.pad_nav));
    assert(pad_nav_aim_mode(&step.pad_nav));
    assert_moved(from, 0, 0);
    assert_cursor_at_centre(&step);
    run_frame(&step, P8_PAD_RIGHT);
    assert_moved(from, 1, 0);
}

static void test_the_bar_takes_and_aim_mode_places_or_cancels(void) {
    input_step_t step;
    start_game_in_jump_mode(&step);
    hold(&step, 0, 2);
    open_build_bar(&step);
    entity_t *structure = take_structure(&step);
    assert(pad_nav_aim_mode(&step.pad_nav) && !pad_nav_build_bar_open(&step.pad_nav));
    assert_no_crosshair_drawn();
    pan_clear_of_every_node(&step, structure);
    world_count_t before = world_count();
    p8_fixed_t cost = targeting_of(structure)->cost;
    run_frame(&step, P8_PAD_CROSS);
    run_frame(&step, 0);
    assert(!structure->is_dead && structure->state == ENTITY_STATE_SETTLED);
    assert(world_count().credits == p8_fixed_sub(before.credits, cost));
    assert_placement_over(&step);
    open_build_bar(&step);
    structure = take_structure(&step);
    before = world_count();
    tap(&step, P8_PAD_CIRCLE);
    assert(structure->is_dead);
    assert_world_unchanged(before);
    assert_placement_over(&step);
    tap(&step, P8_PAD_CIRCLE);
    assert(pad_nav_jump_mode(&step.pad_nav) && !pad_nav_build_bar_open(&step.pad_nav));
}

/* The bar has no panel to leave: the landing clicks on the R1 frame itself, as from the map. */
static void test_r1_on_the_bar_closes_it_and_lands_on_tech_on_the_same_frame(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    run_frame(&step, P8_PAD_R1);
    assert(!pad_nav_build_bar_open(&step.pad_nav) && pad_nav_tech_open(&step.pad_nav));
    assert_clicked(&step, 2);
    run_frame(&step, 0);
    assert(!pad_nav_build_bar_open(&step.pad_nav) && pad_nav_tech_open(&step.pad_nav));
    assert(pad_nav_panel_open() && main_ui()->ephemerals.count > 0);
    assert(pad_nav_focus(&step.pad_nav) == pad_nav_first_item());
}

static void test_cross_held_through_r1_on_the_bar_presses_nothing_on_tech_until_pressed_again(void) {
    input_step_t step;
    start_game(&step);
    main_economy_typed()->research_points = p8_fixed_from_int(5);
    open_build_bar(&step);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_CROSS | P8_PAD_R1);
    assert(pad_nav_tech_open(&step.pad_nav));
    assert_clicked(&step, 2);
    for (int i = 0; i < 10; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert(main_ui()->ephemerals.count > 0 && pad_nav_tech_open(&step.pad_nav) && invented_count() == 0);
    assert(ui_left_held(main_ui()) == NULL && pad_nav_placing(&step.pad_nav) == NULL);
    assert_world_unchanged(before);
    run_frame(&step, 0);
    tap(&step, P8_PAD_DOWN);
    (void)focus_affordable_row(&step);
    run_frame(&step, P8_PAD_CROSS);
    assert(invented_count() == 1);
}

static void test_the_bar_keeps_its_icon_through_r1_to_tech_and_l1_back(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    tap(&step, P8_PAD_LEFT);
    entity_t *icon = processor_icon();
    assert(pad_nav_build_icon(&step.pad_nav) == icon);
    tap(&step, P8_PAD_R1);
    settle(&step);
    assert(pad_nav_tech_open(&step.pad_nav) && pad_nav_build_icon(&step.pad_nav) == NULL);
    tap(&step, P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step.pad_nav) && !pad_nav_tech_open(&step.pad_nav));
    assert_build_focus(&step, icon);
}

/* Under pause, then on the release frame of the click that closes it, the bar stays: R1 acts only after. */
static void test_r1_on_the_bar_waits_under_pause_and_on_a_release_frame(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    open_pause(&step);
    assert_shoulders_wait(&step);
    assert(pause_menu_shown() && pad_nav_build_bar_open(&step.pad_nav));
    run_frame(&step, P8_PAD_START);
    assert(!pause_menu_shown());
    run_frame(&step, P8_PAD_R1);
    assert(pad_nav_build_bar_open(&step.pad_nav) && !pad_nav_tech_open(&step.pad_nav));
    assert(input_step_played_frame(&step)->mouse_buttons == 0);
    run_frame(&step, 0);
    open_tech(&step);
}

/* Score's leaving click lands on its backdrop, which takes presses anywhere above the HUD: never on the icon. */
static void test_l1_on_tech_and_score_opens_the_bar_and_hands_over_nothing(void) {
    static const off_map_t places[2] = { THE_TECH_PANEL, THE_SCORE_PANEL };
    input_step_t step;
    start_game(&step);
    for (int t = 0; t < 2; t++) {
        if (in_forked_copy()) {
            open_from_the_map(&step, places[t]);
            hold(&step, 0, 3);
            assert(places[t] != THE_TECH_PANEL || main_ui()->ephemerals.count > 0);
            assert(places[t] != THE_SCORE_PANEL || score_panel_shown());
            world_count_t before = world_count();
            run_frame(&step, P8_PAD_L1);
            assert(pad_nav_build_bar_open(&step.pad_nav));
            assert(input_step_played_frame(&step)->mouse_buttons == (places[t] == THE_SCORE_PANEL ? P8_MOUSE_LEFT : 0));
            for (int i = 0; i < 3; i++) {
                run_frame(&step, 0);
                assert(ui_left_held(main_ui()) == NULL && pad_nav_placing(&step.pad_nav) == NULL);
            }
            assert(main_ui()->ephemerals.count == 0 && !score_panel_shown() && backdrop() == NULL);
            assert_world_unchanged(before);
            assert(pad_nav_build_bar_open(&step.pad_nav));
            assert_build_focus(&step, basic_structure_icon());
            end_forked_copy();
        }
    }
}

static void test_r1_on_the_bar_opens_tech_both_shoulders_do_nothing_and_a_held_l1_toggles_once(void) {
    input_step_t step;
    start_game(&step);
    tap(&step, P8_PAD_L1 | P8_PAD_R1);
    assert(!pad_nav_build_bar_open(&step.pad_nav) && on_the_map(&step.pad_nav));
    open_build_bar(&step);
    tap(&step, P8_PAD_L1 | P8_PAD_R1);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    hold(&step, P8_PAD_L1, 10);
    assert(!pad_nav_build_bar_open(&step.pad_nav) && on_the_map(&step.pad_nav));
    run_frame(&step, 0);
    hold(&step, P8_PAD_L1, 10);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    run_frame(&step, 0);
    tap(&step, P8_PAD_R1);
    assert(!pad_nav_build_bar_open(&step.pad_nav) && pad_nav_tech_open(&step.pad_nav));
}

static void test_l1_closes_a_planet_menu_and_opens_the_bar(void) {
    input_step_t step;
    planet_t *planet = jump_on_known_planet(&step);
    tap(&step, P8_PAD_TRIANGLE);
    assert(main_ui()->ephemerals.count > 0);
    run_frame(&step, P8_PAD_L1);
    assert(main_ui()->ephemerals.count == 0 && pad_nav_build_bar_open(&step.pad_nav));
    run_frame(&step, 0);
    assert(main_ui()->ephemerals.count == 0);
    tap(&step, P8_PAD_L1);
    run_frame(&step, 0);
    assert_hovering(&step, &planet->header);
}

static void test_l1_cancels_a_held_slipway_and_opens_the_bar(void) {
    input_step_t step;
    planet_t *start, *target;
    start_game_in_jump_mode(&step);
    find_pair(near_pair, &start, &target);
    make_known(target);
    slipway_from(&step, start);
    aim_at(&step, &target->header);
    world_count_t before = world_count();
    tap(&step, P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    assert(ui_left_held(main_ui()) == NULL && slipways_left_placing() == 0);
    assert_world_unchanged(before);
    tap(&step, P8_PAD_L1);
    run_frame(&step, 0);
    assert_hovering(&step, &start->header);
    assert_world_unchanged(before);
}

static void test_l1_keeps_the_year_box(void) {
    input_step_t step;
    start_game(&step);
    score_panel_new_year(p8_fixed_from_int(3421));
    entity_t *year_box = &shown_message_box()->header;
    run_frame(&step, 0);
    open_build_bar(&step);
    assert(entity_list_has(live_entities(), year_box) && shown_message_box() != NULL);
    tap(&step, P8_PAD_L1);
    assert(!pad_nav_build_bar_open(&step.pad_nav) && pad_nav_aim_mode(&step.pad_nav));
    assert(entity_list_has(live_entities(), year_box) && shown_message_box() != NULL);
}

static void assert_l1_waits(input_step_t *step) {
    run_frame(step, P8_PAD_L1);
    assert(!pad_nav_build_bar_open(&step->pad_nav) && on_the_map(&step->pad_nav));
}

static void test_l1_waits_as_the_shoulders_do(void) {
    input_step_t step;
    if (in_forked_copy()) {
        start_title(&step);
        tap(&step, P8_PAD_L1);
        assert(!pad_nav_build_bar_open(&step.pad_nav));
        assert_focused_and_hovered(&step, "forgiving");
        end_forked_copy();
    }
    (void)jump_on_colony(&step);
    if (in_forked_copy()) {
        open_pause(&step);
        assert_l1_waits(&step);
        run_frame(&step, 0);
        assert(pause_menu_shown() && !pad_nav_build_bar_open(&step.pad_nav));
        end_forked_copy();
    }
    if (in_forked_copy()) {
        run_frame(&step, P8_PAD_TRIANGLE);
        assert(input_step_played_frame(&step)->mouse_buttons == P8_MOUSE_LEFT);
        assert_l1_waits(&step);
        end_forked_copy();
    }
    planet_t *a, *b, *c;
    settled_row(&step, &a, &b, &c);
    hover_object(&step, &a->header);
    start_slipway(&step, &a->header);
    tap(&step, P8_PAD_RIGHT);
    world_count_t before = world_count();
    run_frame(&step, P8_PAD_CROSS);
    assert_l1_waits(&step);
    run_frame(&step, 0);
    assert(!pad_nav_build_bar_open(&step.pad_nav));
    assert(slipway_between(&a->header, &b->header) != NULL && world_count().slipways == before.slipways + 1);
}

static void test_cross_held_through_l1_takes_nothing_until_pressed_again(void) {
    input_step_t step;
    start_game(&step);
    int live = live_entities()->count;
    run_frame(&step, P8_PAD_CROSS | P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    for (int i = 0; i < 10; i++) {
        run_frame(&step, P8_PAD_CROSS);
        assert(input_step_played_frame(&step)->mouse_buttons == 0);
    }
    assert(live_entities()->count == live && ui_left_held(main_ui()) == NULL);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    run_frame(&step, 0);
    (void)take_structure(&step);
}

static void test_four_chords_on_the_bar_cheat_once_and_change_nothing_else(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    tap(&step, P8_PAD_LEFT);
    int live = live_entities()->count;
    assert(type_chords(&step) == 1);
    assert(main_economy_typed()->credits == p8_fixed_from_int(200));
    assert(pad_nav_build_bar_open(&step.pad_nav));
    assert_build_focus(&step, processor_icon());
    assert(live_entities()->count == live && ui_left_held(main_ui()) == NULL && pad_nav_placing(&step.pad_nav) == NULL);
}

/* Take ×, place ×, close the bar ○, leave aim mode ○, twice: the cheat's × × ○ ○ × × ○ ○ as lone presses. */
static void test_bar_and_placement_play_never_types_the_cheat(void) {
    input_step_t step;
    start_game(&step);
    cheats_fired = 0;
    for (int round = 0; round < 2; round++) {
        open_build_bar(&step);
        tap(&step, P8_PAD_LEFT);
        tap(&step, P8_PAD_RIGHT);
        entity_t *structure = take_structure(&step);
        pan_clear_of_every_node(&step, structure);
        tap(&step, P8_PAD_CROSS);
        assert(structure->state == ENTITY_STATE_SETTLED);
        open_build_bar(&step);
        tap(&step, P8_PAD_CIRCLE);
        assert(!pad_nav_build_bar_open(&step.pad_nav) && pad_nav_aim_mode(&step.pad_nav));
        tap(&step, P8_PAD_CIRCLE);
        assert(pad_nav_jump_mode(&step.pad_nav));
    }
    open_build_bar(&step);
    tap(&step, P8_PAD_L1);
    open_build_bar(&step);
    (void)take_structure(&step);
    tap(&step, P8_PAD_DOWN);
    tap(&step, P8_PAD_CIRCLE);
    assert(pad_nav_placing(&step.pad_nav) == NULL);
    tap(&step, P8_PAD_R1);
    tap(&step, P8_PAD_CIRCLE);
    assert(on_the_map(&step.pad_nav));
    assert(cheats_fired == 0 && main_economy_typed()->credits != p8_fixed_from_int(200));
}

static void test_select_on_the_bar_ends_no_year_and_presses_nothing(void) {
    input_step_t step;
    start_game(&step);
    open_build_bar(&step);
    assert_select_inert(&step, 0, 0);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    assert_build_focus(&step, basic_structure_icon());
}

static void test_the_bar_leaves_the_game_stream_unchanged(void) {
    input_step_t step;
    start_game(&step);
    uint32_t game = p8_fixed_rnd_state();
    open_build_bar(&step);
    tap(&step, P8_PAD_L1);
    open_build_bar(&step);
    tap(&step, P8_PAD_CIRCLE);
    open_build_bar(&step);
    entity_t *structure = take_structure(&step);
    pan_clear_of_every_node(&step, structure);
    tap(&step, P8_PAD_CROSS);
    assert(structure->state == ENTITY_STATE_SETTLED);
    open_build_bar(&step);
    (void)take_structure(&step);
    tap(&step, P8_PAD_CIRCLE);
    open_build_bar(&step);
    (void)take_structure(&step);
    tap(&step, P8_PAD_L1);
    assert(pad_nav_build_bar_open(&step.pad_nav));
    assert(p8_fixed_rnd_state() == game);
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
    in_fresh_child(test_aim_mode_holds_the_cursor_at_the_centre);
    in_fresh_child(test_aim_pans_with_the_speed_curve);
    in_fresh_child(test_aim_mode_diagonal_cancel_and_reset);
    in_fresh_child(test_aim_mode_bound_brings_every_planet_under_the_centre);
    in_fresh_child(test_aim_mode_triangle_on_empty_space_does_nothing);
    in_fresh_child(test_aim_mode_triangle_over_a_known_planet_or_a_colony_does_nothing);
    in_fresh_child(test_aim_mode_first_cross_places_the_probe);
    in_fresh_child(test_aim_mode_short_cross_hold_abandons_the_fold);
    in_fresh_child(test_aim_mode_cross_on_a_planet_presses_nothing);
    in_fresh_child(test_aim_pans_while_cross_is_held);
    in_fresh_child(test_aim_mode_cross_engagement_ends_under_a_panel);
    in_fresh_child(test_panel_takes_precedence_then_the_cursor_returns_to_the_centre);
    in_fresh_child(test_pause_menu_is_reachable);
    in_fresh_child(test_planet_menu_is_reachable);
    in_fresh_child(test_tech_panel_is_reachable);
    in_fresh_child(test_score_panel_during_play_has_no_item);
    in_fresh_child(test_score_panel_at_game_end_is_reachable);
    in_fresh_child(test_cross_held_into_the_final_score_panel_on_the_map_presses_nothing);
    in_fresh_child(test_score_panel_at_game_end_left_and_right);
    in_fresh_child(test_year_message_box_adds_and_removes_nothing);
    in_fresh_child(test_help_is_the_title_screen);
    in_fresh_child(test_how_to_play_is_reachable_and_opens_nothing);
    in_fresh_child(test_triangle_on_empty_space_or_an_unexplored_planet_does_nothing);
    in_fresh_child(test_colonising_through_the_menu_ends_in_jump_mode_on_the_colony);
    in_fresh_child(test_triangle_click_releases_even_with_cross_held);
    in_fresh_child(test_circle_on_the_title_does_nothing);
    in_fresh_child(test_circle_closes_the_pause_menu);
    in_fresh_child(test_triangle_while_cross_holds_a_fold_does_nothing);
    in_fresh_child(test_a_new_game_starts_in_jump_mode_with_nothing_hovered);
    in_fresh_child(test_a_new_game_after_aim_mode_starts_in_jump_mode);
    in_fresh_child(test_jump_mode_seeds_on_the_first_object_that_appears);
    in_fresh_child(test_the_edge_scroll_brings_the_hovered_object_just_inside_at_8_px_a_frame);
    in_fresh_child(test_jump_mode_seeds_on_the_colony_over_a_nearer_known_planet);
    in_fresh_child(test_the_edge_scroll_reads_the_rest_point_not_the_bob);
    in_fresh_child(test_square_in_jump_mode_enters_aim_mode_where_the_camera_is);
    in_fresh_child(test_square_in_aim_mode_goes_back_to_jump_mode_on_the_nearest_target);
    in_fresh_child(test_circle_in_aim_mode_goes_back_to_jump_mode_on_the_nearest_target);
    in_fresh_child(test_square_in_aim_mode_over_a_colony_hovers_it_and_presses_nothing);
    in_fresh_child(test_square_in_aim_mode_with_no_target_hovers_nothing_until_one_appears);
    in_fresh_child(test_circle_in_aim_mode_with_no_target_hovers_nothing_until_one_appears);
    in_fresh_child(test_circle_in_aim_mode_closes_a_box_before_leaving);
    in_fresh_child(test_square_and_circle_wait_while_cross_holds_a_fold);
    in_fresh_child(test_aim_pans_and_toggles_leave_the_game_stream_unchanged);
    in_fresh_child(test_crosshair_is_drawn_in_aim_mode_only);
    in_fresh_child(test_circle_with_nothing_acted_on_and_cross_on_nothing_change_nothing);
    in_fresh_child(test_the_hovered_object_blinks_a_circle);
    in_fresh_child(test_a_dead_hovered_object_reseeds_on_the_object_nearest_the_centre);
    in_fresh_child(test_pad_nav_hovered_is_null_outside_jump_mode);
    in_fresh_child(test_r1_cycles_tech_score_closed_from_jump_mode_back_on_the_hovered_object);
    in_fresh_child(test_taking_a_structure_from_jump_mode_enters_plain_aim_mode_where_the_camera_is);
    in_fresh_child(test_a_d_pad_press_jumps_to_the_picked_target);
    in_fresh_child(test_a_d_pad_press_with_an_empty_cone_changes_nothing);
    in_fresh_child(test_holding_a_direction_repeats_the_jump_after_18_then_every_8_frames);
    in_fresh_child(test_a_hold_through_pause_repeats_no_jump_until_pressed_again);
    in_fresh_child(test_a_jump_scrolls_only_to_bring_the_target_just_inside);
    in_fresh_child(test_jumps_repeats_and_edge_scrolls_leave_the_game_stream_unchanged);
    in_fresh_child(test_triangle_on_a_hovered_known_planet_opens_its_menu_and_circle_comes_back);
    in_fresh_child(test_triangle_on_a_hovered_colony_opens_its_panel_and_no_jump_shares_the_click);
    in_fresh_child(test_triangle_on_a_hovered_object_still_scrolling_in_presses_nothing);
    in_fresh_child(test_cross_on_a_hovered_known_planet_opens_its_menu_and_records_it);
    in_fresh_child(test_cross_held_from_the_opening_click_presses_no_row_until_pressed_again);
    in_fresh_child(test_cross_on_a_known_planet_scrolling_in_or_under_the_hud_presses_nothing);
    in_fresh_child(test_cross_and_triangle_on_one_frame_give_one_click);
    in_fresh_child(test_triangle_again_closes_a_known_planets_menu);
    in_fresh_child(test_triangle_again_closes_a_colonys_inspection_panel);
    in_fresh_child(test_triangle_on_the_release_frame_closes_nothing_and_closes_a_menu_cross_opened_after);
    in_fresh_child(test_triangle_under_the_year_box_closes_only_a_planet_menu);
    in_fresh_child(test_cross_in_aim_mode_on_a_colony_presses_nothing);
    in_fresh_child(test_square_while_cross_holds_a_fold_selects_nothing);
    in_fresh_child(test_r1_cycles_tech_score_closed_from_aim_mode_back_in_it_panning_from_slow);
    in_fresh_child(test_cross_on_the_triangle_release_frame_is_ignored);
    in_fresh_child(test_r1_cycles_tech_score_closed_from_the_map_and_opens_tech_from_the_bar);
    in_fresh_child(test_a_held_shoulder_moves_one_tab_and_both_together_move_none);
    in_fresh_child(test_build_focuses_the_basic_structure_icon_from_the_tab_change_frame);
    in_fresh_child(test_build_left_and_right_move_between_the_icons_without_wrapping);
    in_fresh_child(test_build_left_reaches_every_revealed_icon_in_decreasing_x);
    in_fresh_child(test_the_bar_remembers_its_icon_across_a_close_until_a_new_game);
    in_fresh_child(test_build_up_down_square_and_triangle_press_nothing);
    in_fresh_child(test_circle_on_a_tab_returns_to_the_map);
    in_fresh_child(test_cross_takes_the_structure_and_a_second_cross_places_it);
    in_fresh_child(test_the_cross_that_takes_the_structure_drops_nothing_while_held);
    in_fresh_child(test_a_refused_drop_kills_the_structure_and_charges_nothing);
    in_fresh_child(test_circle_cancels_the_structure_and_charges_nothing);
    in_fresh_child(test_a_panel_opened_while_carrying_releases_at_the_icon);
    in_fresh_child(test_circle_cancels_a_structure_taken_from_jump_mode_then_leaves_aim_mode);
    in_fresh_child(test_triangle_and_square_wait_while_carrying);
    in_fresh_child(test_r1_cancels_the_carried_structure_then_lands_on_tech_once_the_cancel_is_past);
    in_fresh_child(test_cross_held_on_arrival_on_build_takes_nothing);
    in_fresh_child(test_a_press_that_hands_over_no_structure_stays_on_build);
    in_fresh_child(test_a_new_game_starts_on_the_map);
    in_fresh_child(test_aim_mode_survives_a_tab_visit_and_pans_from_slow);
    in_fresh_child(test_r1_closes_a_planet_menu_and_opens_tech);
    in_fresh_child(test_shoulders_wait_under_the_pause_menu);
    in_fresh_child(test_final_score_ignores_shoulders_circle_and_start);
    in_fresh_child(test_shoulders_wait_on_the_triangle_release_frame);
    in_fresh_child(test_shoulders_switch_and_aim_pans_under_the_year_box);
    in_fresh_child(test_start_opens_pause_from_aim_mode);
    in_fresh_child(test_start_and_circle_close_pause_pressing_nothing_on_the_map);
    in_fresh_child(test_pause_from_jump_mode_gives_back_the_hovered_object_and_camera);
    in_fresh_child(test_pause_from_the_bar_keeps_the_bar_and_its_icon);
    in_fresh_child(test_pause_keeps_aim_mode_for_the_next_circle);
    in_fresh_child(test_cross_on_each_row_but_restart_closes_pause);
    in_fresh_child(test_cross_on_restart_gives_the_title_then_a_new_game_on_the_map);
    in_fresh_child(test_the_d_pad_on_pause_moves_among_its_rows_and_no_camera);
    in_fresh_child(test_cross_held_into_pause_presses_no_row_until_pressed_again);
    in_fresh_child(test_start_is_ignored_on_the_title);
    in_fresh_child(test_start_is_ignored_under_a_planet_menu_and_on_tech_and_score);
    in_fresh_child(test_start_is_ignored_while_cross_holds_a_fold);
    in_fresh_child(test_start_is_ignored_on_the_triangle_release_frame);
    in_fresh_child(test_start_is_ignored_on_the_frame_tech_returns_to_the_map);
    in_fresh_child(test_start_with_r1_opens_pause_and_keeps_the_tab);
    in_fresh_child(test_start_opens_pause_over_the_year_box);
    in_fresh_child(test_start_while_carrying_suspends_the_structure_under_pause);
    in_fresh_child(test_closing_pause_resumes_the_carry_where_it_was);
    in_fresh_child(test_restart_under_pause_drops_the_suspended_structure);
    in_fresh_child(test_circle_on_the_release_frame_of_the_pause_click_is_dropped);
    in_fresh_child(test_the_ui_pad_tab_is_the_open_cycle_panels_hud_button);
    in_fresh_child(test_nothing_is_lit_on_the_bar_while_carrying_under_pause_or_on_the_final_score_panel);
    in_fresh_child(test_no_pad_selection_is_shown_behind_the_bar_or_a_cycle_panel);
    in_fresh_child(test_the_tech_panel_lights_the_tech_button_under_it);
    in_fresh_child(test_the_score_panel_lights_the_score_button_from_the_r1_on_tech);
    in_fresh_child(test_landing_on_tech_opens_its_panel_and_an_invention_returns_to_the_map);
    in_fresh_child(test_cross_on_the_tech_header_closes_the_box_and_returns_to_the_map);
    in_fresh_child(test_circle_on_tech_closes_the_box_and_returns_to_the_map_in_one_press);
    in_fresh_child(test_landing_on_score_shows_its_panel_and_cross_presses_nothing);
    in_fresh_child(test_r1_on_tech_closes_the_box_and_shows_score_on_the_same_frame);
    in_fresh_child(test_r1_on_score_hides_it_and_returns_to_the_map);
    in_fresh_child(test_tech_and_score_read_open_from_the_tab_change_frame_and_closed_without_a_game);
    in_fresh_child(test_circle_on_tech_and_on_score_returns_to_jump_mode_on_the_hovered_object);
    in_fresh_child(test_r1_does_nothing_with_no_game_or_on_the_final_score_panel);
    in_fresh_child(test_cross_held_through_r1_presses_nothing_on_tech_until_pressed_again);
    in_fresh_child(test_cross_held_through_r1_r1_presses_nothing_on_score);
    in_fresh_child(test_r1_cycles_circle_exits_inventions_and_bar_visits_never_type_the_cheat);
    in_fresh_child(test_r1_cycles_leave_the_game_stream_unchanged);
    in_fresh_child(test_a_cycle_panel_whose_button_is_gone_returns_to_the_map);
    in_fresh_child(test_triangle_on_the_release_frame_of_leaving_score_does_nothing);
    in_fresh_child(test_circle_leaving_score_is_back_in_aim_mode_before_the_next_frame);
    in_fresh_child(test_l1_while_carrying_cancels_at_the_icon_then_opens_the_bar_pressing_nothing);
    in_fresh_child(test_pad_pause_draws_no_crosshair);
    in_fresh_child(test_a_carried_structure_draws_no_crosshair);
    in_fresh_child(test_a_planet_menu_draws_no_crosshair);
    in_fresh_child(test_pad_aim_mode_draws_a_crosshair_at_the_centre);
    in_fresh_child(test_four_chords_on_a_hovered_colony_cheat_and_change_nothing_else);
    in_fresh_child(test_a_chord_on_a_planet_menu_presses_no_row_and_closes_nothing);
    in_fresh_child(test_buttons_held_from_a_chord_act_on_nothing_until_both_are_up);
    in_fresh_child(test_the_pad_hides_cross_and_circle_exactly_from_the_chords_cheat_step_counts);
    in_fresh_child(test_four_chords_in_aim_mode_cheat_and_change_nothing_else);
    in_fresh_child(test_probing_and_leaving_aim_mode_never_types_the_cheat);
    in_fresh_child(test_cross_dpad_cross_dpad_cross_builds_a_chain_of_two_slipways);
    in_fresh_child(test_a_built_slipway_records_its_target_and_a_refused_one_keeps_the_start);
    in_fresh_child(test_four_chords_on_a_hovered_known_planet_cheat_and_open_nothing);
    in_fresh_child(test_circle_closes_the_year_box_over_jump_mode_and_moves_nothing);
    in_fresh_child(test_circle_in_aim_mode_closes_the_year_box_before_leaving);
    in_fresh_child(test_circle_closes_the_year_box_over_a_slipway_then_cancels_it);
    in_fresh_child(test_circle_closes_the_year_box_over_a_carried_structure_and_keeps_it);
    in_fresh_child(test_circle_closes_a_planet_menu_then_the_year_box);
    in_fresh_child(test_circle_on_the_bar_or_tech_returns_to_the_map_under_the_year_box);
    in_fresh_child(test_cross_on_a_hovered_colony_holds_its_slipway_and_the_d_pad_aims_past_it);
    in_fresh_child(test_no_direction_aims_at_the_start);
    in_fresh_child(test_a_refused_start_holds_nothing);
    in_fresh_child(test_cross_with_nothing_hovered_presses_nothing);
    in_fresh_child(test_a_refused_target_aims_again_from_the_same_start);
    in_fresh_child(test_circle_cancels_the_slipway_back_on_its_start);
    in_fresh_child(test_an_off_screen_start_still_builds);
    in_fresh_child(test_start_and_shoulders_wait_on_the_frame_after_a_slipway_release);
    in_fresh_child(test_no_re_press_while_the_hud_covers_the_new_start);
    in_fresh_child(test_pause_a_tab_change_and_a_box_cancel_the_slipway);
    in_fresh_child(test_a_start_that_dies_drops_the_slipway);
    in_fresh_child(test_triangle_and_square_do_nothing_in_slipway_mode);
    in_fresh_child(test_chords_in_slipway_mode_neither_build_nor_cancel);
    in_fresh_child(test_building_and_backing_out_never_types_the_cheat);
    in_fresh_child(test_building_refusing_and_cancelling_leave_the_game_stream_unchanged);
    in_fresh_child(test_circle_cancels_a_slipway_onto_its_start_and_the_next_does_nothing);
    in_fresh_child(test_circle_cancels_a_structure_then_leaves_aim_mode_then_jumps_back);
    in_fresh_child(test_circle_at_the_root_jumps_back_and_edge_scrolls_at_8_px_a_frame);
    in_fresh_child(test_circle_after_a_resume_does_nothing);
    in_fresh_child(test_circle_with_the_last_object_gone_or_in_a_new_game_does_nothing);
    in_fresh_child(test_ordinary_cross_triangle_and_circle_play_never_types_the_cheat);
    in_fresh_child(test_pause_a_tab_and_a_slipway_keep_the_last_object);
    in_fresh_child(test_select_taps_cycle_the_colonies_needing_attention_in_order);
    in_fresh_child(test_a_select_tap_to_a_colony_outside_edge_scrolls_at_8_px_a_frame);
    in_fresh_child(test_a_select_hold_under_30_frames_ends_nothing);
    in_fresh_child(test_a_select_hold_of_30_frames_presses_the_year_skip_button_once);
    in_fresh_child(test_a_select_hold_of_90_frames_ends_one_year);
    in_fresh_child(test_fresh_select_presses_after_a_year_chain_more_years);
    in_fresh_child(test_a_select_hold_ends_the_year_under_the_year_box);
    in_fresh_child(test_select_in_aim_mode_ends_the_year_and_taps_back_to_jump_mode);
    in_fresh_child(test_a_select_tap_with_nothing_needing_attention_changes_nothing);
    in_fresh_child(test_select_does_nothing_while_something_is_held_or_open);
    in_fresh_child(test_a_refused_year_press_presses_nothing_and_is_not_retried);
    in_fresh_child(test_a_new_game_restarts_the_select_count);
    in_fresh_child(test_select_is_no_part_of_the_cheat_chord);
    in_fresh_child(test_l1_opens_the_bar_from_jump_mode_and_l1_closes_it_on_the_same_hovered_object);
    in_fresh_child(test_circle_closes_the_bar_on_the_same_hovered_object);
    in_fresh_child(test_l1_twice_in_aim_mode_comes_back_to_aim_mode_with_the_pan_slow_again);
    in_fresh_child(test_the_bar_takes_and_aim_mode_places_or_cancels);
    in_fresh_child(test_l1_on_tech_and_score_opens_the_bar_and_hands_over_nothing);
    in_fresh_child(test_r1_on_the_bar_opens_tech_both_shoulders_do_nothing_and_a_held_l1_toggles_once);
    in_fresh_child(test_r1_on_the_bar_closes_it_and_lands_on_tech_on_the_same_frame);
    in_fresh_child(test_cross_held_through_r1_on_the_bar_presses_nothing_on_tech_until_pressed_again);
    in_fresh_child(test_the_bar_keeps_its_icon_through_r1_to_tech_and_l1_back);
    in_fresh_child(test_r1_on_the_bar_waits_under_pause_and_on_a_release_frame);
    in_fresh_child(test_l1_closes_a_planet_menu_and_opens_the_bar);
    in_fresh_child(test_l1_cancels_a_held_slipway_and_opens_the_bar);
    in_fresh_child(test_l1_keeps_the_year_box);
    in_fresh_child(test_l1_waits_as_the_shoulders_do);
    in_fresh_child(test_cross_held_through_l1_takes_nothing_until_pressed_again);
    in_fresh_child(test_four_chords_on_the_bar_cheat_once_and_change_nothing_else);
    in_fresh_child(test_bar_and_placement_play_never_types_the_cheat);
    in_fresh_child(test_select_on_the_bar_ends_no_year_and_presses_nothing);
    in_fresh_child(test_the_bar_leaves_the_game_stream_unchanged);
    printf("test_pad_nav: ok\n");
    return 0;
}
