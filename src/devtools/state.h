#ifndef DEVTOOLS_STATE_H
#define DEVTOOLS_STATE_H

#include <stdio.h>

#include "game/save_state.h"

void state_write(FILE *out);

/* Reads only *state, which came out of a save_state_decode that returned 1:
   its difficulty, clock year, resources credits and count nodes lines in
   state_write's format, each after prefix. */
void state_write_save_summary(FILE *out, const save_state_t *state, const char *prefix);

void state_write_on_exit(const char *path, const int *frame);
void state_release(void);

#endif
