#include "game/scroll.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "engine/vec.h"
#include "engine/game_zone.h"
#include "game/background.h"

static int g_camera_up = 0;

static void camera_update_method(entity_t *self, p8_fixed_t t) {
    (void)t;
    camera_update(dispatch_context_input(), (camera_t *)self);
}

static void camera_apply_method(entity_t *self, p8_fixed_t parallax) {
    camera_apply(dispatch_context_gfx(), (camera_t *)self, parallax);
}

static const entity_class_t camera_class_def = {
    .states = {[ENTITY_STATE_NORMAL] = camera_update_method},
    .apply_camera = camera_apply_method,
};

static camera_t *camera_create(void) {
    camera_t *self = (camera_t *)allocator_alloc(game_zone(), sizeof *self);

    entity_init_instance(&self->header, &camera_class_def, camera_archetype.state, camera_archetype.state_time);
    self->archetype = &camera_archetype;
    self->header.has |= ENTITY_HAS_SHAKE;
    self->velocity = self->archetype->velocity;
    self->header.shake = self->archetype->shake;
    entity_set_map_pos(&self->header, self->archetype->pos_before_shake);
    entity_set_pos(&self->header, self->archetype->pos);
    entity_construct(&self->header);
    return self;
}

void camera_init(void) {
    camera_shutdown();
    assert(entity_ready() && "camera_init: entity_init() runs first");
    g_camera_up = 1;
}

void camera_shutdown(void) {
    g_camera_up = 0;
}

int camera_ready(void) {
    return g_camera_up;
}

const entity_class_t *camera_class(void) {
    return &camera_class_def;
}

camera_t *camera_new(void) {
    return camera_create();
}

/* GAP-27 */
static p8_fixed_t camera_bound(p8_fixed_t p, int frame_size) {
    p8_fixed_t far = p8_fixed_div(p8_fixed_from_int(BACKGROUND_GRID_EDGE + 1 - frame_size), layer_fades_table[0]);

    return p8_fixed_mid(p8_fixed_from_int(0), far, p);
}

static vec_t camera_bounded(vec_t p) {
    return vec_make(camera_bound(p.x, P8_SCREEN_WIDTH), camera_bound(p.y, P8_SCREEN_HEIGHT));
}

void camera_update(const p8_input_t *input, camera_t *self) {
    vec_t direction = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    const vec_seq_t *dpad_directions = NULL;
    vec_t p;
    vec_t v;
    int b;

    if (self == NULL || input == NULL) {
        return;
    }

    dpad_directions = self->archetype->dpad_directions;

    for (b = 0; b <= 3; b++) {
        if (p8_btn(input, b)) {
            vec_t dir = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
            if (dpad_directions != NULL && b < dpad_directions->count) {
                dir = dpad_directions->items[b];
            }
            direction = vec_add(direction, vec_mul(dir, p8_fixed_from_int(3)));
        }
    }

    p = camera_pos_before_shake(self);
    v = self->velocity;

    p = vec_add(p, v);
    v = vec_add(v, vec_mul(vec_sub(direction, v), P8_FIXED_LIT(0x00003333, 0.2)));

    p = camera_bounded(p);

    entity_set_map_pos(&self->header, p);
    camera_set_velocity(self, v);

    entity_set_pos(&self->header, camera_bounded(entity_geom_shake(p, (entity_t *)self)));
}

void camera_set_velocity(camera_t *self, vec_t v) {
    self->velocity = v;
}

void camera_apply(p8_graphics_t *gfx, camera_t *self, p8_fixed_t parallax) {
    vec_t cp;

    if (self == NULL || gfx == NULL) {
        return;
    }

    cp = vec_mul(entity_pos(&self->header), parallax);
    p8_camera(gfx, p8_fixed_flr_int(cp.x), p8_fixed_flr_int(cp.y));
}

camera_t *main_camera_typed(void) {
    entity_t *mo = main_camera();

    if (mo == NULL || mo->class == main_camera_placeholder_class()) {
        return NULL;
    }
    assert(mo->class == camera_class() && "main_camera_typed: only the placeholder or a camera is ever stored");
    return (camera_t *)mo;
}
