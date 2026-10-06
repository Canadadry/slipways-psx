#include "devtools/replay_source.h"

#include <assert.h>
#include <stddef.h>

replay_source_status_t replay_source_open(replay_source_t *source, const char *path) {
    assert(source);

    source->path = path;
    source->next_frame_index = 0;
    p8_input_init(&source->recording);

    if (!path) {
        source->status = REPLAY_SOURCE_NONE;
        return source->status;
    }

    if (!p8_input_load(&source->recording, path)) {
        source->status = REPLAY_SOURCE_LOAD_FAILED;
        return source->status;
    }

    source->status = REPLAY_SOURCE_LOADED;
    return source->status;
}

int replay_source_frame_count(const replay_source_t *source) {
    assert(source);

    return source->recording.frame_count;
}

int replay_source_frames_served(const replay_source_t *source) {
    assert(source);

    return source->next_frame_index;
}

int replay_source_next_frame(replay_source_t *source, p8_input_frame_t *out_frame) {
    assert(source);
    assert(out_frame);

    if (source->next_frame_index >= source->recording.frame_count) {
        return 0;
    }

    *out_frame = source->recording.frames[source->next_frame_index];
    source->next_frame_index++;
    return 1;
}

void replay_source_close(replay_source_t *source) {
    assert(source);

    p8_input_free(&source->recording);
    source->status = REPLAY_SOURCE_NONE;
    source->path = NULL;
    source->next_frame_index = 0;
}
