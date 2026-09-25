#ifndef PICO8_WORMHOLE_H
#define PICO8_WORMHOLE_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_mem.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL struct argument is a safe no-op, and a nil
 * or wrong-kind member reads as its zero/false default rather than
 * crashing. */

/* `nu`, `kk` and `jj` point at the tables the module owns, shared by every
 * `mz` and never modified in place. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_ke_t ke;
    pico8_value_t nu;
    pico8_value_t dw;
    pico8_value_t dq;
    pico8_value_t spawn_snd;
    pico8_value_t ik;
    pico8_value_t oi;
    pico8_value_t od;
    pico8_value_t fp;
    pico8_value_t fa;
    pico8_value_t kk;
    pico8_value_t jj;
    pico8_value_t ox;
    pico8_value_t ra;
    pico8_value_t p;
    pico8_value_t et;
    pico8_value_t ij;
    pico8_value_t p_;
} pico8_mz_t;

void pico8_wormhole_init(void);

/* Frees the `mz` class, the `jj` wrapper and the module's `nu`/`kk`/`jj`
 * tables, never any `mz`. Idempotent, and safe before the first
 * `pico8_wormhole_init()`. */
void pico8_wormhole_shutdown(void);

/* NULL before `pico8_wormhole_init()` and after `pico8_wormhole_shutdown()`.
 * Also reachable as `pico8_class_lookup_named("mz")`. */
pico8_class_t *pico8_mz_class(void);
const pico8_struct_class_t *pico8_mz_struct_class(void);

/* `overrides` is read, not kept, and must be NULL or empty. The struct
 * registers itself into `hv`/`cv`/`cg` through `lt`, and nothing frees
 * it. */
pico8_mz_t *pico8_mz_new(const pico8_ob_table_t *overrides);

/* `mz.jj` wrapped in a classless instance, so that `self.et` holds an
 * instance whichever branch wrote it and `dl` can read `.qs` off it
 * uniformly. Its `fields` are the module's `jj` table -- shared by every
 * `mz`, never freed nor written through this wrapper. NULL before
 * `pico8_wormhole_init()`. */
pico8_instance_t *pico8_mz_jj_wrapper(void);

/* TROU-7 */
void pico8_mz_pq(pico8_mz_t *self);

/* Calls `pico8_bv` first, which is what sets `ra`/`p` and a first `et`;
 * the "too far" branch here then overwrites `et`. */
void pico8_mz_hq(pico8_mz_t *self, pico8_value_t mp, pico8_value_t rp);

void pico8_mz_kz(pico8_mz_t *self, pico8_fixed_t t);

void pico8_mz_ey(pico8_mz_t *self);

/* TROU-3 */
void pico8_mz_pr(pico8_mz_t *self, pico8_fixed_t t);

/* Reads `self.oi` only; `self` is non-const for signature uniformity. */
pico8_fixed_t pico8_mz_ej(pico8_mz_t *self, pico8_fixed_t t);

#endif
