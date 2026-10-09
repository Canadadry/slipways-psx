#include "platform/psx_pad.h"

#include "platform/psx_pad_map.h"

#include <psxapi.h>
#include <psxpad.h>
#include <string.h>

#define BIT_CHECK(button) typedef char button##_bit_check[(int)PSX_PAD_HARDWARE_##button == (int)PAD_##button ? 1 : -1]
BIT_CHECK(SELECT);
BIT_CHECK(L3);
BIT_CHECK(R3);
BIT_CHECK(START);
BIT_CHECK(UP);
BIT_CHECK(RIGHT);
BIT_CHECK(DOWN);
BIT_CHECK(LEFT);
BIT_CHECK(L2);
BIT_CHECK(R2);
BIT_CHECK(L1);
BIT_CHECK(R1);
BIT_CHECK(TRIANGLE);
BIT_CHECK(CIRCLE);
BIT_CHECK(CROSS);
BIT_CHECK(SQUARE);

enum { PAD_BUFFER_BYTES = 34 };

static uint8_t g_pad_buffers[2][PAD_BUFFER_BYTES];

void psx_pad_start(void) {
    InitPAD(g_pad_buffers[0], PAD_BUFFER_BYTES, g_pad_buffers[1], PAD_BUFFER_BYTES);
    StartPAD();
    ChangeClearPAD(0);
    /* Until the first poll, at the next vsync, the zeroes left here read as every button held: 0xff reads as no pad. */
    memset(g_pad_buffers, 0xff, sizeof g_pad_buffers);
}

uint16_t psx_pad_read_pressed(void) {
    const PADTYPE *pad = (const PADTYPE *)g_pad_buffers[0];
    if (pad->stat != 0) return 0;
    return (uint16_t)~pad->btn;
}
