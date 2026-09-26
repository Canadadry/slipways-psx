#ifndef PICO8_HELP_H
#define PICO8_HELP_H

#include "pico8_class.h"
#include "pico8_fixed.h"
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
/* `ra`, `hi`, `gq`, `k_`, `kq`, `rb` are the tables of `fv`'s class,
 * borrowed and never modified; `nu` is `g_`'s. */
typedef struct {
    pico8_g__t g_;
    pico8_value_t hi;
    pico8_value_t ca;
    pico8_value_t gq;
    pico8_value_t eo;
    pico8_value_t ff;
    pico8_value_t kq;
    pico8_value_t ql;
    pico8_value_t fg;
    pico8_value_t rb;
    pico8_value_t m_;
} pico8_fv_t;

/* Builds the `fv` class and `bo`; `pico8_help_shutdown()` frees both and is
 * called first by an init, so both are idempotent. Both also reset `eu` to
 * the placeholder, so the real one must be installed after every init. */
void pico8_help_init(void);
void pico8_help_shutdown(void);

/* NULL before `pico8_help_init()` and after `pico8_help_shutdown()`. */
pico8_class_t *pico8_fv_class(void);
const pico8_struct_class_t *pico8_fv_struct_class(void);

/* TROU-8 */
/* `overrides` must be NULL or empty: a key asserts. The help inherits `g_`'s
 * `nu`, so `lt` registers it into `cg.g_` and `cg.prober` too; nothing ever
 * frees it. Asserts when called before `pico8_help_init()`. */
pico8_fv_t *pico8_fv_new(const pico8_ob_table_t *overrides);

/* Owned by this module and only borrowed by callers: built by
 * `pico8_help_init()`, freed by `pico8_help_shutdown()`, NULL in between. */
const pico8_ob_table_t *pico8_bo(void);

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
