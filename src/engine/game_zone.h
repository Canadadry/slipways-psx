#ifndef ENGINE_GAME_ZONE_H
#define ENGINE_GAME_ZONE_H

#include <stddef.h>

#include "engine/allocator.h"

/* Game zone: one generation per game and one per title screen, each opened
 * by a flip of cart_init() or cart_start_game(). A block it allocates lives
 * until its generation is released, by the exit-queue flush that follows
 * the flip. Never free such a block. The zone initialises itself on the
 * first call to any of these functions. */
const allocator_t *game_zone(void);
void game_zone_flip(void);
int game_zone_pending(void);
/* Does nothing when no generation is pending. */
void game_zone_release(void);
/* Also resets the load sub-zone. */
void game_zone_reset(void);
/* Bytes taken by both generations, and their maximum over the session. */
size_t game_zone_used(void);
size_t game_zone_max(void);

/* Load sub-zone: a block it allocates lives until the next
 * game_zone_reset(), so until the next cart_load() or cart_shutdown();
 * neither the flip nor the release touches it. Never free such a block. */
const allocator_t *game_zone_load_zone(void);
/* Bytes taken, and their maximum over the session, which the reset keeps. */
size_t load_zone_used(void);
size_t game_zone_load_zone_max(void);

#endif
