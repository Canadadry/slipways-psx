#include "platform/psx_gpu_textures.h"

#include "platform/psx_colour.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void test_a_transparent_entry_is_zero_and_black_stays_opaque(void) {
    const unsigned char palette[PSX_GPU_TEXTURES_CLUT_ENTRIES] = { 0x80, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0 };
    uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    psx_gpu_textures_clut(palette, clut);
    assert(clut[0] == 0x0000);
    assert(clut[15] == 0x8000 && "black remapped onto colour 15 is drawn, not skipped");
    for (int i = 1; i < 15; i++) {
        assert(clut[i] == psx_colour_rgb555(i));
    }
}

static void test_each_entry_follows_its_draw_palette_byte(void) {
    const unsigned char palette[PSX_GPU_TEXTURES_CLUT_ENTRIES] = { 0x00, 0x87, 0x3C, 7, 7, 0x8F, 1, 0, 0xFF, 9, 2, 2, 5, 0x40, 14, 3 };
    uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    psx_gpu_textures_clut(palette, clut);
    for (int i = 0; i < PSX_GPU_TEXTURES_CLUT_ENTRIES; i++) {
        assert(clut[i] == psx_colour_clut_entry(palette[i]));
    }
    assert(clut[2] == psx_colour_rgb555(12) && "remap uses the low nibble");
    assert(clut[1] == 0 && clut[5] == 0 && clut[8] == 0);
}

int main(void) {
    test_a_transparent_entry_is_zero_and_black_stays_opaque();
    test_each_entry_follows_its_draw_palette_byte();
    printf("test_psx_gpu_textures: ok\n");
    return 0;
}
