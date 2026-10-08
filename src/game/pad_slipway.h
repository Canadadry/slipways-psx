#ifndef GAME_PAD_SLIPWAY_H
#define GAME_PAD_SLIPWAY_H

#include "engine/entity_class.h"
#include "engine/entity_id_type.h"
#include "engine/entity_list.h"

typedef enum { PAD_SLIPWAY_NONE, PAD_SLIPWAY_PRESSED, PAD_SLIPWAY_HELD, PAD_SLIPWAY_RELEASED } pad_slipway_phase_t;

typedef enum { PAD_SLIPWAY_REFUSED, PAD_SLIPWAY_BUILT } pad_slipway_outcome_t;

/* target is ENTITY_ID_NONE while the aim is on the start. */
typedef struct {
    entity_id_t start;
    entity_id_t target;
    pad_slipway_phase_t phase;
    int linked_before;
} pad_slipway_t;

void pad_slipway_init(pad_slipway_t *slipway);

/* 1 for a live planet-family entity in state SETTLED that takes a slipway; 0 otherwise, NULL included. */
int pad_slipway_can_start(const entity_t *e);

/* Points out at the caller's storage, which it owns, holding the entries of candidates other than start, in order;
 * returns their count. Allocates nothing. */
int pad_slipway_candidates(const entity_list_t *candidates, const entity_t *start, entity_t **storage, int capacity,
                           entity_list_t *out);

/* The slipway on start's slipways list whose other end is target, or NULL. start is planet-family. */
entity_t *pad_slipway_between(const entity_t *start, const entity_t *target);
/* PAD_SLIPWAY_BUILT when a slipway joins start and target now and linked_before is 0; PAD_SLIPWAY_REFUSED otherwise,
 * a NULL target included. */
pad_slipway_outcome_t pad_slipway_outcome(const entity_t *start, const entity_t *target, int linked_before);

/* From HELD with a target: RELEASED, with linked_before kept for pad_slipway_outcome. */
void pad_slipway_release(pad_slipway_t *slipway, int linked_before);
/* From RELEASED: PRESSED with no target, the start moved to the target when outcome is PAD_SLIPWAY_BUILT. */
void pad_slipway_rebase(pad_slipway_t *slipway, pad_slipway_outcome_t outcome);

#endif
