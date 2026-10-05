#ifndef GAME_SCROLL_H
#define GAME_SCROLL_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

typedef struct {
    entity_t header;
    const camera_archetype_t *archetype;
    const vec_t *velocity;
    const vec_t *pos_before_shake;
    const vec_t *pos;
    vec_t pos_before_shake_own;
    vec_t velocity_own;
    vec_t pos_own;
} camera_t;

void camera_init(void);

void camera_shutdown(void);

int camera_ready(void);
const entity_class_t *camera_class(void);

camera_t *camera_new(void);

void camera_update(const p8_input_t *input, camera_t *self);

void camera_set_velocity(camera_t *self, vec_t v);

void camera_apply(p8_graphics_t *gfx, camera_t *self, p8_fixed_t parallax);

camera_t *main_camera_typed(void);

#endif
