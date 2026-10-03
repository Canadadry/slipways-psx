#include "game/scroll.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "game/draw_functions.h"
#include "engine/vec.h"
#include "engine/game_zone.h"

static int g_camera_up = 0;

/* NULL reads as the zero vector. */
static vec_t camera_vec(const vec_t *v) {
    if (v != NULL) {
        return *v;
    }
    return vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

/* update_entities() calls the per-tick state method with the entity's
 * frame counter, which camera_update does not read. */
static void camera_update_method(entity_t *self, p8_fixed_t t) {
    (void)t;
    camera_update(dispatch_context_input(), (camera_t *)self);
}

static void camera_apply_method(entity_t *self, p8_fixed_t fy) {
    camera_apply(dispatch_context_gfx(), (camera_t *)self, fy);
}

static const entity_class_t camera_class_def = {
    .states = {[ENTITY_STATE_NORMAL] = camera_update_method},
    .apply_camera = camera_apply_method,
};

static camera_t *camera_create(void) {
    camera_t *self = (camera_t *)allocator_alloc(game_zone(), sizeof *self);

    entity_init_instance(&self->header, &camera_class_def, camera_archetype.state, camera_archetype.state_time);
    self->archetype = &camera_archetype;
    self->header.pos = &self->pos;
    self->header.map_pos = &self->pos_before_shake;
    self->header.shake = &self->shake;
    self->velocity = &self->archetype->velocity;
    self->shake = self->archetype->shake;
    self->pos_before_shake = &self->archetype->pos_before_shake;
    self->pos = &self->archetype->pos;
#ifdef CLASS_COVERAGE_BUILD
    class_coverage_record("camera");
#endif
    entity_construct(&self->header);
    return self;
}

void camera_init(void) {
    camera_shutdown();
    draw_functions_init();
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
        int btn_p1 = p8_btn(input, b);
        /* TROU-2 */
        int btn_p2 = 0;

        if (btn_p1 || btn_p2) {
            vec_t dir = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
            if (dpad_directions != NULL && b < dpad_directions->count) {
                dir = dpad_directions->items[b];
            }
            direction = vec_add(direction, vec_mul(dir, p8_fixed_from_int(3)));
        }
    }

    p = camera_vec(self->pos_before_shake);
    v = camera_vec(self->velocity);

    p = vec_add(p, v);
    v = vec_add(v, vec_mul(vec_sub(direction, v), P8_FIXED_LIT(0x00003333, 0.2)));

    p.x = p8_fixed_mid(p8_fixed_from_int(-64), p8_fixed_from_int(320), p.x);
    p.y = p8_fixed_mid(p8_fixed_from_int(-64), p8_fixed_from_int(320), p.y);

    self->pos_before_shake_own = p;
    self->pos_before_shake = &self->pos_before_shake_own;
    camera_set_velocity(self, v);

    self->pos_own = entity_geom_shake(p, (entity_t *)self);
    self->pos = &self->pos_own;
}

void camera_set_velocity(camera_t *self, vec_t v) {
    self->velocity_own = v;
    self->velocity = &self->velocity_own;
}

void camera_apply(p8_graphics_t *gfx, camera_t *self, p8_fixed_t fy) {
    vec_t cp;

    if (self == NULL || gfx == NULL) {
        return;
    }

    cp = vec_mul(camera_vec(self->pos), fy);
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
