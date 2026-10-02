#ifndef GAME_ECONOMY_TIERS_H
#define GAME_ECONOMY_TIERS_H

#include "engine/entity_class.h"
#include "p8/fixed.h"

/* The planet tag must hold planets only (asserted); an absent list or
 * field is skipped, never fatal. */

/* Each planet's `incoming_routes` is a list this function owns: made once
 * in the game zone, then emptied in place by every later call.
 * `outgoing_routes` is not copied, it is whatever planet_rebuild_trades()
 * returned. */
void economy_tiers_rebuild_incoming_routes(void);

/* Returns 1 when at least one planet's tier was first set or ROSE this
 * pass, which is what ends the rebuild/update loop of economy_recompute(). */
int economy_tiers_update_tiers(void);

/* Writes the match count back onto each need's `satisfied` field: the side
 * effect is what this function's own later branches, and the planet
 * display, read. A NULL entity has tier 0. */
p8_fixed_t economy_tiers_compute_tier(const entity_t *entity);

#endif
