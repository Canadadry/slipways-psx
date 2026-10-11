#ifndef DEVTOOLS_EMBEDDED_REPLAY_H
#define DEVTOOLS_EMBEDDED_REPLAY_H

#include "p8/input.h"

#include <stdint.h>

/* Defined by the C source that src/cmd/embed_replay.c writes from one recording;
 * a binary links exactly one such source. The frames are never empty;
 * embedded_replay_initial_save_count is 0 when the recording has no
 * initial_save, and the storage is then to be erased, as the PC replay does.
 * The recording starts on the title and plays the default bindings. */

extern const char embedded_replay_name[];
extern const int embedded_replay_frame_count;
extern const p8_input_frame_t embedded_replay_frames[];
extern const int embedded_replay_initial_save_count;
extern const uint8_t embedded_replay_initial_save[];
extern const int embedded_replay_attract_after;

#endif
