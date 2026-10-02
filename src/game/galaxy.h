#ifndef GAME_GALAXY_H
#define GAME_GALAXY_H

#include "p8/fixed.h"

/* Spawns `planet_count` planets -- each registers itself in the live list
 * and the registries through entity_construct(), so the returned pointers
 * are dropped -- then installs the planets' type picker and vanish roll
 * (planet_set_type_picker, planet_set_vanish_roll). */
void galaxy_populate(p8_fixed_t planet_count);

#endif
