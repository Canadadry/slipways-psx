/* See pico8_replay_control.h for what this module is, why the default
 * speed is what it is, and why a frame-rate cap is the only mechanism
 * it can express. */

#include "pico8_replay_control.h"

#include <assert.h>
#include <stdio.h>

/* The ladder, in order. Doubling rather than a linear ramp so that
 * every step is a real, noticeable change and four levels span 1x to
 * 8x; 0 is the uncapped level and is last, which is what makes
 * "clamped at the top" and "uncapped" the same boundary. Values are
 * multiples of the game's own 60 FPS target, never raw frame rates --
 * see the header. */
#define LEVEL_COUNT 5
static const int k_multiplier[LEVEL_COUNT] = { 1, 2, 4, 8, 0 };

/* 4x: a thirty-minute recording comes back in under eight minutes, and
 * the game is still running slowly enough for a human to recognise the
 * moment something went wrong (user story 2). See the header for the
 * arbitration this implements. */
#define DEFAULT_LEVEL 2

void pico8_replay_control_init(pico8_replay_control_t *control) {
    assert(control);
    control->level = DEFAULT_LEVEL;
}

void pico8_replay_control_faster(pico8_replay_control_t *control) {
    assert(control);
    if (control->level < LEVEL_COUNT - 1) {
        control->level++;
    }
}

void pico8_replay_control_slower(pico8_replay_control_t *control) {
    assert(control);
    if (control->level > 0) {
        control->level--;
    }
}

void pico8_replay_control_uncap(pico8_replay_control_t *control) {
    assert(control);
    control->level = LEVEL_COUNT - 1;
}

int pico8_replay_control_speed_multiplier(const pico8_replay_control_t *control) {
    assert(control);
    /* The only way to be outside the ladder is to have written `level`
     * directly, which the header says not to do; caught here rather
     * than read out of bounds. */
    assert(control->level >= 0 && control->level < LEVEL_COUNT);
    return k_multiplier[control->level];
}

int pico8_replay_control_target_fps(const pico8_replay_control_t *control, int replay_active) {
    assert(control);

    /* The one branch that makes forgetting to reset the cap impossible
     * rather than merely unlikely -- see the header. Checked before the
     * level is even consulted, so no future edit to the ladder can
     * change what a live frame runs at. */
    if (!replay_active) {
        return PICO8_REPLAY_CONTROL_LIVE_FPS;
    }

    /* 0 * 60 == 0, and 0 is exactly what SetTargetFPS treats as "no
     * wait": the uncapped level needs no special case here, which is
     * why it is spelled 0 in the ladder rather than as a named
     * sentinel. */
    return pico8_replay_control_speed_multiplier(control) * PICO8_REPLAY_CONTROL_LIVE_FPS;
}

void pico8_replay_control_format_status(const pico8_replay_control_t *control, int frames_played, int frames_total, char *out, size_t out_size) {
    assert(control);
    assert(out);
    assert(out_size > 0);

    /* Widened before multiplying: a long session is tens of thousands
     * of frames, and frames_played * 100 has no business being the
     * place this overflows. A total of zero prints 0% rather than
     * dividing by it -- a progress line must not be able to kill the
     * session it is reassuring the player about. */
    int percent = 0;
    if (frames_total > 0) {
        percent = (int)(((long)frames_played * 100) / frames_total);
    }

    int multiplier = pico8_replay_control_speed_multiplier(control);
    if (multiplier == 0) {
        snprintf(out, out_size, "replay %d/%d (%d%%) speed max", frames_played, frames_total, percent);
    } else {
        snprintf(out, out_size, "replay %d/%d (%d%%) speed x%d", frames_played, frames_total, percent, multiplier);
    }
}
