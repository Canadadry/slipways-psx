#include "game/pad_jump.h"

#include "engine/entity_id.h"
#include "engine/entity_registry.h"
#include "game/galaxy.h"
#include "game/pad_target.h"
#include "game/planet.h"
#include "p8/fixed.h"
#include "p8/input.h"
#include "p8/vram.h"

#include <assert.h>

void pad_jump_init(pad_jump_t *jump) {
    *jump = (pad_jump_t){.hovered = ENTITY_ID_NONE};
}

void pad_jump_hover(pad_jump_t *jump, const entity_t *e) {
    jump->hovered = entity_id_link(e);
}

entity_t *pad_jump_hovered(const pad_jump_t *jump) {
    entity_t *hovered = entity_get(jump->hovered);
    return pad_target_is_candidate(hovered) ? hovered : NULL;
}

void pad_jump_hover_seeded(pad_jump_t *jump, const entity_t *e) {
    pad_jump_hover(jump, e);
    jump->seeded |= e != NULL;
}

entity_t *pad_jump_reseed(pad_jump_t *jump, const entity_list_t *candidates, int centre_x, int centre_y) {
    entity_t *picked = jump->seeded ? pad_jump_nearest(centre_x, centre_y, candidates)
                                    : pad_jump_seed(candidates, centre_x, centre_y);
    pad_jump_hover_seeded(jump, picked);
    return picked;
}

static const int directions_by_priority[PAD_JUMP_DIRECTIONS] = {P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT};

int pad_jump_direction(int index) {
    assert(index >= 0 && index < PAD_JUMP_DIRECTIONS && "pad_jump_direction: index 0 to PAD_JUMP_DIRECTIONS - 1");
    return directions_by_priority[index];
}

int pad_jump_first_direction(int buttons) {
    for (int i = 0; i < PAD_JUMP_DIRECTIONS; i++) {
        if (buttons & directions_by_priority[i]) {
            return directions_by_priority[i];
        }
    }
    return 0;
}

int pad_jump_repeat_step(pad_jump_t *jump, int pad_buttons, int pressed) {
    int direction = pad_jump_first_direction(pressed);
    if (direction != 0) {
        jump->repeat_direction = direction;
        jump->repeat_frames = 0;
        return jump->repeat_direction;
    }
    if (jump->repeat_direction == 0 || !(pad_buttons & jump->repeat_direction)) {
        pad_jump_repeat_clear(jump);
        return 0;
    }
    jump->repeat_frames++;
    if (jump->repeat_frames == PAD_JUMP_REPEAT_DELAY + PAD_JUMP_REPEAT_PERIOD) {
        jump->repeat_frames = PAD_JUMP_REPEAT_DELAY;
    }
    return jump->repeat_frames == PAD_JUMP_REPEAT_DELAY ? jump->repeat_direction : 0;
}

void pad_jump_repeat_clear(pad_jump_t *jump) {
    jump->repeat_direction = 0;
    jump->repeat_frames = 0;
}

static const int inner_left = PAD_JUMP_SCROLL_MARGIN;
static const int inner_right = P8_SCREEN_WIDTH - 1 - PAD_JUMP_SCROLL_MARGIN;
static const int inner_top = PAD_JUMP_SCROLL_MARGIN;
static const int inner_bottom = P8_SCREEN_HEIGHT - 1 - PAD_JUMP_SCROLL_MARGIN_BOTTOM;

int pad_jump_inside(int x, int y) {
    return x >= inner_left && x <= inner_right && y >= inner_top && y <= inner_bottom;
}

static int shift_into(int value, int low, int high) {
    int shift = value < low ? value - low : value > high ? value - high : 0;
    if (shift > PAD_JUMP_SCROLL_SPEED) {
        return PAD_JUMP_SCROLL_SPEED;
    }
    return shift < -PAD_JUMP_SCROLL_SPEED ? -PAD_JUMP_SCROLL_SPEED : shift;
}

void pad_jump_scroll(int x, int y, int *dx, int *dy) {
    *dx = shift_into(x, inner_left, inner_right);
    *dy = shift_into(y, inner_top, inner_bottom);
}

static int world_pixel(p8_fixed_t map_units) {
    return p8_fixed_flr_int(p8_fixed_mul(map_units, p8_fixed_from_int(GALAXY_MAP_SCALE)));
}

void pad_jump_world_point(const entity_t *e, int *x, int *y) {
    assert(e != NULL && entity_has_map_pos(e) && "pad_jump_world_point: the entity has a map position");
    vec_t map_pos = entity_map_pos(e);
    *x = world_pixel(map_pos.x);
    *y = world_pixel(map_pos.y);
}

entity_t *pad_jump_nearest(int x, int y, const entity_list_t *candidates) {
    if (candidates == NULL) {
        return NULL;
    }
    entity_t *best = NULL;
    int best_distance = 0;
    for (int i = 0; i < candidates->count; i++) {
        entity_t *e = candidates->items[i];
        int ex, ey;
        pad_jump_world_point(e, &ex, &ey);
        int distance = (ex - x) * (ex - x) + (ey - y) * (ey - y);
        if (best == NULL || distance < best_distance) {
            best = e;
            best_distance = distance;
        }
    }
    return best;
}

static int is_colony(const entity_t *e) {
    return e->class == planet_class() && e->state == ENTITY_STATE_SETTLED;
}

entity_t *pad_jump_seed(const entity_list_t *candidates, int centre_x, int centre_y) {
    if (candidates == NULL) {
        return NULL;
    }
    for (int i = 0; i < candidates->count; i++) {
        if (is_colony(candidates->items[i])) {
            return candidates->items[i];
        }
    }
    return pad_jump_nearest(centre_x, centre_y, candidates);
}

static int is_candidate(entity_t *e, void *ctx) {
    (void)ctx;
    return pad_target_is_candidate(e);
}

entity_list_t pad_jump_candidates(const allocator_t *zone, const entity_list_t *nodes) {
    return entity_list_filter(zone, nodes, is_candidate, NULL);
}
