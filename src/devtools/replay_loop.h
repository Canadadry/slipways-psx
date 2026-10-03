#ifndef DEVTOOLS_REPLAY_LOOP_H
#define DEVTOOLS_REPLAY_LOOP_H

#include "game/cheat.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/mem.h"

/* The engine keeps pointers to memory, graphics and input: `r` must live as
 * long as the replay and never be copied. */
typedef struct {
    p8_memory_t memory;
    p8_graphics_t graphics;
    p8_input_t input;
    cheat_t cheat;
} replay_loop_t;

/* Loads gfx and map, the inputs of `events_path`, then boots the cart on the
 * title screen; an unreadable file kills the process through assert. */
void replay_loop_open(replay_loop_t *r, const char *events_path);
int replay_loop_frames(const replay_loop_t *r);
/* Plays frame `i`, cheat included, and steps the inputs except after the
 * last one. Frames are played in order, from 0 to frames-1. */
void replay_loop_play_frame(replay_loop_t *r, int i);
void replay_loop_close(replay_loop_t *r);

#endif
