/* See pico8_session_recorder.h for what this module is and, above all,
 * why this bookkeeping lives outside the raylib backend that used to
 * own it. */

#include "pico8_session_recorder.h"

#include <assert.h>
#include <stdio.h>

void pico8_session_recorder_open(pico8_session_recorder_t *recorder, const char *path) {
    assert(recorder);
    assert(path);

    int written = snprintf(recorder->path, sizeof(recorder->path), "%s", path);
    /* Truncation would silently record to a different file than the one
     * the caller named -- and, worse, one whose name no longer pairs
     * with the backend's marker file. A caller bug, caught here rather
     * than discovered when the recording cannot be found. */
    assert(written > 0 && (size_t)written < sizeof(recorder->path));

    recorder->frame_count = 0;
    /* 0 is pico8_input_append's documented "start a new recording":
     * the first recorded frame creates or truncates the file. */
    recorder->tail_offset = 0;
}

int pico8_session_recorder_record_frame(pico8_session_recorder_t *recorder, const pico8_input_frame_t *frame) {
    assert(recorder);
    assert(frame);

    if (!pico8_input_append(frame, recorder->path, &recorder->tail_offset)) {
        /* Dropped, not fatal: a live playtest outranks a frame of its
         * own recording. pico8_input_append leaves *tail_offset
         * untouched on failure, so the next frame takes the slot this
         * one never reached, and the count is left alone to match. */
        return 0;
    }

    recorder->frame_count++;
    return 1;
}

int pico8_session_recorder_frame_count(const pico8_session_recorder_t *recorder) {
    assert(recorder);

    return recorder->frame_count;
}

void pico8_session_recorder_close(pico8_session_recorder_t *recorder) {
    assert(recorder);

    recorder->path[0] = '\0';
    recorder->frame_count = 0;
    recorder->tail_offset = 0;
}
