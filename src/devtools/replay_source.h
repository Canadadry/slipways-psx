/* Whether this session was started from a recording, and then, frame by
 * frame, the recorded input until there is none left. Kept outside the
 * raylib modules, which include raylib.h and can therefore be neither
 * built nor run here, so that this decision can be unit-tested. Nothing
 * here includes raylib.h. */

#ifndef PICO8_REPLAY_SOURCE_H
#define PICO8_REPLAY_SOURCE_H

#include "p8/input.h"

typedef enum {
    /* No path was given on the command line: an ordinary new game. */
    PICO8_REPLAY_SOURCE_NONE = 0,
    /* A path was given and its recording is loaded and non-empty. */
    PICO8_REPLAY_SOURCE_LOADED = 1,
    /* A path was given and could not be loaded as a recording: missing,
     * unreadable, or malformed. The caller must treat this as a launch
     * error -- never as PICO8_REPLAY_SOURCE_NONE, which would start a new
     * game the player cannot tell from a lost save. */
    PICO8_REPLAY_SOURCE_LOAD_FAILED = 2
} pico8_replay_source_status_t;

/* `path` is borrowed, never copied and never freed, and is kept only so a
 * caller's error message can name it (argv outlives main). `recording`
 * holds the loaded frames and is meaningful only when status is
 * PICO8_REPLAY_SOURCE_LOADED -- otherwise it is an initialized, empty
 * pico8_input_t, so that close is valid whatever the outcome was.
 * `next_frame_index` is this module's own cursor, deliberately NOT
 * `recording.current_frame`, which is never touched here. Read through
 * the accessors below. */
typedef struct {
    pico8_replay_source_status_t status;
    const char *path;
    p8_input_t recording;
    int next_frame_index;
} pico8_replay_source_t;

/* Decides, once, whether this session replays a recording. `path` may be
 * NULL (no positional argument), which is PICO8_REPLAY_SOURCE_NONE and
 * loads nothing; otherwise the file goes through pico8_input_load, giving
 * LOADED or LOAD_FAILED. The status is both stored in `*source` and
 * returned, and `*source` is left safe to close in every case. */
pico8_replay_source_status_t pico8_replay_source_open(pico8_replay_source_t *source, const char *path);

/* How many frames the recording holds: 0 unless the status is
 * PICO8_REPLAY_SOURCE_LOADED. Says how long the replay will be, not what
 * any frame contains. */
int pico8_replay_source_frame_count(const pico8_replay_source_t *source);

/* How many frames this source has already served: 0 before the first
 * next_frame call, N once the recording is exhausted, and never more than
 * N however often exhaustion is reported afterwards. A count, not an
 * index: on the frame being played it is one more than that frame's
 * 0-based position, which is the number the marker file writes. */
int pico8_replay_source_frames_served(const pico8_replay_source_t *source);

/* Serves the next recorded frame, or reports that there are none left.
 * Returns 1 and copies the frame into *out_frame while the recording has
 * one; returns 0 and leaves *out_frame untouched once it is exhausted --
 * the normal end of every replay, not an error, and deliberately not
 * pico8_input_step's abort-on-overrun contract, which is right for the
 * headless driver and wrong here. Given N frames it returns 1 exactly N
 * times, in recorded order, then 0 forever: no looping, no clamping, no
 * abort. A source opened with no path is exhausted from its first call,
 * which is what lets the driver call this unconditionally.
 *
 * The frame is copied rather than returned by pointer so that a replayed
 * and a live frame are the same shape at the call site. */
int pico8_replay_source_next_frame(pico8_replay_source_t *source, p8_input_frame_t *out_frame);

/* Releases the loaded frames (pico8_input_free) and returns `*source` to
 * the PICO8_REPLAY_SOURCE_NONE state. Safe on a source that loaded
 * nothing, and safe to call twice. */
void pico8_replay_source_close(pico8_replay_source_t *source);

#endif
