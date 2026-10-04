#include "devtools/replay_control.h"

#include <assert.h>
#include <stdio.h>

/* The uncapped level (0) must stay last: replay_control_uncap and the top clamp use LEVEL_COUNT - 1. */
#define LEVEL_COUNT 5
static const int level_multipliers[LEVEL_COUNT] = { 1, 2, 4, 8, 0 };

#define DEFAULT_LEVEL 2

void replay_control_init(replay_control_t *control) {
    assert(control);
    control->level = DEFAULT_LEVEL;
}

void replay_control_faster(replay_control_t *control) {
    assert(control);
    if (control->level < LEVEL_COUNT - 1) {
        control->level++;
    }
}

void replay_control_slower(replay_control_t *control) {
    assert(control);
    if (control->level > 0) {
        control->level--;
    }
}

void replay_control_uncap(replay_control_t *control) {
    assert(control);
    control->level = LEVEL_COUNT - 1;
}

int replay_control_speed_multiplier(const replay_control_t *control) {
    assert(control);
    assert(control->level >= 0 && control->level < LEVEL_COUNT &&
           "level was written directly instead of through the ladder functions");
    return level_multipliers[control->level];
}

int replay_control_target_fps(const replay_control_t *control, int replay_active) {
    assert(control);

    if (!replay_active) {
        return REPLAY_CONTROL_LIVE_FPS;
    }

    return replay_control_speed_multiplier(control) * REPLAY_CONTROL_LIVE_FPS;
}

void replay_control_format_status(const replay_control_t *control, int frames_played, int frames_total, char *out, size_t out_size) {
    assert(control);
    assert(out);
    assert(out_size > 0);

    /* Widened before multiplying: frames_played * 100 overflows a 32-bit int past 21 474 836 frames. */
    int percent = 0;
    if (frames_total > 0) {
        percent = (int)(((long)frames_played * 100) / frames_total);
    }

    int multiplier = replay_control_speed_multiplier(control);
    if (multiplier == 0) {
        snprintf(out, out_size, "replay %d/%d (%d%%) speed max", frames_played, frames_total, percent);
    } else {
        snprintf(out, out_size, "replay %d/%d (%d%%) speed x%d", frames_played, frames_total, percent, multiplier);
    }
}
