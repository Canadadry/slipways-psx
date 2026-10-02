#ifndef GAME_GALAXY_H
#define GAME_GALAXY_H

#include "p8/fixed.h"

/* Spawns `n` planets -- each one registers itself into hv/cv/cg through
 * jo's inherited constructor, so the returned pointers are dropped -- then
 * installs the class-level jo.oc/jo.ir generators. */
void galaxy_populate(p8_fixed_t planet_count);

#endif
