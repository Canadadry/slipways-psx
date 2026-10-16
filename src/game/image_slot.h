#ifndef GAME_IMAGE_SLOT_H
#define GAME_IMAGE_SLOT_H

#include "game/game_data.h"
#include "p8/image.h"

/* Every p8 image slot the game uses; ring slot k holds ring colour k. */
typedef enum {
    IMAGE_SLOT_BACKGROUND,
    IMAGE_SLOT_PLANET_DISC,
    IMAGE_SLOT_PLANET_RING_FIRST,
    IMAGE_SLOT_COUNT = IMAGE_SLOT_PLANET_RING_FIRST + PLANET_RING_COLORS_COUNT
} image_slot_t;

typedef char image_slot_count_matches_the_runtime[IMAGE_SLOT_COUNT == P8_IMAGE_SLOT_COUNT ? 1 : -1];

#endif
