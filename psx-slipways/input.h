#ifndef SLIPWAYS_INPUT_H
#define SLIPWAYS_INPUT_H

#include <stdint.h>

typedef struct {
	int      connected; /* 1 if PADTYPE::stat == 0 for port 1, else 0 */
	uint16_t buttons;   /* raw PADTYPE::btn bitmask, active-low, port 1;
	                       0xffff (nothing pressed) when disconnected --
	                       same convention psx-hello/main.c already uses.
	                       Test with the PAD_* macros from psxpad.h. */
} input_state_t;

void input_init(void);
void input_update(void);
input_state_t input_get_state(void);

#endif
