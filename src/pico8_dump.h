#ifndef PICO8_DUMP_H
#define PICO8_DUMP_H

#include "pico8_mem.h"

#include <stdio.h>

/* Writes the framebuffer to `out` as text: a "<width> <height>" line, then
 * one line per row of one lowercase hex digit per pixel. Returns 1 if the
 * whole dump was written, 0 on the first write failure. */
int pico8_dump_screen(const pico8_mem_t *mem, FILE *out);

#endif
