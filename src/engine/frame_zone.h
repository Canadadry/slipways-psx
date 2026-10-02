#ifndef ENGINE_FRAME_ZONE_H
#define ENGINE_FRAME_ZONE_H

#include "engine/allocator.h"

/* Zone frame : un bloc qu'elle alloue vit jusqu'a la prochaine remise a
 * zero, faite en tete de pico8__update60(). Jamais de free sur ce bloc.
 * La zone s'initialise au premier appel de l'une ou l'autre fonction. */
const allocator_t *frame_zone(void);
void frame_zone_reset(void);

#endif
