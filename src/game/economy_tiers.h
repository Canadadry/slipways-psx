#ifndef GAME_ECONOMY_TIERS_H
#define GAME_ECONOMY_TIERS_H

#include "engine/entity_class.h"
#include "p8/fixed.h"

void economy_tiers_rebuild_incoming_routes(void);

int economy_tiers_update_tiers(void);

p8_fixed_t economy_tiers_compute_tier(const entity_t *entity);

#endif
