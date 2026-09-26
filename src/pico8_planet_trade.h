#ifndef PICO8_PLANET_TRADE_H
#define PICO8_PLANET_TRADE_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_list.h"
#include "pico8_planet.h"
#include "pico8_value.h"

/* Throughout this module a NULL instance or NULL proposal argument is a
 * safe no-op (Lua nil for the value-returning function), and an absent or
 * wrong-kind field reads as absent rather than crashing. */

typedef struct pico8_jo_trade_proposal pico8_jo_trade_proposal_t;

/* `self` is the proposal itself (Lua's `lw:kf(a)` colon call), never the
 * planet that built it. */
typedef void (*pico8_jo_trade_proposal_kf_fn)(pico8_jo_trade_proposal_t *self, pico8_value_t a);

/* `kf` is this table's own closure field, unrelated to the `jo:kf` method
 * of the same name. `ib` is the one list every proposal built by a single
 * `pico8_jo_ib()` call shares and appends to. */
struct pico8_jo_trade_proposal {
    pico8_instance_t *t;
    pico8_fixed_t mu;
    pico8_jo_trade_proposal_kf_fn kf;
    pico8_list_t *ib;
};

void pico8_jo_kf(pico8_jo_t *self, pico8_instance_t *sw);

/* TROU-7 */
void pico8_jo_cn(pico8_jo_t *self, pico8_jo_trade_proposal_t *lw);

/* TROU-7 */
/* Returns `self.by` itself when that field holds a list, else a fresh heap
 * list; frees neither, and never writes `self.by` back -- its caller does. */
pico8_list_t *pico8_jo_ib(pico8_jo_t *self);

#endif
