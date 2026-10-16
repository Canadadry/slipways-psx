#include "platform/psx_vram_layout.h"

#include "p8/gpu_sizes.h"
#include "p8/image.h"

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

static void test_sheet_font_and_patterns_share_one_4_bit_page(void) {
    psx_vram_rect_t page = psx_vram_layout_texture_page();
    assert_page_aligned(page);
    assert(page.w == 64);
    assert(inside(psx_vram_layout_region(PSX_VRAM_SPRITE_SHEET), page));
    assert(inside(psx_vram_layout_region(PSX_VRAM_FONT), page));
    assert(inside(psx_vram_layout_region(PSX_VRAM_PATTERNS), page));
}

static void test_the_upscale_page_starts_at_the_draw_area(void) {
    psx_vram_rect_t page = psx_vram_layout_upscale_page();
    psx_vram_rect_t draw_area = psx_vram_layout_region(PSX_VRAM_DRAW_AREA);
    assert_page_aligned(page);
    assert(page.w == 256);
    assert(page.x == draw_area.x && page.y == draw_area.y);
    assert(inside(draw_area, page));
}

static void test_pattern_slots_fit_their_region_on_texture_window_bounds(void) {
    psx_vram_rect_t region = psx_vram_layout_region(PSX_VRAM_PATTERNS);
    psx_vram_rect_t page = psx_vram_layout_texture_page();
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

#define SLOT_SIZE(width, height) {(width), (height)},
static const p8_image_size_t SLOT_SIZES[P8_IMAGE_SLOT_COUNT] = {P8_IMAGE_SLOTS(SLOT_SIZE)};

static void test_image_slots_fit_the_images_region_on_fill_bounds(void) {
    psx_vram_rect_t region = psx_vram_layout_region(PSX_VRAM_IMAGES);
    for (int slot = 0; slot < P8_IMAGE_SLOT_COUNT; slot++) {
        psx_vram_rect_t image = psx_vram_layout_image(slot);
        assert(image.w == SLOT_SIZES[slot].width && image.h == SLOT_SIZES[slot].height);
        assert(image.x % 16 == 0 && image.w % 16 == 0);
        assert(inside(image, region));
        for (int other = slot + 1; other < P8_IMAGE_SLOT_COUNT; other++) {
            assert(!overlap(image, psx_vram_layout_image(other)));
        }
    }
}

static void test_each_image_slot_lies_on_its_own_15_bit_page(void) {
    for (int slot = 0; slot < P8_IMAGE_SLOT_COUNT; slot++) {
        psx_vram_rect_t page = psx_vram_layout_image_page(slot);
        assert_page_aligned(page);
        assert(page.w == 256);
        assert(inside(psx_vram_layout_image(slot), page));
    }
    psx_vram_rect_t first = psx_vram_layout_image(0);
    psx_vram_rect_t first_page = psx_vram_layout_image_page(0);
    assert(first.x == first_page.x && first.y == first_page.y);
}

static void test_the_small_slots_follow_the_map_on_the_lower_half_page(void) {
    assert(P8_IMAGE_SLOT_COUNT == 6);
    for (int slot = 1; slot < P8_IMAGE_SLOT_COUNT; slot++) {
        psx_vram_rect_t image = psx_vram_layout_image(slot);
        assert(image.x == 704 + (slot - 1) * 16 && image.y == 256 && image.w == 16 && image.h == 16);
        psx_vram_rect_t page = psx_vram_layout_image_page(slot);
        assert(page.x == 704 && page.y == 256);
        assert(!overlap(image, psx_vram_layout_image(0)));
    }
}

int main(void) {
    test_regions_are_inside_vram_and_never_overlap();
    test_sheet_font_and_patterns_share_one_4_bit_page();
    test_the_upscale_page_starts_at_the_draw_area();
    test_pattern_slots_fit_their_region_on_texture_window_bounds();
    test_every_clut_fits_the_clut_region_on_16_unit_bounds();
    test_image_slots_fit_the_images_region_on_fill_bounds();
    test_each_image_slot_lies_on_its_own_15_bit_page();
    test_the_small_slots_follow_the_map_on_the_lower_half_page();
    printf("test_psx_vram_layout: ok\n");
    return 0;
}
