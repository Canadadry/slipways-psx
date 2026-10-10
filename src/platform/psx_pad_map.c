#include "platform/psx_pad_map.h"

#include "p8/input.h"

#include <string.h>

typedef struct {
    const char *name;
    uint16_t hardware;
    int pad_buttons;
} psx_pad_map_entry_t;

static const psx_pad_map_entry_t entries[] = {
    { "SELECT", PSX_PAD_HARDWARE_SELECT, P8_PAD_SELECT },
    { "L3", PSX_PAD_HARDWARE_L3, 0 },
    { "R3", PSX_PAD_HARDWARE_R3, 0 },
    { "START", PSX_PAD_HARDWARE_START, P8_PAD_START },
    { "UP", PSX_PAD_HARDWARE_UP, P8_PAD_UP },
    { "RIGHT", PSX_PAD_HARDWARE_RIGHT, P8_PAD_RIGHT },
    { "DOWN", PSX_PAD_HARDWARE_DOWN, P8_PAD_DOWN },
    { "LEFT", PSX_PAD_HARDWARE_LEFT, P8_PAD_LEFT },
    { "L2", PSX_PAD_HARDWARE_L2, 0 },
    { "R2", PSX_PAD_HARDWARE_R2, 0 },
    { "L1", PSX_PAD_HARDWARE_L1, P8_PAD_L1 },
    { "R1", PSX_PAD_HARDWARE_R1, P8_PAD_R1 },
    { "TRIANGLE", PSX_PAD_HARDWARE_TRIANGLE, P8_PAD_TRIANGLE },
    { "CIRCLE", PSX_PAD_HARDWARE_CIRCLE, P8_PAD_CIRCLE },
    { "CROSS", PSX_PAD_HARDWARE_CROSS, P8_PAD_CROSS },
    { "SQUARE", PSX_PAD_HARDWARE_SQUARE, P8_PAD_SQUARE },
};

enum { ENTRY_COUNT = sizeof entries / sizeof entries[0] };

int psx_pad_map_buttons(uint16_t pressed) {
    int pad_buttons = 0;
    for (int i = 0; i < ENTRY_COUNT; i++) {
        if (pressed & entries[i].hardware) pad_buttons |= entries[i].pad_buttons;
    }
    return pad_buttons;
}

uint16_t psx_pad_map_hardware_bit(const char *name) {
    for (int i = 0; i < ENTRY_COUNT; i++) {
        if (strcmp(entries[i].name, name) == 0) return entries[i].hardware;
    }
    return 0;
}
