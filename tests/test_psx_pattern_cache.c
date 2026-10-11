#include "platform/psx_pattern_cache.h"

#include "p8/draw_cost.h"
#include "p8/gfx.h"
#include "p8/gpu_sizes.h"
#include "platform/psx_gpu_textures.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void assert_slot_holds(const psx_pattern_cache_t *cache, int slot, unsigned int pattern, int col, int transparent) {
    uint16_t tile[PSX_GPU_TEXTURES_PATTERN_X2_UNITS];
    uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    psx_gpu_textures_pattern_x2(pattern, tile);
    psx_gpu_textures_pattern_clut(col, transparent, clut);
    assert(memcmp(psx_pattern_cache_tile(cache, slot), tile, sizeof tile) == 0);
    assert(memcmp(psx_pattern_cache_clut(cache, slot), clut, sizeof clut) == 0);
}

static void test_one_pattern_and_colour_used_twice_takes_one_slot(void) {
    psx_pattern_cache_t cache;
    psx_pattern_cache_reset(&cache);
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_fillp(&gfx, 0xA5A5);
    int slot = psx_pattern_cache_slot(&cache, &gfx, 0x12);
    assert(slot == 0);
    assert(psx_pattern_cache_slot(&cache, &gfx, 0x12) == slot);
    assert(psx_pattern_cache_count(&cache) == 1);
    assert_slot_holds(&cache, slot, 0xA5A5, 0x12, 0);
}

static void test_the_pattern_the_colour_and_transparency_each_make_a_new_slot_until_none_is_left(void) {
    psx_pattern_cache_t cache;
    psx_pattern_cache_reset(&cache);
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_fillp(&gfx, 0x0F0F);
    assert(psx_pattern_cache_slot(&cache, &gfx, 0x10) == 0);
    assert(psx_pattern_cache_slot(&cache, &gfx, 0x11) == 1);
    p8_fillp_transparent(&gfx, 0x0F0F);
    assert(psx_pattern_cache_slot(&cache, &gfx, 0x11) == 2);
    p8_fillp(&gfx, 0xAAAA);
    assert(psx_pattern_cache_slot(&cache, &gfx, 0x11) == 3);
    assert(GPU_PATTERN_SLOTS == 4);
    p8_fillp(&gfx, 0x5555);
    assert(psx_pattern_cache_slot(&cache, &gfx, 0x11) == -1);
    assert(psx_pattern_cache_count(&cache) == GPU_PATTERN_SLOTS);
    assert_slot_holds(&cache, 0, 0x0F0F, 0x10, 0);
    assert_slot_holds(&cache, 1, 0x0F0F, 0x11, 0);
    assert_slot_holds(&cache, 2, 0x0F0F, 0x11, 1);
    assert_slot_holds(&cache, 3, 0xAAAA, 0x11, 0);
}

static void test_slots_are_told_apart_as_the_meter_tells_patterns_apart(void) {
    psx_pattern_cache_t cache;
    psx_pattern_cache_reset(&cache);
    p8_graphics_t a;
    p8_graphics_t b;
    p8_gfx_init(&a);
    p8_gfx_init(&b);
    p8_fillp(&a, 0xA5A5);
    p8_fillp(&b, 0xA5A5);
    b.fillp_transparent = 7;
    assert(p8_draw_cost_pattern_key(&a, 0x12) != p8_draw_cost_pattern_key(&b, 0x12));
    assert(psx_pattern_cache_slot(&cache, &a, 0x12) != psx_pattern_cache_slot(&cache, &b, 0x12));
    a.fillp_transparent = 1;
    assert(psx_pattern_cache_slot(&cache, &a, 0x12) == psx_pattern_cache_slot(&cache, &b, 0x12));
}

static void test_reset_frees_every_slot(void) {
    psx_pattern_cache_t cache;
    psx_pattern_cache_reset(&cache);
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    for (int n = 1; n <= GPU_PATTERN_SLOTS; n++) {
        p8_fillp(&gfx, (unsigned)n);
        psx_pattern_cache_slot(&cache, &gfx, 7);
    }
    psx_pattern_cache_reset(&cache);
    assert(psx_pattern_cache_count(&cache) == 0);
    p8_fillp_transparent(&gfx, 0xAAAA);
    assert(psx_pattern_cache_slot(&cache, &gfx, 0x3C) == 0);
    assert_slot_holds(&cache, 0, 0xAAAA, 0x3C, 1);
}

int main(void) {
    test_one_pattern_and_colour_used_twice_takes_one_slot();
    test_the_pattern_the_colour_and_transparency_each_make_a_new_slot_until_none_is_left();
    test_slots_are_told_apart_as_the_meter_tells_patterns_apart();
    test_reset_frees_every_slot();
    printf("test_psx_pattern_cache: ok\n");
    return 0;
}
