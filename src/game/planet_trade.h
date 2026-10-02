#ifndef PICO8_PLANET_TRADE_H
#define PICO8_PLANET_TRADE_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "game/planet.h"
#include "game/trade.h"

/* Throughout this module a NULL instance or NULL proposal argument is a
 * safe no-op (Lua nil for the value-returning function), and an absent or
 * wrong-kind field reads as absent rather than crashing. */

typedef struct pico8_jo_trade_proposal pico8_jo_trade_proposal_t;

/* `self` is the proposal itself (Lua's `lw:kf(a)` colon call), never the
 * planet that built it. */
typedef void (*pico8_jo_trade_proposal_kf_fn)(pico8_jo_trade_proposal_t *self, pico8_trade_t *a);

/* `kf` is this table's own closure field, unrelated to the `jo:kf` method
 * of the same name. `ib` is the one list every proposal built by a single
 * `pico8_jo_ib()` call shares and appends to. */
struct pico8_jo_trade_proposal {
    pico8_trade_t *t;
    p8_fixed_t mu;
    pico8_jo_trade_proposal_kf_fn kf;
    pico8_ents_t *ib;
};

void pico8_jo_kf(pico8_jo_t *self, entity_t *sw);

void pico8_jo_cn(pico8_jo_t *self, pico8_jo_trade_proposal_t *lw);

/* Returns `self.by` itself when it is set, else a container made in the
 * current game generation, never given back before it; never writes
 * `self.by` back -- its caller does. */
pico8_ents_t *pico8_jo_ib(pico8_jo_t *self);

#endif
