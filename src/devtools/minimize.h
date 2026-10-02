#ifndef DEVTOOLS_MINIMIZE_H
#define DEVTOOLS_MINIMIZE_H

#include "p8/input.h"

/* The minimizer's mechanical pass, pure: copies the kept frames of
 * `frames` into `kept_frames` (room for `frame_count` frames), in order and
 * unmodified, and returns how many there are. `frame_count` must be at
 * least 1, and so is the result.
 *
 * Each frame is compared with the one before it in the input; frame 0,
 * which has none, is a transition if anything is pressed, a move otherwise.
 * - transition: mouse_buttons or the d-pad differs; always kept;
 * - idle: no transition, mouse still, no button nor d-pad pressed; always
 *   dropped;
 * - move: no transition, mouse moved, buttons and d-pad possibly held (a
 *   drag is one); of the moves of a segment between two transitions, a
 *   fifth, rounded down, is kept (none under five moves), evenly spaced,
 *   the last move among them; the transition carries its own position, so
 *   the click lands on the same spot;
 * - still hold (a button or the d-pad held, mouse still): always kept.
 * A kept frame that is idle relative to the kept frame before it (a path
 * that came back on itself) is dropped. If nothing is kept, the last frame
 * of the input is. */
int minimize_mechanical(const p8_input_frame_t *frames, int frame_count, p8_input_frame_t *kept_frames);

#endif
