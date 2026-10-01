#ifndef ZONE_TAS_H
#define ZONE_TAS_H

#include "../src/pico8_zone_choisie.h"

/* Zone des TU adossee a malloc, realloc et free : un bloc qu'elle alloue se
 * libere aussi par free. */
extern const pico8_zone_choisie_t zone_tas;

#endif
