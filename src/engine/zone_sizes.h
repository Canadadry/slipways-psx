#ifndef ENGINE_ZONE_SIZES_H
#define ENGINE_ZONE_SIZES_H

/* Real sizes in bytes, without the two halves the ASan build reserves for
 * the frame zone: the budget measures what the PSX will pay.
 * Reserves no memory. */
#define FRAME_ZONE_BYTES 20256u
#define UI_ZONE_BYTES 203961u
#define PARTICLES_ZONE_BYTES (128u * 1024u)
/* The game zone is its generation plus the load sub-zone. */
#define GENERATION_ZONE_BYTES 462520u
#define LOAD_ZONE_BYTES 24680u
#define GAME_ZONE_BYTES (GENERATION_ZONE_BYTES + LOAD_ZONE_BYTES)

/* PSX ceiling: it is not moved to make room. */
#define ZONE_BUDGET_BYTES (1536u * 1024u)

typedef char zone_budget_exceeded_t[
    (FRAME_ZONE_BYTES + UI_ZONE_BYTES +
     PARTICLES_ZONE_BYTES + GAME_ZONE_BYTES
     <= ZONE_BUDGET_BYTES) ? 1 : -1];

typedef char load_zone_outside_game_zone_t[
    (LOAD_ZONE_BYTES < GAME_ZONE_BYTES &&
     GENERATION_ZONE_BYTES + LOAD_ZONE_BYTES == GAME_ZONE_BYTES) ? 1 : -1];

#endif
