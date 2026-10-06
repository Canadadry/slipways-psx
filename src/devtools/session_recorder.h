#ifndef DEVTOOLS_SESSION_RECORDER_H
#define DEVTOOLS_SESSION_RECORDER_H

#include "game/save_state.h"
#include "p8/input.h"

#include <stdint.h>

#define SESSION_RECORDER_PATH_MAX 128

typedef struct {
    char path[SESSION_RECORDER_PATH_MAX];
    int frame_count;
    long tail_offset;
    uint8_t initial_save[SAVE_STATE_MAX_BYTES];
    int initial_save_count;
} session_recorder_t;

void session_recorder_open(session_recorder_t *recorder, const char *path);

/* Copies the bytes; a count of 0 means the session started with no save,
 * and the recording then carries no initial_save. Before the first frame. */
void session_recorder_set_initial_save(session_recorder_t *recorder, const uint8_t *bytes, int count);

int session_recorder_record_frame(session_recorder_t *recorder, const p8_input_frame_t *frame);

int session_recorder_frame_count(const session_recorder_t *recorder);

void session_recorder_close(session_recorder_t *recorder);

#endif
