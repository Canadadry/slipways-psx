#ifndef PICO8_ZONE_TRANSITOIRE_H
#define PICO8_ZONE_TRANSITOIRE_H

#include "pico8_zone_choisie.h"

/* Zone designee adossee a malloc, realloc et free : un bloc qu'elle alloue se
 * libere aussi par free. Chaque reference compte au verrou alloc-guard. */
extern const pico8_zone_choisie_t pico8_zone_transitoire;

#endif
