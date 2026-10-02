#ifndef PICO8_SCORE_PANEL_H
#define PICO8_SCORE_PANEL_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/map_tables.h"
#include "game/message_box.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* Throughout this module a NULL panel argument is a safe no-op. */

/* `ed` NULL reads as the class's `ed`, the number 1, never nil. */
typedef struct {
    entity_t header;
    const score_panel_archetype_t *def;
    p8_fixed_t fa;
    pico8_mh_t *ed;
    int c_;
    int has_c_;
    p8_fixed_t gz;
    int has_gz;
} pico8_pj_t;

/* `pj.cr`: no parent, never an entity, and no writer. Trap: `ep` is a
 * frame-zone text, dangling after the next reset; only `cr()`, which has
 * just written it, reads it. */
typedef struct {
    entity_t header;
    const vec_t *hi;
    const vec_t *gq;
    p8_fixed_t ca;
    const char *ep;
} pico8_pj_cr_t;

/* The longest text `hb` or `hk` writes: "year " and a year, or a `ne` or
 * `tostr` of a number with its sign, its four decimals and a suffix. */
enum { PICO8_PJ_TEXT_CAP = 16 };

/* `pj.ou`: a copy of `pico8_pj_ou_row` whose sub-rows `hb` rewrites
 * (item 1 of each), each an article of `bl`; a text `hb` writes is a copy
 * in `texts[i]`. */
typedef struct {
    data_row_t row;
    data_item_t items[7];
    data_row_t subs[7];
    data_item_t sub_items[7][4];
    char texts[7][PICO8_PJ_TEXT_CAP + 1];
} pico8_pj_ou_t;

/* `pj.ea`: `row` holds `ea[1]`, `ea[2]` (a copy of its generated row whose
 * item 1 `hk` rewrites, to a copy in `year`) and `ea[3]` when set (a row of
 * `gs`); `lh` NULL is nil. */
typedef struct {
    data_row_t row;
    data_item_t items[3];
    data_row_t second;
    data_item_t second_items[5];
    char year[PICO8_PJ_TEXT_CAP + 1];
    pico8_mh_lh_fn lh;
} pico8_pj_ea_t;

/* A row of `bz`, a copy of its entry of `pico8_bz_table`. Without a score it
 * is an article of `bl` (`row`, never rewritten); with one, `pj:ph`
 * rewrites its positions, `has[i]` saying which hold a value, and makes it
 * a line (`ra` its position). Trap: the texts it writes there are
 * frame-zone texts, dangling after the next reset; only `pj:ph`, which has
 * just written them, reads them. */
typedef struct {
    data_row_t row;
    data_item_t items[5];
    int has[5];
    vec_t ra;
    const score_rows_entry_t *e;
} pico8_bz_row_t;

/* Calls `pico8_pi_init()` first. Resets `bz`, `ou` and `ea` to their
 * generated rows. */
void pico8_score_panel_init(void);

/* Resets `ju` to NULL without freeing the instance it named: `ju` is a
 * bookmark, `hv` owns the entity. */
void pico8_score_panel_shutdown(void);

/* 1 from pico8_score_panel_init() to pico8_score_panel_shutdown(), 0 otherwise. */
int pico8_pj_ready(void);
const entity_class_t *pico8_pj_struct_class(void);

/* Shared by every panel, past and future: `hb` rewrites `ou`, `hk` rewrites
 * `ea`, in place; both are statics of this module. `cr` is the one static
 * `pico8_pj_cr_t`, never freed, whose `ep` `cr()` rewrites and the init
 * resets to nil. All NULL before `pico8_score_panel_init()` and after the
 * shutdown. */
pico8_pj_ou_t *pico8_pj_ou(void);
pico8_pj_ea_t *pico8_pj_ea(void);
pico8_pj_cr_t *pico8_pj_cr(void);
const entity_class_t *pico8_pj_cr_struct_class(void);
/* `pico8_pj_i__row`, `pico8_pj_gs_row`: never NULL. */
const data_row_t *pico8_pj_i_(void);
const data_row_t *pico8_pj_gs(void);

/* The panel registers itself through `lt` into `hv`, `cv.kj`, `cv.hk`,
 * `cv.hb` and no `cg`; it lives in the game-zone generation `eu` opens. Asserts when called before the
 * init. */
pico8_pj_t *pico8_pj_new(void);

/* The PICO8_BZ_COUNT rows of `bz`, statics of this module: every `pj:ph()`
 * run rewrites them in place. NULL before the init and after the shutdown. */
pico8_bz_row_t *pico8_pj_bz(void);

/* Never NULL: an inert placeholder until the real `_init` bridge is
 * installed, which a NULL argument resets. A button built earlier keeps
 * whichever value it captured then. */
button_fn_t pico8_init_field_value(void);
void pico8_score_panel_set_init(button_fn_t fn);

/* NULL until `eu()` builds the one `pj` instance and sets it here. The
 * setter takes no ownership. */
pico8_pj_t *pico8_ju(void);
void pico8_pj_set_ju(pico8_pj_t *instance);

void pico8_pj_hb(void);

void pico8_pj_kj(p8_graphics_t *gfx, p8_memory_t *mem, pico8_pj_t *self);

void pico8_eg(void);

void pico8_pj_hk_dispatch(entity_t *self, p8_fixed_t yr);
void pico8_pj_ph_dispatch(entity_t *self, p8_fixed_t t);

void pico8_score_panel_display_init(void);
void pico8_score_panel_display_shutdown(void);

#endif
