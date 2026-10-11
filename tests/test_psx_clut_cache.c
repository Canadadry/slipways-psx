#include "platform/psx_clut_cache.h"

#include "p8/gpu_sizes.h"
#include "platform/psx_gpu_textures.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void palette_numbered(unsigned char palette[PSX_GPU_TEXTURES_CLUT_ENTRIES], int number) {
    for (int i = 0; i < PSX_GPU_TEXTURES_CLUT_ENTRIES; i++) {
        palette[i] = (unsigned char)i;
    }
    palette[0] = 0x80;
    palette[1] = (unsigned char)(number & 0x0F);
    palette[2] = (unsigned char)((number >> 4) & 0x0F);
}

static void assert_slot_holds(const psx_clut_cache_t *cache, int slot, const unsigned char palette[]) {
    uint16_t expected[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    psx_gpu_textures_clut(palette, expected);
    assert(memcmp(psx_clut_cache_clut(cache, slot), expected, sizeof expected) == 0);
}

static void test_one_palette_used_twice_takes_one_slot(void) {
    psx_clut_cache_t cache;
    psx_clut_cache_reset(&cache);
    unsigned char palette[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    palette_numbered(palette, 3);
    int slot = psx_clut_cache_slot(&cache, palette);
    assert(slot == 0);
    assert(psx_clut_cache_slot(&cache, palette) == slot);
    assert(psx_clut_cache_count(&cache) == 1);
    assert_slot_holds(&cache, slot, palette);
}

static void test_distinct_palettes_fill_the_slots_in_order_then_none_is_left(void) {
    psx_clut_cache_t cache;
    psx_clut_cache_reset(&cache);
    unsigned char palette[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    for (int n = 0; n < GPU_CLUT_SLOTS; n++) {
        palette_numbered(palette, n);
        assert(psx_clut_cache_slot(&cache, palette) == n);
        assert(psx_clut_cache_count(&cache) == n + 1);
    }
    for (int n = 0; n < GPU_CLUT_SLOTS; n++) {
        palette_numbered(palette, n);
        assert(psx_clut_cache_slot(&cache, palette) == n);
        assert_slot_holds(&cache, n, palette);
    }
    palette_numbered(palette, GPU_CLUT_SLOTS);
    assert(psx_clut_cache_slot(&cache, palette) == -1);
    assert(psx_clut_cache_count(&cache) == GPU_CLUT_SLOTS);
}

static void test_palettes_with_the_same_colours_but_other_bytes_are_distinct_keys(void) {
    psx_clut_cache_t cache;
    psx_clut_cache_reset(&cache);
    unsigned char palette[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    palette_numbered(palette, 1);
    assert(psx_clut_cache_slot(&cache, palette) == 0);
    palette[0] = 0x81;
    assert(psx_clut_cache_slot(&cache, palette) == 1 && "keyed on the draw-palette bytes, as the meter counts them");
}

static void test_reset_frees_every_slot(void) {
    psx_clut_cache_t cache;
    psx_clut_cache_reset(&cache);
    unsigned char palette[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    for (int n = 0; n < GPU_CLUT_SLOTS; n++) {
        palette_numbered(palette, n);
        psx_clut_cache_slot(&cache, palette);
    }
    psx_clut_cache_reset(&cache);
    assert(psx_clut_cache_count(&cache) == 0);
    palette_numbered(palette, 9);
    assert(psx_clut_cache_slot(&cache, palette) == 0);
    assert_slot_holds(&cache, 0, palette);
}

int main(void) {
    test_one_palette_used_twice_takes_one_slot();
    test_distinct_palettes_fill_the_slots_in_order_then_none_is_left();
    test_palettes_with_the_same_colours_but_other_bytes_are_distinct_keys();
    test_reset_frees_every_slot();
    printf("test_psx_clut_cache: ok\n");
    return 0;
}
