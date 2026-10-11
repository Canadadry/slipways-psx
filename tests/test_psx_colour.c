#include "platform/psx_colour.h"

#include "p8/palette.h"

#include <assert.h>
#include <stdio.h>

static void test_each_palette_colour_is_its_top_five_bits_with_the_mask_bit(void) {
    for (int i = 0; i < P8_PALETTE_COLORS; i++) {
        p8_rgb_t rgb = P8_PALETTE_RGB[i];
        unsigned expected = 0x8000u | (unsigned)(rgb.blue >> 3) << 10 | (unsigned)(rgb.green >> 3) << 5
            | (unsigned)(rgb.red >> 3);
        assert(psx_colour_rgb555(i) == expected);
    }
    assert(psx_colour_rgb555(0) == 0x8000 && "black stays opaque");
    assert(psx_colour_rgb555(7) == (0x8000 | 29 << 10 | 30 << 5 | 31));
}

static void test_the_sixteen_colours_stay_distinct_in_rgb555(void) {
    for (int i = 0; i < P8_PALETTE_COLORS; i++) {
        for (int j = i + 1; j < P8_PALETTE_COLORS; j++) {
            assert(psx_colour_rgb555(i) != psx_colour_rgb555(j));
        }
    }
}

static void test_a_transparent_draw_palette_byte_is_the_zero_entry(void) {
    assert(psx_colour_clut_entry(0x80) == 0x0000);
    assert(psx_colour_clut_entry(0x87) == 0x0000);
}

static void test_a_draw_palette_byte_uses_its_low_nibble(void) {
    assert(psx_colour_clut_entry(0x00) == psx_colour_rgb555(0));
    assert(psx_colour_clut_entry(0x0C) == psx_colour_rgb555(12));
    assert(psx_colour_clut_entry(0x3C) == psx_colour_rgb555(12) && "a high nibble without 0x80 is ignored");
}

int main(void) {
    test_each_palette_colour_is_its_top_five_bits_with_the_mask_bit();
    test_the_sixteen_colours_stay_distinct_in_rgb555();
    test_a_transparent_draw_palette_byte_is_the_zero_entry();
    test_a_draw_palette_byte_uses_its_low_nibble();
    printf("test_psx_colour: ok\n");
    return 0;
}
