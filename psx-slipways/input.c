#include <stdint.h>
#include <psxpad.h>
#include <psxapi.h>

#include "input.h"

static char pad_buff[2][34];

static int     g_pad_connected;
static int32_t g_pad_buttons;

void input_init(void) {
	InitPAD(pad_buff[0], 34, pad_buff[1], 34);
	StartPAD();
	ChangeClearPAD(0);
}

void input_update(void) {
	PADTYPE *pad = (PADTYPE *) pad_buff[0];

	g_pad_connected = (pad->stat == 0);
	g_pad_buttons = g_pad_connected ? pad->btn : 0xffff;
}

input_state_t input_get_state(void) {
	input_state_t state;
	state.connected = g_pad_connected;
	state.buttons = (uint16_t) g_pad_buttons;
	return state;
}
