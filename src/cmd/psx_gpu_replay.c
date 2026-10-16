/* src/cmd/psx_replay.c's loop and TTY protocol, drawn through the GPU backend instead of into the
 * RAM framebuffer: the same lines minus the "psx-digest: " ones, plus
 *   psx-replay: backend gpu
 *   psx-replay: draw-filter on|off
 *   psx-replay: draw-meter on|off
 * after the format line,
 *   psx-replay: gpu-peak bytes <b> cluts <c> patterns <p> build <k>
 * (psx_gpu_peaks) before the done line, never on a p8_fatal, and
 *   psx-replay: capture <frame> <mask hex>
 * at each frame the capture script (platform/psx_capture_protocol.h) asks for (a display capture,
 * mask CAPTURE_LIST_DISPLAY_MASK, at the top of its frame before the input step, the display left as
 * the previous frame end set it; any other after its frame is drawn and shown), plus the frame times
 * (devtools/frame_times.h) behind "psx-frame: ": the calibration line before the first frame, then
 * every frame's line before the done line, or before the stopped state on a p8_fatal. Without a
 * capture script driving the handshake it waits forever before the first frame.
 * Built with PSX_GPU_REPLAY_DRAW_FILTER (make psx-check), the draw filter sits on the backend (under
 * the meter when both are built) and masks each captured frame; without it, a masked capture is a
 * p8_fatal.
 * Built with PSX_GPU_REPLAY_DRAW_METER (make psx-check), the meter wraps the backend and each frame's
 * metered bytes are its byte limit; without it, the meter's state stays empty (zero gpu_* lines) and
 * the limit is the whole buffer.
 * Built with PSX_GPU_REPLAY_BREAKDOWN (psx-gpu-replay-breakdown), it also times every backend call
 * per draw family (devtools/draw_timing.h) and writes, after the "psx-frame: " lines, the fine
 * clock's calibration then one line per frame behind "psx-draw: ".
 * Built with PSX_PROBE (make psx-probe), it starts platform/psx_probe.h's delay helper and writes its
 * "psx-probe: " lines before the done line. */

#include "devtools/capture_list.h"
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
#include "p8/gpu_sizes.h"
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

#ifdef PSX_GPU_REPLAY_DRAW_FILTER
static const char DRAW_FILTER_MODE[] = "on";

static void filter_install(void) {
    draw_filter_install();
}

static void filter_set_mask(unsigned mask) {
    draw_filter_set_mask(mask);
}

static void filter_frame_end(void) {
    draw_filter_frame_end();
}

static capture_request_t filter_check_capture(capture_request_t request) {
    return request;
}
#else
static const char DRAW_FILTER_MODE[] = "off";

static void filter_install(void) {
}

static void filter_set_mask(unsigned mask) {
    (void)mask;
}

static void filter_frame_end(void) {
}

static capture_request_t filter_check_capture(capture_request_t request) {
    if (request.frame != PSX_CAPTURE_NO_FRAME && request.mask != DRAW_FILTER_ALL
        && request.mask != CAPTURE_LIST_DISPLAY_MASK) {
        char message[96];
        snprintf(message, sizeof message, "capture mask %04x at frame %u needs PSX_GPU_REPLAY_DRAW_FILTER",
                 request.mask, (unsigned)request.frame);
        p8_fatal(message);
    }
    return request;
}
#endif

#ifdef PSX_GPU_REPLAY_DRAW_METER
static const char DRAW_METER_MODE[] = "on";

static void meter_install(void) {
    p8_draw_meter_install();
}

static void meter_frame_end(void) {
    p8_draw_meter_frame_end();
}

/* After meter_frame_end. */
static unsigned long meter_frame_limit(void) {
    return p8_draw_meter_last_frame().bytes;
}
#else
static const char DRAW_METER_MODE[] = "off";

static void meter_install(void) {
}

static void meter_frame_end(void) {
}

static unsigned long meter_frame_limit(void) {
    return GPU_FRAME_BUFFER_BYTES;
}
#endif

static capture_request_t capture_wait_next(void) {
    while (*CAPTURE_WORD(PSX_CAPTURE_STATE_OFFSET) != PSX_CAPTURE_NEXT_READY) {
    }
    capture_request_t request = { *CAPTURE_WORD(PSX_CAPTURE_FRAME_OFFSET), *CAPTURE_WORD(PSX_CAPTURE_MASK_OFFSET) };
    return filter_check_capture(request);
}

static capture_request_t capture_start(void) {
    *CAPTURE_WORD(PSX_CAPTURE_STATE_OFFSET) = PSX_CAPTURE_PROGRAM_READY;
    return capture_wait_next();
}

static capture_request_t capture_shown(int frame, unsigned mask) {
    psx_tty_printf("psx-replay: capture %d %04x\n", frame, mask);
    *CAPTURE_WORD(PSX_CAPTURE_FRAME_OFFSET) = (uint32_t)frame;
    *CAPTURE_WORD(PSX_CAPTURE_STATE_OFFSET) = PSX_CAPTURE_FRAME_SHOWN;
    return capture_wait_next();
}

