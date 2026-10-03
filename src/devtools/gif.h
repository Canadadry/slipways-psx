/* A capture as an animated GIF89a, so that a human can review an e2e
 * without raylib: global PICO-8 palette of 16 colors, scale 3, three
 * hundredths of a second per frame, infinite loop (NETSCAPE2.0), one full
 * image per frame. */

#ifndef DEVTOOLS_GIF_H
#define DEVTOOLS_GIF_H

#include "p8/input.h"

#include <stdio.h>

#define GIF_SCALE 3

typedef struct {
    FILE *file;
    int succeeded;
} gif_writer_t;

/* Writes the header to `file`, which the caller opens in binary mode and
 * closes itself. Returns 1 if everything was written, 0 for a NULL file. */
int gif_open(gif_writer_t *writer, FILE *file);

/* Appends a frame of 128x128 color indices (one byte per pixel, 0 to 15,
 * in the order screencast_read_frame fills). With a non-NULL `event`, that
 * event's mouse is drawn on top: a ring of radius 2 in color 7 with no
 * button down, of radius 3 in color 8 with a button down. `pixels` is not
 * modified. */
int gif_write_frame(gif_writer_t *writer, const unsigned char *pixels, const p8_input_frame_t *event);

/* Writes the trailer; returns 1 if no write has failed since gif_open.
 * Does not close the file. */
int gif_close(gif_writer_t *writer);

#endif
