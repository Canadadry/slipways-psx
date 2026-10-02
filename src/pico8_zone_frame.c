#include "pico8_zone_frame.h"

#include "pico8_zone_sizes.h"

PICO8_BUMP_ZONE_STORAGE(frame_cells, PICO8_ZONE_FRAME_BYTES);

static pico8_bump_zone_t frame;
/* Statique : une liste ents/ptrs garde un pointeur vers sa zone. */
static pico8_zone_choisie_t frame_handle;
static int frame_ready;
static size_t frame_max;

/* Paresseuse : pico8_bump_zone_init empoisonne le stockage sous ASan, ce
 * qu'un initialiseur statique ne ferait pas. */
static void frame_init_once(void) {
    if (frame_ready) return;
    pico8_bump_zone_init(&frame, "frame", frame_cells, PICO8_ZONE_FRAME_BYTES);
    frame_handle = pico8_zone_choisie_bump(&frame);
    frame_ready = 1;
}

const pico8_zone_choisie_t *pico8_zone_frame(void) {
    frame_init_once();
    return &frame_handle;
}

/* `used` only grows between two resets: reading it before each reset and
 * at each read of the maximum is enough. */
static void frame_note_max(void) {
    if (frame.used > frame_max) frame_max = frame.used;
}

void pico8_zone_frame_reset(void) {
    frame_init_once();
    frame_note_max();
    pico8_bump_zone_reset(&frame);
}

size_t pico8_zone_frame_used(void) {
    frame_init_once();
    return frame.used;
}

size_t pico8_zone_frame_max(void) {
    frame_init_once();
    frame_note_max();
    return frame_max;
}
