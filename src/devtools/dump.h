#ifndef DEVTOOLS_DUMP_H
#define DEVTOOLS_DUMP_H

#include "p8/vram.h"

#include <stdio.h>

/* Writes the framebuffer to the stream as text: a "<width> <height>" line,
 * then one line per row of one lowercase hex digit per pixel. Returns 1 if
 * the whole dump was written, 0 on the first write failure. */
int dump_screen(const p8_vram_t *mem, FILE *out);

/* The same text, from `pixels`: 128x128 bytes in scan order, one color
 * index 0-15 per byte (a byte above 15 fails an assert) -- what
 * screencast_read_frame fills. */
int dump_pixels(const unsigned char *pixels, FILE *out);

#endif
