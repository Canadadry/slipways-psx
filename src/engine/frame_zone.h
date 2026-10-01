#ifndef PICO8_ZONE_FRAME_H
#define PICO8_ZONE_FRAME_H

#include "pico8_zone_choisie.h"

/* Zone frame : un bloc qu'elle alloue vit jusqu'a la prochaine remise a
 * zero, faite en tete de pico8__update60(). Jamais de free sur ce bloc.
 * La zone s'initialise au premier appel de l'une ou l'autre fonction. */
const pico8_zone_choisie_t *pico8_zone_frame(void);
void pico8_zone_frame_reset(void);

#endif
