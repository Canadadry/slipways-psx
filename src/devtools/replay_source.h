/* Whether this session was started from a recording, and then, frame by
 * frame, the recorded input until there is none left. Kept outside the
 * raylib modules, which include raylib.h and are never run here nor built
 * by `make unit`, so that this decision can be unit-tested. Nothing here
 * includes raylib.h. */

#ifndef DEVTOOLS_REPLAY_SOURCE_H
#define DEVTOOLS_REPLAY_SOURCE_H

#include "p8/input.h"

typedef enum {
    /* No path was given on the command line: an ordinary new game. */
    REPLAY_SOURCE_NONE = 0,
    /* A path was given and its recording is loaded and non-empty. */
    REPLAY_SOURCE_LOADED = 1,
    /* A path was given and could not be loaded as a recording: missing,
     * unreadable, or malformed. The caller must treat this as a launch
     * error -- never as REPLAY_SOURCE_NONE, which would start a new game
     * the player cannot tell from a lost save. */
    REPLAY_SOURCE_LOAD_FAILED = 2
} replay_source_status_t;

/* `path` is borrowed, never copied and never freed, and is kept only so a
 * caller's error message can name it (it comes from argv, which outlives
 * every call made from main). `recording` holds the loaded frames and is
 * meaningful only when status is REPLAY_SOURCE_LOADED -- otherwise it is
 * an initialized, empty p8_input_t, so that replay_source_close is valid
 * whatever the outcome was. `next_frame_index` is this module's own
 * cursor, not `recording.current_frame`, which is never touched here.
 * Read through the accessors below. */
typedef struct {
    replay_source_status_t status;
    const char *path;
    p8_input_t recording;
    int next_frame_index;
} replay_source_t;

/* Decides, once, whether this session replays a recording. `path` may be
 * NULL (no positional argument), which is REPLAY_SOURCE_NONE and loads
 * nothing; otherwise the file goes through p8_input_load, giving LOADED
 * or LOAD_FAILED. The status is both stored in `*source` and
 * returned, and `*source` is left safe to close in every case. */
replay_source_status_t replay_source_open(replay_source_t *source, const char *path);

/* How many frames the recording holds: 0 unless the status is
 * REPLAY_SOURCE_LOADED. Says how long the replay will be, not what
 * any frame contains. */
int replay_source_frame_count(const replay_source_t *source);

/* How many frames this source has already served: 0 before the first
 * replay_source_next_frame call, N once the recording is exhausted, and
 * never more than N however often exhaustion is reported afterwards. A
 * count, not an index: on the frame being played it is one more than
 * that frame's 0-based position, the number the marker file writes. */
int replay_source_frames_served(const replay_source_t *source);

/* Whether this session runs the keyboard cursor: 1 for a new game (no
 * path), the recording's own declaration when one loaded, 0 when the load
 * failed. A resumed session follows its recording because a file holds one
 * declaration for all its frames, replayed and live alike. */
int replay_source_keyboard_cursor(const replay_source_t *source);

/* Serves the next recorded frame, or reports that there are none left.
 * Returns 1 and copies the frame into *out_frame while the recording has
 * one; returns 0 and leaves *out_frame untouched once it is exhausted --
 * the normal end of every replay, not an error, and unlike
 * p8_input_step's abort-on-overrun contract, which is right for the
 * headless driver and wrong here. Given N frames it returns 1 exactly N
 * times, in recorded order, then 0 forever: no looping, no clamping, no
 * abort. A source opened with no path is exhausted from its first call,
 * which is what lets the driver call this unconditionally.
 *
 * The frame is copied rather than returned by pointer so that a replayed
 * and a live frame are the same shape at the call site. */
int replay_source_next_frame(replay_source_t *source, p8_input_frame_t *out_frame);

/* Releases the loaded frames (p8_input_free) and returns `*source` to
 * the REPLAY_SOURCE_NONE state. Safe on a source that loaded
 * nothing, and safe to call twice. */
void replay_source_close(replay_source_t *source);

#endif
