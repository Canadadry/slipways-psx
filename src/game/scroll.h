#ifndef GAME_SCROLL_H
#define GAME_SCROLL_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* camera_update returns early on a NULL struct or input, camera_apply on a
 * NULL struct or graphics state; camera_set_velocity does not. A NULL
 * vector member reads as the zero vector. */

/* The velocity, the position before shake and the position start on the
 * vectors of the archetype, which every camera shares and nothing writes;
 * the first camera_update points them at `velocity_own`,
 * `pos_before_shake_own` and `pos_own`, rewritten in place from then on.
 * Never copy the struct. */
typedef struct {
    entity_t header;
    const camera_archetype_t *archetype;
    const vec_t *velocity;
    p8_fixed_t shake;
    const vec_t *pos_before_shake;
    const vec_t *pos;
    vec_t pos_before_shake_own;
    vec_t velocity_own;
    vec_t pos_own;
} camera_t;

/* Requires entity_init() to have run --
 * asserted, not repaired. The camera has no constructor of its own:
 * entity_construct registers it into the live entities and its messages. */
void camera_init(void);

void camera_shutdown(void);

/* 1 from camera_init() to camera_shutdown(), 0 otherwise. */
int camera_ready(void);
const entity_class_t *camera_class(void);

/* The struct registers itself through entity_register; it lives in the
 * current game-zone generation. */
camera_t *camera_new(void);

/* Nothing outside the camera keeps its velocity, position before shake
 * or position: each tick rewrites them in place. */
void camera_update(const p8_input_t *input, camera_t *self);

/* Called by background_drag: writes `velocity_own` and points `velocity` at it. */
void camera_set_velocity(camera_t *self, vec_t v);

/* The parallax factor is the layer's own, from draw_layers, never absent:
 * there is no entry point with a default factor. */
void camera_apply(p8_graphics_t *gfx, camera_t *self, p8_fixed_t parallax);

/* `main_camera()` (engine/entity_registry.h) as a `camera_t`: NULL when
 * there is none and on the placeholder. */
camera_t *main_camera_typed(void);

#endif
