#include "devtools/minimize.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef enum { KIND_TRANSITION, KIND_IDLE, KIND_MOVE, KIND_HOLD } frame_kind_t;

static int same_buttons(const p8_input_frame_t *a, const p8_input_frame_t *b) {
    return a->mouse_buttons == b->mouse_buttons && memcmp(a->dpad, b->dpad, sizeof a->dpad) == 0;
}

static int nothing_pressed(const p8_input_frame_t *f) {
    static const int released[6];
    return f->mouse_buttons == 0 && memcmp(f->dpad, released, sizeof released) == 0;
}

static int same_position(const p8_input_frame_t *a, const p8_input_frame_t *b) {
    return a->mouse_x == b->mouse_x && a->mouse_y == b->mouse_y;
}

static frame_kind_t kind_after(const p8_input_frame_t *prev, const p8_input_frame_t *f) {
    if (!same_buttons(prev, f)) {
        return KIND_TRANSITION;
    }
    if (!same_position(prev, f)) {
        return KIND_MOVE;
    }
    return nothing_pressed(f) ? KIND_IDLE : KIND_HOLD;
}

/* Garde m / 5 des m déplacements, espacés régulièrement, le dernier
 * compris dès qu'on en garde un. */
static void keep_moves(const int *moves, int m, char *keep) {
    int k = m / 5;
    for (int j = 0; j < k; j++) {
        keep[moves[m - 1 - (int)((long)j * m / k)]] = 1;
    }
}

int minimize_mechanical(const p8_input_frame_t *frames, int frame_count, p8_input_frame_t *kept_frames) {
    assert(frames != NULL && kept_frames != NULL && frame_count >= 1);
    char *keep = calloc((size_t)frame_count, 1);
    int *moves = malloc(sizeof *moves * (size_t)frame_count);
    assert(keep != NULL && moves != NULL);

    int m = 0;
    for (int i = 0; i < frame_count; i++) {
        /* La frame 0 pose la position de la souris : un déplacement, sauf
         * si elle appuie déjà. */
        frame_kind_t kind = i == 0 ? (nothing_pressed(&frames[0]) ? KIND_MOVE : KIND_TRANSITION)
                                   : kind_after(&frames[i - 1], &frames[i]);
        switch (kind) {
        case KIND_TRANSITION:
            keep_moves(moves, m, keep);
            m = 0;
            keep[i] = 1;
            break;
        case KIND_MOVE:
            moves[m++] = i;
            break;
        case KIND_HOLD:
            keep[i] = 1;
            break;
        case KIND_IDLE:
            break;
        }
    }
    keep_moves(moves, m, keep);

    int n = 0;
    for (int i = 0; i < frame_count; i++) {
        if (keep[i] && !(n > 0 && kind_after(&kept_frames[n - 1], &frames[i]) == KIND_IDLE)) {
            kept_frames[n++] = frames[i];
        }
    }
    if (n == 0) {
        kept_frames[n++] = frames[frame_count - 1];
    }
    free(moves);
    free(keep);
    return n;
}
