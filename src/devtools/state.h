#ifndef DEVTOOLS_STATE_H
#define DEVTOOLS_STATE_H

#include <stdio.h>

void state_write(FILE *out);

void state_write_on_exit(const char *path, const int *frame);
void state_release(void);

#endif
