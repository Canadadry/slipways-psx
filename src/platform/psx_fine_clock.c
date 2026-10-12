#include "platform/psx_fine_clock.h"

#include "platform/psx_frame_timer.h"

#include <hwregs_c.h>

enum { TIMER_SOURCE_CLOCK_DIV8 = 0x0200, CALIBRATION_HBLANKS = 64 };

void psx_fine_clock_start(void) {
    TIMER_CTRL(2) = TIMER_SOURCE_CLOCK_DIV8;
}

uint16_t psx_fine_clock_read(void) {
    return (uint16_t)TIMER_VALUE(2);
}

static uint16_t next_hblank(void) {
    uint16_t start = psx_frame_timer_read();
    uint16_t now;
    while ((now = psx_frame_timer_read()) == start) {
    }
    return now;
}

uint32_t psx_fine_clock_calibrate(void) {
    uint16_t first_hblank = next_hblank();
    uint16_t first = psx_fine_clock_read();
    while ((uint16_t)(psx_frame_timer_read() - first_hblank) < CALIBRATION_HBLANKS) {
    }
    return (uint16_t)(psx_fine_clock_read() - first);
}
