#include "game/pad_nav.h"

#include "engine/entity_id.h"
#include "engine/entity_registry.h"
#include "engine/geom.h"
#include "game/background.h"
#include "game/main_economy.h"
#include "game/menu.h"
#include "game/planet.h"
#include "game/scroll.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "p8/vram.h"

#include <assert.h>

enum { SCREEN_SPACE_LAYER = 10 };

typedef struct {
    int from_ephemerals;
    int index;
} candidate_iter_t;

static int modal_backdrop_registered(void) {
    const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
    for (int i = 0; i < bucket->count; i++) {
        if (bucket->items[i]->class == modal_backdrop_class()) {
            return 1;
        }
    }
    return 0;
}

int pad_nav_panel_open(void) {
    const ui_t *ui = main_ui();
    if (ui == NULL) {
        return 0;
    }
    return main_economy_typed() == NULL || ui->ephemerals.count > 0 || modal_backdrop_registered();
}

int pad_nav_free_mode(const pad_nav_t *nav) {
    return main_economy_typed() != NULL && !pad_nav_panel_open() && nav->map_mode == PAD_NAV_MAP_FREE;
}

entity_t *pad_nav_selection(const pad_nav_t *nav) {
    if (nav->map_mode != PAD_NAV_MAP_NETWORK) {
        return NULL;
    }
    entity_t *selected = entity_get(nav->selection);
    return selected != NULL && !selected->is_dead ? selected : NULL;
}

/* Mirrors ui_update: while a box is open, a left press reaches only its ephemerals. */
static candidate_iter_t candidates_all(void) {
    const ui_t *ui = main_ui();
    candidate_iter_t it = { ui != NULL && ui->ephemerals.count > 0, 0 };
    return it;
}

static int is_candidate(const entity_t *e) {
    return e->class->left_press != NULL && entity_has(e, ENTITY_HAS_LAYER);
}

static entity_t *candidates_next(candidate_iter_t *it) {
    const ui_t *ui = main_ui();
    if (ui == NULL) {
        return NULL;
    }
    for (;;) {
        entity_t *e;
        if (it->from_ephemerals) {
            if (it->index >= ui->ephemerals.count) {
                return NULL;
            }
            e = entity_id_list_at(&ui->ephemerals, it->index++);
        } else {
            const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
            if (it->index >= bucket->count) {
                return NULL;
            }
            e = bucket->items[it->index++];
        }
        if (is_candidate(e)) {
            return e;
        }
    }
}

/* -1 when there is no candidate. */
static int top_layer(void) {
    candidate_iter_t it = candidates_all();
    int top = -1;
    entity_t *e;
    while ((e = candidates_next(&it)) != NULL) {
        int layer = p8_fixed_flr_int(e->layer);
        if (layer > top) {
            top = layer;
        }
    }
    return top;
}

static int is_item_on(const entity_t *e, int layer) {
    return is_candidate(e) && p8_fixed_flr_int(e->layer) == layer && entity_has(e, ENTITY_HAS_HIT_BOX);
}

int pad_nav_is_item(const entity_t *e) {
    if (e == NULL || !pad_nav_panel_open()) {
        return 0;
    }
    int layer = top_layer();
    candidate_iter_t it = candidates_all();
    entity_t *candidate;
    while ((candidate = candidates_next(&it)) != NULL) {
        if (candidate == e) {
            return is_item_on(e, layer);
        }
    }
    return 0;
}

entity_t *pad_nav_first_item(void) {
    if (!pad_nav_panel_open()) {
        return NULL;
    }
    int layer = top_layer();
    candidate_iter_t it = candidates_all();
    entity_t *e;
    while ((e = candidates_next(&it)) != NULL) {
        if (is_item_on(e, layer)) {
            return e;
        }
    }
    return NULL;
}

