#ifndef PICO8_TRADE_H
#define PICO8_TRADE_H

#include "pico8_class.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* `trade` is a root class declared with no spec string and no `pq`, not
 * ke-descended, so it never self-registers into hv/cv/cg.
 *
 * This port has no operator overloading, so `trade.ns.__eq` is the plain
 * named pico8_trade_eq below. The generic membership helpers (pico8_gu,
 * pico8_value_equal) therefore compare two trade REFs by pointer identity,
 * not by pico8_trade_eq; membership that must honour trade equality goes
 * through `gu_trade` (src/pico8_planet_trade.c). */
void pico8_trade_init(void);
void pico8_trade_shutdown(void);

/* NULL before pico8_trade_init() and after pico8_trade_shutdown(); not
 * retrievable through pico8_class_lookup(), the class registering under the
 * shared "" name-registry slot. */
pico8_class_t *pico8_trade_class(void);

pico8_instance_t *pico8_trade_new(const pico8_ob_table_t *overrides);

/* Returns 1 and writes *out_value for a non-empty "lp"; returns 0 with
 * *out_value untouched for a NULL self, an absent or non-REF "lp", or an
 * empty one. */
int pico8_trade_to(const pico8_instance_t *self, pico8_value_t *out_value);

/* Returns a brand-new instance, never mutating self; NULL for a NULL self.
 * An absent "lp"/"rl" on self reads as an empty list. */
pico8_instance_t *pico8_trade_kg(const pico8_instance_t *self, pico8_value_t np, pico8_value_t jc);

/* Two trades are equal iff they start (`lp[1]`) and end (`:to()`) at the
 * same planet, compared by pointer identity, whatever lies between. Returns 0
 * for a NULL l/r or an absent, non-REF or empty "lp" on either side. */
int pico8_trade_eq(const pico8_instance_t *l, const pico8_instance_t *r);

#endif
