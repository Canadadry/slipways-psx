/* The whole capture format; no other module knows it: the replay driver
 * computes no offset.
 * Raw frames concatenated in capture order, with no header, no index and
 * no version number. A frame takes SCREENCAST_FRAME_BYTES = 8192 bytes:
 * the 16384 pixels of the 128x128 screen, two pixels per byte, in the
 * scan order of the screen memory. Each byte holds two horizontally
 * adjacent pixels: the even-indexed one in its LOW nibble, the
 * odd-indexed one in its high nibble. Frame N therefore starts at offset
 * N x 8192, and the frame count is the file size divided by 8192, never a
 * stored counter. This contract is public -- an outside script must be
 * able to read the file without the project's code -- and
 * tests/test_screencast.c asserts the nibble order on the raw bytes,
 * which a round trip would not prove.
 *
 * Nothing here includes raylib.h and nothing here may: a file that links
 * raylib leaves `make unit`, so the format, the reading and the size
 * validation could no longer be checked without a window. */

#ifndef DEVTOOLS_SCREENCAST_H
#define DEVTOOLS_SCREENCAST_H

#include "p8/gfx.h"
#include "p8/vram.h"

#include <stdio.h>

/* The 16384 pixels of a frame, one per byte: the size of the buffer that
 * screencast_read_frame fills. */
#define SCREENCAST_FRAME_PIXELS (P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT)

/* The 8192 bytes a frame takes IN THE FILE, two pixels per byte. */
#define SCREENCAST_FRAME_BYTES (SCREENCAST_FRAME_PIXELS / 2)

/* The capture being written; `file` is only written through this module. */
typedef struct {
    FILE *file;
} screencast_writer_t;

/* Starts a capture at `path`, truncating an existing file. Returns 1 if
 * the file is open, 0 otherwise; after a failure, `*writer` is safe to
 * pass to screencast_writer_close. */
int screencast_writer_open(screencast_writer_t *writer, const char *path);

/* Appends a frame to the capture, read from mem->screen and packed in
 * the format above. Returns 1 if the 8192 bytes were written, 0
 * otherwise. The failure goes back to the caller instead of being
 * absorbed: unlike session_recorder_record_frame, which drops a frame it
 * cannot append, losing a frame here yields a truncated file whose
 * sha256 would be wrong. */
int screencast_write_frame(screencast_writer_t *writer, const p8_vram_t *mem);

/* Ends the capture and returns 1 if closing succeeded, 0 otherwise --
 * fclose is the last place where a buffered write can fail. On a writer
 * whose opening failed, does nothing and returns 1; a second call has no
 * effect. */
int screencast_writer_close(screencast_writer_t *writer);

/* The outcome of opening a capture for reading. Telling the causes apart
 * belongs to this module, the message to the caller. */
typedef enum {
    /* The file is open and its size is a whole, non-zero number of
     * frames. */
    SCREENCAST_OK = 0,
    /* Missing, or unreadable. */
    SCREENCAST_UNREADABLE = 1,
    /* Present and empty. A value of its own, not a case of invalid size:
     * zero IS a multiple of 8192, so without this explicit refusal an
     * empty file would pass for a capture of zero frames. */
    SCREENCAST_EMPTY = 2,
    /* The size is not a multiple of SCREENCAST_FRAME_BYTES: the file
     * ends in the middle of a frame. */
    SCREENCAST_TRUNCATED = 3
} screencast_status_t;

/* A capture open for reading. Both fields mean something only if opening
 * returned SCREENCAST_OK; in the three other cases the file is already
 * closed and the count is zero, so that screencast_reader_close is valid
 * whatever the outcome. */
typedef struct {
    FILE *file;
    int frame_count;
} screencast_reader_t;

/* Opens a capture for reading and validates its size. The file is opened
 * read-only and NEVER for writing: reading a capture back must not be
 * able to change one of its bytes. */
screencast_status_t screencast_reader_open(screencast_reader_t *reader, const char *path);

/* How many frames the capture holds. 0 if opening failed. It bounds the
 * indexes accepted below. */
int screencast_frame_count(const screencast_reader_t *reader);

/* Reads frame `index` (0 to frame_count - 1) into `out_pixels`, which must
 * hold SCREENCAST_FRAME_PIXELS bytes. Each byte returned is ONE pixel,
 * from 0 to 15: the nibbles are unpacked here. Returns 1 on success;
 * returns 0 and leaves `out_pixels` untouched if the index is outside the
 * file's frames or if the read fails. No current position is kept, so
 * indexes may be asked for in any order and as many times as needed. */
int screencast_read_frame(screencast_reader_t *reader, int index, unsigned char *out_pixels);

/* Closes the capture and returns `*reader` to its state before opening.
 * On an opening that failed, and on a reader already closed, does
 * nothing. Nothing can fail: no write is pending. */
void screencast_reader_close(screencast_reader_t *reader);

#endif
