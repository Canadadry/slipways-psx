#ifndef DEVTOOLS_SESSION_RECORDER_H
#define DEVTOOLS_SESSION_RECORDER_H

#include "p8/input.h"

#define SESSION_RECORDER_PATH_MAX 128

typedef struct {
    char path[SESSION_RECORDER_PATH_MAX];
    int frame_count;
    long tail_offset;
} session_recorder_t;

void session_recorder_open(session_recorder_t *recorder, const char *path);

int session_recorder_record_frame(session_recorder_t *recorder, const p8_input_frame_t *frame);

int session_recorder_frame_count(const session_recorder_t *recorder);

void session_recorder_close(session_recorder_t *recorder);

#endif
