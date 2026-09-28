#ifndef PICO8_SCORE_PANEL_H
#define PICO8_SCORE_PANEL_H

#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_mem.h"
#include "pico8_msgbox.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL panel argument is a safe no-op. */

/* TROU-7 */
/* `gs` points at `pico8_pj_gs_row`, `i_` at `pico8_pj_i__row`. `ed` NULL
 * reads as `pico8_pj_start.ed`, never nil. */
typedef struct {
    pico8_ke_t ke;
    pico8_fixed_t fa;
    const pico8_jb_row_t *gs;
    const pico8_jb_row_t *i_;
    pico8_mh_t *ed;
    int c_;
    int has_c_;
    pico8_fixed_t gz;
    int has_gz;
} pico8_pj_t;

/* `pj.cr`: no parent, never an entity, and no writer. `hi`, `gq` point at
 * `pico8_pj_cr_start`'s. */
typedef struct {
    pico8_struct_header_t header;
    const pico8_vec_t *hi;
    const pico8_vec_t *gq;
    pico8_fixed_t ca;
    const char *ep;
} pico8_pj_cr_t;

/* Calls `pico8_pi_init()` first. Builds `bz`, and `ou`/`ea` from
 * `pico8_pj_ou_row`/`pico8_pj_ea_row`. */
void pico8_score_panel_init(void);

/* Frees `bz`; drops `ou`/`ea` without freeing them. Resets `ju` to
 * NULL without freeing the instance it named: `ju` is a bookmark, `hv` owns
 * the entity. */
void pico8_score_panel_shutdown(void);

/* 1 from pico8_score_panel_init() to pico8_score_panel_shutdown(), 0 otherwise. */
int pico8_pj_ready(void);
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
/* `pico8_pj_i__row`, `pico8_pj_gs_row`: never NULL. */
const pico8_jb_row_t *pico8_pj_i_(void);
const pico8_jb_row_t *pico8_pj_gs(void);

/* TROU-7 */
/* The panel registers itself through `lt` into `hv`, `cv.kj`, `cv.hk`,
 * `cv.hb` and no `cg`; nothing ever frees it. Asserts when called before the
 * init. */
pico8_pj_t *pico8_pj_new(void);

/* REF NULL for a NULL panel. */
pico8_value_t pico8_pj_ed(const pico8_pj_t *self);

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
