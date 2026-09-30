#ifndef PICO8_SCROLL_H
#define PICO8_SCROLL_H

#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_ke.h"
#include "pico8_struct.h"
#include "pico8_vec.h"

/* Throughout this module a NULL struct, `gfx` or `input` argument is a
 * safe no-op, and a nil (NULL) vector member reads as the zero vector
 * rather than crashing. */

/* `v`, `p` and `ra` start on the vectors of `pico8_qb_start`, which every qb
 * shares, `pc` on `pico8_qb_pc_vecs`: none is ever modified in place, only
 * rebound. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_struct_header_t header;
    const pico8_vec_t *v;
    pico8_fixed_t ma;
    const pico8_vec_t *p;
    const pico8_vec_t *ra;
    const pico8_vec_seq_t *pc;
} pico8_qb_t;

/* Calls pico8_pi_init(), and requires pico8_ke_init() to have run --
 * asserted, not repaired. qb has no `pq` of its own; it inherits ke's,
 * which is what registers the camera into hv/cv/cg. */
void pico8_qb_init(void);

void pico8_qb_shutdown(void);

/* 1 from pico8_qb_init() to pico8_qb_shutdown(), 0 otherwise. */
int pico8_qb_ready(void);
const pico8_struct_class_t *pico8_qb_struct_class(void);

/* The struct registers itself through `lt`, and nothing frees it. */
pico8_qb_t *pico8_qb_new(void);

/* Rebinds `p`, `v` and `ra` to fresh heap vectors each tick -- these are
 * whole-member writes, not component mutations, so no earlier reader of
 * the old vectors sees the new values. */
void pico8_qb_ph(const pico8_input_t *input, pico8_qb_t *self);

/* `fy` is `dc`'s `ci[o]`, never nil: the cart's `fy or 1` fallback has no
 * caller, so there is no separate _default entry point here. */
void pico8_qb_md(pico8_gfx_t *gfx, pico8_qb_t *self, pico8_fixed_t fy);

#endif
