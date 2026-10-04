#ifndef DEVTOOLS_REPLAY_SOURCE_H
#define DEVTOOLS_REPLAY_SOURCE_H

#include "p8/input.h"

typedef enum {
    REPLAY_SOURCE_NONE = 0,
    REPLAY_SOURCE_LOADED = 1,
    REPLAY_SOURCE_LOAD_FAILED = 2
} replay_source_status_t;

typedef struct {
    replay_source_status_t status;
    const char *path;
    p8_input_t recording;
    int next_frame_index;
} replay_source_t;

replay_source_status_t replay_source_open(replay_source_t *source, const char *path);

int replay_source_frame_count(const replay_source_t *source);

int replay_source_frames_served(const replay_source_t *source);

int replay_source_keyboard_cursor(const replay_source_t *source);

int replay_source_next_frame(replay_source_t *source, p8_input_frame_t *out_frame);

void replay_source_close(replay_source_t *source);

#endif