/* The frame was just ended with psx_gpu_frame_end. */
static capture_request_t capture_frame(int frame, unsigned mask) {
    psx_gpu_frame_show_last();
    return capture_shown(frame, mask);
}

/* Before the frame's input step. No psx_gpu_frame_show_last: the screenshot must be what the frame
 * loop displays, frame - 2. */
static capture_request_t capture_display(capture_request_t capture, int frame) {
    if (capture.frame != (uint32_t)frame || capture.mask != CAPTURE_LIST_DISPLAY_MASK) return capture;
    return capture_shown(frame, capture.mask);
}

#ifdef PSX_GPU_REPLAY_BREAKDOWN
#include "devtools/draw_timing.h"
#include "platform/psx_fine_clock.h"

enum { BREAKDOWN_WRAP_HBLANKS = 200 };

static const draw_timing_clock_t BREAKDOWN_CLOCK = {
    psx_fine_clock_read, psx_frame_timer_read, BREAKDOWN_WRAP_HBLANKS,
};
static uint32_t breakdown_calibration;

/* The frame timer must be started. */
static void breakdown_start(int frame_count) {
    if (frame_count > DRAW_TIMING_CAP) {
        char message[80];
        snprintf(message, sizeof message, "replay has %d frames, draw timing capacity %d", frame_count,
                 DRAW_TIMING_CAP);
        p8_fatal(message);
    }
    psx_fine_clock_start();
    breakdown_calibration = psx_fine_clock_calibrate();
    draw_timing_install(&BREAKDOWN_CLOCK);
}

static void breakdown_frame_end(void) {
    draw_timing_frame_end();
}

static void breakdown_write(void) {
    state_sink_t sink = psx_tty_draw_sink();
    draw_timing_write_calibration(&sink, breakdown_calibration);
    draw_timing_write(&sink);
}
#else
static void breakdown_start(int frame_count) {
    (void)frame_count;
}

static void breakdown_frame_end(void) {
}

static void breakdown_write(void) {
}
#endif

#ifdef PSX_PROBE
#include "platform/psx_probe.h"

/* The frame timer must be started. */
static void probe_start(void) {
    psx_probe_start();
}

static void probe_write(void) {
    psx_probe_write();
}
#else
static void probe_start(void) {
}

static void probe_write(void) {
}
#endif

static void write_stopped_state(int code) {
    if (code != PSX_EXIT_FATAL) return;
    state_sink_t frame_sink = psx_tty_frame_sink();
    frame_times_write(&frame_sink);
    breakdown_write();
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
    psx_tty_printf("psx-replay: draw-filter %s\n", DRAW_FILTER_MODE);
    psx_tty_printf("psx-replay: draw-meter %s\n", DRAW_METER_MODE);

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
    filter_install();
    meter_install();
    psx_gpu_frame_begin(0);
    cart_load();
    cart_init();

    frame_times_init();
    check_frame_capacity(input.frame_count);
    psx_frame_timer_start();
    state_sink_t frame_sink = psx_tty_frame_sink();
    frame_times_write_calibration(&frame_sink, psx_frame_timer_calibrate());

    probe_start();
    breakdown_start(input.frame_count);

    capture_request_t capture = capture_start();
    int start_vsync = VSync(-1);
    input_step_t step;
    input_step_init(&step, &bindings_default);
    input_step_set_attract_after(&step, embedded_replay_attract_after);
    int cheat_activations = 0;
    for (g_frame = 0; g_frame < input.frame_count; g_frame++) {
        /* Before the frame's marks: the handshake's time and vsyncs stay out of the frame's. */
        capture = capture_display(capture, g_frame);
        uint32_t frame_start_vsync = psx_frame_timer_vsyncs();
        uint16_t marks[FRAME_TIMES_MARKS];
        marks[0] = psx_frame_timer_read();
        cheat_activations += input_step_run(&step, &input.frames[input.current_frame]);
        cart_update60();
        marks[1] = psx_frame_timer_read();
        int captured = capture.frame == (uint32_t)g_frame;
        filter_set_mask(captured ? capture.mask : DRAW_FILTER_ALL);
        cart_draw();
        marks[2] = psx_frame_timer_read();
        meter_frame_end();
        filter_frame_end();
        breakdown_frame_end();
        psx_gpu_frame_end(meter_frame_limit());
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
    breakdown_write();
    probe_write();
    psx_gpu_peaks_t peaks = psx_gpu_peaks();
    psx_tty_printf("psx-replay: gpu-peak bytes %lu cluts %d patterns %d build %lu\n", peaks.bytes, peaks.cluts, peaks.patterns,
                   peaks.build_bytes);
    psx_tty_printf("psx-replay: done %s %d vsyncs %d\n", embedded_replay_name, input.frame_count, vsyncs);
    state_sink_t sink = psx_tty_state_sink();
    state_format(&sink);
    state_format_cheat_activations(&sink, cheat_activations);
    psx_exit(PSX_EXIT_OK);
}
