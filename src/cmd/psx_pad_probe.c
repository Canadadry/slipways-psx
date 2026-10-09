/* Proves that scripted pad input reaches a PS-EXE. Once the BIOS pad driver
 * runs it writes PAD_PROBE_READY into the last scratchpad word (the pad
 * script's handshake), then prints "psx-pad: <hex>" each time the pressed
 * buttons change, hardware bit order, 1 = pressed. When the pad script writes
 * PAD_SCRIPT_END into the word below, it leaves with code 0 if no button is
 * held, PAD_PROBE_HELD_AT_END otherwise. Without a script it never ends: the
 * boot's vsync ceiling does. */

#include "platform/psx_exit.h"
#include "platform/psx_pad.h"
#include "platform/psx_tty.h"

#include <psxgpu.h>
#include <stdint.h>

#define SCRATCHPAD_HANDSHAKE_WORD ((volatile uint32_t *)0x1f8003fc)
#define SCRATCHPAD_END_WORD ((volatile uint32_t *)0x1f8003f8)

enum {
    PAD_PROBE_READY = 0x50414421,
    PAD_SCRIPT_END = 0x454e4421,
    PAD_PROBE_HELD_AT_END = 2
};

int main(void) {
    ResetGraph(0);
    psx_pad_start();
    *SCRATCHPAD_END_WORD = 0;
    *SCRATCHPAD_HANDSHAKE_WORD = PAD_PROBE_READY;
    psx_tty_write("psx-pad: ready\n");

    uint16_t previous = 0;
    for (;;) {
        VSync(0);
        uint16_t pressed = psx_pad_read_pressed();
        if (pressed != previous) {
            psx_tty_printf("psx-pad: %04x\n", (unsigned)pressed);
            previous = pressed;
        }
        if (*SCRATCHPAD_END_WORD != PAD_SCRIPT_END) continue;
        if (pressed != 0) {
            psx_tty_printf("psx-pad: %04x still held at the script's end\n", (unsigned)pressed);
            psx_exit(PAD_PROBE_HELD_AT_END);
        }
        psx_tty_write("psx-pad: done\n");
        psx_exit(PSX_EXIT_OK);
    }
}
