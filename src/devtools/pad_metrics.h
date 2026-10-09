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
/* A press is judged on the snapshots after its frame to this many frames after its release, cut before a later
 * press edge of another button (G-6). */
#define PAD_METRICS_RELEASE_FRAMES 2
#define PAD_METRICS_PENDING_MAX 64
#define PAD_METRICS_LISTED_MAX 8192

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
    /* Outside both change tests; filled only for a judging pad_metrics_t. */
    entity_id_t last_acted;
    /* Direction bits whose cone is empty, in jump or slipway mode only. */
    int cone_empty;
} pad_metrics_view_t;

typedef enum { PAD_METRICS_PRESS, PAD_METRICS_REPEAT, PAD_METRICS_KIND_COUNT } pad_metrics_kind_t;

typedef enum {
    PAD_METRICS_LIVE,
    PAD_METRICS_DEAD_REQUIRED,
    PAD_METRICS_DEAD_OTHER,
    PAD_METRICS_VERDICT_COUNT
} pad_metrics_verdict_t;

/* A press or repeat whose window is still open; end is -1 until its release is seen. */
typedef struct {
    int frame;
    int end;
    pad_metrics_kind_t kind;
    int button;
    pad_metrics_mode_t mode;
    int required_if_dead;
    int moved;
    pad_metrics_view_t before;
} pad_metrics_pending_t;

typedef struct {
    int frame;
    pad_metrics_kind_t kind;
    int button;
    pad_metrics_mode_t mode;
    pad_metrics_verdict_t verdict;
} pad_metrics_listed_t;

typedef struct {
    int cheat_activations;
    int jump_dpad_dead_strict;
    int jump_dpad_dead_lenient;
    int circle_dead_other;
    int repeats_dead_required;
    int repeats_dead_other;
    int aim_ok;
    int ok;
} pad_metrics_section4_t;

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
    int judging;
    int repeats[PAD_METRICS_MODE_COUNT][PAD_METRICS_BUTTON_COUNT];
    int chords[PAD_METRICS_MODE_COUNT][PAD_METRICS_BUTTON_COUNT];
    int chord_frames;
    int verdicts[PAD_METRICS_KIND_COUNT][PAD_METRICS_MODE_COUNT][PAD_METRICS_BUTTON_COUNT][PAD_METRICS_VERDICT_COUNT];
    pad_metrics_pending_t pending[PAD_METRICS_PENDING_MAX];
    int pending_count;
    /* Every dead event and every repeat, in frame order once pad_metrics_finish ran. */
    pad_metrics_listed_t listed[PAD_METRICS_LISTED_MAX];
    int listed_count;
    int listed_dropped;
} pad_metrics_t;

/* Counts frames, presses and repeats only, as the generator does. */
void pad_metrics_init(pad_metrics_t *metrics, bump_zone_t *scratch);
/* Also judges every press and repeat, and reads the group facts into each snapshot. */
void pad_metrics_init_judging(pad_metrics_t *metrics, bump_zone_t *scratch);

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

/* pad_metrics_observe and pad_metrics_record on snapshots the caller built. */
void pad_metrics_begin(pad_metrics_t *metrics, const pad_metrics_view_t *view);
void pad_metrics_step(pad_metrics_t *metrics, int buttons, int cheat, const pad_metrics_view_t *after);
/* At the end of the file: judges every event still open on the frames seen. */
void pad_metrics_finish(pad_metrics_t *metrics);

int pad_metrics_verdict_count(const pad_metrics_t *metrics, pad_metrics_kind_t kind, pad_metrics_mode_t mode,
                              int button, pad_metrics_verdict_t verdict);
int pad_metrics_chord_count(const pad_metrics_t *metrics, pad_metrics_mode_t mode, int button);
/* Frames whose mode is neither title nor pause. */
int pad_metrics_in_game_frames(const pad_metrics_t *metrics);
/* 4 x aim frames < pad_metrics_in_game_frames; true with no aim frame. */
int pad_metrics_aim_ok(const pad_metrics_t *metrics);
pad_metrics_section4_t pad_metrics_section4(const pad_metrics_t *metrics);

int pad_metrics_mode_frames(const pad_metrics_t *metrics, pad_metrics_mode_t mode);
int pad_metrics_cheat_activations(const pad_metrics_t *metrics);

/* Frames and presses per mode and button, aim share, hold repeats, required no-ops, cheat activations; each line
 * after prefix. */
void pad_metrics_write_totals(const pad_metrics_t *metrics, FILE *out, const char *prefix);
/* After pad_metrics_finish (asserted). */
void pad_metrics_write_report(const pad_metrics_t *metrics, FILE *out);

#endif
