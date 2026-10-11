#include "platform/psx_gpu_textures.h"

#include "p8/gfx.h"
#include "platform/psx_colour.h"

#include <assert.h>
#include <string.h>

enum { FONT_CELL_HEIGHT = P8_FONT_GLYPH_HEIGHT + 1, FONT_COLUMNS = PSX_GPU_TEXTURES_FONT_SIZE / P8_FONT_CELL_WIDTH };

typedef char every_character_code_has_a_cell[256 / FONT_COLUMNS * FONT_CELL_HEIGHT <= PSX_GPU_TEXTURES_FONT_SIZE ? 1 : -1];

void psx_gpu_textures_clut(const unsigned char draw_palette[PSX_GPU_TEXTURES_CLUT_ENTRIES],
                           uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES]) {
    for (int i = 0; i < PSX_GPU_TEXTURES_CLUT_ENTRIES; i++) {
        clut[i] = psx_colour_clut_entry(draw_palette[i]);
    }
}

psx_gpu_textures_texel_t psx_gpu_textures_font_cell(char character) {
    int code = (unsigned char)character;
    psx_gpu_textures_texel_t cell = { code % FONT_COLUMNS * P8_FONT_CELL_WIDTH, code / FONT_COLUMNS * FONT_CELL_HEIGHT };
    return cell;
}

void psx_gpu_textures_font(uint16_t texture[PSX_GPU_TEXTURES_FONT_UNITS]) {
    memset(texture, 0, PSX_GPU_TEXTURES_FONT_UNITS * sizeof texture[0]);
    for (int code = 0; code < 256; code++) {
        psx_gpu_textures_texel_t cell = psx_gpu_textures_font_cell((char)code);
        const unsigned char *rows = p8_font_glyph_rows((char)code);
        for (int row = 0; row < P8_FONT_GLYPH_HEIGHT; row++) {
            for (int column = 0; column < P8_FONT_GLYPH_WIDTH; column++) {
                if ((rows[row] >> (P8_FONT_GLYPH_WIDTH - 1 - column)) & 1) {
                    int u = cell.u + column;
                    texture[(cell.v + row) * (PSX_GPU_TEXTURES_FONT_SIZE / 4) + u / 4] |= (uint16_t)(1u << (4 * (u % 4)));
                }
            }
        }
    }
}

void psx_gpu_textures_font_clut(int colour, uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES]) {
    assert(colour >= 0 && colour < 16);
    memset(clut, 0, PSX_GPU_TEXTURES_CLUT_ENTRIES * sizeof clut[0]);
    clut[1] = psx_colour_rgb555(colour);
}

void psx_gpu_textures_pattern(unsigned int pattern, uint16_t tile[PSX_GPU_TEXTURES_PATTERN_UNITS]) {
    memset(tile, 0, PSX_GPU_TEXTURES_PATTERN_UNITS * sizeof tile[0]);
    for (int v = 0; v < PSX_GPU_TEXTURES_PATTERN_SIZE; v++) {
        for (int u = 0; u < PSX_GPU_TEXTURES_PATTERN_SIZE; u++) {
            int bit = 15 - (v % 4 * 4 + u % 4);
            if ((pattern >> bit) & 1) {
                tile[v * (PSX_GPU_TEXTURES_PATTERN_SIZE / 4) + u / 4] |= (uint16_t)(1u << (4 * (u % 4)));
            }
        }
    }
}

void psx_gpu_textures_pattern_clut(int col, int transparent, uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES]) {
    memset(clut, 0, PSX_GPU_TEXTURES_CLUT_ENTRIES * sizeof clut[0]);
    clut[0] = psx_colour_rgb555(col & 0x0F);
    clut[1] = transparent ? 0x0000 : psx_colour_rgb555((col >> 4) & 0x0F);
}
