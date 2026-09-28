#ifndef PICO8_MSGBOX_H
#define PICO8_MSGBOX_H

#include "pico8_struct.h"
#include "pico8_game_data.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL instance argument is a safe no-op, and an
 * absent or wrong-kind field reads as its zero/false default. */

/* `items` is the cart's `self[1..n]`, the option row `mh:ev` lays out
 * through `bl`, in the order the construction table stored them: a table
 * among them stays shared with the caller. Nothing frees the struct or
 * its list: TROU-7. `nu` points at `pico8_mh_nu_tags`. */
typedef struct {
    pico8_ke_t ke;
    const pico8_str_seq_t *nu;
    pico8_fixed_t fa;
    pico8_fixed_t rh;
    pico8_value_function_t lh; /* NULL: `lh` is nil */
    int p_;
    int has_p_; /* 0: `p_` is nil, and `p_` is then never read */
    pico8_list_t items;
} pico8_mh_t;

/* The message box currently open, or NULL for none. The setter never frees
 * the box it replaces: `kc` borrows a struct that `lt` registered into
 * `hv`/`cv`/`cg` and that nothing here owns. */
pico8_mh_t *pico8_kc(void);
void pico8_msgbox_set_kc(pico8_mh_t *mh);

void pico8_mh_init(void);

/* Frees the `mh` spec and clears `kc`, never any
 * box. Idempotent, and safe before the first `pico8_mh_init()`. */
void pico8_mh_shutdown(void);

/* 1 from pico8_mh_init() to pico8_mh_shutdown(), 0 otherwise. */
int pico8_mh_ready(void);

/* `mh`'s spec default `name`, then ke's; 0 when absent, or when not ready. A
 * REF is borrowed from the spec, valid until pico8_mh_shutdown(). */
int pico8_mh_default(const char *name, pico8_value_t *out_value);

/* `overrides` is read, not kept: its positional entries, `rh` and `lh`
 * only. The struct registers itself through `lt`, and nothing frees it. */
pico8_mh_t *pico8_mh_new(const pico8_ob_table_t *overrides);

void pico8_mh_pq(pico8_mh_t *self);

/* A NULL `gfx`/`mem` skips the drawing and the camera reset only; the
 * closing timer and the `kc` bookkeeping still run. */
void pico8_mh_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mh_t *self);

/* Registers the `pi` constructors first: the class spec holds `v(0,-0.34)`,
 * so `'v'` must resolve when `ob()` parses it, or the field ends up a bare
 * positional table instead of a vector. */
void pico8_iv_init(void);

/* `at1`/`at2` are the cart's `self[1]` (the text, a string never freed)
 * and `self[2]` (the forced color). `at1` NULL is nil; a missing `[2]`
 * leaves `at2` at 0, the color `kj` gave nil. Never freed, nor are the
 * vectors `ra` points at: TROU-7. `v` points at one vector shared by every
 * iv. */
typedef struct {
    pico8_ke_t ke;
    pico8_fixed_t fa;
    pico8_fixed_t l;
    pico8_vec_t *v;
    pico8_vec_t *ra;
    int p_;
    int has_p_; /* 0: `p_` is nil, and `p_` is then never read */
    const char *at1;
    pico8_fixed_t at2;
} pico8_iv_t;

void pico8_iv_shutdown(void);

/* 1 from pico8_iv_init() to pico8_iv_shutdown(), 0 otherwise. */
int pico8_iv_ready(void);

/* `iv`'s spec default `name`, then ke's; 0 when absent, or when not ready. A
 * REF is borrowed from the spec, valid until pico8_iv_shutdown(). */
int pico8_iv_default(const char *name, pico8_value_t *out_value);

/* `overrides` is read, not kept: `ra`, `[1]` and `[2]` only. The struct
 * registers itself through `lt`, and nothing frees it. */
pico8_iv_t *pico8_iv_new(const pico8_ob_table_t *overrides);

void pico8_iv_pq(pico8_iv_t *self);
void pico8_iv_ph(pico8_iv_t *self);

/* A NULL `gfx`/`mem` makes the whole call a no-op. */
void pico8_iv_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_iv_t *self, pico8_vec_t p);

/* TROU-7 */
/* Every item of `bs` must be a REF to a `pico8_ob_table_t`: each one is
 * mutated in place, then turned into a `js` instance stored in `ok.le`. The
 * list itself stays the caller's, its tables become the instances'. `p` is
 * NULL (a no-op) or a struct whose descriptor has a field reader. */
void pico8_pz(const pico8_struct_header_t *p, const pico8_list_t *bs);

#endif
