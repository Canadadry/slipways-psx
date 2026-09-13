#ifndef SLIPWAYS_WORLD_H
#define SLIPWAYS_WORLD_H

#include "planetgen.h"

void world_init(void);
const galaxy_t *world_get_galaxy(void);

/* Index into world_get_galaxy()'s positions[] designated, by world_init(),
 * as the wormhole marker's screen position. This is a placeholder for the
 * sole purpose of exercising the POLY_F4 GPU-primitive path (see
 * docs/prd/triage/psx-galaxy-rendering.md, "The wormhole placeholder --
 * deliberately not the real mechanic") -- it is not a placement rule, and
 * index 0 has no significance beyond being a real generated planet position
 * that lands inside the galaxy's visual footprint. The real wormhole is
 * placed by the player's first click, Phase 4 step 15 -- this is not that. */
#define WORLD_WORMHOLE_INDEX 0

int world_get_wormhole_index(void);

#endif
