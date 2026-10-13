#ifndef DEVTOOLS_SHORT_RUN_CUT_H
#define DEVTOOLS_SHORT_RUN_CUT_H

#include "devtools/draw_timing.h"
#include "p8/input.h"

#include <stddef.h>
#include <stdint.h>

enum {
    SHORT_RUN_CUT_NAME_CAP = 32,
    SHORT_RUN_CUT_ERROR_CAP = 160,
    SHORT_RUN_CUT_MAP_CAP = 128,
    SHORT_RUN_CUT_FRAMES_CAP = DRAW_TIMING_CAP,
    SHORT_RUN_CUT_TITLE_FRAMES = 13,
    SHORT_RUN_CUT_LEAD = 9
};

typedef enum { SHORT_RUN_CUT_PREFIX, SHORT_RUN_CUT_RESUME } short_run_cut_kind_t;

/* A psx/short_runs.txt line, frames of the full recording: "<name> prefix <first> <last> <end>"
 * or "<name> resume <first> <last>". */
typedef struct {
    char name[SHORT_RUN_CUT_NAME_CAP];
    short_run_cut_kind_t kind;
    int window_first;
    int window_last;
    int end;
} short_run_cut_entry_t;

/* The record the game held in storage after the frame that wrote it. */
typedef struct {
    int frame;
    const uint8_t *bytes;
    int count;
} short_run_cut_save_t;

/* Run frame first_game_frame + k plays full frame source_first + k; earlier run frames are never
 * compared. reference_last is the full frame matching the run's last frame. */
typedef struct {
    p8_input_frame_t frames[SHORT_RUN_CUT_FRAMES_CAP];
    int frame_count;
    /* -1 for a prefix run, which has no initial_save. */
    int save_index;
    int cut;
    int first_game_frame;
    int source_first;
    int reference_last;
} short_run_cut_plan_t;

/* Returns 1 with *entry filled, 0 with error naming the fault. Blank and '#' lines are the
 * caller's to skip. */
int short_run_cut_parse_entry(const char *line, short_run_cut_entry_t *entry, char error[SHORT_RUN_CUT_ERROR_CAP]);

/* saves in increasing frame order. Returns 1 with *plan filled, 0 with error naming the entry
 * when no save is eligible, the run ends past the recording or would pass SHORT_RUN_CUT_FRAMES_CAP
 * frames. */
int short_run_cut_plan(const p8_input_frame_t *full, int full_count, const short_run_cut_save_t *saves,
                       int save_count, const short_run_cut_entry_t *entry, short_run_cut_plan_t *plan,
                       char error[SHORT_RUN_CUT_ERROR_CAP]);

/* "cut <W> first-game-frame 13 source-first <W+1> window <first> <last>\n", or, for a prefix run,
 * "cut none first-game-frame 0 source-first 0 window <first> <last>\n" (the identity). */
void short_run_cut_format_map(const short_run_cut_entry_t *entry, const short_run_cut_plan_t *plan,
                              char line[SHORT_RUN_CUT_MAP_CAP]);

#endif
