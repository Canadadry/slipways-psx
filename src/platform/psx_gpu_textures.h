#ifndef PLATFORM_PSX_GPU_TEXTURES_H
#define PLATFORM_PSX_GPU_TEXTURES_H

#include <stdint.h>

enum {
    PSX_GPU_TEXTURES_CLUT_ENTRIES = 16,
    PSX_GPU_TEXTURES_FONT_SIZE = 128,
    PSX_GPU_TEXTURES_FONT_UNITS = PSX_GPU_TEXTURES_FONT_SIZE * PSX_GPU_TEXTURES_FONT_SIZE / 4,
    PSX_GPU_TEXTURES_PATTERN_SIZE = 8,
    PSX_GPU_TEXTURES_PATTERN_UNITS = PSX_GPU_TEXTURES_PATTERN_SIZE * PSX_GPU_TEXTURES_PATTERN_SIZE / 4,
};

typedef struct {
    int u, v;
} psx_gpu_textures_texel_t;

/* spr's CLUT: entry i is psx_colour_clut_entry(draw_palette[i]), so 0x0000 is transparent. */
void psx_gpu_textures_clut(const unsigned char draw_palette[PSX_GPU_TEXTURES_CLUT_ENTRIES],
                           uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES]);

/* The font texture, PSX_GPU_TEXTURES_FONT_SIZE texels square, 4 bits a texel, the leftmost of a
 * unit's four in its low nibble: each of the 256 character codes' glyph in its own cell, texel 1
 * lit, every other texel 0. */
void psx_gpu_textures_font(uint16_t texture[PSX_GPU_TEXTURES_FONT_UNITS]);

/* The top-left texel of character's glyph in the font texture. */
psx_gpu_textures_texel_t psx_gpu_textures_font_cell(char character);

/* print's CLUT for colour (0..15): entry 1 is psx_colour_rgb555(colour), every other entry 0x0000. */
void psx_gpu_textures_font_clut(int colour, uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES]);

/* A fillp pattern's tile, PSX_GPU_TEXTURES_PATTERN_SIZE texels square, laid out as the font: texel
 * (u, v) is 1 when the pattern's bit for world (u, v) is set, else 0. */
void psx_gpu_textures_pattern(unsigned int pattern, uint16_t tile[PSX_GPU_TEXTURES_PATTERN_UNITS]);

/* A patterned fill's CLUT: entry 0 is colour col & 0x0F, entry 1 is (col >> 4) & 0x0F, or 0x0000
 * when transparent; every other entry 0x0000. */
void psx_gpu_textures_pattern_clut(int col, int transparent, uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES]);

#endif
