/* How fast a replay runs, and what it tells the player while it does.
 * Kept outside the raylib modules, which include raylib.h and can be
 * neither built nor run here, so the speed ladder and the progress line
 * are pinned to exact values by tests/test_replay_control.c.
 * Nothing here includes raylib.h.
 *
 * Abbreviating a replay may only shorten the wait between frames, never
 * skip one, or the state reached is no longer the recorded one. This
 * module is incapable of expressing a skip: it never sees a frame, and
 * its whole output is a number for SetTargetFPS. */

#ifndef DEVTOOLS_REPLAY_CONTROL_H
#define DEVTOOLS_REPLAY_CONTROL_H

#include <stddef.h>

/* The game's own frame rate, and the one the player gets back the instant
 * the recording runs out. */
#define REPLAY_CONTROL_LIVE_FPS 60

/* Big enough for the longest line format_status can produce; that
 * function truncates rather than overflowing, so this is a convenience
 * for callers, not a contract they can violate. */
#define REPLAY_CONTROL_STATUS_MAX 64

/* The replay's speed setting and nothing else: one index into a fixed
 * ladder. Whether a replay is running at all is the driver's question,
 * answered per frame by pico8_replay_source_next_frame and passed in
 * below rather than mirrored here. Read through the accessors; `level` is
 * not meant to be written directly. */
typedef struct {
    int level;
} replay_control_t;

/* Starts at 4x: a thirty-minute recording comes back in under eight
 * minutes, still slow enough to watch something go wrong. */
void replay_control_init(replay_control_t *control);

/* One step up / down the ladder, clamped at both ends rather than
 * wrapping: one extra tap on "faster" must never drop the replay back to
 * the slowest setting. The slow end stops at game speed -- this mechanism
 * shortens the wait between frames, it never lengthens it. */
void replay_control_faster(replay_control_t *control);
void replay_control_slower(replay_control_t *control);

/* Jumps straight to the uncapped level from whatever level is current.
 * Not a one-way door: slower() steps back down from it. */
void replay_control_uncap(replay_control_t *control);

/* The current level as a multiple of the game's 60 FPS -- 1, 2, 4 or 8 --
 * or 0 for the uncapped level, which is not a multiple of anything. */
int replay_control_speed_multiplier(const replay_control_t *control);

/* The number to hand SetTargetFPS right now. With `replay_active` zero --
 * the recording is exhausted and the player has the controls -- this is
 * always PICO8_REPLAY_CONTROL_LIVE_FPS, whatever speed the replay was
 * left on, so no driver can forget to reset the cap; the level itself is
 * deliberately not reset. With it non-zero, it is the current level's
 * cap, or 0, which is raylib's own "do not wait between frames" rather
 * than a sentinel this port invented. */
int replay_control_target_fps(const replay_control_t *control, int replay_active);

/* Writes the progress line into `out`, always NUL-terminated and
 * truncated rather than overflowed if `out_size` is too small: "replay
 * 1234/9589 (12%) speed x4", or "... speed max" at the uncapped level,
 * which prints a word because "x0" would read as "stopped".
 * `frames_played` is a 1-based count, so the line reads 1/N on the first
 * frame and N/N on the last -- deliberately not the 0-based index the
 * marker file writes. A `frames_total` of zero prints 0% instead of
 * dividing by it. */
void replay_control_format_status(const replay_control_t *control, int frames_played, int frames_total, char *out, size_t out_size);

#endif
