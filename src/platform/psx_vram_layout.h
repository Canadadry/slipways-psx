#ifndef PLATFORM_PSX_VRAM_LAYOUT_H
#define PLATFORM_PSX_VRAM_LAYOUT_H

/* In 16-bit VRAM units: a 4-bit texture holds 4 texels per unit. */
typedef struct {
    int x, y, w, h;
} psx_vram_rect_t;

enum { PSX_VRAM_WIDTH = 1024, PSX_VRAM_HEIGHT = 512, PSX_VRAM_FONT_COLOURS = 16 };

typedef enum {
    PSX_VRAM_DISPLAY_0,
    PSX_VRAM_DISPLAY_1,
    PSX_VRAM_SPRITE_SHEET,
    PSX_VRAM_FONT,
    PSX_VRAM_PATTERNS,
    PSX_VRAM_CLUTS,
    PSX_VRAM_REGION_COUNT
} psx_vram_region_t;

psx_vram_rect_t psx_vram_layout_region(psx_vram_region_t region);

/* The 4-bit texture page (64x256 units) that holds region. */
psx_vram_rect_t psx_vram_layout_texture_page(psx_vram_region_t region);

/* A 16x16 4-bit texture (4x16 units), on the 16-texel bounds of the x2 texture window. slot <
 * GPU_PATTERN_SLOTS. */
psx_vram_rect_t psx_vram_layout_pattern_slot(int slot);

/* Each a 16-entry CLUT, 16x1 units. slot < GPU_CLUT_SLOTS, colour < PSX_VRAM_FONT_COLOURS,
 * pattern slot < GPU_PATTERN_SLOTS. */
psx_vram_rect_t psx_vram_layout_palette_clut(int slot);
psx_vram_rect_t psx_vram_layout_font_clut(int colour);
psx_vram_rect_t psx_vram_layout_pattern_clut(int slot);

#endif
