#include "engine/frame_zone.h"

#include "engine/zone_sizes.h"

BUMP_ZONE_STORAGE(frame_cells, FRAME_ZONE_BYTES);

static bump_zone_t frame;
/* Static: an entity_list_t or pointer_list_t keeps a pointer to its zone. */
static allocator_t frame_handle;
static int frame_ready;

/* Lazy: bump_zone_init poisons the storage under ASan, which a static
 * initialiser would not do. */
static void frame_init_once(void) {
    if (frame_ready) return;
    bump_zone_init(&frame, "frame", frame_cells, FRAME_ZONE_BYTES);
    frame_handle = allocator_from_bump_zone(&frame);
    frame_ready = 1;
}

const allocator_t *frame_zone(void) {
    frame_init_once();
    return &frame_handle;
}

void frame_zone_reset(void) {
    frame_init_once();
    bump_zone_reset(&frame);
}
