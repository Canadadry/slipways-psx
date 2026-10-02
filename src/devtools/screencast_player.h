/* The two decisions of the screencast viewer that need no window: which
 * frame to show now, and what to say about a file it refuses to play.
 * Nothing here includes raylib.h, and nothing here may include it: that
 * is what lets `make unit` exercise this module, which it cannot do for
 * src/cmd/screencast_viewer.c. */

#ifndef DEVTOOLS_SCREENCAST_PLAYER_H
#define DEVTOOLS_SCREENCAST_PLAYER_H

#include "devtools/screencast.h"

#include <stddef.h>

/* A convenience for the caller, not a contract it could break: the
 * function truncates rather than overflowing what it is given. The
 * longest of the three refusals is 184 characters without the path, so
 * 256 holds a path of up to 71. */
#define SCREENCAST_PLAYER_REFUSAL_MAX 256

/* Where playback stands. A single number moves, the index of the frame
 * shown; `frame_count` is set when the file is opened and never changes. */
typedef struct {
    int frame_count;
    int current;
} screencast_player_t;

/* Puts playback on frame 0. `frame_count` is what screencast_frame_count
 * has just returned, so at least 1. */
void screencast_player_init(screencast_player_t *player, int frame_count);

/* The index of the frame to show now, from 0 to frame_count-1, to pass
 * as is to screencast_read_frame. Not as is to
 * replay_control_format_status, which counts from 1: the caller adds 1. */
int screencast_player_current_frame(const screencast_player_t *player);

/* Moves to the next frame. On the last frame the call does nothing,
 * indefinitely: playback freezes on the end of the file, it does not
 * loop. Nothing here touches the file. */
void screencast_player_advance(screencast_player_t *player);

/* Writes into the buffer (always terminated by a NUL, truncated rather
 * than overflowed) the sentence screencast_viewer prints on stderr before
 * it stops, for a `status` that screencast_reader_open returned and that
 * is not SCREENCAST_OK. */
void screencast_player_format_refusal(screencast_status_t status, const char *path, char *out, size_t out_size);

#endif
