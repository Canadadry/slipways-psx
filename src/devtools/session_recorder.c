#include "devtools/session_recorder.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void session_recorder_open(session_recorder_t *recorder, const char *path) {
    assert(recorder);
    assert(path);

    int written = snprintf(recorder->path, sizeof(recorder->path), "%s", path);
    assert(written > 0 && (size_t)written < sizeof(recorder->path) &&
           "a truncated path records to a file the caller did not name, and one that no longer pairs with the marker file");

    recorder->frame_count = 0;
    recorder->tail_offset = 0;
    recorder->initial_save_count = 0;
}

void session_recorder_set_initial_save(session_recorder_t *recorder, const uint8_t *bytes, int count) {
    assert(recorder);
    assert(recorder->frame_count == 0 && "initial_save is written into the header with the first frame");
    assert(count >= 0 && count <= SAVE_STATE_MAX_BYTES);
    assert(count == 0 || bytes);

    if (count > 0) {
        memcpy(recorder->initial_save, bytes, (size_t)count);
    }
    recorder->initial_save_count = count;
}

/* Caller frees; NULL when there is no save or no memory. */
static char *format_initial_save_member(const session_recorder_t *recorder) {
    static const char key[] = "\"initial_save\": \"";
    static const char hex_digits[] = "0123456789abcdef";
    if (recorder->initial_save_count == 0) {
        return NULL;
    }
    size_t key_length = strlen(key);
    char *member = malloc(key_length + 2 * (size_t)recorder->initial_save_count + 2);
    if (!member) {
        return NULL;
    }
    memcpy(member, key, key_length);
    char *digit = member + key_length;
    for (int i = 0; i < recorder->initial_save_count; i++) {
        *digit++ = hex_digits[recorder->initial_save[i] >> 4];
        *digit++ = hex_digits[recorder->initial_save[i] & 0x0f];
    }
    *digit++ = '"';
    *digit = '\0';
    return member;
}

int session_recorder_record_frame(session_recorder_t *recorder, const p8_input_frame_t *frame) {
    assert(recorder);
    assert(frame);

    char *member = NULL;
    if (recorder->frame_count == 0 && recorder->initial_save_count > 0) {
        member = format_initial_save_member(recorder);
        if (!member) {
            return 0;
        }
    }
    int appended = p8_input_append(frame, recorder->path, &recorder->tail_offset, member);
    free(member);
    if (!appended) {
        /* Dropped, not fatal: the count must stay unchanged, since the marker file indexes frames by it. */
        return 0;
    }

    recorder->frame_count++;
    return 1;
}

int session_recorder_frame_count(const session_recorder_t *recorder) {
    assert(recorder);

    return recorder->frame_count;
}

void session_recorder_close(session_recorder_t *recorder) {
    assert(recorder);

    recorder->path[0] = '\0';
    recorder->frame_count = 0;
    recorder->tail_offset = 0;
    recorder->initial_save_count = 0;
}
