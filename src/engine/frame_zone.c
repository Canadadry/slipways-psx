#include "engine/frame_zone.h"

#include "engine/zone_sizes.h"

BUMP_ZONE_STORAGE(frame_cells, FRAME_ZONE_BYTES);

static bump_zone_t frame;
/* Statique : une liste ents/ptrs garde un pointeur vers sa zone. */
static allocator_t frame_handle;
static int frame_ready;

/* Paresseuse : pico8_bump_zone_init empoisonne le stockage sous ASan, ce
 * qu'un initialiseur statique ne ferait pas. */
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
