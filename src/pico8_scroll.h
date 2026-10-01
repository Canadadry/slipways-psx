#ifndef PICO8_SCROLL_H
#define PICO8_SCROLL_H

#include "p8/fixed.h"
#include "pico8_game_data.h"
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
    pico8_struct_header_t header;
    const pico8_qb_def_t *def;
    const pico8_vec_t *v;
    pico8_fixed_t ma;
    const pico8_vec_t *p;
    const pico8_vec_t *ra;
    pico8_vec_t p_own;
    pico8_vec_t v_own;
    pico8_vec_t ra_own;
} pico8_qb_t;

/* Calls pico8_pi_init(), and requires pico8_ke_init() to have run --
 * asserted, not repaired. qb has no `pq` of its own; it inherits ke's,
 * which is what registers the camera into hv/cv/cg. */
void pico8_qb_init(void);

void pico8_qb_shutdown(void);

/* 1 from pico8_qb_init() to pico8_qb_shutdown(), 0 otherwise. */
int pico8_qb_ready(void);
const pico8_struct_class_t *pico8_qb_struct_class(void);

/* The struct registers itself through `lt`; it lives in the current
 * game-zone generation. */
pico8_qb_t *pico8_qb_new(void);

/* Nothing outside the camera keeps `p`, `v` or `ra`: each tick rewrites
 * them in place. */
void pico8_qb_ph(const pico8_input_t *input, pico8_qb_t *self);

/* `mo.v=v` (bg:hq): writes `v_own` and points `v` at it. */
void pico8_qb_set_v(pico8_qb_t *self, pico8_vec_t v);

/* `fy` is `dc`'s `ci[o]`, never nil: the cart's `fy or 1` fallback has no
 * caller, so there is no separate _default entry point here. */
void pico8_qb_md(pico8_gfx_t *gfx, pico8_qb_t *self, pico8_fixed_t fy);

/* `mo` (pico8_entity.h) as its `qb`: NULL when there is no mo and on the
 * placeholder. */
pico8_qb_t *pico8_mo_qb(void);

#endif
