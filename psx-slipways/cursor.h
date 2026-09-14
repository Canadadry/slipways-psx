#ifndef SLIPWAYS_CURSOR_H
#define SLIPWAYS_CURSOR_H

#include <stdint.h>

/* "Mouse mode" cursor state -- see docs/prd/triage/psx-mouse-mode-cursor.md
 * (mode + position) and docs/prd/triage/psx-simulated-click.md (click
 * events). Circle's press/held/release mirrors PICO-8's own bu()'s
 * jn/on/hm trio (pico8/lua.lua:204-208) -- see that PRD's "Implementation
 * Decisions" for the full source-cited rationale. */
typedef struct {
	int mouse_mode;     /* 0 = off, 1 = on */
	int x;              /* pixel-space, [0, SCREEN_XRES-1] */
	int y;              /* pixel-space, [0, SCREEN_YRES-1] */
	int click_pressed;  /* edge: Circle press this frame, mouse_mode on */
	int click_held;     /* level: Circle currently held, mouse_mode on */
	int click_released; /* edge: Circle release this frame, mouse_mode on */
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
 * [0, SCREEN_YRES-1]. No movement at all while mouse_mode is off.
 *
 * click_pressed/click_held/click_released are computed from the Circle bit
 * of the same prev_buttons/buttons pair, gated on the *resulting*
 * mouse_mode (same post-toggle-state convention the D-pad movement above
 * already uses): click_pressed is the Circle press edge, click_held is the
 * current Circle level, click_released is the Circle release edge. All
 * three are forced to 0 whenever mouse_mode ends up off that frame --
 * including the exact frame mode is toggled off mid-hold, which does not
 * synthesize a click_released for the interrupted hold. */
cursor_state_t cursor_step(cursor_state_t prev_state,
                            uint16_t prev_buttons,
                            uint16_t buttons);

void cursor_init(void);
void cursor_update(uint16_t buttons);
cursor_state_t cursor_get_state(void);

#endif
