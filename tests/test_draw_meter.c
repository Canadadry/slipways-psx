#include "p8/draw_backend.h"
#include "p8/draw_cost.h"
#include "p8/draw_meter.h"
#include "p8/gfx.h"
#include "p8/gpu_sizes.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

enum { KINDS = 9 };

static int forwarded[KINDS];
static const p8_graphics_t *forwarded_gfx;
static p8_vram_t *forwarded_mem;

static void seen(int kind, const p8_graphics_t *gfx, p8_vram_t *mem) {
    forwarded[kind]++;
    forwarded_gfx = gfx;
    forwarded_mem = mem;
}

static void counting_cls(p8_vram_t *mem) {
    seen(0, NULL, mem);
}

static void counting_pset(const p8_graphics_t *gfx, p8_vram_t *mem, int x, int y, int col) {
    (void)x, (void)y, (void)col;
    seen(1, gfx, mem);
}

static void counting_line(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)x1, (void)y1, (void)x2, (void)y2, (void)col;
    seen(2, gfx, mem);
}

static void counting_rect(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)x1, (void)y1, (void)x2, (void)y2, (void)col;
    seen(3, gfx, mem);
}

static void counting_rectfill(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)x1, (void)y1, (void)x2, (void)y2, (void)col;
    seen(4, gfx, mem);
}

static void counting_circ(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    (void)cx, (void)cy, (void)r, (void)col;
    seen(5, gfx, mem);
}

static void counting_circfill(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    (void)cx, (void)cy, (void)r, (void)col;
    seen(6, gfx, mem);
}

static void counting_spr(const p8_graphics_t *gfx, p8_vram_t *mem, int n, int x, int y, int w, int h) {
    (void)n, (void)x, (void)y, (void)w, (void)h;
    seen(7, gfx, mem);
}

static void counting_print(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col) {
    (void)str, (void)x, (void)y, (void)col;
    seen(8, gfx, mem);
}

static const p8_draw_backend_t COUNTING = {
    counting_cls, counting_pset, counting_line, counting_rect, counting_rectfill,
    counting_circ, counting_circfill, counting_spr, counting_print,
};

static p8_vram_t mem;
static p8_graphics_t gfx;
static const p8_draw_backend_t *before_meter;

static void meter_over_counting(void) {
    memset(forwarded, 0, sizeof forwarded);
    p8_vram_init(&mem);
    p8_gfx_init(&gfx);
    p8_draw_backend_set(&COUNTING);
    before_meter = p8_draw_meter_install();
    assert(before_meter == &COUNTING);
    assert(p8_draw_backend_get() != &COUNTING);
}

static void meter_removed(void) {
    p8_draw_backend_set(NULL);
}

static void draw_one_of_each(void) {
    p8_cls(&mem);
    p8_pset(&gfx, &mem, 1, 2, 3);
    p8_line(&gfx, &mem, 1, 2, 3, 4, 5);
    p8_rect(&gfx, &mem, 1, 2, 3, 4, 5);
    p8_rectfill(&gfx, &mem, 1, 2, 3, 4, 5);
    p8_circ(&gfx, &mem, 10, 10, 3, 5);
    p8_circfill(&gfx, &mem, 10, 10, 3, 5);
    p8_spr(&gfx, &mem, 1, 0, 0, 1, 1);
    p8_print(&gfx, &mem, "ab", 0, 0, 7);
}

static void test_every_call_reaches_the_wrapped_backend(void) {
    meter_over_counting();
    draw_one_of_each();
    for (int kind = 0; kind < KINDS; kind++) {
        assert(forwarded[kind] == 1);
    }
    assert(forwarded_gfx == &gfx && forwarded_mem == &mem);
    meter_removed();
}


static p8_draw_cost_t overhead(void) {
    p8_draw_cost_window_t window;
    return p8_draw_cost_frame_begin(&window);
}

