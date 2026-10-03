#ifndef ENGINE_FRAME_ZONE_H
#define ENGINE_FRAME_ZONE_H

#include <stddef.h>

#include "engine/allocator.h"

/* Frame zone: a block it allocates lives until the next reset, made at the
 * top of cart_update60(). Never free such a block. The zone initialises
 * itself on the first call to either function. */
const allocator_t *frame_zone(void);
void frame_zone_reset(void);
/* Bytes used since the last reset, and their session maximum, which the
 * reset keeps. */
size_t frame_zone_used(void);
size_t frame_zone_max(void);

#endif
