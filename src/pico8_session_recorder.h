/* The memory of the session currently being recorded -- where the
 * recording lives, how many frames it holds, where the next one goes --
 * and the one operation that moves it forward. Kept outside
 * pico8_raylib_backend.c, which includes raylib.h and can therefore be
 * neither built nor run by `make unit`, so that the continuity of a
 * resumed recording can be tested (tests/test_pico8_recording_
 * continuity.c). Nothing here includes raylib.h. */

#ifndef PICO8_SESSION_RECORDER_H
#define PICO8_SESSION_RECORDER_H

#include "pico8_input.h"

/* Room for the backend's tmp/events_recorded_<timestamp>.json and then
 * some; not a limit any caller is expected to reach. */
#define PICO8_SESSION_RECORDER_PATH_MAX 128

/* `path` is copied in, so the caller need not keep its buffer alive.
 * `frame_count` is how many frames have landed in the file -- equivalently
 * the 0-based index the next one will occupy -- and `tail_offset` is
 * pico8_input_append's resume point. Read `frame_count` through the
 * accessor below; no field is meant to be written directly. */
typedef struct {
    char path[PICO8_SESSION_RECORDER_PATH_MAX];
    int frame_count;
    long tail_offset;
} pico8_session_recorder_t;

/* Starts a new, empty recording at `path`. The file is created by the
 * first recorded frame, not here, so a session that plays nothing leaves
 * no file; an existing file at `path` is truncated by that first frame,
 * never appended to. `path` must be shorter than
 * PICO8_SESSION_RECORDER_PATH_MAX -- a longer one aborts. */
void pico8_session_recorder_open(pico8_session_recorder_t *recorder, const char *path);

/* Adds one played frame to the recording, replayed or live alike -- this
 * module never asks which, which is what keeps a resumed session's file
 * complete. Returns 1 if the frame landed, 0 if the append failed, in
 * which case the recording and the frame count are left exactly as they
 * were and the session carries on. After a successful call the file on
 * disk is already a complete, valid events.json. */
int pico8_session_recorder_record_frame(pico8_session_recorder_t *recorder, const pico8_input_frame_t *frame);

/* How many frames have been recorded, i.e. the 0-based index of the next
 * recorded frame in the file. A frame whose append failed is neither in
 * the file nor counted. */
int pico8_session_recorder_frame_count(const pico8_session_recorder_t *recorder);

/* Ends the recording and returns the struct to its pre-open state. Purely
 * in-memory: the file on disk is complete and is left in place. Optional
 * for correctness; it exists so a reused recorder cannot inherit the
 * previous session's tail offset. */
void pico8_session_recorder_close(pico8_session_recorder_t *recorder);

#endif
