#ifndef GAME_PAD_BUILD_H
#define GAME_PAD_BUILD_H

#include "engine/entity_class.h"
#include "engine/entity_id_type.h"

/* The build bar's state: the focused structure icon. */
typedef struct {
    entity_id_t icon;
} pad_build_t;

void pad_build_init(pad_build_t *b);

/* Reads only. The focused icon while it is a revealed structure icon; NULL otherwise. */
entity_t *pad_build_icon(const pad_build_t *b);

/* The focused icon, after a stale or missing focus has moved to the rightmost revealed icon, the basic structure.
 * NULL when no icon is revealed. */
entity_t *pad_build_focus(pad_build_t *b);

/* Step -1 moves the focus to the nearest revealed icon on its left, 1 to the nearest on its right. Nothing moves past
 * an end (no wrap), nor while pad_build_icon is NULL. */
void pad_build_pick(pad_build_t *b, int step);

#endif
