#include "platform/psx_vram_layout.h"

#include "p8/gpu_sizes.h"
#include "p8/image.h"

#include <assert.h>

static const psx_vram_rect_t regions[PSX_VRAM_REGION_COUNT] = {
    [PSX_VRAM_DISPLAY_0] = { 0, 0, 320, 240 },
    [PSX_VRAM_DISPLAY_1] = { 0, 240, 320, 240 },
    [PSX_VRAM_DRAW_AREA] = { 320, 0, 160, 120 },
    [PSX_VRAM_SPRITE_SHEET] = { 512, 0, 32, 128 },
    [PSX_VRAM_FONT] = { 512, 128, 32, 128 },
    [PSX_VRAM_PATTERNS] = { 544, 0, 32, 256 },
    [PSX_VRAM_CLUTS] = { 640, 0, 16, 64 },
    [PSX_VRAM_IMAGES] = { 704, 0, 256, 512 },
};

/* The 48x8 slots go five a band down from (704, 256), each on a 16-unit bound. */
#define SMALL_SLOT(i) { 704 + (i) % 5 * 48, 256 + (i) / 5 * 8, 48, 8 },
#define SMALL_SLOTS_8(first)                                                                                      \
    SMALL_SLOT(first) SMALL_SLOT(first + 1) SMALL_SLOT(first + 2) SMALL_SLOT(first + 3) SMALL_SLOT(first + 4) \
    SMALL_SLOT(first + 5) SMALL_SLOT(first + 6) SMALL_SLOT(first + 7)

/* The 128x8 slots go two a band down from (704, 312), below the 48x8 ones. */
#define WIDE_SLOT(i) { 704 + (i) % 2 * 128, 312 + (i) / 2 * 8, 128, 8 },
#define WIDE_SLOTS_8(first)                                                                                   \
    WIDE_SLOT(first) WIDE_SLOT(first + 1) WIDE_SLOT(first + 2) WIDE_SLOT(first + 3) WIDE_SLOT(first + 4) \
    WIDE_SLOT(first + 5) WIDE_SLOT(first + 6) WIDE_SLOT(first + 7)

/* The 16x16 slots go in a row from (704, 440), below the 128x8 ones. */
#define SQUARE_SLOT(i) { 704 + (i) * 16, 440, 16, 16 },

/* The 16x11 button slots go in a row from (784, 440), right of the 16x16 ones. */
#define BUTTON_ICON_SLOT(i) { 784 + (i) * 16, 440, 16, 11 },

/* The 112x11 button slots go two a band down from (704, 456), below the 16x16 ones. */
#define BUTTON_WIDE_SLOT(i) { 704 + (i) % 2 * 112, 456 + (i) / 2 * 11, 112, 11 },

static const psx_vram_rect_t image_slots[P8_IMAGE_SLOT_COUNT] = {
    { 704, 0, 256, 256 },
    SMALL_SLOTS_8(0) SMALL_SLOTS_8(8) SMALL_SLOTS_8(16) SMALL_SLOTS_8(24)
    WIDE_SLOTS_8(0) WIDE_SLOTS_8(8) WIDE_SLOTS_8(16) WIDE_SLOTS_8(24)
    SQUARE_SLOT(0) SQUARE_SLOT(1) SQUARE_SLOT(2) SQUARE_SLOT(3) SQUARE_SLOT(4)
    BUTTON_ICON_SLOT(0) BUTTON_ICON_SLOT(1) BUTTON_ICON_SLOT(2) BUTTON_ICON_SLOT(3)
    BUTTON_ICON_SLOT(4) BUTTON_ICON_SLOT(5) BUTTON_ICON_SLOT(6) BUTTON_ICON_SLOT(7)
    BUTTON_WIDE_SLOT(0) BUTTON_WIDE_SLOT(1) BUTTON_WIDE_SLOT(2) BUTTON_WIDE_SLOT(3) BUTTON_WIDE_SLOT(4)
    BUTTON_WIDE_SLOT(5) BUTTON_WIDE_SLOT(6) BUTTON_WIDE_SLOT(7) BUTTON_WIDE_SLOT(8) BUTTON_WIDE_SLOT(9)
};

psx_vram_rect_t psx_vram_layout_region(psx_vram_region_t region) {
    assert(region >= 0 && region < PSX_VRAM_REGION_COUNT);
    return regions[region];
}

psx_vram_rect_t psx_vram_layout_texture_page(void) {
    psx_vram_rect_t page = { regions[PSX_VRAM_SPRITE_SHEET].x, regions[PSX_VRAM_SPRITE_SHEET].y, 64, 256 };
    return page;
}

psx_vram_rect_t psx_vram_layout_upscale_page(void) {
    psx_vram_rect_t page = { regions[PSX_VRAM_DRAW_AREA].x, regions[PSX_VRAM_DRAW_AREA].y, 256, 256 };
    return page;
}

enum { PATTERN_SLOT_WIDTH = 2, PATTERN_SLOT_HEIGHT = 8 };

psx_vram_rect_t psx_vram_layout_pattern_slot(int slot) {
    assert(slot >= 0 && slot < GPU_PATTERN_SLOTS);
    psx_vram_rect_t region = regions[PSX_VRAM_PATTERNS];
    int per_row = region.w / PATTERN_SLOT_WIDTH;
    psx_vram_rect_t texture = { region.x + slot % per_row * PATTERN_SLOT_WIDTH,
                                region.y + slot / per_row * PATTERN_SLOT_HEIGHT, PATTERN_SLOT_WIDTH,
                                PATTERN_SLOT_HEIGHT };
    return texture;
}

static psx_vram_rect_t clut_at(int line) {
    psx_vram_rect_t clut = { regions[PSX_VRAM_CLUTS].x, regions[PSX_VRAM_CLUTS].y + line, 16, 1 };
    return clut;
}

psx_vram_rect_t psx_vram_layout_palette_clut(int slot) {
    assert(slot >= 0 && slot < GPU_CLUT_SLOTS);
    return clut_at(slot);
}

psx_vram_rect_t psx_vram_layout_font_clut(int colour) {
    assert(colour >= 0 && colour < PSX_VRAM_FONT_COLOURS);
    return clut_at(GPU_CLUT_SLOTS + colour);
}

psx_vram_rect_t psx_vram_layout_pattern_clut(int slot) {
    assert(slot >= 0 && slot < GPU_PATTERN_SLOTS);
    return clut_at(GPU_CLUT_SLOTS + PSX_VRAM_FONT_COLOURS + slot);
}

psx_vram_rect_t psx_vram_layout_image(int slot) {
    assert(slot >= 0 && slot < P8_IMAGE_SLOT_COUNT);
    return image_slots[slot];
}

enum { PAGE_X_STEP = 64, PAGE_SIZE = 256 };

psx_vram_rect_t psx_vram_layout_image_page(int slot) {
    psx_vram_rect_t image = psx_vram_layout_image(slot);
    psx_vram_rect_t page = { image.x - image.x % PAGE_X_STEP, image.y - image.y % PAGE_SIZE, PAGE_SIZE, PAGE_SIZE };
    return page;
}
