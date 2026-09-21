/* See pico8_replay_source.h for what this module is and why it exists
 * outside the raylib modules. */

#include "pico8_replay_source.h"

#include <assert.h>
#include <stddef.h>

pico8_replay_source_status_t pico8_replay_source_open(pico8_replay_source_t *source, const char *path) {
    assert(source);

    source->path = path;
    pico8_input_init(&source->recording);

    if (!path) {
        source->status = PICO8_REPLAY_SOURCE_NONE;
        return source->status;
    }

    /* pico8_input_load's own documented contract is exactly the hard
     * error this chunk needs -- 0 for missing, unreadable, malformed or
     * empty, with nothing half-loaded -- so the two failure shapes the
     * PRD names (fichier absent, fichier malforme) collapse into one
     * outcome here on purpose: the launch error is the same either way,
     * and the caller's message names the path, which is what actually
     * tells the two apart when a human reads it. */
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

void pico8_replay_source_close(pico8_replay_source_t *source) {
    assert(source);

    pico8_input_free(&source->recording);
    source->status = PICO8_REPLAY_SOURCE_NONE;
    source->path = NULL;
}
