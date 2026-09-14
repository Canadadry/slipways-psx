#ifndef SLIPWAYS_CURSOR_H
#define SLIPWAYS_CURSOR_H

#include <stdint.h>

/* "Mouse mode" cursor state -- see docs/prd/triage/psx-mouse-mode-cursor.md.
 * Deliberately minimal (mode + position, nothing else) so step 6's own
 * click/press-release semantics decision isn't pre-baked by this shape. */
typedef struct {
	int mouse_mode; /* 0 = off, 1 = on */
	int x;          /* pixel-space, [0, SCREEN_XRES-1] */
	int y;          /* pixel-space, [0, SCREEN_YRES-1] */
} cursor_state_t;

/* Per-frame D-pad movement step, pixels/frame. Matches psx-hello/main.c's
 * own CURSOR_SPEED -- a v1 UX/tuning choice, not a slipways-mechanics.md
 * traceable game-design constant (see PRD). */
#define CURSOR_STEP 2

/* Pure core: no statics, no I/O, zero PSX headers. Same active-low raw
 * bitmask convention input.h already established (0 = pressed).
 *
 * Computes the press-edge (pressed = (~buttons) & prev_buttons, the same
 * idiom psx-hello/main.c already uses inline with its local prev_btn) and
 * flips mouse_mode when Cross is in that edge. While the *resulting*
 * mouse_mode is on, each held D-pad direction (tested against the current
 * buttons, for continuous per-frame movement) nudges x/y by CURSOR_STEP;
 * all four directions are summed before clamping (diagonal movement isn't
 * double-clamped), then both axes are clamped to [0, SCREEN_XRES-1] /
 * [0, SCREEN_YRES-1]. No movement at all while mouse_mode is off. */
cursor_state_t cursor_step(cursor_state_t prev_state,
                            uint16_t prev_buttons,
                            uint16_t buttons);

void cursor_init(void);
void cursor_update(uint16_t buttons);
cursor_state_t cursor_get_state(void);

#endif
