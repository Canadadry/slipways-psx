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

/* `mz.jj`: no parent, never an entity, and no writer. */
typedef struct {
    pico8_struct_header_t header;
    pico8_value_t qs;
} pico8_mz_jj_t;

/* `nu` and `kk` point at the tables the module owns, `jj` at the one
 * `pico8_mz_jj_t`; all shared by every `mz` and never modified in place.
 * Nothing frees the struct: TROU-7. */
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

/* Frees the `mz` spec and the module's `nu`/`kk` tables, never any `mz`
 * nor the `jj` wrapper. Idempotent, and safe before the first
 * `pico8_wormhole_init()`. */
void pico8_wormhole_shutdown(void);

/* 1 from pico8_wormhole_init() to pico8_wormhole_shutdown(), 0 otherwise. */
int pico8_mz_ready(void);

/* `mz`'s spec default `name`, then ke's; 0 when absent, or when not ready. A
 * REF is borrowed from the spec, valid until pico8_wormhole_shutdown(). */
int pico8_mz_default(const char *name, pico8_value_t *out_value);
const pico8_struct_class_t *pico8_mz_struct_class(void);

/* `overrides` is read, not kept, and must be NULL or empty. The struct
 * registers itself into `hv`/`cv`/`cg` through `lt`, and nothing frees
 * it. */
pico8_mz_t *pico8_mz_new(const pico8_ob_table_t *overrides);

/* The one `mz.jj`, shared by every `mz`: static, never NULL, never
 * freed, and without a writer, so a write reaching it stops the run. */
pico8_mz_jj_t *pico8_mz_jj_wrapper(void);
const pico8_struct_class_t *pico8_mz_jj_struct_class(void);

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
