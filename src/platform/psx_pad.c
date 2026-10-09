#include "platform/psx_pad.h"

#include <psxapi.h>
#include <psxpad.h>

enum { PAD_BUFFER_BYTES = 34 };

static uint8_t g_pad_buffers[2][PAD_BUFFER_BYTES];

void psx_pad_start(void) {
    InitPAD(g_pad_buffers[0], PAD_BUFFER_BYTES, g_pad_buffers[1], PAD_BUFFER_BYTES);
    StartPAD();
    ChangeClearPAD(0);
}

uint16_t psx_pad_read_pressed(void) {
    const PADTYPE *pad = (const PADTYPE *)g_pad_buffers[0];
    if (pad->stat != 0) return 0;
    return (uint16_t)~pad->btn;
}
