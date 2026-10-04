/* The session currently being recorded -- where the recording lives, how
 * many frames it holds, where the next one goes -- and the one operation
 * that moves it forward. */

#ifndef DEVTOOLS_SESSION_RECORDER_H
#define DEVTOOLS_SESSION_RECORDER_H

#include "p8/input.h"

#define SESSION_RECORDER_PATH_MAX 128

/* `path` is a copy. `tail_offset` is p8_input_append's resume point.
 * No field is meant to be written directly. */
typedef struct {
    char path[SESSION_RECORDER_PATH_MAX];
    int frame_count;
    long tail_offset;
    int keyboard_cursor;
} session_recorder_t;

/* Starts a new, empty recording at `path`. The file is created, or an
 * existing one truncated, by the first recorded frame, not here: a session
 * that records nothing leaves no file. An empty `path`, or one of
 * SESSION_RECORDER_PATH_MAX characters or more, aborts. `keyboard_cursor`
 * (0 or 1) is the declaration written in the recording's head. */
void session_recorder_open(session_recorder_t *recorder, const char *path, int keyboard_cursor);

/* Adds one played frame, replayed or live alike. Returns 1 if the frame
 * landed, after which the file is a complete events.json; 0 if the append
 * failed, leaving the recording and the frame count unchanged. */
int session_recorder_record_frame(session_recorder_t *recorder, const p8_input_frame_t *frame);

/* Frames recorded so far, i.e. the 0-based index of the next one. A frame
 * whose append failed is not counted. */
int session_recorder_frame_count(const session_recorder_t *recorder);

/* Returns the struct to its pre-open state; the file on disk is left in
 * place. */
void session_recorder_close(session_recorder_t *recorder);

#endif
