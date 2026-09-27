#ifndef PICO8_TRADE_H
#define PICO8_TRADE_H

#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* Never freed: TROU-7. `lp`/`rl` are the REFs the overrides held, borrowed,
 * not copied. */
typedef struct {
    pico8_struct_header_t header;
    pico8_value_t lp;
    pico8_value_t rl;
    pico8_value_t qe;
} pico8_trade_t;

/* `trade` is a root class with an empty spec and no `pq`, not
 * ke-descended, so it never self-registers into hv/cv/cg.
 *
 * This port has no operator overloading, so `trade.ns.__eq` is the plain
 * named pico8_trade_eq below. The generic membership helpers (pico8_gu,
 * pico8_value_equal) therefore compare two trade REFs by pointer identity,
 * not by pico8_trade_eq; membership that must honour trade equality goes
 * through `gu_trade` (src/pico8_planet_trade.c). */
void pico8_trade_init(void);
void pico8_trade_shutdown(void);

/* 1 from pico8_trade_init() to pico8_trade_shutdown(), 0 otherwise. */
int pico8_trade_ready(void);

/* `trade`'s spec default `name`; 0 when absent, or when not ready. A REF is
 * borrowed from the spec, valid until pico8_trade_shutdown(). */
int pico8_trade_default(const char *name, pico8_value_t *out_value);
const pico8_struct_class_t *pico8_trade_struct_class(void);

/* Takes only the keys "lp", "rl", "qe", each optional; a missing one is
 * nil. */
pico8_trade_t *pico8_trade_new(const pico8_ob_table_t *overrides);

/* Returns 1 and writes *out_value for a non-empty "lp"; returns 0 with
 * *out_value untouched for a NULL self, or a nil or empty "lp". */
int pico8_trade_to(const pico8_trade_t *self, pico8_value_t *out_value);

/* Returns a brand-new route, never mutating self; NULL for a NULL self.
 * A nil "lp"/"rl" on self reads as an empty list, a nil "qe" as false
 * (TROU-24). */
pico8_trade_t *pico8_trade_kg(const pico8_trade_t *self, pico8_value_t np, pico8_value_t jc);

/* Two trades are equal iff they start (`lp[1]`) and end (`:to()`) at the
 * same planet, compared by pointer identity, whatever lies between. Returns 0
 * for a NULL l/r or a nil or empty "lp" on either side. */
int pico8_trade_eq(const pico8_trade_t *l, const pico8_trade_t *r);

#endif
