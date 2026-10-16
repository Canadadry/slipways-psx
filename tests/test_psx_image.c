#include "platform/psx_image.h"

#include "p8/image.h"
#include "platform/psx_vram_layout.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

enum { NOTHING = -1 };

static p8_graphics_t camera_at(int x, int y) {
    p8_graphics_t gfx;
    memset(&gfx, 0, sizeof gfx);
    gfx.camera_x = x;
    gfx.camera_y = y;
    return gfx;
}

static int texel(int u, int v) {
    return v * 256 + u;
}

/* The screen the copy's SPRT paints, each pixel the texel it reads (u, v in the slot's page). */
static void paint_copy(const p8_graphics_t *gfx, int slot, int w, int h, int x, int y,
                       int screen[P8_SCREEN_HEIGHT][P8_SCREEN_WIDTH]) {
    for (int sy = 0; sy < P8_SCREEN_HEIGHT; sy++) {
        for (int sx = 0; sx < P8_SCREEN_WIDTH; sx++) {
            screen[sy][sx] = NOTHING;
        }
    }
    psx_sprite_t sprite;
    if (!psx_image_copy_place(gfx, slot, w, h, x, y, &sprite)) {
        return;
    }
    assert(sprite.w >= 1 && sprite.h >= 1);
    assert(sprite.x >= 0 && sprite.x + sprite.w <= P8_SCREEN_WIDTH);
    assert(sprite.y >= 0 && sprite.y + sprite.h <= P8_SCREEN_HEIGHT);
    assert(sprite.u >= 0 && sprite.u + sprite.w <= 256 && sprite.v >= 0 && sprite.v + sprite.h <= 256);
    for (int j = 0; j < sprite.h; j++) {
        for (int i = 0; i < sprite.w; i++) {
            screen[sprite.y + j][sprite.x + i] = texel(sprite.u + i, sprite.v + j);
        }
    }
}

/* What p8_image_draw shows: image pixel (ix, iy) at world (x + ix, y + iy) under the camera. */
static void expected_copy(const p8_graphics_t *gfx, int slot, int w, int h, int x, int y,
                          int screen[P8_SCREEN_HEIGHT][P8_SCREEN_WIDTH]) {
    psx_vram_rect_t image = psx_vram_layout_image(slot);
    psx_vram_rect_t page = psx_vram_layout_image_page(slot);
    for (int sy = 0; sy < P8_SCREEN_HEIGHT; sy++) {
        for (int sx = 0; sx < P8_SCREEN_WIDTH; sx++) {
            int ix = sx + gfx->camera_x - x;
            int iy = sy + gfx->camera_y - y;
            int inside = ix >= 0 && ix < w && iy >= 0 && iy < h;
            screen[sy][sx] = inside ? texel(image.x - page.x + ix, image.y - page.y + iy) : NOTHING;
        }
    }
}

static void assert_slot_copy_matches(int slot, int camera_x, int camera_y, int w, int h, int x, int y) {
    static int painted[P8_SCREEN_HEIGHT][P8_SCREEN_WIDTH];
    static int expected[P8_SCREEN_HEIGHT][P8_SCREEN_WIDTH];
    p8_graphics_t gfx = camera_at(camera_x, camera_y);
    paint_copy(&gfx, slot, w, h, x, y, painted);
    expected_copy(&gfx, slot, w, h, x, y, expected);
    if (memcmp(painted, expected, sizeof painted) != 0) {
        fprintf(stderr, "copy differs: slot %d camera (%d, %d) image %dx%d at (%d, %d)\n", slot, camera_x, camera_y, w,
                h, x, y);
        assert(0);
    }
}

static void assert_copy_matches(int camera_x, int camera_y, int w, int h, int x, int y) {
    assert_slot_copy_matches(0, camera_x, camera_y, w, h, x, y);
}

static void test_the_copy_shows_the_image_under_the_camera_over_a_sweep(void) {
    static const int cameras[] = { -300, -257, -161, -160, -40, -7, -1, 0, 1, 3, 47, 96, 97, 120, 135, 136, 159, 255, 256, 400 };
    int count = (int)(sizeof cameras / sizeof cameras[0]);
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count; j++) {
            assert_copy_matches(cameras[i], cameras[j], 256, 256, 0, 0);
            assert_copy_matches(cameras[i], cameras[j], 100, 50, 30, -20);
        }
    }
}

static void test_a_copy_off_screen_places_nothing(void) {
    psx_sprite_t sprite;
    p8_graphics_t gfx = camera_at(256, 0);
    assert(!psx_image_copy_place(&gfx, 0, 256, 256, 0, 0, &sprite));
    gfx = camera_at(0, -120);
    assert(!psx_image_copy_place(&gfx, 0, 256, 256, 0, 0, &sprite));
    gfx = camera_at(0, 0);
    assert(!psx_image_copy_place(&gfx, 0, 0, 16, 0, 0, &sprite));
}

