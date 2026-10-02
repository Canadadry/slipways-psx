#ifndef PICO8_DUMP_H
#define PICO8_DUMP_H

#include "p8/mem.h"

#include <stdio.h>

/* Writes the framebuffer to `out` as text: a "<width> <height>" line, then
 * one line per row of one lowercase hex digit per pixel. Returns 1 if the
 * whole dump was written, 0 on the first write failure. */
int pico8_dump_screen(const p8_memory_t *mem, FILE *out);

/* The same text, from `pixels`: 128x128 bytes in scan order, one color
 * index 0-15 per byte -- what pico8_screencast_read_frame fills. */
int pico8_dump_pixels(const unsigned char *pixels, FILE *out);

#endif
