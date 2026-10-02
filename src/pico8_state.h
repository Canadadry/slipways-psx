#ifndef PICO8_STATE_H
#define PICO8_STATE_H

#include <stdio.h>

/* Writes the current engine state to `out`, one line
 * "<section> <key> <values...>" per fact. Read-only: allocates in no game
 * zone and calls nothing in the cart that writes. */
void pico8_state_write(FILE *out);

/* Until pico8_state_release(), any process exit, pico8_fatal included,
 * writes the state to `path` after a "stopped pico8_fatal frame <*frame>"
 * line. `path` and `frame` must live until exit. */
void pico8_state_write_on_exit(const char *path, const int *frame);
void pico8_state_release(void);

#endif
