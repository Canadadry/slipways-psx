/* The largest-case scenarios of devtools/largest_slots on MIPS; every output
 * goes to TTY:
 *   psx-largest: asserts on|off
 *   psx-largest: bytes_<slot> <peak> <size>     one line per proven-ceiling slot
 *   psx-largest: done
 * then it leaves the emulator with code 0. After a p8_fatal: the message,
 * "psx-state: stopped p8_fatal frame 0", the state, code 3; a failed assert
 * leaves with code 4. */

#include "devtools/largest_slots.h"
#include "devtools/state_format.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/sprite_sheet.h"
#include "p8/vram.h"
#include "platform/psx_exit.h"
#include "platform/psx_tty.h"

#include <assert.h>
#include <psxgpu.h>

static void write_stopped_state(int code) {
    if (code != PSX_EXIT_FATAL) return;
    state_sink_t sink = psx_tty_state_sink();
    state_format_stopped(&sink, 0);
    state_format(&sink);
}

static int asserts_enabled(void) {
    int enabled = 0;
    assert((enabled = 1) != 0);
    return enabled;
}

static void write_largest_line(void *context, const char *line) {
    (void)context;
    psx_tty_write("psx-largest: ");
    psx_tty_write(line);
}

int main(void) {
    ResetGraph(0);
    psx_tty_printf("psx-largest: asserts %s\n", asserts_enabled() ? "on" : "off");

    static p8_vram_t memory;
    p8_vram_init(&memory);
    static p8_graphics_t graphics;
    p8_gfx_init(&graphics);
    p8_sprite_sheet_load(&memory);
    static p8_input_t input;
    p8_input_init(&input);

    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);

    psx_exit_set_hook(write_stopped_state);
    cart_load();
    largest_slots_run();
    psx_exit_set_hook(NULL);

    state_sink_t sink = { write_largest_line, NULL };
    largest_slots_format(&sink);
    psx_tty_write("psx-largest: done\n");
    psx_exit(PSX_EXIT_OK);
}