/* The hit box where the hit test reads it: layers under 10 are hit-tested in world space. */
static box_t screen_box(const entity_t *e) {
    vec_t origin = entity_pos(e);
    if (p8_fixed_flr_int(e->layer) < SCREEN_SPACE_LAYER) {
        const camera_t *camera = main_camera_typed();
        if (camera != NULL) {
            origin = vec_sub(origin, entity_pos(&camera->header));
        }
    }
    box_t box = { p8_fixed_add(origin.x, e->hit_box.left), p8_fixed_add(origin.y, e->hit_box.top),
                  p8_fixed_add(origin.x, e->hit_box.right), p8_fixed_add(origin.y, e->hit_box.bottom) };
    return box;
}

static p8_fixed_t middle(p8_fixed_t low, p8_fixed_t high) {
    return p8_fixed_flr(p8_fixed_div(p8_fixed_add(low, high), p8_fixed_from_int(2)));
}

static void focus_point(const entity_t *e, int *x, int *y) {
    box_t box = screen_box(e);
    *x = p8_fixed_flr_int(middle(box.left, box.right));
    *y = p8_fixed_flr_int(middle(box.top, box.bottom));
}

static int is_direction(int direction) {
    return direction == P8_PAD_UP || direction == P8_PAD_DOWN || direction == P8_PAD_LEFT || direction == P8_PAD_RIGHT;
}

static int absolute(int value) {
    return value < 0 ? -value : value;
}

/* 0 when the two spans overlap. */
static int span_gap(p8_fixed_t low_a, p8_fixed_t high_a, p8_fixed_t low_b, p8_fixed_t high_b) {
    if (low_b > high_a) {
        return p8_fixed_flr_int(p8_fixed_sub(low_b, high_a));
    }
    if (low_a > high_b) {
        return p8_fixed_flr_int(p8_fixed_sub(low_a, high_b));
    }
    return 0;
}

/* -1 when the candidate does not lie strictly beyond the item's box in that direction. */
static int neighbor_score(const entity_t *item, const entity_t *candidate, int direction) {
    box_t from = screen_box(item);
    box_t to = screen_box(candidate);
    int from_x, from_y, to_x, to_y;
    focus_point(item, &from_x, &from_y);
    focus_point(candidate, &to_x, &to_y);
    switch (direction) {
    case P8_PAD_UP:
        if (p8_fixed_from_int(to_y) >= from.top) return -1;
        break;
    case P8_PAD_DOWN:
        if (p8_fixed_from_int(to_y) <= from.bottom) return -1;
        break;
    case P8_PAD_LEFT:
        if (p8_fixed_from_int(to_x) >= from.left) return -1;
        break;
    default:
        if (p8_fixed_from_int(to_x) <= from.right) return -1;
        break;
    }
    if (direction == P8_PAD_UP || direction == P8_PAD_DOWN) {
        return absolute(to_y - from_y) + 2 * span_gap(from.left, from.right, to.left, to.right);
    }
    return absolute(to_x - from_x) + 2 * span_gap(from.top, from.bottom, to.top, to.bottom);
}

entity_t *pad_nav_neighbor(const entity_t *item, int direction) {
    assert(is_direction(direction) && "pad_nav_neighbor: the direction is exactly one of P8_PAD_UP, DOWN, LEFT or RIGHT");
    int layer = top_layer();
    candidate_iter_t it = candidates_all();
    entity_t *best = NULL;
    int best_score = -1;
    entity_t *e;
    while ((e = candidates_next(&it)) != NULL) {
        if (e == item || !is_item_on(e, layer)) {
            continue;
        }
        int score = neighbor_score(item, e, direction);
        if (score >= 0 && (best == NULL || score < best_score)) {
            best = e;
            best_score = score;
        }
    }
    return best;
}

/* 0 when no direction went down; Up, Down, Left, Right is the priority order. */
static int pressed_direction(int pressed) {
    static const int order[4] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT };
    for (int i = 0; i < 4; i++) {
        if (pressed & order[i]) {
            return order[i];
        }
    }
    return 0;
}

