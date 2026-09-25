#ifndef PICO8_MSGBOX_H
#define PICO8_MSGBOX_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL instance argument is a safe no-op, and an
 * absent or wrong-kind field reads as its zero/false default. */

/* The message box currently open, or NULL for none. The setter never frees
 * the instance it replaces: `kc` is a borrowed bookmark into an entity that
 * `lt` registered into `hv`/`cv`/`cg` and that nothing here owns. */
pico8_instance_t *pico8_kc(void);
void pico8_msgbox_set_kc(pico8_instance_t *instance);

void pico8_mh_init(void);

/* Frees the `mh` class and clears `kc`, never any instance. Idempotent, and
 * safe before the first `pico8_mh_init()`. */
void pico8_mh_shutdown(void);

pico8_class_t *pico8_mh_class(void);

/* `overrides` is copied, not kept; its positional entries become the
 * instance's own `self[1..n]`, the option row `mh:ev` lays out through
 * `bl`. The instance registers itself through `lt`, and nothing frees it. */
pico8_instance_t *pico8_mh_new(const pico8_ob_table_t *overrides);

void pico8_mh_pq(pico8_instance_t *self);

/* A NULL `gfx`/`mem` skips the drawing and the camera reset only; the
 * closing timer and the `kc` bookkeeping still run. */
void pico8_mh_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self);

/* Registers the `pi` constructors first: the class spec holds `v(0,-0.34)`,
 * so `'v'` must resolve when `ob()` parses it, or the field ends up a bare
 * positional table instead of a vector. */
void pico8_iv_init(void);

/* `at1`/`at2` are the cart's `self[1]` (the text, a string never freed)
 * and `self[2]` (the forced color). Never freed, nor are the vectors `ra`
 * points at: TROU-7. `v` points at one vector shared by every iv. */
typedef struct {
    pico8_ke_t ke;
    pico8_value_t fa;
    pico8_value_t l;
    pico8_value_t v;
    pico8_value_t ra;
    pico8_value_t p_;
    pico8_value_t at1;
    pico8_value_t at2;
} pico8_iv_t;

void pico8_iv_shutdown(void);
pico8_class_t *pico8_iv_class(void);

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
 * list itself stays the caller's, its tables become the instances'. `p` may
 * be a classless field-bag -- a stack `pico8_instance_t` with `cls` NULL --
 * which is how `pz(ha,ha)` passes a plain table. */
void pico8_pz(const pico8_instance_t *p, const pico8_list_t *bs);

#endif
