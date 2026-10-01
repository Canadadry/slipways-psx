#ifndef PICO8_MSGBOX_H
#define PICO8_MSGBOX_H

#include "pico8_struct.h"
#include "pico8_game_data.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ui_button.h"
#include "pico8_vec.h"

/* Throughout this module a NULL instance argument is a safe no-op, and an
 * absent or wrong-kind field reads as its zero/false default. */

typedef struct pico8_mh pico8_mh_t;

/* `mh.lh`: the Lua truth of what it returns, which `mh:ev` tests. */
typedef int (*pico8_mh_lh_fn)(pico8_mh_t *self);

/* The cart builds a box twice: `pj:hk` (up to 3 articles) and `pj:ph`
 * (one per row of `bz`). */
enum { PICO8_MH_ITEMS_CAP = PICO8_BZ_COUNT };

/* `items` is the cart's `self[1..n]`, the option row `mh:ev` lays out
 * through `bl`, in the order the construction table stored them: an
 * article stays shared with the caller. Nothing frees the struct: TROU-7. */
struct pico8_mh {
    pico8_struct_header_t header;
    const pico8_mh_def_t *def;
    pico8_fixed_t rh;
    pico8_mh_lh_fn lh; /* NULL: `lh` is nil */
    int p_;
    const pico8_jb_row_t *items[PICO8_MH_ITEMS_CAP];
    int items_count;
};

/* The message box currently open, or NULL for none. The setter never frees
 * the box it replaces: `kc` borrows a struct that `lt` registered into
 * `hv`/`cv`/`cg` and that nothing here owns. */
pico8_mh_t *pico8_kc(void);
void pico8_msgbox_set_kc(pico8_mh_t *mh);

void pico8_mh_init(void);

/* Clears `kc`, never frees any box. Idempotent, and safe before the first
 * `pico8_mh_init()`. */
void pico8_mh_shutdown(void);

/* 1 from pico8_mh_init() to pico8_mh_shutdown(), 0 otherwise. */
int pico8_mh_ready(void);

/* The cart's keys of a message box: `items` its positionals in order, each
 * an article of `bl` (borrowed, never copied), `rh` under its flag, `lh`
 * NULL for nil. */
typedef struct {
    const pico8_jb_row_t *const *items;
    int count;
    int has_rh; /* 0: `rh` keeps the archetype's */
    pico8_fixed_t rh;
    pico8_mh_lh_fn lh;
} pico8_mh_keys_t;

/* `keys` is read, not kept (NULL: none). The struct registers itself
 * through `lt`, and nothing frees it. */
pico8_mh_t *pico8_mh_new(const pico8_mh_keys_t *keys);

void pico8_mh_pq(pico8_mh_t *self);

/* A NULL `gfx`/`mem` skips the drawing and the camera reset only; the
 * closing timer and the `kc` bookkeeping still run. */
void pico8_mh_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mh_t *self);

/* Calls pico8_pi_init(). */
void pico8_iv_init(void);

enum { PICO8_IV_TEXT_CAP = 16 };

/* `at1`/`at2` are the cart's `self[1]` (the text) and `self[2]` (the forced
 * color). `at1` NULL is nil, else it points at `at1_buf`, a copy of the
 * key's text; a missing `[2]` leaves `at2` at 0, the color `kj` gave nil.
 * `ra` deliberately stays the key's vector, borrowed, until the first `ph`:
 * `jo:hk` passes its planet's `ra`, which `na` moves in place before that
 * `ph`, and the cart reads it moved. From then on, and when `pq` places the
 * toast itself, `ra` points at `ra_box`. The struct lives in `place`, given
 * back by the exit queue once `he` takes the toast out. */
typedef struct {
    pico8_struct_header_t header;
    const pico8_zone_choisie_t *place;
    pico8_vec_t ra_box;
    char at1_buf[PICO8_IV_TEXT_CAP + 1];
    const pico8_iv_def_t *def;
    pico8_fixed_t l;
    const pico8_vec_t *ra;
    int p_;
    const char *at1;
    pico8_fixed_t at2;
} pico8_iv_t;

void pico8_iv_shutdown(void);

/* 1 from pico8_iv_init() to pico8_iv_shutdown(), 0 otherwise. */
int pico8_iv_ready(void);

/* The cart's keys `ra`, `[1]` and `[2]` of a toast. NULL or 0 is an absent
 * key, which leaves the member's default. */
typedef struct {
    const pico8_vec_t *ra;
    const char *at1;
    pico8_fixed_t at2;
} pico8_iv_keys_t;

/* `keys` is read, not kept (NULL: none), but for `ra`, borrowed until the
 * first `ph`; `at1` is copied. The struct registers itself through `lt`. */
pico8_iv_t *pico8_iv_new(const pico8_iv_keys_t *keys);

void pico8_iv_pq(pico8_iv_t *self);
void pico8_iv_ph(pico8_iv_t *self);

/* A NULL `gfx`/`mem` makes the whole call a no-op. */
void pico8_iv_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_iv_t *self, pico8_vec_t p);

/* What `pz` reads of its `p`, filled by the caller from its own members: a
 * NULL pointer or a 0 has_* is nil; `ff` is the Lua truth of `p.ff`. `pz`
 * reads it during the call and keeps none of it. */
typedef struct {
    const char *ep;
    int has_eo;
    pico8_fixed_t eo;
    const pico8_vec_t *hi;
    int ff;
    int has_ca;
    pico8_fixed_t ca;
    const pico8_vec_t *gq;
    const pico8_vec_t *ra;
} pico8_pz_frame_t;

/* TROU-7 */
/* The `n` rows of `bs` are the keys of `ok.le`'s buttons, each read on a
 * copy that gets `ra`, `qf` and `fa`; `bs` stays the caller's, unchanged.
 * A NULL `frame` is a no-op. */
void pico8_pz(const pico8_pz_frame_t *frame, const pico8_js_keys_t *bs, int n);

#endif
