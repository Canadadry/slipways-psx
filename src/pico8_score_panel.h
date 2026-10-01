#ifndef PICO8_SCORE_PANEL_H
#define PICO8_SCORE_PANEL_H

#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_map_tables.h"
#include "pico8_msgbox.h"
#include "pico8_struct.h"
#include "pico8_vec.h"

/* Throughout this module a NULL panel argument is a safe no-op. */

/* TROU-7 */
/* `ed` NULL reads as the class's `ed`, the number 1, never nil. */
typedef struct {
    pico8_struct_header_t header;
    const pico8_pj_def_t *def;
    pico8_fixed_t fa;
    pico8_mh_t *ed;
    int c_;
    int has_c_;
    pico8_fixed_t gz;
    int has_gz;
} pico8_pj_t;

/* `pj.cr`: no parent, never an entity, and no writer. Trap: `ep` is a
 * frame-zone text, dangling after the next reset; only `cr()`, which has
 * just written it, reads it. */
typedef struct {
    pico8_struct_header_t header;
    const pico8_vec_t *hi;
    const pico8_vec_t *gq;
    pico8_fixed_t ca;
    const char *ep;
} pico8_pj_cr_t;

/* `pj.ou`: a copy of `pico8_pj_ou_row` whose sub-rows `hb` rewrites
 * (item 1 of each), each an article of `bl`. */
typedef struct {
    pico8_jb_row_t row;
    pico8_jb_item_t items[7];
    pico8_jb_row_t subs[7];
    pico8_jb_item_t sub_items[7][4];
} pico8_pj_ou_t;

/* `pj.ea`: `row` holds `ea[1]`, `ea[2]` (a copy of its generated row whose
 * item 1 `hk` rewrites) and `ea[3]` when set (a row of `gs`); `lh` NULL is
 * nil. */
typedef struct {
    pico8_jb_row_t row;
    pico8_jb_item_t items[3];
    pico8_jb_row_t second;
    pico8_jb_item_t second_items[5];
    pico8_mh_lh_fn lh;
} pico8_pj_ea_t;

/* A row of `bz`, a copy of its entry of `pico8_bz_table`. Without a score it
 * is an article of `bl` (`row`, never rewritten); with one, `pj:ph`
 * rewrites its positions, `has[i]` saying which hold a value, and makes it
 * a line (`ra` its position). */
typedef struct {
    pico8_jb_row_t row;
    pico8_jb_item_t items[5];
    int has[5];
    pico8_vec_t ra;
    const pico8_bz_entry_t *e;
} pico8_bz_row_t;

/* Calls `pico8_pi_init()` first. Resets `bz`, `ou` and `ea` to their
 * generated rows. */
void pico8_score_panel_init(void);

/* Resets `ju` to NULL without freeing the instance it named: `ju` is a
 * bookmark, `hv` owns the entity. */
void pico8_score_panel_shutdown(void);

/* 1 from pico8_score_panel_init() to pico8_score_panel_shutdown(), 0 otherwise. */
int pico8_pj_ready(void);
const pico8_struct_class_t *pico8_pj_struct_class(void);

/* Shared by every panel, past and future: `hb` rewrites `ou`, `hk` rewrites
 * `ea`, in place; both are statics of this module. `cr` is the one static
 * `pico8_pj_cr_t`, never freed, whose `ep` `cr()` rewrites and the init
 * resets to nil. All NULL before `pico8_score_panel_init()` and after the
 * shutdown. */
pico8_pj_ou_t *pico8_pj_ou(void);
pico8_pj_ea_t *pico8_pj_ea(void);
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

/* The PICO8_BZ_COUNT rows of `bz`, statics of this module: every `pj:ph()`
 * run rewrites them in place. NULL before the init and after the shutdown. */
pico8_bz_row_t *pico8_pj_bz(void);

/* Never NULL: an inert placeholder until the real `_init` bridge is
 * installed, which a NULL argument resets. A button built earlier keeps
 * whichever value it captured then. */
pico8_js_fn pico8_init_field_value(void);
void pico8_score_panel_set_init(pico8_js_fn fn);

/* NULL until `eu()` builds the one `pj` instance and sets it here. The
 * setter takes no ownership. */
pico8_pj_t *pico8_ju(void);
void pico8_pj_set_ju(pico8_pj_t *instance);

void pico8_pj_hb(void);

void pico8_pj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_pj_t *self);

void pico8_eg(void);

void pico8_pj_hk_dispatch(pico8_struct_header_t *self, pico8_fixed_t yr);
void pico8_pj_ph_dispatch(pico8_struct_header_t *self, pico8_fixed_t t);

void pico8_score_panel_display_init(void);
void pico8_score_panel_display_shutdown(void);

#endif
