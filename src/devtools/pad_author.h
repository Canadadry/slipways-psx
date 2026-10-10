#ifndef DEVTOOLS_PAD_AUTHOR_H
#define DEVTOOLS_PAD_AUTHOR_H

#include "devtools/pad_metrics.h"
#include "game/bindings.h"
#include "game/input_step.h"
#include "game/save_state.h"
#include "p8/input.h"

#include <stdio.h>

#define PAD_AUTHOR_FRAME_BUDGET 7200
#define PAD_AUTHOR_ERROR_MAX 512
#define PAD_AUTHOR_PATH_MAX 256

typedef struct {
    p8_input_frame_t frames[PAD_AUTHOR_FRAME_BUDGET];
    int frame_count;
    input_step_t step;
    const char *stop_label;
    int stopped;
    char error[PAD_AUTHOR_ERROR_MAX];
    pad_metrics_t metrics;
    int back_pending;
    int back_required;
    int next_back_required;
    pad_metrics_view_t back_before;
    bindings_mode_t back_mode;
    FILE *mode_log;
    /* 0 unless from-record resumed record[0..record_count) before the first frame. */
    uint8_t record[SAVE_STATE_MAX_BYTES];
    int record_count;
    char record_path[PAD_AUTHOR_PATH_MAX];
} pad_author_t;

void pad_author_init(pad_author_t *author);

int pad_author_run(pad_author_t *author, const char *script_path, const char *stop_label);

int pad_author_run_line(pad_author_t *author, const char *commands);

#endif
