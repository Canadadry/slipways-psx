/* Whether this session was started from a recording, and then, frame by
 * frame, the recorded input until there is none left. */

#ifndef DEVTOOLS_REPLAY_SOURCE_H
#define DEVTOOLS_REPLAY_SOURCE_H

#include "p8/input.h"

typedef enum {
    /* No path given: an ordinary new game. */
    REPLAY_SOURCE_NONE = 0,
    /* A path was given and its recording is loaded and non-empty. */
    REPLAY_SOURCE_LOADED = 1,
    /* A path was given and could not be loaded. The caller must treat this
     * as a launch error, never as REPLAY_SOURCE_NONE: a new game cannot be
     * told from a lost save. */
    REPLAY_SOURCE_LOAD_FAILED = 2
} replay_source_status_t;

/* `path` is borrowed, never copied nor freed. `recording` is empty unless
 * status is REPLAY_SOURCE_LOADED. `next_frame_index` is this module's
 * cursor; `recording.current_frame` is never touched. Read through the
 * accessors below. */
typedef struct {
    replay_source_status_t status;
    const char *path;
    p8_input_t recording;
    int next_frame_index;
} replay_source_t;

/* A NULL `path` gives REPLAY_SOURCE_NONE and loads nothing; otherwise
 * p8_input_load decides between LOADED and LOAD_FAILED. The status is
 * stored and returned, and `*source` is safe to close in every case. */
replay_source_status_t replay_source_open(replay_source_t *source, const char *path);

/* 0 unless the status is REPLAY_SOURCE_LOADED. */
int replay_source_frame_count(const replay_source_t *source);

/* 0 before the first replay_source_next_frame call, never more than the
 * frame count. A count, not an index: during a frame it is one more than
 * that frame's 0-based position, which is what the marker file writes. */
int replay_source_frames_served(const replay_source_t *source);

/* 1 for a new game (no path), the recording's own declaration when one
 * loaded, 0 when the load failed. */
int replay_source_keyboard_cursor(const replay_source_t *source);

/* Returns 1 and copies the next recorded frame into *out_frame; once the
 * recording is exhausted, returns 0 forever and leaves *out_frame
 * untouched -- the normal end of a replay, not an error (unlike
 * p8_input_step, which aborts). A source with no path is exhausted from
 * its first call. */
int replay_source_next_frame(replay_source_t *source, p8_input_frame_t *out_frame);

/* Frees the loaded frames and returns `*source` to REPLAY_SOURCE_NONE.
 * Safe on a source that loaded nothing, and safe to call twice. */
void replay_source_close(replay_source_t *source);

#endif