static void test_the_copy_reads_the_slot_from_its_page(void) {
    psx_sprite_t sprite;
    p8_graphics_t gfx = camera_at(100, 200);
    assert(psx_image_copy_place(&gfx, 0, 256, 256, 0, 0, &sprite));
    psx_sprite_t expected = { 0, 0, 100, 200, 156, 56 };
    assert(memcmp(&sprite, &expected, sizeof sprite) == 0);
}

enum { SLOT_AT_752 = 2, SLOT_AT_800 = 3 };

static void test_a_48_by_8_slot_off_its_page_edge_copies_and_clips_at_every_edge(void) {
    static const int xs[] = { -44, -43, -1, 0, 60, 116, 117, 160 };
    static const int ys[] = { -8, -7, -1, 0, 56, 112, 113, 120 };
    int count = (int)(sizeof xs / sizeof xs[0]);
    assert(psx_vram_layout_image(SLOT_AT_752).x == 752 && psx_vram_layout_image(SLOT_AT_800).x == 800);
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count; j++) {
            assert_slot_copy_matches(SLOT_AT_752, 0, 0, 44, 8, xs[i], ys[j]);
            assert_slot_copy_matches(SLOT_AT_800, -13, 21, 44, 8, xs[i] - 13, ys[j] + 21);
        }
    }
}

static void test_a_48_by_8_slot_reads_from_its_own_page_column(void) {
    psx_sprite_t sprite;
    p8_graphics_t gfx = camera_at(0, 0);
    assert(psx_image_copy_place(&gfx, SLOT_AT_752, 44, 8, 30, 40, &sprite));
    psx_sprite_t at_752 = { 30, 40, 48, 0, 44, 8 };
    assert(memcmp(&sprite, &at_752, sizeof sprite) == 0);
    psx_vram_rect_t page = psx_vram_layout_image_page(SLOT_AT_800);
    assert(page.x == 768 && page.y == 256);
    assert(psx_image_copy_place(&gfx, SLOT_AT_800, 44, 8, -5, 116, &sprite));
    psx_sprite_t at_800 = { 0, 116, 32 + 5, 0, 39, 4 };
    assert(memcmp(&sprite, &at_800, sizeof sprite) == 0);
}

static void assert_rect(psx_vram_rect_t rect, int x, int y, int w, int h) {
    assert(rect.x == x && rect.y == y && rect.w == w && rect.h == h);
}

static void test_each_window_draws_into_its_part_of_the_slot(void) {
    assert_rect(psx_image_window_area(0, 256, 256, 0, 0), 704, 0, 160, 120);
    assert_rect(psx_image_window_area(0, 256, 256, 160, 0), 864, 0, 96, 120);
    assert_rect(psx_image_window_area(0, 256, 256, 0, 240), 704, 240, 160, 16);
    assert_rect(psx_image_window_area(0, 256, 256, 160, 240), 864, 240, 96, 16);
    assert_rect(psx_image_window_area(0, 100, 50, 0, 0), 704, 0, 100, 50);
}

/* Every pixel of a w x h build is in exactly one window's area, at its image position. */
static void assert_windows_tile(int w, int h) {
    static int hits[256][256];
    memset(hits, 0, sizeof hits);
    psx_vram_rect_t image = psx_vram_layout_image(0);
    for (int y = 0; y < h; y += P8_IMAGE_WINDOW_HEIGHT) {
        for (int x = 0; x < w; x += P8_IMAGE_WINDOW_WIDTH) {
            psx_vram_rect_t area = psx_image_window_area(0, w, h, x, y);
            assert(area.x == image.x + x && area.y == image.y + y);
            assert(area.w >= 1 && area.w <= P8_SCREEN_WIDTH && area.h >= 1 && area.h <= P8_SCREEN_HEIGHT);
            for (int j = 0; j < area.h; j++) {
                for (int i = 0; i < area.w; i++) {
                    hits[area.y - image.y + j][area.x - image.x + i]++;
                }
            }
        }
    }
    for (int iy = 0; iy < 256; iy++) {
        for (int ix = 0; ix < 256; ix++) {
            assert(hits[iy][ix] == (ix < w && iy < h));
        }
    }
}

static void test_the_windows_tile_the_build_exactly(void) {
    assert_windows_tile(256, 256);
    assert_windows_tile(161, 121);
    assert_windows_tile(16, 1);
    assert_windows_tile(160, 120);
}

int main(void) {
    test_the_copy_shows_the_image_under_the_camera_over_a_sweep();
    test_a_copy_off_screen_places_nothing();
    test_the_copy_reads_the_slot_from_its_page();
    test_a_48_by_8_slot_off_its_page_edge_copies_and_clips_at_every_edge();
    test_a_48_by_8_slot_reads_from_its_own_page_column();
    test_each_window_draws_into_its_part_of_the_slot();
    test_the_windows_tile_the_build_exactly();
    printf("test_psx_image: ok\n");
    return 0;
}
