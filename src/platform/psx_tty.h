#ifndef PLATFORM_PSX_TTY_H
#define PLATFORM_PSX_TTY_H

#include "devtools/state_format.h"

void psx_tty_write(const char *text);

/* Formats with the SDK's vsnprintf, the formatter the state uses, then writes
   the text; a line longer than 255 characters is cut. */
void psx_tty_printf(const char *format, ...) __attribute__((format(printf, 1, 2)));

/* Writes each state line behind "psx-state: ", the prefix
   tools/psx_state_compare.sh extracts. */
state_sink_t psx_tty_state_sink(void);

#endif
