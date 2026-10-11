/* The PC replay loop on PSX, over the recording linked in from
 * tmp/embed_replay's generated source; every output goes to TTY:
 *   psx-replay: asserts on|off
 *   psx-replay: format <format_self_test line>
 *   the devtools/vram_digest lines behind "psx-digest: ": "sprite-sheet" once
 *   loaded, "screen" after cart_draw on the sampled frames, "map" at the end
 *   psx-replay: frame <n>                       every 256 frames
 *   psx-replay: done <name> <frames> vsyncs <n>
 *   the "psx-state: " block of devtools/state_format
 * then it leaves the emulator with code 0. After a p8_fatal: the message,
 * "psx-state: stopped p8_fatal frame <n>", the state, code 3. */

#include "devtools/embedded_replay.h"
#include "devtools/format_self_test.h"
#include "devtools/state_format.h"
#include "devtools/vram_digest.h"
#include "engine/dispatch_context.h"
#include "game/bindings.h"
#include "game/cart.h"
#include "game/input_step.h"
#include "game/save_storage.h"
#include "p8/draw_meter.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/sprite_sheet.h"
#include "p8/vram.h"
#include "platform/psx_exit.h"
#include "platform/psx_tty.h"

#include <assert.h>
#include <psxgpu.h>

enum { PROGRESS_PERIOD = 256 };

static int g_frame;

static void write_stopped_state(int code) {
    if (code != PSX_EXIT_FATAL) return;
    state_sink_t sink = psx_tty_state_sink();
    state_format_stopped(&sink, g_frame);
    state_format(&sink);
}

static int asserts_enabled(void) {
    int enabled = 0;
    assert((enabled = 1) != 0);
    return enabled;
}

static void seed_storage(void) {
    int seeded = embedded_replay_initial_save_count == 0
        ? save_storage_erase()
        : save_storage_write(embedded_replay_initial_save, embedded_replay_initial_save_count);
    assert(seeded);
    (void)seeded;
}

int main(void) {
    ResetGraph(0);
    psx_tty_printf("psx-replay: asserts %s\n", asserts_enabled() ? "on" : "off");
    char format_line[FORMAT_SELF_TEST_CAP];
    format_self_test(format_line);
    psx_tty_printf("psx-replay: format %s\n", format_line);

    static p8_vram_t memory;
    p8_vram_init(&memory);
    static p8_graphics_t graphics;
    p8_gfx_init(&graphics);
    p8_sprite_sheet_load(&memory);
    vram_digest_sink_t digest = psx_tty_digest_sink();
    vram_digest_write_sprite_sheet(&digest, &memory);

    seed_storage();
    /* The core never writes the frames: p8_input_t only lacks the const. */
    static p8_input_t input;
    p8_input_init(&input);
    input.frames = (p8_input_frame_t *)embedded_replay_frames;
    input.frame_count = embedded_replay_frame_count;

    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);

    psx_exit_set_hook(write_stopped_state);
    p8_draw_meter_install();
    cart_load();
    cart_init();

    int start_vsync = VSync(-1);
    input_step_t step;
    input_step_init(&step, &bindings_default);
    input_step_set_attract_after(&step, embedded_replay_attract_after);
    int cheat_activations = 0;
    for (g_frame = 0; g_frame < input.frame_count; g_frame++) {
        cheat_activations += input_step_run(&step, &input.frames[input.current_frame]);
        cart_update60();
        cart_draw();
        p8_draw_meter_frame_end();
        if (vram_digest_samples_frame(g_frame, input.frame_count)) {
            vram_digest_write_screen(&digest, &memory, g_frame);
        }
        if (g_frame < input.frame_count - 1) {
            p8_input_step(&input);
        }
        if ((g_frame + 1) % PROGRESS_PERIOD == 0) {
            psx_tty_printf("psx-replay: frame %d\n", g_frame + 1);
        }
    }
    int vsyncs = VSync(-1) - start_vsync;
    psx_exit_set_hook(NULL);
    vram_digest_write_map(&digest, &memory);

    psx_tty_printf("psx-replay: done %s %d vsyncs %d\n", embedded_replay_name, input.frame_count, vsyncs);
    state_sink_t sink = psx_tty_state_sink();
    state_format(&sink);
    state_format_cheat_activations(&sink, cheat_activations);
    psx_exit(PSX_EXIT_OK);
}
