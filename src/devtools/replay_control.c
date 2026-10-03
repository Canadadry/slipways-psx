#include "devtools/replay_control.h"

#include <assert.h>
#include <stdio.h>

/* The ladder, in order: multiples of the game's 60 FPS, never raw frame
 * rates. 0 is the uncapped level and is last, which makes "clamped at the
 * top" and "uncapped" the same boundary. */
#define LEVEL_COUNT 5
static const int level_multipliers[LEVEL_COUNT] = { 1, 2, 4, 8, 0 };

/* 4x. */
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

    /* Checked before the level is even consulted, so no future edit to
     * the ladder can change what a live frame runs at. */
    if (!replay_active) {
        return REPLAY_CONTROL_LIVE_FPS;
    }

    /* 0 * 60 == 0, and 0 is what SetTargetFPS treats as "no wait": the
     * uncapped level needs no special case here. */
    return replay_control_speed_multiplier(control) * REPLAY_CONTROL_LIVE_FPS;
}

void replay_control_format_status(const replay_control_t *control, int frames_played, int frames_total, char *out, size_t out_size) {
    assert(control);
    assert(out);
    assert(out_size > 0);

    /* Widened before multiplying: a long session is tens of thousands of
     * frames. A total of zero prints 0% rather than dividing by it -- a
     * progress line must not kill the session it is reassuring the player
     * about. */
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
