#include "devtools/replay_source.h"

#include <assert.h>
#include <stddef.h>

pico8_replay_source_status_t pico8_replay_source_open(pico8_replay_source_t *source, const char *path) {
    assert(source);

    source->path = path;
    source->next_frame_index = 0;
    p8_input_init(&source->recording);

    if (!path) {
        source->status = PICO8_REPLAY_SOURCE_NONE;
        return source->status;
    }

    /* Missing, unreadable, malformed and empty all collapse into one
     * outcome: pico8_input_load reports them the same way, nothing is
     * half-loaded, and the caller's message names the path, which is what
     * tells them apart for a human. */
    if (!pico8_input_load(&source->recording, path)) {
        source->status = PICO8_REPLAY_SOURCE_LOAD_FAILED;
        return source->status;
    }

    source->status = PICO8_REPLAY_SOURCE_LOADED;
    return source->status;
}

int pico8_replay_source_frame_count(const pico8_replay_source_t *source) {
    assert(source);

    return source->recording.frame_count;
}

int pico8_replay_source_frames_served(const pico8_replay_source_t *source) {
    assert(source);

    return source->next_frame_index;
}

int pico8_replay_source_next_frame(pico8_replay_source_t *source, p8_input_frame_t *out_frame) {
    assert(source);
    assert(out_frame);

    /* A NONE source has a frame_count of 0 and an index of 0, so it is
     * exhausted on its first call without any status check: "there never
     * was a recording" and "the recording ran out" are one code path. */
    if (source->next_frame_index >= source->recording.frame_count) {
        return 0;
    }

    *out_frame = source->recording.frames[source->next_frame_index];
    source->next_frame_index++;
    return 1;
}

void pico8_replay_source_close(pico8_replay_source_t *source) {
    assert(source);

    pico8_input_free(&source->recording);
    source->status = PICO8_REPLAY_SOURCE_NONE;
    source->path = NULL;
    source->next_frame_index = 0;
}
