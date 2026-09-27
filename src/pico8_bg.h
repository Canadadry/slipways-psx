#ifndef PICO8_BG_H
#define PICO8_BG_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_ke.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* Throughout this module a NULL `self`, `gfx` or `mem` argument is a safe
 * no-op, and an absent or wrong-kind field reads as absent rather than
 * crashing. `hd=1` is flat data, not a method: `bg` always answers the hover
 * test, since nothing lies behind the background to miss. */

/* `pl` points at the table the module owns, shared by every `bg` and never
 * modified in place. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_ke_t ke;
    pico8_value_t fa;
    pico8_value_t hd;
    pico8_value_t pl;
    pico8_value_t mq;
    pico8_value_t m_;
} pico8_bg_t;

void pico8_bg_init(void);

/* Frees the `bg` spec and the module's `pl` table, never any `bg`.
 * Idempotent, and safe before the first `pico8_bg_init()`. */
void pico8_bg_shutdown(void);

/* 1 from pico8_bg_init() to pico8_bg_shutdown(), 0 otherwise. */
int pico8_bg_ready(void);

/* `bg`'s spec default `name`, then ke's; 0 when absent, or when not ready. A
 * REF is borrowed from the spec, valid until pico8_bg_shutdown(). */
int pico8_bg_default(const char *name, pico8_value_t *out_value);
const pico8_struct_class_t *pico8_bg_struct_class(void);

/* `overrides` is read, not kept, and must be NULL or empty. The struct
 * registers itself into `hv`/`cv` through `lt`, and nothing frees it. */
pico8_bg_t *pico8_bg_new(const pico8_ob_table_t *overrides);

/* `pl` is the module's table, as `bg.pl` rather than `self.pl` in the
 * cart. An index outside `pl`'s 1..15 range -- which extreme `vh` values
 * can produce, unchecked in the cart too -- paints tile 0. */
void pico8_bg_pq(pico8_mem_t *mem, pico8_bg_t *self);

/* The second `rectfill` of the dashed grid omits its color in the cart and
 * inherits the previous call's; that previous call is the statement right
 * above it, color 1, so `1` is passed explicitly here. */
void pico8_bg_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_bg_t *self);

/* Returns the freshly built `mz` or `g_`, which `ui:bh` then holds as
 * `lheld`, or Lua nil when neither class is registered. Nothing frees it. */
pico8_value_t pico8_bg_ldown(pico8_bg_t *self, pico8_value_t mp);

/* Stores `rp` as-is and returns `self`, which becomes `ui.rheld` for as long
 * as the button stays down. */
pico8_value_t pico8_bg_rdown(pico8_bg_t *self, pico8_value_t mp, pico8_value_t rp);

void pico8_bg_hq(pico8_bg_t *self, pico8_value_t mp, pico8_value_t rp);

#endif
