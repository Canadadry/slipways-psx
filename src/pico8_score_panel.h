#ifndef PICO8_SCORE_PANEL_H
#define PICO8_SCORE_PANEL_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Throughout this module a NULL instance argument is a safe no-op, and an
 * absent or wrong-kind field reads as absent rather than crashing. */

/* Calls `pico8_pi_init()` first: the `pj` spec and `bz` both contain
 * `v(...)`, so the 'v' constructor must be registered before either spec is
 * parsed. Builds `ef` and `bz` alongside the class. */
void pico8_score_panel_init(void);

/* Frees the `pj` class, `ef` and `bz`. Resets `ju` to NULL without freeing
 * the instance it named: `ju` is a bookmark, `hv` owns the entity. */
void pico8_score_panel_shutdown(void);

/* NULL before `pico8_score_panel_init()` and after the shutdown. */
pico8_class_t *pico8_pj_class(void);

/* `overrides` is copied, not kept. The new instance registers itself into
 * `hv`/`cv`/`cg` through `lt`, and nothing ever frees it. */
pico8_instance_t *pico8_pj_new(const pico8_ob_table_t *overrides);

/* Owned by this module, freed by `pico8_score_panel_shutdown()`. */
const pico8_ob_table_t *pico8_ef(void);

/* Owned by this module, freed by `pico8_score_panel_shutdown()`. Its rows
 * are rewritten in place by every `pj:ph()` run -- they are the shared
 * originals, never copies. */
const pico8_ob_table_t *pico8_bz(void);

/* Always a valid FUNCTION value: an inert placeholder until the real
 * `_init` bridge is installed, which a NULL argument resets. A button built
 * earlier keeps whichever value it captured then. */
pico8_value_t pico8_init_field_value(void);
void pico8_score_panel_set_init(pico8_value_function_t fn);

/* NULL until `eu()` builds the one `pj` instance and sets it here. The
 * setter takes no ownership. */
pico8_instance_t *pico8_ju(void);
void pico8_pj_set_ju(pico8_instance_t *instance);

void pico8_pj_hb(void);

void pico8_pj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self);

void pico8_eg(void);

#endif
