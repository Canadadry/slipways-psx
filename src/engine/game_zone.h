#ifndef ENGINE_GAME_ZONE_H
#define ENGINE_GAME_ZONE_H

#include <stddef.h>

#include "engine/allocator.h"

/* Zone partie : une generation par partie, de `eu` a `eu`. Un bloc qu'elle
 * alloue vit jusqu'au rendu de sa generation, fait par le vidage de la file
 * qui suit la bascule. Jamais de free sur ce bloc. La zone s'initialise au
 * premier appel de l'une des fonctions. */
const allocator_t *game_zone(void);
void game_zone_flip(void);
int game_zone_pending(void);
/* Sans generation en attente, ne fait rien. */
void game_zone_release(void);
/* Remet aussi la sous-zone chargement a zero. */
void game_zone_reset(void);
/* Octets pris par les deux generations, et leur maximum sur la session. */
size_t game_zone_used(void);
size_t game_zone_max(void);

/* Sous-zone chargement : un bloc qu'elle alloue vit jusqu'au prochain
 * pico8_zone_partie_reset(), donc au chargement ou a l'arret suivant ; ni la
 * bascule ni le rendu ne la touchent. Jamais de free sur ce bloc. */
const allocator_t *game_zone_load_zone(void);
/* Octets pris, et leur maximum sur la session, que le reset garde. */
size_t load_zone_used(void);
size_t game_zone_load_zone_max(void);

#endif
