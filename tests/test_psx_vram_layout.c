#include "platform/psx_vram_layout.h"

#include "p8/gpu_sizes.h"

#include <assert.h>
#include <stdio.h>

static int overlap(psx_vram_rect_t a, psx_vram_rect_t b) {
    return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
}

static void test_regions_are_inside_vram_and_never_overlap(void) {
    for (int i = 0; i < PSX_VRAM_REGION_COUNT; i++) {
        psx_vram_rect_t region = psx_vram_layout_region((psx_vram_region_t)i);
        assert(region.w > 0 && region.h > 0);
        assert(region.x >= 0 && region.x + region.w <= PSX_VRAM_WIDTH);
        assert(region.y >= 0 && region.y + region.h <= PSX_VRAM_HEIGHT);
        for (int j = i + 1; j < PSX_VRAM_REGION_COUNT; j++) {
            assert(!overlap(region, psx_vram_layout_region((psx_vram_region_t)j)));
        }
    }
}

static int inside(psx_vram_rect_t inner, psx_vram_rect_t outer) {
    return inner.x >= outer.x && inner.x + inner.w <= outer.x + outer.w && inner.y >= outer.y
        && inner.y + inner.h <= outer.y + outer.h;
}

static void assert_page_aligned(psx_vram_rect_t page) {
    assert(page.x % 64 == 0);
    assert(page.y % 256 == 0);
    assert(page.h == 256);
}

static void test_sheet_font_and_patterns_each_lie_in_one_4_bit_page(void) {
    const psx_vram_region_t textures[] = { PSX_VRAM_SPRITE_SHEET, PSX_VRAM_FONT, PSX_VRAM_PATTERNS };
    for (int i = 0; i < 3; i++) {
        psx_vram_rect_t page = psx_vram_layout_texture_page(textures[i]);
        assert_page_aligned(page);
        assert(page.w == 64);
        assert(inside(psx_vram_layout_region(textures[i]), page));
    }
    psx_vram_rect_t sheet = psx_vram_layout_region(PSX_VRAM_SPRITE_SHEET);
    psx_vram_rect_t sheet_page = psx_vram_layout_texture_page(PSX_VRAM_SPRITE_SHEET);
    assert(sheet.x == sheet_page.x && sheet.y == sheet_page.y && sheet.w == sheet_page.w && sheet.h == sheet_page.h);
}

static void test_the_font_holds_256_cells_of_8x12_texels(void) {
    psx_vram_rect_t font = psx_vram_layout_region(PSX_VRAM_FONT);
    assert(font.w * 4 == 32 * 8 && font.h == 8 * 12);
}

static void test_pattern_slots_fit_their_region_on_texture_window_bounds(void) {
    psx_vram_rect_t region = psx_vram_layout_region(PSX_VRAM_PATTERNS);
    psx_vram_rect_t page = psx_vram_layout_texture_page(PSX_VRAM_PATTERNS);
    for (int slot = 0; slot < GPU_PATTERN_SLOTS; slot++) {
        psx_vram_rect_t texture = psx_vram_layout_pattern_slot(slot);
        assert(texture.w == 2 && texture.h == 8);
        assert(inside(texture, region));
        assert((texture.x - page.x) % 2 == 0);
        assert((texture.y - page.y) % 8 == 0);
        for (int other = slot + 1; other < GPU_PATTERN_SLOTS; other++) {
            assert(!overlap(texture, psx_vram_layout_pattern_slot(other)));
        }
    }
}

enum { CLUT_COUNT = GPU_CLUT_SLOTS + PSX_VRAM_FONT_COLOURS + GPU_PATTERN_SLOTS };

static void test_every_clut_fits_the_clut_region_on_16_unit_bounds(void) {
    psx_vram_rect_t cluts[CLUT_COUNT];
    int count = 0;
    for (int slot = 0; slot < GPU_CLUT_SLOTS; slot++) {
        cluts[count++] = psx_vram_layout_palette_clut(slot);
    }
    for (int colour = 0; colour < PSX_VRAM_FONT_COLOURS; colour++) {
        cluts[count++] = psx_vram_layout_font_clut(colour);
    }
    for (int slot = 0; slot < GPU_PATTERN_SLOTS; slot++) {
        cluts[count++] = psx_vram_layout_pattern_clut(slot);
    }
    psx_vram_rect_t region = psx_vram_layout_region(PSX_VRAM_CLUTS);
    for (int i = 0; i < count; i++) {
        assert(cluts[i].w == 16 && cluts[i].h == 1);
        assert(cluts[i].x % 16 == 0);
        assert(inside(cluts[i], region));
        for (int j = i + 1; j < count; j++) {
            assert(!overlap(cluts[i], cluts[j]));
        }
    }
}

int main(void) {
    test_regions_are_inside_vram_and_never_overlap();
    test_sheet_font_and_patterns_each_lie_in_one_4_bit_page();
    test_the_font_holds_256_cells_of_8x12_texels();
    test_pattern_slots_fit_their_region_on_texture_window_bounds();
    test_every_clut_fits_the_clut_region_on_16_unit_bounds();
    printf("test_psx_vram_layout: ok\n");
    return 0;
}
