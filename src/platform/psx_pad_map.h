#ifndef PLATFORM_PSX_PAD_MAP_H
#define PLATFORM_PSX_PAD_MAP_H

#include <stdint.h>

/* The pad word of psx_pad_read_pressed: PSn00bSDK's bit order, 1 = pressed. */
enum {
    PSX_PAD_HARDWARE_SELECT = 1 << 0,
    PSX_PAD_HARDWARE_L3 = 1 << 1,
    PSX_PAD_HARDWARE_R3 = 1 << 2,
    PSX_PAD_HARDWARE_START = 1 << 3,
    PSX_PAD_HARDWARE_UP = 1 << 4,
    PSX_PAD_HARDWARE_RIGHT = 1 << 5,
    PSX_PAD_HARDWARE_DOWN = 1 << 6,
    PSX_PAD_HARDWARE_LEFT = 1 << 7,
    PSX_PAD_HARDWARE_L2 = 1 << 8,
    PSX_PAD_HARDWARE_R2 = 1 << 9,
    PSX_PAD_HARDWARE_L1 = 1 << 10,
    PSX_PAD_HARDWARE_R1 = 1 << 11,
    PSX_PAD_HARDWARE_TRIANGLE = 1 << 12,
    PSX_PAD_HARDWARE_CIRCLE = 1 << 13,
    PSX_PAD_HARDWARE_CROSS = 1 << 14,
    PSX_PAD_HARDWARE_SQUARE = 1 << 15
};

/* The P8_PAD_* mask of a pad word; L3, R3, L2 and R2 set nothing, as keymap.default.cfg names none of them. */
int psx_pad_map_buttons(uint16_t pressed);

/* The bit of an uppercase button name as pad scripts and PCSX.CONSTS.PAD.BUTTON spell it; 0 for an unknown name. */
uint16_t psx_pad_map_hardware_bit(const char *name);

#endif
