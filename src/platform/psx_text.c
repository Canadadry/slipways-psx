#include "platform/psx_text.h"

#include "platform/psx_gpu_textures.h"

static int glyph_lit(const unsigned char *rows) {
    for (int row = 0; row < P8_FONT_GLYPH_HEIGHT; row++) {
        if (rows[row] != 0) {
            return 1;
        }
    }
    return 0;
}

void psx_text_place(const p8_graphics_t *gfx, const char *str, int x, int y,
                    void (*emit)(psx_sprite_t glyph, void *context), void *context) {
    int cx = x - gfx->camera_x;
    for (const char *p = str; *p != '\0'; p++, cx += P8_FONT_CELL_WIDTH) {
        if (!glyph_lit(p8_font_glyph_rows(*p))) {
            continue;
        }
        psx_gpu_textures_texel_t cell = psx_gpu_textures_font_cell(*p);
        psx_sprite_t glyph = { cx, y - gfx->camera_y, cell.u, cell.v, P8_FONT_GLYPH_WIDTH, P8_FONT_GLYPH_HEIGHT };
        if (psx_sprite_clip(&glyph)) {
            emit(glyph, context);
        }
    }
}
