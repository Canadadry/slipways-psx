#ifndef DEVTOOLS_STATE_H
#define DEVTOOLS_STATE_H

#include <stdio.h>

/* Writes the current engine state to `out`, one line
 * "<section> <key> <values...>" per fact. Read-only: allocates in no game
 * zone and calls nothing in the cart that writes. */
void state_write(FILE *out);

/* Until state_release(), any process exit, p8_fatal included, writes the
 * state to `path` after a "stopped p8_fatal frame <*frame>" line. `path`
 * and `frame` must live until exit. */
void state_write_on_exit(const char *path, const int *frame);
void state_release(void);

#endif
