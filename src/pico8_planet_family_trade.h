#ifndef PICO8_PLANET_FAMILY_TRADE_H
#define PICO8_PLANET_FAMILY_TRADE_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_planet_family.h"
#include "pico8_planet_trade.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* TROU-8 */
/* Calls `pico8_pi_init()` first: `nr`'s spec contains `b(-4,-4,5,5)`, so
 * the 'b' constructor must already be registered when it is parsed. */
void pico8_planet_family_trade_init(void);

/* Frees the three classes, never any instance. Idempotent, and safe before
 * the first `pico8_planet_family_trade_init()`. */
void pico8_planet_family_trade_shutdown(void);

/* A synthesizer: an ascension, then mx's own default `ht`, which no
 * ancestor has a member for. A default of mx whose name is a member of
 * `hj`, `qh` or `jo` (`nu`, `rr`, `hp`, `s`, `px`, `od` and the 11 emitter
 * defaults) lives in that member. `nu`, `px`, `ht` and `k_` hold the very
 * tables of mx's class, `ba` the one of qh's, borrowed; `mx:kf` swaps `ht`
 * for a fresh table it never frees. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_hj_t hj;
    pico8_value_t ht;
} pico8_mx_t;

/* Requires `pico8_planet_family_trade_init()`. `overrides` is copied, not
 * kept; the new instance registers itself into `hv`/`cv`/`cg` through `lt`,
 * and nothing ever frees it. */
pico8_class_t *pico8_nr_class(void);
pico8_instance_t *pico8_nr_new(const pico8_ob_table_t *overrides);

pico8_class_t *pico8_fr_class(void);
pico8_instance_t *pico8_fr_new(const pico8_ob_table_t *overrides);

pico8_class_t *pico8_mx_class(void);
const pico8_struct_class_t *pico8_mx_struct_class(void);

/* `overrides` (NULL means none) is read, not kept: `id` only. The new
 * synthesizer registers itself into `hv`/`cv`/`cg` through `lt`, and
 * nothing ever frees it. Requires `pico8_planet_family_trade_init()`. */
pico8_mx_t *pico8_mx_new(const pico8_ob_table_t *overrides);

/* Never stores `s`, which may be NULL: `nr:ldown` passes a stack instance
 * it frees as soon as the call returns. */
pico8_value_t pico8_nr_gv(const pico8_instance_t *self, const pico8_instance_t *s);

/* TROU-6 */
/* Returns a freshly built `jc` instance, or `gv`'s own false; nothing ever
 * frees the instance. */
pico8_value_t pico8_nr_ldown(pico8_instance_t *self);

void pico8_nr_hb(pico8_instance_t *self);

/* Writes the quantity left after forwarding back onto `*lw`, so the caller
 * sees what every hop downstream consumed. */
void pico8_nr_cn(pico8_instance_t *self, pico8_jo_trade_proposal_t *lw);

void pico8_mx_kf(pico8_mx_t *self, pico8_instance_t *sw);

#endif
