#ifndef GAME_PLANET_TRADE_H
#define GAME_PLANET_TRADE_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "game/planet.h"
#include "game/trade.h"

/* Throughout this module a NULL instance or NULL offer argument is a safe
 * no-op (NULL for the value-returning function), and an absent or
 * wrong-kind field reads as absent rather than crashing. */

typedef struct trade_offer trade_offer_t;

/* `self` is the offer itself, never the planet that built it. */
typedef void (*trade_offer_accept_fn_t)(trade_offer_t *self, trade_route_t *a);

/* `accepted_routes` is the one list every offer built by a single
 * planet_rebuild_trades() call shares and appends to. */
struct trade_offer {
    trade_route_t *route;
    p8_fixed_t amount;
    trade_offer_accept_fn_t accept;
    entity_list_t *accepted_routes;
};

void planet_attach_slipway(planet_t *self, entity_t *sw);

void planet_offer_trade(planet_t *self, trade_offer_t *lw);

/* Returns the planet's `outgoing_routes` itself when it is set, else a list
 * made in the game zone, never given back before it; never writes
 * `outgoing_routes` back -- its caller does. */
entity_list_t *planet_rebuild_trades(planet_t *self);

#endif
