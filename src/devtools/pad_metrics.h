#ifndef DEVTOOLS_PAD_METRICS_H
#define DEVTOOLS_PAD_METRICS_H

#include "engine/entity_class.h"
#include "engine/entity_id_type.h"
#include "engine/zone.h"
#include "game/input_step.h"
#include "game/pad_jump.h"
#include "p8/fixed.h"

#include <stdio.h>

#define PAD_METRICS_BUTTON_COUNT 12

typedef enum {
    PAD_METRICS_MODE_TITLE,
    PAD_METRICS_MODE_PAUSE,
    PAD_METRICS_MODE_PLACEMENT,
    PAD_METRICS_MODE_BAR,
    PAD_METRICS_MODE_TECH,
    PAD_METRICS_MODE_SCORE,
    PAD_METRICS_MODE_MENU,
    PAD_METRICS_MODE_JUMP,
    PAD_METRICS_MODE_AIM,
    PAD_METRICS_MODE_SLIPWAY,
    PAD_METRICS_MODE_COUNT
} pad_metrics_mode_t;

typedef struct {
    int has_economy;
    int pause_shown;
    int placing;
    int build_bar_open;
    int tech_open;
    int score_open;
    int panel_open;
    int aim_mode;
    int slipway_started;
} pad_metrics_flags_t;

typedef struct {
    pad_metrics_mode_t mode;
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
} pad_metrics_view_t;

typedef struct {
    int held[PAD_JUMP_DIRECTIONS];
} pad_metrics_hold_t;

typedef struct {
    /* The caller's; reset by every cone check. */
    bump_zone_t *scratch;
    pad_metrics_view_t view;
    int frames;
    int previous_buttons;
    pad_metrics_hold_t hold;
    int mode_frames[PAD_METRICS_MODE_COUNT];
    int presses[PAD_METRICS_MODE_COUNT][PAD_METRICS_BUTTON_COUNT];
    int hold_repeats;
    int required_noops;
    int cheat_activations;
} pad_metrics_t;

void pad_metrics_init(pad_metrics_t *metrics, bump_zone_t *scratch);

pad_metrics_flags_t pad_metrics_flags_read(const input_step_t *step);
/* First match wins, in the order of the enum; jump is what remains. */
pad_metrics_mode_t pad_metrics_classify(const pad_metrics_flags_t *flags);
pad_metrics_mode_t pad_metrics_mode(const input_step_t *step);
const char *pad_metrics_mode_name(pad_metrics_mode_t mode);
/* index is a bit number of the pad buttons, 0 to PAD_METRICS_BUTTON_COUNT - 1. */
const char *pad_metrics_button_name(int index);

pad_metrics_view_t pad_metrics_view_read(const input_step_t *step);
/* Mode, the ids, camera, panel, box and message box. */
int pad_metrics_navigation_changed(const pad_metrics_view_t *a, const pad_metrics_view_t *b);
/* navigation_changed, or credits, year, year ticks, node, probe or tech count. */
int pad_metrics_view_changed(const pad_metrics_view_t *a, const pad_metrics_view_t *b);

/* Steps the raw hold of each direction; returns the direction bits that repeat this frame: only in jump or slipway
 * mode, at 18 + 8k frames of hold, ignoring which direction pad_jump repeats. */
int pad_metrics_hold_step(pad_metrics_hold_t *hold, pad_metrics_mode_t mode, int buttons);

/* 1 when from is NULL or no jump candidate lies in the cone of direction from it. */
int pad_metrics_cone_empty(const pad_metrics_t *metrics, const entity_t *from, int direction);

/* Snapshots the state the next frame's input is read in. */
void pad_metrics_observe(pad_metrics_t *metrics, const input_step_t *step);
/* The last snapshot: from pad_metrics_observe, or after the last recorded frame. */
const pad_metrics_view_t *pad_metrics_view(const pad_metrics_t *metrics);
/* The bits of buttons newly down since the last recorded frame. */
int pad_metrics_pressed(const pad_metrics_t *metrics, int buttons);
/* After one frame ran with buttons: counts it in the mode of the last snapshot, then snapshots again. cheat is
 * input_step_run's return. */
void pad_metrics_record(pad_metrics_t *metrics, const input_step_t *step, int buttons, int cheat);
void pad_metrics_count_required_noop(pad_metrics_t *metrics);

int pad_metrics_mode_frames(const pad_metrics_t *metrics, pad_metrics_mode_t mode);
int pad_metrics_cheat_activations(const pad_metrics_t *metrics);

/* Frames and presses per mode and button, aim share, hold repeats, required no-ops, cheat activations; each line
 * after prefix. */
void pad_metrics_write_totals(const pad_metrics_t *metrics, FILE *out, const char *prefix);

#endif
