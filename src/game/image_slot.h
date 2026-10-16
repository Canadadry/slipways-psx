#ifndef GAME_IMAGE_SLOT_H
#define GAME_IMAGE_SLOT_H

#include "p8/image.h"

/* Every p8 image slot the game uses; row slot k is IMAGE_SLOT_ROW_FIRST + k, text slot k
 * IMAGE_SLOT_TEXT_FIRST + k. */
typedef enum {
    IMAGE_SLOT_BACKGROUND,
    IMAGE_SLOT_ROW_FIRST,
    IMAGE_SLOT_ROW_COUNT = 32,
    IMAGE_SLOT_TEXT_FIRST = IMAGE_SLOT_ROW_FIRST + IMAGE_SLOT_ROW_COUNT,
    IMAGE_SLOT_TEXT_COUNT = 32,
    IMAGE_SLOT_COUNT = IMAGE_SLOT_TEXT_FIRST + IMAGE_SLOT_TEXT_COUNT
} image_slot_t;

typedef char image_slot_count_matches_the_runtime[IMAGE_SLOT_COUNT == P8_IMAGE_SLOT_COUNT ? 1 : -1];

#endif