static void test_frame_end_folds_the_frame_into_the_peaks_and_resets_it(void) {
    meter_over_counting();
    assert(p8_draw_meter_last_frame().bytes == 0 && p8_draw_meter_peak().bytes == 0);
    unsigned long base = overhead().bytes;
    int base_primitives = overhead().primitives;

    p8_cls(&mem);
    p8_pset(&gfx, &mem, 1, 1, 7);
    p8_pset(&gfx, &mem, 500, 1, 7);
    p8_draw_meter_frame_end();
    p8_draw_meter_counts_t first = p8_draw_meter_last_frame();
    assert(first.bytes == base + 2 * P8_GPU_TILE_BYTES);
    assert(first.primitives == base_primitives + 2);
    assert(p8_draw_meter_peak().bytes == first.bytes);

    p8_cls(&mem);
    p8_draw_meter_frame_end();
    assert(p8_draw_meter_last_frame().bytes == base + P8_GPU_TILE_BYTES && "the frame was reset");
    assert(p8_draw_meter_peak().bytes == first.bytes && p8_draw_meter_peak().primitives == first.primitives);

    p8_rect(&gfx, &mem, 0, 0, 5, 5, 7);
    p8_rect(&gfx, &mem, 0, 0, 5, 5, 7);
    p8_draw_meter_frame_end();
    assert(p8_draw_meter_peak().bytes == base + 8 * P8_GPU_TILE_BYTES);
    meter_removed();
}


static void test_distinct_palettes_and_patterns_are_counted_once_per_frame(void) {
    meter_over_counting();
    p8_spr(&gfx, &mem, 1, 0, 0, 1, 1);
    p8_spr(&gfx, &mem, 2, 50, 0, 1, 1);
    p8_map(&gfx, &mem, 0, 0, 0, 0, 1, 1);
    p8_palt(&mem, 0, 0);
    p8_spr(&gfx, &mem, 1, 0, 0, 1, 1);
    mem.draw_palette[5] = 9;
    p8_spr(&gfx, &mem, 1, 0, 0, 1, 1);
    p8_rectfill(&gfx, &mem, 0, 0, 5, 5, 7);
    p8_circfill(&gfx, &mem, 0, 0, 5, 7);
    p8_fillp(&gfx, 0xA5A5);
    p8_rectfill(&gfx, &mem, 0, 0, 5, 5, 0x15);
    p8_circfill(&gfx, &mem, 0, 0, 5, 0x15);
    p8_rectfill(&gfx, &mem, 0, 0, 5, 5, 0x16);
    p8_fillp_transparent(&gfx, 0xA5A5);
    p8_rectfill(&gfx, &mem, 0, 0, 5, 5, 0x16);
    p8_draw_meter_frame_end();
    assert(p8_draw_meter_last_frame().cluts == 3);
    assert(p8_draw_meter_last_frame().patterns == 3);

    p8_spr(&gfx, &mem, 1, 0, 0, 1, 1);
    p8_draw_meter_frame_end();
    assert(p8_draw_meter_last_frame().cluts == 1 && p8_draw_meter_last_frame().patterns == 0);
    assert(p8_draw_meter_peak().cluts == 3 && p8_draw_meter_peak().patterns == 3);
    meter_removed();
}

static void test_a_count_past_the_table_is_clamped_above_the_limit(void) {
    meter_over_counting();
    int palettes = 2 * GPU_CLUT_SLOTS + 3;
    int patterns = 2 * GPU_PATTERN_SLOTS + 3;
    assert(palettes <= 256 && patterns <= 0xFFFF);
    for (int i = 0; i < palettes; i++) {
        mem.draw_palette[0] = (unsigned char)i;
        p8_spr(&gfx, &mem, 1, 0, 0, 1, 1);
    }
    for (int i = 1; i <= patterns; i++) {
        p8_fillp(&gfx, (unsigned int)i);
        p8_rectfill(&gfx, &mem, 0, 0, 5, 5, 7);
    }
    p8_draw_meter_frame_end();
    assert(p8_draw_meter_peak().cluts == 2 * GPU_CLUT_SLOTS && p8_draw_meter_peak().cluts > GPU_CLUT_SLOTS);
    assert(p8_draw_meter_peak().patterns == 2 * GPU_PATTERN_SLOTS && p8_draw_meter_peak().patterns > GPU_PATTERN_SLOTS);
    meter_removed();
}

int main(void) {
    test_every_call_reaches_the_wrapped_backend();
    test_frame_end_folds_the_frame_into_the_peaks_and_resets_it();
    test_distinct_palettes_and_patterns_are_counted_once_per_frame();
    test_a_count_past_the_table_is_clamped_above_the_limit();
    printf("test_draw_meter: ok\n");
    return 0;
}
