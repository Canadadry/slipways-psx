#ifndef PICO8_HELP_H
#define PICO8_HELP_H

#include "pico8_fixed.h"
#include "pico8_geom.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_probe.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL instance, gfx or mem argument is a safe
 * no-op, and an absent or wrong-kind field reads as absent. */

/* TROU-7 */
/* `ra`, `hi`, `gq`, `kq` point at `pico8_fv_start`'s, shared by every help;
 * `k_` points at `pico8_fv_k__seq`, `rb` at `pico8_fv_rb_row`, `nu` is
 * `g_`'s. */
typedef struct {
    pico8_g__t g_;
    const pico8_vec_t *hi;
    pico8_fixed_t ca;
    const pico8_vec_t *gq;
    pico8_fixed_t eo;
    pico8_fixed_t ff;
    const pico8_qz_box_t *kq;
    pico8_fixed_t ql;
    pico8_fixed_t fg;
    const pico8_jb_row_t *rb;
    int m_;
    int has_m_; /* 0: `m_` is nil, and is then never read */
} pico8_fv_t;

/* Builds `bo`; `pico8_help_shutdown()` frees it and is called first by an
 * init, so both are idempotent. Both also reset `eu` to the placeholder, so
 * the real one must be installed after every init. */
void pico8_help_init(void);
void pico8_help_shutdown(void);

/* 1 from pico8_help_init() to pico8_help_shutdown(), 0 otherwise. */
int pico8_fv_ready(void);
const pico8_struct_class_t *pico8_fv_struct_class(void);

/* TROU-8 */
/* The help inherits `g_`'s `nu`, so `lt` registers it into `cg.g_` and
 * `cg.prober` too; nothing ever frees it. Asserts when called before
 * `pico8_help_init()`. */
pico8_fv_t *pico8_fv_new(void);

/* `pico8_eu_field_value()` always returns a PICO8_VALUE_FUNCTION value: an
 * inert placeholder until `pico8_help_set_eu()` installs the real `eu`, which
 * `pico8_help_set_eu(NULL)` puts back. */
pico8_value_t pico8_eu_field_value(void);
void pico8_help_set_eu(pico8_value_function_t fn);

/* Stamps the `eu` value current at call time onto every row of `bo`: rows
 * stamped before a `pico8_help_set_eu()` keep what they were given. A NULL
 * `self` still stamps them and only skips the `pz()` call. */
void pico8_fv_pq(pico8_fv_t *self);

void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fv_t *self, pico8_vec_t p);

pico8_value_t pico8_fv_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

#endif
