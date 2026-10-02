#ifndef ENGINE_ZONE_SIZES_H
#define ENGINE_ZONE_SIZES_H

/* Real sizes in bytes, without the two halves the ASan build reserves for
 * the frame zone: the budget measures what the PSX will pay.
 * Reserves no memory. */
#define FRAME_ZONE_BYTES (256u * 1024u)
#define UI_ZONE_BYTES (335u * 1024u)
#define ZONE_PARTICLES_BYTES (128u * 1024u)
#define ZONE_GAME_BYTES (768u * 1024u)
/* Load sub-zone, taken from the game zone: the game generation gets only
 * the rest. */
#define LOAD_ZONE_BYTES (64u * 1024u)
#define ZONE_GENERATION_BYTES (ZONE_GAME_BYTES - LOAD_ZONE_BYTES)

/* PSX ceiling: it is not moved to make room. */
#define ZONE_BUDGET_BYTES (1536u * 1024u)

typedef char zone_budget_exceeded_t[
    (FRAME_ZONE_BYTES + UI_ZONE_BYTES +
     ZONE_PARTICLES_BYTES + ZONE_GAME_BYTES
     <= ZONE_BUDGET_BYTES) ? 1 : -1];

typedef char load_zone_outside_game_zone_t[
    (LOAD_ZONE_BYTES < ZONE_GAME_BYTES &&
     ZONE_GENERATION_BYTES + LOAD_ZONE_BYTES == ZONE_GAME_BYTES) ? 1 : -1];

#endif
