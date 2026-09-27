#ifndef PICO8_SCORE_PANEL_H
#define PICO8_SCORE_PANEL_H

#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* Throughout this module a NULL panel argument is a safe no-op. */

/* TROU-7 */
/* `gs`, `i_` are the tables of `pj`'s class, borrowed and never modified. */
typedef struct {
    pico8_ke_t ke;
    pico8_value_t fa;
    pico8_value_t gs;
    pico8_value_t i_;
    pico8_value_t ed;
    pico8_value_t c_;
    pico8_value_t gz;
} pico8_pj_t;

/* `pj.cr`: no parent, never an entity, and no writer. */
typedef struct {
    pico8_struct_header_t header;
    pico8_value_t hi;
    pico8_value_t gq;
    pico8_value_t ca;
    pico8_value_t ep;
} pico8_pj_cr_t;

/* Calls `pico8_pi_init()` first: the `pj` spec and `bz` both contain
 * `v(...)`, so the 'v' constructor must be registered before either spec is
 * parsed. Builds `ef` and `bz` alongside the spec. */
void pico8_score_panel_init(void);

/* Frees the `pj` spec, `ef` and `bz`. Resets `ju` to NULL without freeing
 * the instance it named: `ju` is a bookmark, `hv` owns the entity. */
void pico8_score_panel_shutdown(void);

/* 1 from pico8_score_panel_init() to pico8_score_panel_shutdown(), 0 otherwise. */
int pico8_pj_ready(void);

/* `pj`'s spec default `name`, then ke's; 0 when absent, or when not ready. A
 * REF is borrowed from the spec, valid until pico8_score_panel_shutdown(). */
int pico8_pj_default(const char *name, pico8_value_t *out_value);
const pico8_struct_class_t *pico8_pj_struct_class(void);

/* Shared by every panel, past and future: `hb` rewrites `ou`, `hk` rewrites
 * `ea`, in place; the tables are owned by the class. `cr` is the one static
 * `pico8_pj_cr_t`, never freed, whose `ep` `cr()` rewrites and the init
 * resets to nil. All NULL before `pico8_score_panel_init()` and after the
 * shutdown. */
pico8_ob_table_t *pico8_pj_ou(void);
pico8_ob_table_t *pico8_pj_ea(void);
pico8_pj_cr_t *pico8_pj_cr(void);
const pico8_struct_class_t *pico8_pj_cr_struct_class(void);
const pico8_ob_table_t *pico8_pj_i_(void);
const pico8_ob_table_t *pico8_pj_gs(void);

/* TROU-7 */
/* `overrides` must be NULL or empty: a key asserts. The panel registers
 * itself through `lt` into `hv`, `cv.kj`, `cv.hk`, `cv.hb` and no `cg`;
 * nothing ever frees it. Asserts when called before the init. */
pico8_pj_t *pico8_pj_new(const pico8_ob_table_t *overrides);

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
pico8_pj_t *pico8_ju(void);
void pico8_pj_set_ju(pico8_pj_t *instance);

void pico8_pj_hb(void);

void pico8_pj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_pj_t *self);

void pico8_eg(void);

#endif