enum { HELD_FRAMES_CAP = 41 };

static const int pan_directions[4] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT };

static void reset_free_mode(pad_nav_t *nav) {
    for (int i = 0; i < 4; i++) {
        nav->held_frames[i] = 0;
    }
    nav->cross_engaged = 0;
}

static int pan_speed(int held_frames) {
    if (held_frames == 0) {
        return 0;
    }
    if (held_frames <= 10) {
        return 1;
    }
    return held_frames <= 40 ? 2 : 4;
}

static void free_mode_pan(pad_nav_t *nav, int pad_buttons) {
    int speed[4];
    for (int i = 0; i < 4; i++) {
        if (!(pad_buttons & pan_directions[i])) {
            nav->held_frames[i] = 0;
        } else if (nav->held_frames[i] < HELD_FRAMES_CAP) {
            nav->held_frames[i]++;
        }
        speed[i] = pan_speed(nav->held_frames[i]);
    }
    camera_t *camera = main_camera_typed();
    if (camera != NULL) {
        camera_pan(camera, speed[3] - speed[2], speed[1] - speed[0]);
    }
}

static void reset_map_mode(pad_nav_t *nav) {
    nav->map_mode = PAD_NAV_MAP_FREE;
    nav->selection = ENTITY_ID_NONE;
}

void pad_nav_init(pad_nav_t *nav) {
    nav->focus = ENTITY_ID_NONE;
    reset_map_mode(nav);
    nav->release_pending = 0;
    nav->previous_pad = 0;
    nav->cursor_x = 0;
    nav->cursor_y = 0;
    reset_free_mode(nav);
}

entity_t *pad_nav_focus(const pad_nav_t *nav) {
    entity_t *focus = entity_get(nav->focus);
    return pad_nav_is_item(focus) ? focus : NULL;
}

static void panel_step(pad_nav_t *nav, int pad_buttons, int direction, p8_input_frame_t *played) {
    entity_t *focus = pad_nav_focus(nav);
    if (focus == NULL) {
        focus = pad_nav_first_item();
    }
    if (focus != NULL && direction != 0) {
        entity_t *next = pad_nav_neighbor(focus, direction);
        if (next != NULL) {
            focus = next;
        }
    }
    nav->focus = entity_id_link(focus);
    if (focus != NULL) {
        focus_point(focus, &nav->cursor_x, &nav->cursor_y);
    }
    if (pad_buttons & P8_PAD_CROSS) {
        played->mouse_buttons = P8_MOUSE_LEFT;
    }
}

static mouse_pos_t mouse_at(int x, int y) {
    const camera_t *camera = main_camera_typed();
    mouse_pos_t mouse = { 0 };
    mouse.screen = vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
    mouse.world = camera != NULL ? vec_add(mouse.screen, entity_pos(&camera->header)) : mouse.screen;
    mouse.has = MOUSE_POS_HAS_SCREEN | MOUSE_POS_HAS_WORLD;
    return mouse;
}

/* NULL when no entity whose own hit test contains the screen point passes accept. */
static entity_t *entity_under(int x, int y, int (*accept)(const entity_t *e)) {
    mouse_pos_t mouse = mouse_at(x, y);
    const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
    for (int i = 0; i < bucket->count; i++) {
        entity_t *e = bucket->items[i];
        if (accept(e) && e->class->hit_test != NULL && e->class->hit_test(e, mouse)) {
            return e;
        }
    }
    return NULL;
}

static int presses_off_background(const entity_t *e) {
    return e->class->left_press != NULL && e->class != background_class();
}

/* Only these two states stop the dispatch in planet_left_press: any other lets the press reach the background, a probe. */
static int is_clickable_planet(const entity_t *e) {
    return planet_is_family(e) && (e->state == ENTITY_STATE_KNOWN || e->state == ENTITY_STATE_SETTLED);
}

