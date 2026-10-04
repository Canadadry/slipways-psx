/* How fast a replay runs, and the progress line shown while it does.
 * Abbreviating a replay may only shorten the wait between frames, never
 * skip one, or the state reached is no longer the recorded one. */

#ifndef DEVTOOLS_REPLAY_CONTROL_H
#define DEVTOOLS_REPLAY_CONTROL_H

#include <stddef.h>

#define REPLAY_CONTROL_LIVE_FPS 60

/* Big enough for any line replay_control_format_status produces; that
 * function truncates rather than overflows anyway. */
#define REPLAY_CONTROL_STATUS_MAX 64

/* `level` is an index into a fixed ladder: write it only through the
 * functions below. */
typedef struct {
    int level;
} replay_control_t;

/* Starts at 4x. */
void replay_control_init(replay_control_t *control);

/* One step up / down the ladder, clamped at both ends, never wrapping.
 * The slow end is game speed: the replay is never slower than live. */
void replay_control_faster(replay_control_t *control);
void replay_control_slower(replay_control_t *control);

/* Jumps to the uncapped level; replay_control_slower steps back down from
 * it. */
void replay_control_uncap(replay_control_t *control);

/* The current level as a multiple of the game's 60 FPS -- 1, 2, 4 or 8 --
 * or 0 for the uncapped level. */
int replay_control_speed_multiplier(const replay_control_t *control);

/* The number to hand SetTargetFPS. With `replay_active` zero, always
 * REPLAY_CONTROL_LIVE_FPS whatever the level, which is not reset. With it
 * non-zero, the current level's cap, or 0 -- raylib's "do not wait between
 * frames" -- at the uncapped level. */
int replay_control_target_fps(const replay_control_t *control, int replay_active);

/* Writes "replay 1234/9589 (12%) speed x4", or "... speed max" at the
 * uncapped level, always NUL-terminated and truncated to `out_size`.
 * `frames_played` is a 1-based count (1/N on the first frame), not the
 * 0-based index the marker file writes. A `frames_total` of zero prints
 * 0%. */
void replay_control_format_status(const replay_control_t *control, int frames_played, int frames_total, char *out, size_t out_size);

#endif
