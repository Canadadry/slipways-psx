#ifndef PICO8_ZONE_SIZES_H
#define PICO8_ZONE_SIZES_H

/* Tailles reelles en octets, sans les deux moities que le build ASan
 * reserve a la zone frame : le budget mesure ce que la PSX paiera.
 * Ne reserve aucune memoire. */
#define PICO8_ZONE_FRAME_BYTES (256u * 1024u)
#define PICO8_ZONE_INTERFACE_BYTES (335u * 1024u)
#define PICO8_ZONE_PARTICLES_BYTES (128u * 1024u)
#define PICO8_ZONE_GAME_BYTES (768u * 1024u)
/* Sous-zone chargement, prise dans la zone partie : la generation de partie
 * n'a que le reste. */
#define PICO8_ZONE_CHARGEMENT_BYTES (64u * 1024u)
#define PICO8_ZONE_GENERATION_BYTES (PICO8_ZONE_GAME_BYTES - PICO8_ZONE_CHARGEMENT_BYTES)

/* Plafond PSX : on ne le deplace pas pour faire de la place. */
#define PICO8_ZONE_BUDGET_BYTES (1536u * 1024u)

typedef char pico8_zone_budget_exceeded[
    (PICO8_ZONE_FRAME_BYTES + PICO8_ZONE_INTERFACE_BYTES +
     PICO8_ZONE_PARTICLES_BYTES + PICO8_ZONE_GAME_BYTES
     <= PICO8_ZONE_BUDGET_BYTES) ? 1 : -1];

typedef char pico8_zone_chargement_hors_de_la_partie[
    (PICO8_ZONE_CHARGEMENT_BYTES < PICO8_ZONE_GAME_BYTES &&
     PICO8_ZONE_GENERATION_BYTES + PICO8_ZONE_CHARGEMENT_BYTES == PICO8_ZONE_GAME_BYTES) ? 1 : -1];

#endif
