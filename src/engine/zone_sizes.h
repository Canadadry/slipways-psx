#ifndef ENGINE_ZONE_SIZES_H
#define ENGINE_ZONE_SIZES_H

/* Tailles reelles en octets, sans les deux moities que le build ASan
 * reserve a la zone frame : le budget mesure ce que la PSX paiera.
 * Ne reserve aucune memoire. */
#define FRAME_ZONE_BYTES (256u * 1024u)
#define UI_ZONE_BYTES (335u * 1024u)
#define ZONE_PARTICLES_BYTES (128u * 1024u)
#define ZONE_GAME_BYTES (768u * 1024u)
/* Sous-zone chargement, prise dans la zone partie : la generation de partie
 * n'a que le reste. */
#define LOAD_ZONE_BYTES (64u * 1024u)
#define ZONE_GENERATION_BYTES (ZONE_GAME_BYTES - LOAD_ZONE_BYTES)

/* Plafond PSX : on ne le deplace pas pour faire de la place. */
#define ZONE_BUDGET_BYTES (1536u * 1024u)

typedef char zone_budget_exceeded_t[
    (FRAME_ZONE_BYTES + UI_ZONE_BYTES +
     ZONE_PARTICLES_BYTES + ZONE_GAME_BYTES
     <= ZONE_BUDGET_BYTES) ? 1 : -1];

typedef char load_zone_outside_game_zone_t[
    (LOAD_ZONE_BYTES < ZONE_GAME_BYTES &&
     ZONE_GENERATION_BYTES + LOAD_ZONE_BYTES == ZONE_GAME_BYTES) ? 1 : -1];

#endif
