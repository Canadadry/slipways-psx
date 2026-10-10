#ifndef GAME_PAD_TARGET_H
#define GAME_PAD_TARGET_H

#include "engine/entity_class.h"
#include "engine/entity_list.h"

#define PAD_TARGET_PERPENDICULAR_WEIGHT 2

int pad_target_is_candidate(const entity_t *e);

entity_t *pad_target_pick(const entity_t *origin, int direction, const entity_list_t *candidates);

#endif
