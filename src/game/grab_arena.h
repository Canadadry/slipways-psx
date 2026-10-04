#ifndef GAME_GRAB_ARENA_H
#define GAME_GRAB_ARENA_H

#include <stddef.h>

#include "engine/allocator.h"
#include "engine/entity_class.h"

/* Where the constructors of planets, probes, slipways, wormholes and
 * structure icons allocate: the grab arena between grab_arena_open and
 * grab_arena_close, else the game zone. */
const allocator_t *grab_arena_construction_zone(void);

/* Opens a grab arena for a construction that takes exactly `bytes`
 * (ZONE_BYTES of each block): the smallest free grab that holds them
 * (emptied), else a new one of that size from the game zone; a placed
 * structure keeps it until the game zone is given back. Past `bytes`, the
 * construction is a p8_fatal "zone grab full". */
void grab_arena_open(size_t bytes);
/* From then on, what no longer fits in the arena spills into the game
 * zone (arena_spill). */
void grab_arena_close(void);
/* Called last by the release method of an object built in `zone`, with the
 * object's state: a grab arena (NULL or another zone: nothing) whose object
 * is still placing goes on the free grabs, for the next grab to empty and
 * reuse. The free grabs keep the GRAB_ARENAS_FREE_MAX largest: one
 * dropped stays in the game zone until its generation is given back. */
#define GRAB_ARENAS_FREE_MAX 8
void grab_arena_give_back_if_placing(const allocator_t *zone, state_t state);
/* The same whatever the state: for an object whose memory nothing borrows
 * after its death. */
void grab_arena_give_back(const allocator_t *zone);
/* Drops the free grabs, without reading them: their arenas belong to a
 * game zone about to be given back or reset. */
void grab_arena_forget_free(void);

#endif
