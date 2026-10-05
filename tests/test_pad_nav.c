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
    in_fresh_child(test_no_panel_open_leaves_the_cursor_still);
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
