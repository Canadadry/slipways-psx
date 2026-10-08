#ifndef GAME_PAD_TARGET_H
#define GAME_PAD_TARGET_H

#include "engine/entity_class.h"
#include "engine/entity_list.h"

#define PAD_TARGET_PERPENDICULAR_WEIGHT 2

/* 1 for a live planet-family entity in state KNOWN or SETTLED; 0 otherwise, NULL included. */
int pad_target_is_candidate(const entity_t *e);

/* Reads only map_pos and identities, never pos, and does not filter by pad_target_is_candidate: the caller builds the
 * live set. Returns a pointer into candidates, never origin; NULL when nothing lies in the direction's 90-degree cone,
 * or candidates is NULL or empty. Equal scores go to the smaller |perpendicular|, then to the earlier candidate. */
entity_t *pad_target_pick(const entity_t *origin, int direction, const entity_list_t *candidates);

#endif
