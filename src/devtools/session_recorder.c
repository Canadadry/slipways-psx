#include "devtools/session_recorder.h"

#include <assert.h>
#include <stdio.h>

void pico8_session_recorder_open(pico8_session_recorder_t *recorder, const char *path) {
    assert(recorder);
    assert(path);

    int written = snprintf(recorder->path, sizeof(recorder->path), "%s", path);
    assert(written > 0 && (size_t)written < sizeof(recorder->path) &&
           "a truncated path records to a file the caller did not name, and one that no longer pairs with the marker file");

    recorder->frame_count = 0;
    /* 0 is pico8_input_append's "start a new recording": the first frame
     * creates or truncates the file. */
    recorder->tail_offset = 0;
}

int pico8_session_recorder_record_frame(pico8_session_recorder_t *recorder, const p8_input_frame_t *frame) {
    assert(recorder);
    assert(frame);

    /* Appending, never rewriting: a frame must not cost more as the
     * session grows. The file must stay byte-identical to what
     * pico8_input_save writes for the same frames, or a resumed recording
     * replays to another state. */
    if (!pico8_input_append(frame, recorder->path, &recorder->tail_offset)) {
        /* Dropped, not fatal: a live playtest outranks a frame of its own
         * recording. append leaves *tail_offset untouched on failure, so
         * the next frame takes the slot this one never reached, and the
         * count is left alone to match. */
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
