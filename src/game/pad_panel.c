#include "game/pad_panel.h"

#include "engine/entity_registry.h"
#include "engine/geom.h"
#include "game/main_economy.h"
#include "game/scroll.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "p8/input.h"

#include <assert.h>

enum { SCREEN_SPACE_LAYER = 10 };

typedef struct {
    int from_ephemerals;
    int index;
} candidate_iter_t;

int pad_panel_box_open(void) {
    const ui_t *ui = main_ui();
    return ui != NULL && ui->ephemerals.count > 0;
}

static mouse_pos_t mouse_at(int x, int y) {
    const camera_t *camera = main_camera_typed();
    mouse_pos_t mouse = { 0 };
    mouse.screen = vec_make(p8_fixed_from_int(x), p8_fixed_from_int(y));
    mouse.world = camera != NULL ? vec_add(mouse.screen, entity_pos(&camera->header)) : mouse.screen;
    mouse.has = MOUSE_POS_HAS_SCREEN | MOUSE_POS_HAS_WORLD;
    return mouse;
}

entity_t *pad_panel_entity_under(int x, int y, int above, int (*accept)(const entity_t *e)) {
    mouse_pos_t mouse = mouse_at(x, y);
    const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
    for (int i = 0; i < bucket->count; i++) {
        entity_t *e = bucket->items[i];
        if (!e->is_dead && (above == PAD_PANEL_ANY_LAYER || p8_fixed_flr_int(e->layer) > above) && accept(e) &&
            e->class->hit_test != NULL && e->class->hit_test(e, mouse)) {
            return e;
        }
    }
    return NULL;
}

static int is_modal_backdrop(const entity_t *e) {
    return e->class == modal_backdrop_class();
}

int pad_panel_open(void) {
    if (main_ui() == NULL) {
        return 0;
    }
    /* The backdrop's hit test always holds, so any point finds it. */
    return main_economy_typed() == NULL || pad_panel_box_open() ||
           pad_panel_entity_under(0, 0, PAD_PANEL_ANY_LAYER, is_modal_backdrop) != NULL;
}

/* Mirrors ui_update: while a box is open, a left press reaches only its ephemerals. */
static candidate_iter_t candidates_all(void) {
    candidate_iter_t it = { pad_panel_box_open(), 0 };
    return it;
}

int pad_panel_is_candidate(const entity_t *e) {
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
        if (pad_panel_is_candidate(e)) {
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
    return pad_panel_is_candidate(e) && p8_fixed_flr_int(e->layer) == layer && entity_has(e, ENTITY_HAS_HIT_BOX);
}

int pad_panel_is_item(const entity_t *e) {
    if (e == NULL || !pad_panel_open()) {
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

entity_t *pad_panel_first_item(void) {
    if (!pad_panel_open()) {
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
static box_t box_at(const entity_t *e, vec_t origin) {
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

static box_t screen_box(const entity_t *e) {
    return box_at(e, entity_pos(e));
}

/* A box row still sliding in is judged where it comes to rest, or a Down on its first frames skips a row. */
static box_t rest_box(const entity_t *e) {
    const button_t *button = e->class == button_class() ? (const button_t *)e : NULL;
    return box_at(e, button != NULL && (button->has & BUTTON_HAS_SLIDE_TARGET) ? button->slide_target : entity_pos(e));
}

static p8_fixed_t middle(p8_fixed_t low, p8_fixed_t high) {
    return p8_fixed_flr(p8_fixed_div(p8_fixed_add(low, high), p8_fixed_from_int(2)));
}

static void box_middle(box_t box, int *x, int *y) {
    *x = p8_fixed_flr_int(middle(box.left, box.right));
    *y = p8_fixed_flr_int(middle(box.top, box.bottom));
}

void pad_panel_focus_point(const entity_t *e, int *x, int *y) {
    box_middle(screen_box(e), x, y);
}

static int is_direction(pad_direction_t direction) {
    return direction == PAD_DIRECTION_UP || direction == PAD_DIRECTION_DOWN || direction == PAD_DIRECTION_LEFT ||
           direction == PAD_DIRECTION_RIGHT;
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
static int neighbor_score(const entity_t *item, const entity_t *candidate, pad_direction_t direction) {
    box_t from = rest_box(item);
    box_t to = rest_box(candidate);
    int from_x, from_y, to_x, to_y;
    box_middle(from, &from_x, &from_y);
    box_middle(to, &to_x, &to_y);
    switch (direction) {
    case PAD_DIRECTION_UP:
        if (p8_fixed_from_int(to_y) >= from.top) return -1;
        break;
    case PAD_DIRECTION_DOWN:
        if (p8_fixed_from_int(to_y) <= from.bottom) return -1;
        break;
    case PAD_DIRECTION_LEFT:
        if (p8_fixed_from_int(to_x) >= from.left) return -1;
        break;
    default:
        if (p8_fixed_from_int(to_x) <= from.right) return -1;
        break;
    }
    if (direction == PAD_DIRECTION_UP || direction == PAD_DIRECTION_DOWN) {
        return absolute(to_y - from_y) + 2 * span_gap(from.left, from.right, to.left, to.right);
    }
    return absolute(to_x - from_x) + 2 * span_gap(from.top, from.bottom, to.top, to.bottom);
}

entity_t *pad_panel_neighbor(const entity_t *item, pad_direction_t direction) {
    assert(is_direction(direction) &&
           "pad_panel_neighbor: the direction is exactly one of PAD_DIRECTION_UP, DOWN, LEFT or RIGHT");
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

entity_t *pad_panel_step(entity_t *focus, int pad_buttons, pad_direction_t direction, int *x, int *y,
                         int *mouse_buttons) {
    entity_t *at = pad_panel_is_item(focus) ? focus : pad_panel_first_item();
    if (at != NULL && direction != PAD_DIRECTION_NONE) {
        entity_t *next = pad_panel_neighbor(at, direction);
        if (next != NULL) {
            at = next;
        }
    }
    if (at != NULL) {
        pad_panel_focus_point(at, x, y);
    }
    /* With no focus the cursor still sits on the last point: on the score panel a press there closes it. */
    *mouse_buttons = at != NULL && (pad_buttons & P8_PAD_BUTTON_5) ? P8_MOUSE_LEFT : 0;
    return at;
}
