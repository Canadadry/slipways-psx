#ifndef GAME_PARTICLE_ZONE_H
#define GAME_PARTICLE_ZONE_H

#include <stddef.h>

#include "engine/allocator.h"

/* `particule` est un element de `qy`, `qy` la liste d'un emetteur,
 * `qy_fv` celle de l'ecran d'aide, `ga` un eclat avec son `oe` et sa
 * boite `ra`, `ts` une trainee d'un slipway. */
typedef enum {
    PARTICLE_SLOT_PARTICLE,
    PARTICLE_SLOT_LIST,
    PARTICLE_SLOT_HELP_LIST,
    PARTICLE_SLOT_BURST,
    PARTICLE_SLOT_TRANSIT_DOT,
    PARTICLE_SLOT_COUNT
} particle_slot_kind_t;

#define PARTICLE_SLOT 32u
#define TRANSIT_DOTS_SLOT 24u
/* Elements reserves d'une liste de la sorte, que sa place tient juste. */
#define PARTICLES_CAPACITY 160
#define PARTICLES_HELP_SCREEN_CAPACITY 768

/* Poignee qui donne et reprend une place entiere de la sorte : alloc(n)
 * exige n <= taille de place, grow est refuse. Une place de `qy`, `qy_fv`
 * ou `ga` est une arene, prise par pico8_zone_particules_place. Plus de
 * places vivantes que la limite : pico8_fatal « zone particules <sorte>
 * pleine ». */
const allocator_t *particle_zone(particle_slot_kind_t slot_kind);

/* Une place de `qy`, `qy_fv` ou `ga`, en arene nommee « place <sorte> » ;
 * on la rend par pico8_zone_arene_rendre. */
const allocator_t *particle_zone_slot(particle_slot_kind_t slot_kind);

/* Places prises et pas encore rendues. */
int particle_zone_live(particle_slot_kind_t slot_kind);

/* Maxima de la session. Octets : ceux d'une place-arene, tete comprise,
 * places encore vivantes mesurees a la lecture ; 0 pour `particule`.
 * Elements : ce que pico8_zone_particules_noter_elements a vu de plus grand
 * pour une liste `qy` ou `qy_fv`, ou la liste `ts` d'un slipway. */
int particle_zone_max_live(particle_slot_kind_t slot_kind);
size_t particle_zone_max_bytes(particle_slot_kind_t slot_kind);
void particle_zone_record_elements(particle_slot_kind_t slot_kind, int count);
int particle_zone_max_elements(particle_slot_kind_t slot_kind);

#endif
