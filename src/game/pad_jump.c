#include "game/pad_jump.h"

#include "game/galaxy.h"
#include "game/pad_target.h"
#include "game/planet.h"
#include "p8/fixed.h"
#include "p8/input.h"
#include "p8/vram.h"

#include <assert.h>

void pad_jump_init(pad_jump_t *jump) {
    *jump = (pad_jump_t){.hovered = ENTITY_ID_NONE, .game = ENTITY_ID_NONE};
}

static const int directions_by_priority[] = {P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT};

int pad_jump_repeat_step(pad_jump_t *jump, int pad_buttons, int pressed) {
    for (int i = 0; i < (int)(sizeof directions_by_priority / sizeof directions_by_priority[0]); i++) {
        if (pressed & directions_by_priority[i]) {
            jump->repeat_direction = directions_by_priority[i];
            jump->repeat_frames = 0;
            return jump->repeat_direction;
        }
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

int pad_jump_candidates(const entity_list_t *nodes, entity_t **storage, int capacity, entity_list_t *out) {
    int count = 0;
    for (int i = 0; i < nodes->count; i++) {
        entity_t *e = nodes->items[i];
        if (pad_target_is_candidate(e)) {
            assert(count < capacity && "pad_jump_candidates: the storage holds every candidate; raise the caller's cap");
            storage[count++] = e;
        }
    }
    *out = (entity_list_t){.items = storage, .count = count, .capacity = capacity};
    return count;
}
