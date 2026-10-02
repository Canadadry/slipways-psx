#ifndef GAME_PLANET_TRADE_H
#define GAME_PLANET_TRADE_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "game/planet.h"
#include "game/trade.h"

/* Throughout this module a NULL instance or NULL proposal argument is a
 * safe no-op (Lua nil for the value-returning function), and an absent or
 * wrong-kind field reads as absent rather than crashing. */

typedef struct trade_offer trade_offer_t;

/* `self` is the proposal itself (Lua's `lw:kf(a)` colon call), never the
 * planet that built it. */
typedef void (*trade_offer_accept_fn_t)(trade_offer_t *self, trade_route_t *a);

/* `kf` is this table's own closure field, unrelated to the `jo:kf` method
 * of the same name. `ib` is the one list every proposal built by a single
 * `pico8_jo_ib()` call shares and appends to. */
struct trade_offer {
    trade_route_t *route;
    p8_fixed_t amount;
    trade_offer_accept_fn_t accept;
    entity_list_t *accepted_routes;
};

void planet_attach_slipway(planet_t *self, entity_t *sw);

void planet_offer_trade(planet_t *self, trade_offer_t *lw);

/* Returns `self.by` itself when it is set, else a container made in the
 * current game generation, never given back before it; never writes
 * `self.by` back -- its caller does. */
entity_list_t *planet_rebuild_trades(planet_t *self);

#endif
