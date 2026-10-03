#ifndef ENGINE_FRAME_ZONE_H
#define ENGINE_FRAME_ZONE_H

#include "engine/allocator.h"

/* Frame zone: a block it allocates lives until the next reset, made at the
 * top of cart_update60(). Never free such a block. The zone initialises
 * itself on the first call to either function. */
const allocator_t *frame_zone(void);
void frame_zone_reset(void);

/* A mark taken in the current frame, rewound to before the next reset
 * (an assert stops a mark from an earlier frame). A block taken before the
 * mark stays valid; a block taken after it is dead at the rewind, its room
 * free again. ASan blind spot: the next allocations reuse the same
 * addresses, so a pointer kept from before the rewind reads the new block
 * there without crashing. */
typedef struct {
    size_t used;
    unsigned long generation;
} frame_zone_mark_t;
frame_zone_mark_t frame_zone_mark(void);
void frame_zone_rewind(frame_zone_mark_t mark);

#endif
