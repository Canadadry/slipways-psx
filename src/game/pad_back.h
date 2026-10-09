#ifndef GAME_PAD_BACK_H
#define GAME_PAD_BACK_H

#include "engine/entity_class.h"
#include "engine/entity_id_type.h"

/* The ○ back order's state: the last object × acted on, which ○ at the root returns to. */
typedef struct {
    entity_id_t last_acted;
} pad_back_t;

void pad_back_init(pad_back_t *back);

/* A NULL e keeps the previous record. */
void pad_back_record(pad_back_t *back, const entity_t *e);

/* The recorded object while it is a pad_target_is_candidate other than hovered; NULL otherwise, including when nothing
 * was recorded or it is gone. */
entity_t *pad_back_target(const pad_back_t *back, const entity_t *hovered);

#endif
