#include "platform/psx_gpu_textures.h"

#include "p8/gfx.h"
#include "platform/psx_colour.h"

#include <assert.h>
#include <string.h>

enum {
    FONT_CELL_HEIGHT = P8_FONT_GLYPH_HEIGHT + 1,
    FONT_COLUMNS = PSX_GPU_TEXTURES_FONT_SIZE / P8_FONT_CELL_WIDTH,
    FONT_LINES = 256 / FONT_COLUMNS * FONT_CELL_HEIGHT,
    OUTLINED_CELL_WIDTH = P8_FONT_GLYPH_WIDTH + 2,
    OUTLINED_CELL_HEIGHT = P8_FONT_GLYPH_HEIGHT + 2,
    OUTLINED_COLUMNS = PSX_GPU_TEXTURES_FONT_SIZE / OUTLINED_CELL_WIDTH,
    OUTLINED_ROWS = (256 + OUTLINED_COLUMNS - 1) / OUTLINED_COLUMNS,
};

typedef char every_character_code_has_a_cell[256 % FONT_COLUMNS == 0 && FONT_LINES <= (int)PSX_GPU_TEXTURES_FONT_SIZE ? 1 : -1];
typedef char every_character_code_has_an_outlined_cell[FONT_LINES + OUTLINED_ROWS * OUTLINED_CELL_HEIGHT <= (int)PSX_GPU_TEXTURES_FONT_SIZE ? 1 : -1];

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

psx_gpu_textures_texel_t psx_gpu_textures_outlined_font_cell(char character) {
    int code = (unsigned char)character;
    psx_gpu_textures_texel_t cell = { code % OUTLINED_COLUMNS * OUTLINED_CELL_WIDTH,
                                      FONT_LINES + code / OUTLINED_COLUMNS * OUTLINED_CELL_HEIGHT };
    return cell;
}

static int glyph_lit(const unsigned char *rows, int column, int row) {
    if (column < 0 || column >= P8_FONT_GLYPH_WIDTH || row < 0 || row >= P8_FONT_GLYPH_HEIGHT) {
        return 0;
    }
    return (rows[row] >> (P8_FONT_GLYPH_WIDTH - 1 - column)) & 1;
}

static int outlined(const unsigned char *rows, int column, int row) {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (glyph_lit(rows, column + dx, row + dy)) {
                return 1;
            }
        }
    }
    return 0;
}

static void set_texel(uint16_t texture[PSX_GPU_TEXTURES_FONT_UNITS], int u, int v, unsigned texel) {
    texture[v * (PSX_GPU_TEXTURES_FONT_SIZE / 4) + u / 4] |= (uint16_t)(texel << (4 * (u % 4)));
}

void psx_gpu_textures_font(uint16_t texture[PSX_GPU_TEXTURES_FONT_UNITS]) {
    memset(texture, 0, PSX_GPU_TEXTURES_FONT_UNITS * sizeof texture[0]);
    for (int code = 0; code < 256; code++) {
        psx_gpu_textures_texel_t cell = psx_gpu_textures_font_cell((char)code);
        psx_gpu_textures_texel_t outlined_cell = psx_gpu_textures_outlined_font_cell((char)code);
        const unsigned char *rows = p8_font_glyph_rows((char)code);
        for (int row = -1; row <= P8_FONT_GLYPH_HEIGHT; row++) {
            for (int column = -1; column <= P8_FONT_GLYPH_WIDTH; column++) {
                int u = outlined_cell.u + 1 + column;
                int v = outlined_cell.v + 1 + row;
                if (glyph_lit(rows, column, row)) {
                    set_texel(texture, cell.u + column, cell.v + row, 1);
                    set_texel(texture, u, v, 1);
                } else if (outlined(rows, column, row)) {
                    set_texel(texture, u, v, 2);
                }
            }
        }
    }
}

void psx_gpu_textures_font_clut(int colour, uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES]) {
    assert(colour >= 0 && colour < 16);
    memset(clut, 0, PSX_GPU_TEXTURES_CLUT_ENTRIES * sizeof clut[0]);
    clut[1] = psx_colour_rgb555(colour);
    clut[2] = psx_colour_rgb555(0);
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
