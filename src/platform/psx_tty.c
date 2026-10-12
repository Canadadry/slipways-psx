#include "platform/psx_tty.h"

#include <psxapi.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

/* Character by character through the BIOS putchar: BIOS printf would be a
   second formatter next to the SDK's vsnprintf. IRQs stay masked during each
   call: an IRQ taken on the jump into the BIOS putchar makes PCSX-Redux log
   that character twice. */
void psx_tty_write(const char *text) {
    for (; *text != '\0'; text++) {
        FastEnterCriticalSection();
        putchar(*text);
        FastExitCriticalSection();
    }
}

void psx_tty_printf(const char *format, ...) {
    char text[256];
    va_list args;
    va_start(args, format);
    vsnprintf(text, sizeof text, format, args);
    va_end(args);
    psx_tty_write(text);
}

static void write_state_line(void *context, const char *line) {
    (void)context;
    psx_tty_write("psx-state: ");
    psx_tty_write(line);
}

state_sink_t psx_tty_state_sink(void) {
    state_sink_t sink = { write_state_line, NULL };
    return sink;
}

static void write_frame_line(void *context, const char *line) {
    (void)context;
    psx_tty_write("psx-frame: ");
    psx_tty_write(line);
}

state_sink_t psx_tty_frame_sink(void) {
    state_sink_t sink = { write_frame_line, NULL };
    return sink;
}

static void write_digest_line(void *context, const char *line) {
    (void)context;
    psx_tty_write("psx-digest: ");
    psx_tty_write(line);
}

vram_digest_sink_t psx_tty_digest_sink(void) {
    vram_digest_sink_t sink = { write_digest_line, NULL };
    return sink;
}
