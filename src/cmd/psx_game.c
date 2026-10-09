/* The game as it ships on PSX: the GPU replay's frame (src/cmd/psx_gpu_replay.c) driven by the pad
 * on port 1 instead of a recording, at most one frame a vsync. Its TTY:
 *   psx-game: ready
 *   psx-game: pad <frame> <pad_buttons hex>   on frame 0 and on each change
 * Under check only, once tools/psx_pad_play.lua writes the pad script's end word, it ends with the
 * replays' protocol: "psx-replay: done game <frames> vsyncs <n>", the "psx-state: " block, code 0.
 * On a console nothing writes that word, so the game never ends. A p8_fatal prints
 * "psx-state: stopped p8_fatal frame <n>" and the state, code 3. */

#include "devtools/state_format.h"
#include "engine/dispatch_context.h"
#include "game/cart.h"
#include "game/input_step.h"
#include "game/save_storage.h"
#include "p8/draw_backend.h"
#include "p8/draw_meter.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/sprite_sheet.h"
#include "p8/vram.h"
#include "platform/psx_exit.h"
#include "platform/psx_gpu.h"
#include "platform/psx_pad.h"
#include "platform/psx_pad_map.h"
#include "platform/psx_pad_protocol.h"
#include "platform/psx_tty.h"

#include <assert.h>
#include <psxgpu.h>
#include <stdint.h>

static int g_frame;

static void write_stopped_state(int code) {
    if (code != PSX_EXIT_FATAL) return;
    state_sink_t sink = psx_tty_state_sink();
    state_format_stopped(&sink, g_frame);
    state_format(&sink);
}

int main(void) {
    ResetGraph(0);

    static p8_vram_t memory;
    p8_vram_init(&memory);
    static p8_graphics_t graphics;
    p8_gfx_init(&graphics);
    p8_sprite_sheet_load(&memory);
    psx_gpu_init();

    int erased = save_storage_erase();
    assert(erased);
    (void)erased;
    /* What cart_load and cart_init see, as a pad recording's first frame before any press. */
    static p8_input_frame_t idle_frame;
    static p8_input_t idle_input;
    p8_input_init(&idle_input);
    idle_input.frames = &idle_frame;
    idle_input.frame_count = 1;

    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&idle_input);

    psx_exit_set_hook(write_stopped_state);
    p8_draw_backend_set(psx_gpu_backend());
    p8_draw_meter_install();
    psx_gpu_frame_begin(0);
    cart_load();
    cart_init();

    psx_pad_start();
    *PSX_PAD_WORD(PSX_PAD_END_OFFSET) = 0;
    *PSX_PAD_WORD(PSX_PAD_HANDSHAKE_OFFSET) = PSX_PAD_HANDSHAKE_MAGIC;
    psx_tty_write("psx-game: ready\n");

    int start_vsync = VSync(-1);
    int frame_vsync = start_vsync - 1;
    int previous_pad = 0;
    input_step_t step;
    input_step_init(&step);
    for (g_frame = 0; *PSX_PAD_WORD(PSX_PAD_END_OFFSET) != PSX_PAD_END_MAGIC; g_frame++) {
        if (VSync(-1) == frame_vsync) VSync(0);
        frame_vsync = VSync(-1);
        p8_input_frame_t raw = { 0, 0, 0, psx_pad_map_buttons(psx_pad_read_pressed()) };
        if (g_frame == 0 || raw.pad_buttons != previous_pad) {
            psx_tty_printf("psx-game: pad %d %03x\n", g_frame, (unsigned)raw.pad_buttons);
            previous_pad = raw.pad_buttons;
        }
        input_step_run(&step, &raw);
        cart_update60();
        cart_draw();
        p8_draw_meter_frame_end();
        psx_gpu_frame_end(p8_draw_meter_last_frame().bytes);
        psx_gpu_frame_begin(g_frame + 1);
    }
    int vsyncs = VSync(-1) - start_vsync;
    psx_exit_set_hook(NULL);

    psx_tty_printf("psx-replay: done game %d vsyncs %d\n", g_frame, vsyncs);
    state_sink_t sink = psx_tty_state_sink();
    state_format(&sink);
    psx_exit(PSX_EXIT_OK);
}
