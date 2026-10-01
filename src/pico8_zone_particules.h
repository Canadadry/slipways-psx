#ifndef PICO8_ZONE_PARTICULES_H
#define PICO8_ZONE_PARTICULES_H

#include <stddef.h>

#include "pico8_zone_choisie.h"

/* `particule` est un element de `qy`, `qy` la liste d'un emetteur,
 * `qy_fv` celle de l'ecran d'aide, `ga` un eclat avec son `oe` et sa
 * boite `ra`. */
typedef enum {
    PICO8_PARTICULES_PARTICULE,
    PICO8_PARTICULES_QY,
    PICO8_PARTICULES_QY_FV,
    PICO8_PARTICULES_GA,
    PICO8_PARTICULES_COUNT
} pico8_particules_sorte_t;

#define PICO8_PARTICULE_PLACE 32u
/* Elements reserves d'une liste de la sorte, que sa place tient juste. */
#define PICO8_QY_CAPACITE 160
#define PICO8_QY_FV_CAPACITE 768

/* Poignee qui donne et reprend une place entiere de la sorte : alloc(n)
 * exige n <= taille de place, grow est refuse. Une place de `qy`, `qy_fv`
 * ou `ga` est une arene, prise par pico8_zone_particules_place. Plus de
 * places vivantes que la limite : pico8_fatal « zone particules <sorte>
 * pleine ». */
const pico8_zone_choisie_t *pico8_zone_particules(pico8_particules_sorte_t sorte);

/* Une place de `qy`, `qy_fv` ou `ga`, en arene nommee « place <sorte> » ;
 * on la rend par pico8_zone_arene_rendre. */
const pico8_zone_choisie_t *pico8_zone_particules_place(pico8_particules_sorte_t sorte);

/* Places prises et pas encore rendues. */
int pico8_zone_particules_vivantes(pico8_particules_sorte_t sorte);

/* Maxima de la session. Octets : ceux d'une place-arene, tete comprise,
 * places encore vivantes mesurees a la lecture ; 0 pour `particule`.
 * Elements : ce que pico8_zone_particules_noter_elements a vu de plus grand
 * pour une liste `qy` ou `qy_fv`. */
int pico8_zone_particules_max_vivantes(pico8_particules_sorte_t sorte);
size_t pico8_zone_particules_max_octets(pico8_particules_sorte_t sorte);
void pico8_zone_particules_noter_elements(pico8_particules_sorte_t sorte, int count);
int pico8_zone_particules_max_elements(pico8_particules_sorte_t sorte);

#endif
