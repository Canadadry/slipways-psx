#ifndef DEVTOOLS_PAD_AUTHOR_H
#define DEVTOOLS_PAD_AUTHOR_H

#include "devtools/pad_metrics.h"
#include "game/input_step.h"
#include "p8/input.h"

#include <stdio.h>

#define PAD_AUTHOR_FRAME_BUDGET 7200
#define PAD_AUTHOR_ERROR_MAX 512

typedef struct {
    p8_input_frame_t frames[PAD_AUTHOR_FRAME_BUDGET];
    /* Also the index of the frame being run, for state_write_on_exit. */
    int frame_count;
    input_step_t step;
    const char *stop_label;
    int stopped;
    char error[PAD_AUTHOR_ERROR_MAX];
    pad_metrics_t metrics;
    int circle_pending;
    int circle_required;
    int next_circle_required;
    pad_metrics_view_t circle_before;
    /* NULL for no per-frame mode log; the caller opens and closes it. */
    FILE *mode_log;
} pad_author_t;

/* Call before cart_load: installs an empty frame as the dispatch input. */
void pad_author_init(pad_author_t *author);

/* Plays the script on a cart that has run cart_load and cart_init, one frame
 * at a time, as the replay tool would. With stop_label, the run ends at the
 * first `label <stop_label>` reached. Returns 1 on success; 0 with
 * author->error set to "<script>:<line>: frame <n>: <reason>". */
int pad_author_run(pad_author_t *author, const char *script_path, const char *stop_label);

/* Runs `;`-separated commands after the script, as one more script line each. Returns 1 or 0 as pad_author_run. */
int pad_author_run_line(pad_author_t *author, const char *commands);

#endif
