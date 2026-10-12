/* src/cmd/psx_replay.c's loop and TTY protocol, drawn through the GPU backend wrapped by the draw
 * filter and the meter instead of into the RAM framebuffer: the same lines minus the "psx-digest: "
 * ones, plus
 *   psx-replay: backend gpu
 * after the format line, and
 *   psx-replay: capture <frame> <mask hex>
 * at each frame the capture script (platform/psx_capture_protocol.h) asks for, plus the frame times
 * (devtools/frame_times.h) behind "psx-frame: ": the calibration line before the first frame, then
 * every frame's line before the done line, or before the stopped state on a p8_fatal. Without a
 * capture script driving the handshake it waits forever before the first frame. */

#include "devtools/draw_filter.h"
#include "devtools/embedded_replay.h"
#include "devtools/format_self_test.h"
#include "devtools/frame_times.h"
#include "devtools/state_format.h"
#include "engine/dispatch_context.h"
#include "game/bindings.h"
#include "game/cart.h"
#include "game/input_step.h"
#include "game/save_storage.h"
#include "p8/draw_backend.h"
#include "p8/draw_meter.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "p8/sprite_sheet.h"
#include "p8/vram.h"
#include "platform/psx_capture_protocol.h"
#include "platform/psx_exit.h"
#include "platform/psx_frame_timer.h"
#include "platform/psx_gpu.h"
#include "platform/psx_tty.h"

#include <assert.h>
#include <psxgpu.h>
#include <stdint.h>
#include <stdio.h>

enum { PROGRESS_PERIOD = 256 };

static int g_frame;

#define CAPTURE_WORD(offset) ((volatile uint32_t *)(PSX_CAPTURE_SCRATCHPAD + (offset)))

typedef struct {
    uint32_t frame;
    unsigned mask;
} capture_request_t;

static capture_request_t capture_wait_next(void) {
    while (*CAPTURE_WORD(PSX_CAPTURE_STATE_OFFSET) != PSX_CAPTURE_NEXT_READY) {
    }
    capture_request_t request = { *CAPTURE_WORD(PSX_CAPTURE_FRAME_OFFSET), *CAPTURE_WORD(PSX_CAPTURE_MASK_OFFSET) };
    return request;
}

static capture_request_t capture_start(void) {
    *CAPTURE_WORD(PSX_CAPTURE_STATE_OFFSET) = PSX_CAPTURE_PROGRAM_READY;
    return capture_wait_next();
}

/* The frame was just ended with psx_gpu_frame_end. */
static capture_request_t capture_frame(int frame, unsigned mask) {
    psx_gpu_frame_show_last();
    psx_tty_printf("psx-replay: capture %d %04x\n", frame, mask);
    *CAPTURE_WORD(PSX_CAPTURE_FRAME_OFFSET) = (uint32_t)frame;
    *CAPTURE_WORD(PSX_CAPTURE_STATE_OFFSET) = PSX_CAPTURE_FRAME_SHOWN;
    return capture_wait_next();
}

static void write_stopped_state(int code) {
    if (code != PSX_EXIT_FATAL) return;
    state_sink_t frame_sink = psx_tty_frame_sink();
    frame_times_write(&frame_sink);
    state_sink_t sink = psx_tty_state_sink();
    state_format_stopped(&sink, g_frame);
    state_format(&sink);
}

static void check_frame_capacity(int frame_count) {
    if (frame_count <= FRAME_TIMES_CAP) return;
    char message[80];
    snprintf(message, sizeof message, "replay has %d frames, frame table capacity %d", frame_count, FRAME_TIMES_CAP);
    p8_fatal(message);
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
    psx_tty_printf("psx-replay: backend gpu\n");

    static p8_vram_t memory;
    p8_vram_init(&memory);
    static p8_graphics_t graphics;
    p8_gfx_init(&graphics);
    p8_sprite_sheet_load(&memory);
    psx_gpu_init();

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
    p8_draw_backend_set(psx_gpu_backend());
    draw_filter_install();
    p8_draw_meter_install();
    psx_gpu_frame_begin(0);
    cart_load();
    cart_init();

    frame_times_init();
    check_frame_capacity(input.frame_count);
    psx_frame_timer_start();
    state_sink_t frame_sink = psx_tty_frame_sink();
    frame_times_write_calibration(&frame_sink, psx_frame_timer_calibrate());

    capture_request_t capture = capture_start();
    int start_vsync = VSync(-1);
    input_step_t step;
    input_step_init(&step, &bindings_default);
    input_step_set_attract_after(&step, embedded_replay_attract_after);
    int cheat_activations = 0;
    for (g_frame = 0; g_frame < input.frame_count; g_frame++) {
        uint32_t frame_start_vsync = psx_frame_timer_vsyncs();
        uint16_t marks[FRAME_TIMES_MARKS];
        marks[0] = psx_frame_timer_read();
        cheat_activations += input_step_run(&step, &input.frames[input.current_frame]);
        cart_update60();
        marks[1] = psx_frame_timer_read();
        int captured = capture.frame == (uint32_t)g_frame;
        draw_filter_set_mask(captured ? capture.mask : DRAW_FILTER_ALL);
        cart_draw();
        marks[2] = psx_frame_timer_read();
        p8_draw_meter_frame_end();
        draw_filter_frame_end();
        psx_gpu_frame_end(p8_draw_meter_last_frame().bytes);
        /* The capture handshake waits on the emulator: its time and vsyncs stay out of the frame's. */
        uint16_t capture_time = 0;
        uint32_t capture_vsyncs = 0;
        if (captured) {
            uint16_t capture_start_mark = psx_frame_timer_read();
            uint32_t capture_start_vsync = psx_frame_timer_vsyncs();
            capture = capture_frame(g_frame, capture.mask);
            capture_time = (uint16_t)(psx_frame_timer_read() - capture_start_mark);
            capture_vsyncs = psx_frame_timer_vsyncs() - capture_start_vsync;
        }
        psx_gpu_frame_begin(g_frame + 1);
        marks[3] = (uint16_t)(psx_frame_timer_read() - capture_time);
        frame_times_record(marks, psx_frame_timer_vsyncs() - frame_start_vsync - capture_vsyncs);
        if (g_frame < input.frame_count - 1) {
            p8_input_step(&input);
        }
        if ((g_frame + 1) % PROGRESS_PERIOD == 0) {
            psx_tty_printf("psx-replay: frame %d\n", g_frame + 1);
        }
    }
    int vsyncs = VSync(-1) - start_vsync;
    psx_exit_set_hook(NULL);

    frame_times_write(&frame_sink);
    psx_tty_printf("psx-replay: done %s %d vsyncs %d\n", embedded_replay_name, input.frame_count, vsyncs);
    state_sink_t sink = psx_tty_state_sink();
    state_format(&sink);
    state_format_cheat_activations(&sink, cheat_activations);
    psx_exit(PSX_EXIT_OK);
}
