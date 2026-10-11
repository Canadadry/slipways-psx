/* Proves that scripted pad input reaches a PS-EXE. Once the BIOS pad driver
 * runs it raises the pad script's handshake word, then prints "psx-pad: <hex>"
 * each time the pressed buttons change, hardware bit order, 1 = pressed. When
 * the pad script writes the end word, it leaves with code 0 if no button is
 * held, PAD_PROBE_HELD_AT_END otherwise. Without a script it never ends: the
 * boot's vsync ceiling does. */

#include "platform/psx_exit.h"
#include "platform/psx_pad.h"
#include "platform/psx_pad_protocol.h"
#include "platform/psx_tty.h"

#include <psxgpu.h>
#include <stdint.h>

enum { PAD_PROBE_HELD_AT_END = 2 };

int main(void) {
    ResetGraph(0);
    psx_pad_start();
    *PSX_PAD_WORD(PSX_PAD_END_OFFSET) = 0;
    *PSX_PAD_WORD(PSX_PAD_HANDSHAKE_OFFSET) = PSX_PAD_HANDSHAKE_MAGIC;
    psx_tty_write("psx-pad: ready\n");

    uint16_t previous = 0;
    for (;;) {
        VSync(0);
        uint16_t pressed = psx_pad_read_pressed();
        if (pressed != previous) {
            psx_tty_printf("psx-pad: %04x\n", (unsigned)pressed);
            previous = pressed;
        }
        if (*PSX_PAD_WORD(PSX_PAD_END_OFFSET) != PSX_PAD_END_MAGIC) continue;
        if (pressed != 0) {
            psx_tty_printf("psx-pad: %04x still held at the script's end\n", (unsigned)pressed);
            psx_exit(PAD_PROBE_HELD_AT_END);
        }
        psx_tty_write("psx-pad: done\n");
        psx_exit(PSX_EXIT_OK);
    }
}
