#include "p8/draw_backend.h"
#include "p8/draw_cost.h"
#include "p8/draw_meter.h"
#include "p8/gfx.h"
#include "p8/gpu_sizes.h"
#include "p8/image.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

enum { KINDS = 13 };

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

static void counting_image_begin(int slot, int w, int h) {
    (void)slot, (void)w, (void)h;
    forwarded[9]++;
}

static void counting_image_window(int slot, int x, int y) {
    (void)slot, (void)x, (void)y;
    forwarded[10]++;
}

static void counting_image_end(int slot) {
    (void)slot;
    forwarded[11]++;
}

static void counting_image_draw(const p8_graphics_t *gfx, p8_vram_t *mem, int slot, int x, int y) {
    (void)slot, (void)x, (void)y;
    seen(12, gfx, mem);
}

static const p8_draw_backend_t COUNTING = {
    counting_cls, counting_pset, counting_line, counting_rect, counting_rectfill,
    counting_circ, counting_circfill, counting_spr, counting_print,
    counting_image_begin, counting_image_window, counting_image_end, counting_image_draw,
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

static void draw_nothing(p8_graphics_t *gfx, p8_vram_t *mem, void *context) {
    (void)gfx, (void)mem, (void)context;
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
    p8_image_build(&gfx, &mem, 0, 16, 16, draw_nothing, NULL);
    p8_image_draw(&gfx, &mem, 0, 0, 0);
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
    assert(first.bytes == base + P8_GPU_FILL_BYTES + P8_GPU_TILE_1_BYTES);
    assert(first.primitives == base_primitives + 2);
    assert(p8_draw_meter_peak().bytes == first.bytes);

    p8_cls(&mem);
    p8_draw_meter_frame_end();
    assert(p8_draw_meter_last_frame().bytes == base + P8_GPU_FILL_BYTES && "the frame was reset");
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

static void two_sprites_and_a_palette(p8_graphics_t *build_gfx, p8_vram_t *build_mem, void *context) {
    (void)context;
    p8_spr(build_gfx, build_mem, 1, 0, 0, 1, 1);
    p8_spr(build_gfx, build_mem, 1, 200, 130, 1, 1);
    p8_spr(build_gfx, build_mem, 1, 500, 0, 1, 1);
}

static void test_build_bytes_go_to_the_build_count_and_its_palettes_to_the_frame(void) {
    meter_over_counting();
    unsigned long base = overhead().bytes;
    mem.draw_palette[5] = 9;

    p8_image_build(&gfx, &mem, 0, 256, 256, two_sprites_and_a_palette, NULL);
    p8_draw_meter_frame_end();

    p8_draw_meter_counts_t built = p8_draw_meter_last_frame();
    unsigned long windows = 6 * p8_draw_cost_image_window().bytes;
    assert(built.bytes == base && "nothing of the build in the frame");
    assert(built.build_bytes ==
           p8_draw_cost_image_begin().bytes + windows + 2 * P8_GPU_SPRT_BYTES + p8_draw_cost_image_end().bytes);
    assert(built.cluts == 1 && "the build's palette is a frame CLUT, as on the GPU");

    p8_draw_meter_frame_end();
    assert(p8_draw_meter_last_frame().build_bytes == 0 && "the build count restarts each frame");
    assert(p8_draw_meter_peak().build_bytes == built.build_bytes);
    meter_removed();
}

static void test_a_copy_costs_frame_bytes_on_screen_and_nothing_off_screen(void) {
    meter_over_counting();
    unsigned long base = overhead().bytes;
    p8_image_build(&gfx, &mem, 0, 64, 32, draw_nothing, NULL);
    p8_draw_meter_frame_end();

    p8_image_draw(&gfx, &mem, 0, 10, 10);
    p8_image_draw(&gfx, &mem, 0, -64, 0);
    p8_draw_meter_frame_end();
    p8_draw_cost_window_t window;
    (void)p8_draw_cost_frame_begin(&window);
    assert(p8_draw_meter_last_frame().bytes == base + p8_draw_cost_image_draw(&window, &gfx, 10, 10, 64, 32).bytes);
    assert(p8_draw_meter_last_frame().build_bytes == 0);
    meter_removed();
}

int main(void) {
    test_every_call_reaches_the_wrapped_backend();
    test_frame_end_folds_the_frame_into_the_peaks_and_resets_it();
    test_distinct_palettes_and_patterns_are_counted_once_per_frame();
    test_a_count_past_the_table_is_clamped_above_the_limit();
    test_build_bytes_go_to_the_build_count_and_its_palettes_to_the_frame();
    test_a_copy_costs_frame_bytes_on_screen_and_nothing_off_screen();
    printf("test_draw_meter: ok\n");
    return 0;
}
