/* The capture format, public: an outside script must be able to read it
 * without the project's code.
 * Raw frames concatenated in capture order, with no header, no index and
 * no version number. A frame takes SCREENCAST_FRAME_BYTES = 9600 bytes:
 * the 19200 pixels of the 160x120 screen, two pixels per byte, in the
 * scan order of the screen memory. Each byte holds two horizontally
 * adjacent pixels: the even-indexed one in its LOW nibble, the
 * odd-indexed one in its high nibble. Frame N starts at offset N x 9600,
 * and the frame count is the file size divided by 9600, never a stored
 * counter. */

#ifndef DEVTOOLS_SCREENCAST_H
#define DEVTOOLS_SCREENCAST_H

#include "p8/gfx.h"
#include "p8/vram.h"

#include <stdio.h>

/* One pixel per byte: the size of the buffer screencast_read_frame
 * fills. */
#define SCREENCAST_FRAME_PIXELS (P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT)

/* The size of a frame IN THE FILE, two pixels per byte. */
#define SCREENCAST_FRAME_BYTES (SCREENCAST_FRAME_PIXELS / 2)

typedef struct {
    FILE *file;
} screencast_writer_t;

/* Starts a capture at `path`, truncating an existing file. Returns 1 if
 * the file is open, 0 otherwise; after a failure, `*writer` is safe to
 * pass to screencast_writer_close. */
int screencast_writer_open(screencast_writer_t *writer, const char *path);

/* Appends mem->screen to the capture. Returns 1 if the 9600 bytes were
 * written, 0 otherwise. */
int screencast_write_frame(screencast_writer_t *writer, const p8_vram_t *mem);

/* Returns 1 if closing succeeded, 0 otherwise: fclose is the last place
 * a buffered write can fail. On a writer whose opening failed, or already
 * closed, does nothing and returns 1. */
int screencast_writer_close(screencast_writer_t *writer);

typedef enum {
    SCREENCAST_OK = 0,
    SCREENCAST_UNREADABLE = 1,
    /* Refused on its own: zero IS a multiple of 9600, so the size check
     * alone would accept an empty file as a capture of zero frames. */
    SCREENCAST_EMPTY = 2,
    /* The size is not a multiple of SCREENCAST_FRAME_BYTES. */
    SCREENCAST_TRUNCATED = 3
} screencast_status_t;

/* Unless opening returned SCREENCAST_OK, `file` is NULL and the count
 * zero, and screencast_reader_close is still valid. */
typedef struct {
    FILE *file;
    int frame_count;
} screencast_reader_t;

/* Opens a capture read-only, NEVER for writing, and validates its
 * size. */
screencast_status_t screencast_reader_open(screencast_reader_t *reader, const char *path);

/* 0 if opening failed. */
int screencast_frame_count(const screencast_reader_t *reader);

/* Reads frame `index` into `out_pixels`, which must hold
 * SCREENCAST_FRAME_PIXELS bytes, ONE pixel (0 to 15) per byte. Returns 1
 * on success; returns 0 and leaves `out_pixels` untouched if the index is
 * out of range or the read fails. Indexes may come in any order. */
int screencast_read_frame(screencast_reader_t *reader, int index, unsigned char *out_pixels);

/* Does nothing on a reader whose opening failed or already closed. */
void screencast_reader_close(screencast_reader_t *reader);

#endif
