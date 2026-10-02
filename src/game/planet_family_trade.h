#ifndef GAME_PLANET_FAMILY_TRADE_H
#define GAME_PLANET_FAMILY_TRADE_H

#include "game/planet_trade.h"
#include "engine/entity_class.h"

/* TROU-8 */
/* Calls `pico8_pi_init()` first. */
void planet_family_trade_init(void);

/* Frees nothing, no instance. Idempotent, and safe before the first
 * `pico8_planet_family_trade_init()`. */
void planet_family_trade_shutdown(void);

/* As in pico8_planet_family.h: `_ready` from
 * `pico8_planet_family_trade_init()` to its shutdown. */

/* A slipgate: a laboratory and nothing of its own; its defaults are its
 * archetype's. */
int slipgate_ready(void);
const entity_kind_t *slipgate_kind(void);
const entity_class_t *slipgate_class(void);

/* `keys` (NULL means none) is read, not kept. The new slipgate registers
 * itself into `hv`/`cv`/`cg` through `lt`, and it lives in the game generation.
 * Requires `pico8_planet_family_trade_init()`. */
planet_t *slipgate_new(const planet_keys_t *keys);

/* An infraspace gate: a slipgate and nothing of its own; its defaults are
 * its archetype's. */
int infraspace_gate_ready(void);
const entity_kind_t *infraspace_gate_kind(void);
const entity_class_t *infraspace_gate_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new infraspace gate registers itself into `hv`/`cv`/`cg` through
 * `lt`, and it lives in the game generation. Requires `pico8_planet_family_trade_init()`. */
planet_t *infraspace_gate_new(const planet_keys_t *keys);

/* A synthesizer: an ascension whose `ht` is copied from its archetype;
 * `mx:kf` points it at an empty sequence once it accepts a resource. */
int synthesizer_ready(void);
const entity_kind_t *synthesizer_kind(void);
const entity_class_t *synthesizer_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new synthesizer registers itself into `hv`/`cv`/`cg` through
 * `lt`, and it lives in the game generation. Requires `pico8_planet_family_trade_init()`. */
planet_t *synthesizer_new(const planet_keys_t *keys);

/* Never stores `s`, which may be NULL: `nr:ldown` passes a static struct
 * with no member but its header. */
down_t slipgate_accept_drop(const planet_t *self, const entity_t *s);

/* Returns a freshly built `jc` instance, or `gv`'s own false. The `jc`
 * lives in its grab arena, which it gives back if refused. */
down_t slipgate_left_press(planet_t *self);

void slipgate_refresh(planet_t *self);

/* Writes the quantity left after forwarding back onto `*lw`, so the caller
 * sees what every hop downstream consumed. */
void slipgate_offer_trade(planet_t *self, trade_offer_t *lw);

void synthesizer_attach_slipway(planet_t *self, entity_t *slipway);

#endif
