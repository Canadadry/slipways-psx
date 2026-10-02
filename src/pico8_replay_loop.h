#ifndef PICO8_REPLAY_LOOP_H
#define PICO8_REPLAY_LOOP_H

#include "pico8_cheat.h"
#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_mem.h"

/* The engine keeps pointers to mem, gfx and input: `r` must live as long as
 * the replay and never be copied. */
typedef struct {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_input_t input;
    pico8_cheat_t cheat;
} pico8_replay_loop_t;

/* Loads gfx and map, the inputs of `events_path`, then boots the cart on the
 * title screen; an unreadable file kills the process through assert. */
void pico8_replay_loop_open(pico8_replay_loop_t *r, const char *events_path);
int pico8_replay_loop_frames(const pico8_replay_loop_t *r);
/* Plays frame `i`, cheat included, and steps the inputs except after the
 * last one. Frames are played in order, from 0 to frames-1. */
void pico8_replay_loop_play_frame(pico8_replay_loop_t *r, int i);
void pico8_replay_loop_close(pico8_replay_loop_t *r);

#endif
