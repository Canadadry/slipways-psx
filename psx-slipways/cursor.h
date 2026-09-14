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

/* Pure core: no statics, no I/O, zero PSX headers. Same active-low raw
 * bitmask convention input.h already established (0 = pressed).
 *
 * Chunk 1 scope only: computes the press-edge (pressed = (~buttons) &
 * prev_buttons, the same idiom psx-hello/main.c already uses inline with
 * its local prev_btn) and flips mouse_mode when Cross is in that edge.
 * Position fields are present and carried through unchanged -- movement is
 * chunk 2's scope. */
cursor_state_t cursor_step(cursor_state_t prev_state,
                            uint16_t prev_buttons,
                            uint16_t buttons);

void cursor_init(void);
void cursor_update(uint16_t buttons);
cursor_state_t cursor_get_state(void);

#endif
