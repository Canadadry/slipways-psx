#ifndef GAME_PLANET_FAMILY_TRADE_H
#define GAME_PLANET_FAMILY_TRADE_H

#include "game/planet_trade.h"
#include "engine/entity_class.h"

/* GAP-8 */
/* Calls draw_functions_init(). */
void planet_family_trade_init(void);

/* Frees nothing, no instance. Idempotent, and safe before the first
 * planet_family_trade_init(). */
void planet_family_trade_shutdown(void);

/* As in planet_family.h: each `_ready` is 1 from
 * planet_family_trade_init() to its shutdown. */

/* A slipgate: a laboratory and nothing of its own; its defaults are its
 * archetype's. */
int slipgate_ready(void);
const entity_kind_t *slipgate_kind(void);
const entity_class_t *slipgate_class(void);

/* `keys` (NULL means none) is read, not kept. The new slipgate registers
 * itself (entity_register), and lives in the construction zone. Requires
 * planet_family_trade_init(). */
planet_t *slipgate_new(const planet_keys_t *keys);

/* An infraspace gate: a slipgate and nothing of its own; its defaults are
 * its archetype's. */
int infraspace_gate_ready(void);
const entity_kind_t *infraspace_gate_kind(void);
const entity_class_t *infraspace_gate_class(void);

/* `keys` (NULL means none) is read, not kept: `planet_id` only, `map_pos`
 * and `pos` NULL. The new infraspace gate registers itself
 * (entity_register), and lives in the construction zone. Requires
 * planet_family_trade_init(). */
planet_t *infraspace_gate_new(const planet_keys_t *keys);

/* A synthesizer: an ascension whose synthesizable resources are copied
 * from its archetype; synthesizer_attach_slipway points them at an empty
 * sequence once it accepts a resource. */
int synthesizer_ready(void);
const entity_kind_t *synthesizer_kind(void);
const entity_class_t *synthesizer_class(void);

/* `keys` (NULL means none) is read, not kept: `planet_id` only, `map_pos`
 * and `pos` NULL. The new synthesizer registers itself (entity_register),
 * and lives in the construction zone. Requires planet_family_trade_init(). */
planet_t *synthesizer_new(const planet_keys_t *keys);

/* Never stores the dropped entity, which may be NULL: slipgate_left_press
 * passes a static struct with no member but its header. */
down_t slipgate_accept_drop(const planet_t *self, const entity_t *s);

/* Returns a freshly built slipway, or slipgate_accept_drop's own refusal.
 * The slipway lives in its grab arena, which it gives back if refused. */
down_t slipgate_left_press(planet_t *self);

void slipgate_refresh(planet_t *self);

/* Writes the amount left after forwarding back onto the offer, so the
 * caller sees what every hop downstream consumed. */
void slipgate_offer_trade(planet_t *self, trade_offer_t *offer);

void synthesizer_attach_slipway(planet_t *self, entity_t *slipway);

#endif
