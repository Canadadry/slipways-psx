#ifndef PICO8_WORMHOLE_H
#define PICO8_WORMHOLE_H

#include "pico8_ents.h"
#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_ptrs.h"
#include "pico8_mem.h"
#include "pico8_probe.h"
#include "pico8_struct.h"
#include "pico8_vec.h"

/* Throughout this module a NULL struct argument is a safe no-op, and a nil
 * or wrong-kind member reads as its zero/false default rather than
 * crashing. */

/* `mz.jj`: no parent, never an entity, and no writer. */
typedef struct {
    pico8_struct_header_t header;
    const char *qs;
} pico8_mz_jj_t;

/* `jj` points at the one `pico8_mz_jj_t`, shared by every `mz` and never
 * modified in place. `od` and `et` live in the probe; `probe` points at
 * `probe_own`: never copy the struct. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_struct_header_t header;
    const pico8_mz_def_t *def;
    pico8_fixed_t oi;
    pico8_mz_jj_t *jj;
    pico8_ents_t *ox;
    const pico8_vec_t *ra;
    const pico8_vec_t *p;
    pico8_struct_header_t *ij; /* borrowed, never freed */
    int p_;
    pico8_probe_t *probe;
    pico8_probe_t probe_own;
} pico8_mz_t;

void pico8_wormhole_init(void);

/* Never frees any `mz` nor the `jj` wrapper. Idempotent, and safe before
 * the first `pico8_wormhole_init()`. */
void pico8_wormhole_shutdown(void);

/* 1 from pico8_wormhole_init() to pico8_wormhole_shutdown(), 0 otherwise. */
int pico8_mz_ready(void);
const pico8_struct_class_t *pico8_mz_struct_class(void);

/* The struct registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
 * frees it. */
pico8_mz_t *pico8_mz_new(void);

/* The one `mz.jj`, shared by every `mz`: static, never NULL, never
 * freed, and without a writer, so a write reaching it stops the run. */
pico8_mz_jj_t *pico8_mz_jj_wrapper(void);
const pico8_struct_class_t *pico8_mz_jj_struct_class(void);

/* TROU-7 */
void pico8_mz_pq(pico8_mz_t *self);

/* Calls `pico8_bv` first, which is what sets `ra`/`p` and a first `et`;
 * the "too far" branch here then overwrites `et`. */
void pico8_mz_hq(pico8_mz_t *self, pico8_vec_t *mp, pico8_vec_t *rp);

void pico8_mz_kz(pico8_mz_t *self, pico8_fixed_t t);

void pico8_mz_ey(pico8_mz_t *self);

/* TROU-3 */
void pico8_mz_pr(pico8_mz_t *self, pico8_fixed_t t);

/* Reads `self.oi` only; `self` is non-const for signature uniformity. */
pico8_fixed_t pico8_mz_ej(pico8_mz_t *self, pico8_fixed_t t);

void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra);

#endif