/* A press on this frame and a release on the next: a settled planet opens its panel on the release. */
static void click_planet(pad_nav_t *nav, const entity_t *planet, p8_input_frame_t *played) {
    played->mouse_buttons = P8_MOUSE_LEFT;
    nav->release_pending = 1;
    nav->map_mode = PAD_NAV_MAP_NETWORK;
    nav->selection = entity_id_link(planet);
}

static void free_mode_step(pad_nav_t *nav, int pad_buttons, int pressed, p8_input_frame_t *played) {
    nav->focus = ENTITY_ID_NONE;
    nav->cursor_x = P8_SCREEN_WIDTH / 2;
    nav->cursor_y = P8_SCREEN_HEIGHT / 2;
    free_mode_pan(nav, pad_buttons);
    if (!(pad_buttons & P8_PAD_CROSS)) {
        nav->cross_engaged = 0;
    } else if (pressed & P8_PAD_CROSS) {
        nav->cross_engaged = entity_under(nav->cursor_x, nav->cursor_y, presses_off_background) == NULL;
    }
    if (nav->cross_engaged) {
        played->mouse_buttons = P8_MOUSE_LEFT;
    } else if (pressed & P8_PAD_TRIANGLE) {
        entity_t *planet = entity_under(nav->cursor_x, nav->cursor_y, is_clickable_planet);
        if (planet != NULL) {
            click_planet(nav, planet, played);
        }
    }
}

/* The planet's world point minus the camera, floored: the rule of the world-space panel items. */
static void screen_point(const entity_t *e, int *x, int *y) {
    vec_t at = entity_pos(e);
    const camera_t *camera = main_camera_typed();
    if (camera != NULL) {
        at = vec_sub(at, entity_pos(&camera->header));
    }
    *x = p8_fixed_flr_int(at.x);
    *y = p8_fixed_flr_int(at.y);
}

static void network_step(pad_nav_t *nav, const entity_t *selected, int pressed, p8_input_frame_t *played) {
    nav->focus = ENTITY_ID_NONE;
    screen_point(selected, &nav->cursor_x, &nav->cursor_y);
    if ((pressed & P8_PAD_TRIANGLE) && is_clickable_planet(selected)) {
        click_planet(nav, selected, played);
    }
}

/* The title and a new game rebuild the id table above every old generation, so an old selection resolves to NULL. */
static void drop_stale_map_mode(pad_nav_t *nav) {
    if (nav->map_mode == PAD_NAV_MAP_NETWORK && pad_nav_selection(nav) == NULL) {
        reset_map_mode(nav);
    }
}

/* The title's difficulty rows are no box, so ○ cannot close them; the pause menu's close is Start's, not ○'s. */
static void circle_closes_the_box(void) {
    ui_t *ui = main_ui();
    if (ui != NULL && ui->ephemerals.count > 0 && !pause_menu_shown()) {
        ui_clear_ephemerals(ui);
    }
}

void pad_nav_step(pad_nav_t *nav, int pad_buttons, p8_input_frame_t *played) {
    int pressed = pad_buttons & ~nav->previous_pad;
    int direction = pressed_direction(pressed);
    int release = nav->release_pending;
    nav->previous_pad = pad_buttons;
    nav->release_pending = 0;
    played->mouse_buttons = 0;
    drop_stale_map_mode(nav);
    if (pressed & P8_PAD_CIRCLE) {
        circle_closes_the_box();
    }
    if (pad_nav_panel_open()) {
        reset_free_mode(nav);
        panel_step(nav, pad_buttons, direction, played);
    } else if (pad_nav_free_mode(nav)) {
        free_mode_step(nav, pad_buttons, pressed, played);
    } else if (pad_nav_selection(nav) != NULL) {
        reset_free_mode(nav);
        network_step(nav, pad_nav_selection(nav), pressed, played);
    } else {
        reset_free_mode(nav);
    }
    if (release) {
        played->mouse_buttons = 0;
    }
    played->mouse_x = nav->cursor_x;
    played->mouse_y = nav->cursor_y;
}
