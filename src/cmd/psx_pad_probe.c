/* Proves that scripted pad input reaches a PS-EXE. Once the BIOS pad driver
 * runs it writes PAD_PROBE_READY into the last scratchpad word (the pad
 * script's handshake), then prints "psx-pad: <hex>" each time the pressed
 * buttons change, hardware bit order, 1 = pressed. It leaves with code 0 once
 * the pad has been released for IDLE_END_VSYNCS after a press, code 1 at
 * VSYNC_CEILING. */

#include "platform/psx_exit.h"
#include "platform/psx_pad.h"
#include "platform/psx_tty.h"

#include <psxgpu.h>
#include <stdint.h>

#define SCRATCHPAD_LAST_WORD ((volatile uint32_t *)0x1f8003fc)

enum {
    PAD_PROBE_READY = 0x50414421,
    IDLE_END_VSYNCS = 60,
    VSYNC_CEILING = 3600
};

int main(void) {
    ResetGraph(0);
    psx_pad_start();
    *SCRATCHPAD_LAST_WORD = PAD_PROBE_READY;
    psx_tty_write("psx-pad: ready\n");

    uint16_t previous = 0;
    int pressed_once = 0;
    int idle_vsyncs = 0;
    for (int vsync = 0; vsync < VSYNC_CEILING; vsync++) {
        VSync(0);
        uint16_t pressed = psx_pad_read_pressed();
        if (pressed != previous) {
            psx_tty_printf("psx-pad: %04x\n", (unsigned)pressed);
            previous = pressed;
        }
        if (pressed != 0) {
            pressed_once = 1;
            idle_vsyncs = 0;
        } else if (pressed_once && ++idle_vsyncs >= IDLE_END_VSYNCS) {
            psx_tty_write("psx-pad: done\n");
            psx_exit(PSX_EXIT_OK);
        }
    }
    psx_tty_write("psx-pad: vsync ceiling reached\n");
    psx_exit(1);
}
