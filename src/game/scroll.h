#ifndef GAME_SCROLL_H
#define GAME_SCROLL_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* Throughout this module a NULL struct, `gfx` or `input` argument is a
 * safe no-op, and a nil (NULL) vector member reads as the zero vector
 * rather than crashing. */

/* `v`, `p` and `ra` start on the vectors of the archetype, which every qb
 * shares and nothing writes; the first `ph` points them at `v_own`, `p_own`
 * and `ra_own`, rewritten in place from then on. Never copy the struct. */
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

/* Calls pico8_pi_init(), and requires pico8_ke_init() to have run --
 * asserted, not repaired. qb has no `pq` of its own; it inherits ke's,
 * which is what registers the camera into hv/cv/cg. */
void camera_init(void);

void camera_shutdown(void);

/* 1 from pico8_qb_init() to pico8_qb_shutdown(), 0 otherwise. */
int camera_ready(void);
const entity_class_t *camera_class(void);

/* The struct registers itself through `lt`; it lives in the current
 * game-zone generation. */
camera_t *camera_new(void);

/* Nothing outside the camera keeps `p`, `v` or `ra`: each tick rewrites
 * them in place. */
void camera_update(const p8_input_t *input, camera_t *self);

/* `mo.v=v` (bg:hq): writes `v_own` and points `v` at it. */
void camera_set_velocity(camera_t *self, vec_t v);

/* `fy` is `dc`'s `ci[o]`, never nil: the cart's `fy or 1` fallback has no
 * caller, so there is no separate _default entry point here. */
void camera_apply(p8_graphics_t *gfx, camera_t *self, p8_fixed_t fy);

/* `mo` (pico8_entity.h) as its `qb`: NULL when there is no mo and on the
 * placeholder. */
camera_t *main_camera_typed(void);

#endif
