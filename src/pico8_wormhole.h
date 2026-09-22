#ifndef PICO8_WORMHOLE_H
#define PICO8_WORMHOLE_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL instance argument is a safe no-op, and an
 * absent or wrong-kind field reads as its zero/false default rather than
 * crashing. */

void pico8_wormhole_init(void);

/* Frees the `mz` class and the `jj` wrapper, never any `mz` instance.
 * Idempotent, and safe before the first `pico8_wormhole_init()`. */
void pico8_wormhole_shutdown(void);

/* NULL before `pico8_wormhole_init()` and after `pico8_wormhole_shutdown()`.
 * Also reachable as `pico8_class_lookup_named("mz")`. */
pico8_class_t *pico8_mz_class(void);

/* `overrides` is copied, not kept. The new instance registers itself into
 * `hv`/`cv`/`cg` through `lt`, and nothing ever frees it. */
pico8_instance_t *pico8_mz_new(const pico8_ob_table_t *overrides);

/* `mz.jj` wrapped in a classless instance, so that `self.et` holds an
 * instance whichever branch wrote it and `dl` can read `.qs` off it
 * uniformly. Its `fields` are borrowed from the class defaults -- shared by
 * every `mz`, never freed nor written through this wrapper. NULL before
 * `pico8_wormhole_init()`. */
pico8_instance_t *pico8_mz_jj_wrapper(void);

/* TROU-7 */
void pico8_mz_pq(pico8_instance_t *self);

/* Calls `pico8_bv` first, which is what sets `ra`/`p` and a first `et`;
 * the "too far" branch here then overwrites `et`. */
void pico8_mz_hq(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp);

void pico8_mz_kz(pico8_instance_t *self, pico8_fixed_t t);

void pico8_mz_ey(pico8_instance_t *self);

/* TROU-3 */
void pico8_mz_pr(pico8_instance_t *self, pico8_fixed_t t);

/* Reads `self.oi` only; `self` is non-const for signature uniformity. */
pico8_fixed_t pico8_mz_ej(pico8_instance_t *self, pico8_fixed_t t);

#endif
