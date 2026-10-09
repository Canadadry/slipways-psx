#ifndef DEVTOOLS_PAD_AUTHOR_H
#define DEVTOOLS_PAD_AUTHOR_H

#include "engine/entity_id_type.h"
#include "game/input_step.h"
#include "p8/fixed.h"
#include "p8/input.h"

#include <stdio.h>

#define PAD_AUTHOR_FRAME_BUDGET 7200
#define PAD_AUTHOR_ERROR_MAX 512
#define PAD_AUTHOR_BUTTON_COUNT 12

typedef enum {
    PAD_AUTHOR_MODE_TITLE,
    PAD_AUTHOR_MODE_PAUSE,
    PAD_AUTHOR_MODE_PLACEMENT,
    PAD_AUTHOR_MODE_BAR,
    PAD_AUTHOR_MODE_TECH,
    PAD_AUTHOR_MODE_SCORE,
    PAD_AUTHOR_MODE_MENU,
    PAD_AUTHOR_MODE_JUMP,
    PAD_AUTHOR_MODE_AIM,
    PAD_AUTHOR_MODE_SLIPWAY,
    PAD_AUTHOR_MODE_COUNT
} pad_author_mode_t;

typedef struct {
    pad_author_mode_t mode;
    entity_id_t hovered;
    entity_id_t slipway_start;
    entity_id_t slipway_target;
    entity_id_t build_icon;
    entity_id_t placing;
    entity_id_t focus;
    int camera_x;
    int camera_y;
    int panel_open;
    int box_open;
    int message_box;
    p8_fixed_t credits;
    p8_fixed_t year;
    p8_fixed_t year_ticks;
    int nodes;
    int probes;
    int techs;
} pad_author_view_t;

typedef struct {
    p8_input_frame_t frames[PAD_AUTHOR_FRAME_BUDGET];
    /* Also the index of the frame being run, for state_write_on_exit. */
    int frame_count;
    input_step_t step;
    int cheat_activations;
    const char *stop_label;
    int stopped;
    char error[PAD_AUTHOR_ERROR_MAX];
    int previous_buttons;
    int direction_held[4];
    int mode_frames[PAD_AUTHOR_MODE_COUNT];
    int presses[PAD_AUTHOR_MODE_COUNT][PAD_AUTHOR_BUTTON_COUNT];
    int hold_repeats;
    int required_noops;
    int circle_pending;
    int circle_required;
    int next_circle_required;
    pad_author_view_t circle_before;
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

const char *pad_author_mode_name(pad_author_mode_t mode);

/* The run's totals for step 14: frames and presses per mode and button, aim share, hold repeats, required no-ops. */
void pad_author_write_totals(const pad_author_t *author, FILE *out, const char *prefix);

#endif
