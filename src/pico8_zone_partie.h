#ifndef PICO8_ZONE_PARTIE_H
#define PICO8_ZONE_PARTIE_H

#include <stddef.h>

#include "pico8_zone_choisie.h"

/* Zone partie : une generation par partie, de `eu` a `eu`. Un bloc qu'elle
 * alloue vit jusqu'au rendu de sa generation, fait par le vidage de la file
 * qui suit la bascule. Jamais de free sur ce bloc. La zone s'initialise au
 * premier appel de l'une des fonctions. */
const pico8_zone_choisie_t *pico8_zone_partie(void);
void pico8_zone_partie_basculer(void);
int pico8_zone_partie_en_attente(void);
/* Sans generation en attente, ne fait rien. */
void pico8_zone_partie_rendre(void);
/* Remet aussi la sous-zone chargement a zero. */
void pico8_zone_partie_reset(void);
/* Octets pris par les deux generations, et leur maximum sur la session. */
size_t pico8_zone_partie_prise(void);
size_t pico8_zone_partie_max(void);

/* Sous-zone chargement : un bloc qu'elle alloue vit jusqu'au prochain
 * pico8_zone_partie_reset(), donc au chargement ou a l'arret suivant ; ni la
 * bascule ni le rendu ne la touchent. Jamais de free sur ce bloc. */
const pico8_zone_choisie_t *pico8_zone_chargement(void);
/* Octets pris, et leur maximum sur la session, que le reset garde. */
size_t pico8_zone_chargement_prise(void);
size_t pico8_zone_chargement_max(void);

#endif
