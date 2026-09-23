#ifndef PICO8_SCROLL_H
#define PICO8_SCROLL_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_ob.h"

/* Throughout this module a NULL instance, `gfx` or `input` argument is a
 * safe no-op, and an absent or wrong-kind vector field reads as the zero
 * vector rather than crashing. */

/* Calls pico8_pi_init() first: the spec holds `v(...)`, so the 'v'
 * constructor must already be registered when it is parsed, and requires
 * pico8_ke_init() to have run -- both are asserted, not repaired. qb has
 * no `pq` of its own; it inherits ke's, which is what registers the
 * camera into hv/cv/cg. */
void pico8_qb_init(void);
void pico8_qb_shutdown(void);

/* NULL before pico8_qb_init() and after pico8_qb_shutdown(). Not
 * reachable through pico8_class_lookup(). */
pico8_class_t *pico8_qb_class(void);

/* `overrides` is copied, not kept, and nothing ever frees the instance. */
pico8_instance_t *pico8_qb_new(const pico8_ob_table_t *overrides);

/* Rebinds `p`, `v` and `ra` to fresh heap vectors each tick -- these are
 * whole-field writes, not component mutations, so no earlier reader of
 * the old vectors sees the new values. */
void pico8_qb_ph(const pico8_input_t *input, pico8_instance_t *self);

/* `fy` is already resolved by the caller: Lua's `fy or 1` fallback lives
 * in this module's dispatch adapter, so there is no separate _default
 * entry point here. */
void pico8_qb_md(pico8_gfx_t *gfx, pico8_instance_t *self, pico8_fixed_t fy);

#endif
