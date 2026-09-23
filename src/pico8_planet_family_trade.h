#ifndef PICO8_PLANET_FAMILY_TRADE_H
#define PICO8_PLANET_FAMILY_TRADE_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_planet_trade.h"
#include "pico8_value.h"

/* TROU-8 */
/* Calls `pico8_pi_init()` first: `nr`'s spec contains `b(-4,-4,5,5)`, so
 * the 'b' constructor must already be registered when it is parsed. */
void pico8_planet_family_trade_init(void);

/* Frees the three classes, never any instance. Idempotent, and safe before
 * the first `pico8_planet_family_trade_init()`. */
void pico8_planet_family_trade_shutdown(void);

/* Same accessor and `_new` contract as the six classes of
 * `pico8_planet_family.h`. */
pico8_class_t *pico8_nr_class(void);
pico8_instance_t *pico8_nr_new(const pico8_ob_table_t *overrides);

pico8_class_t *pico8_fr_class(void);
pico8_instance_t *pico8_fr_new(const pico8_ob_table_t *overrides);

pico8_class_t *pico8_mx_class(void);
pico8_instance_t *pico8_mx_new(const pico8_ob_table_t *overrides);

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

void pico8_mx_kf(pico8_instance_t *self, pico8_instance_t *sw);

#endif
