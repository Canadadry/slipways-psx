#include "platform/psx_colour.h"

#include "p8/palette.h"
#include "p8/vram.h"

#include <assert.h>

uint16_t psx_colour_rgb555(int index) {
    assert(index >= 0 && index < P8_PALETTE_COLORS);
    p8_rgb_t rgb = P8_PALETTE_RGB[index];
    return (uint16_t)(0x8000u | (unsigned)(rgb.blue >> 3) << 10 | (unsigned)(rgb.green >> 3) << 5
                      | (unsigned)(rgb.red >> 3));
}

uint16_t psx_colour_clut_entry(unsigned char draw_palette_byte) {
    if (draw_palette_byte & P8_DRAW_PALETTE_TRANSPARENT_BIT) {
        return 0x0000;
    }
    return psx_colour_rgb555(draw_palette_byte & 0x0F);
}
