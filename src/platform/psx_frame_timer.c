#include "platform/psx_frame_timer.h"

#include <hwregs_c.h>
#include <psxgpu.h>

/* ResetGraph sets counter 1 to the horizontal blank too, but with its own
   IRQ bits: the explicit mode keeps the timer independent of the SDK. */
enum { TIMER_SOURCE_HBLANK = 0x0100 };

void psx_frame_timer_start(void) {
    TIMER_CTRL(1) = TIMER_SOURCE_HBLANK;
}

uint16_t psx_frame_timer_read(void) {
    return (uint16_t)TIMER_VALUE(1);
}

uint32_t psx_frame_timer_vsyncs(void) {
    return (uint32_t)VSync(-1);
}

uint32_t psx_frame_timer_calibrate(void) {
    VSync(0);
    uint16_t first = psx_frame_timer_read();
    VSync(0);
    return (uint16_t)(psx_frame_timer_read() - first);
}
