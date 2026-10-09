#include "platform/psx_tiles.h"

#include "p8/draw_cost.h"
#include "p8/gfx.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static p8_vram_t software;
static unsigned char painted[P8_SCREEN_HEIGHT][P8_SCREEN_WIDTH];

static void paint(const psx_tile_t *tiles, int count) {
    memset(painted, 0, sizeof painted);
    for (int t = 0; t < count; t++) {
        assert(tiles[t].w >= 1 && tiles[t].h >= 1);
        assert(tiles[t].x >= 0 && tiles[t].x + tiles[t].w <= P8_SCREEN_WIDTH);
        assert(tiles[t].y >= 0 && tiles[t].y + tiles[t].h <= P8_SCREEN_HEIGHT);
        for (int y = tiles[t].y; y < tiles[t].y + tiles[t].h; y++) {
            for (int x = tiles[t].x; x < tiles[t].x + tiles[t].w; x++) {
                assert(painted[y][x] == 0 && "tiles overlap");
                painted[y][x] = 1;
            }
        }
    }
}

static void assert_painted_is_the_software_screen(void) {
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            assert(painted[y][x] == (software.screen[y][x] != 0));
        }
    }
}

static int rect_tiles_match_software(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2) {
    p8_vram_init(&software);
    p8_rect(gfx, &software, x1, y1, x2, y2, 7);
    psx_tile_t tiles[PSX_TILES_RECT_MAX];
    int count = psx_tiles_rect(gfx, x1, y1, x2, y2, tiles);
    assert(count >= 0 && count <= PSX_TILES_RECT_MAX);
    paint(tiles, count);
    assert_painted_is_the_software_screen();
    return count;
}

static int rectfill_tiles_match_software(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2) {
    p8_vram_init(&software);
    p8_rectfill(gfx, &software, x1, y1, x2, y2, 7);
    psx_tile_t tile;
    int count = psx_tiles_rectfill(gfx, x1, y1, x2, y2, &tile);
    assert(count == 0 || count == 1);
    paint(&tile, count);
    assert_painted_is_the_software_screen();
    return count;
}

static int pset_tile_matches_software(const p8_graphics_t *gfx, int x, int y) {
    p8_vram_init(&software);
    p8_pset(gfx, &software, x, y, 7);
    psx_tile_t tile;
    int count = psx_tiles_pset(gfx, x, y, &tile);
    assert(count == 0 || count == 1);
    assert(count == 0 || (tile.w == 1 && tile.h == 1));
    paint(&tile, count);
    assert_painted_is_the_software_screen();
    return count;
}

static int software_lit_any(void) {
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            if (software.screen[y][x] != 0) {
                return 1;
            }
        }
    }
    return 0;
}

static int endpoint_lit_where_on_screen(int x, int y) {
    if (x < 0 || x >= P8_SCREEN_WIDTH || y < 0 || y >= P8_SCREEN_HEIGHT) {
        return 1;
    }
    return software.screen[y][x] != 0;
}

/* The GPU's stepping between the endpoints is the accepted divergence: only the endpoints, the
 * lines sent and their metering are checked. */
static int line_is_sent_as_the_software_shows_it(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2) {
    p8_vram_init(&software);
    p8_line(gfx, &software, x1, y1, x2, y2, 7);
    psx_line_t line;
    int count = psx_tiles_line(gfx, x1, y1, x2, y2, &line);
    assert(count == 0 || count == 1);
    assert((count == 1 || !software_lit_any()) && "a line the software shows must be sent");
    if (count == 1) {
        assert(line.x1 == x1 - gfx->camera_x && line.y1 == y1 - gfx->camera_y);
        assert(line.x2 == x2 - gfx->camera_x && line.y2 == y2 - gfx->camera_y);
        assert(endpoint_lit_where_on_screen(line.x1, line.y1) && endpoint_lit_where_on_screen(line.x2, line.y2));
        assert(p8_draw_cost_line(gfx, x1, y1, x2, y2).bytes > 0 && "a line sent must be metered");
    }
    return count;
}

static void test_a_line_is_one_segment_between_its_endpoints_after_the_camera(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(line_is_sent_as_the_software_shows_it(&gfx, 10, 20, 90, 70) == 1);
    assert(line_is_sent_as_the_software_shows_it(&gfx, 90, 70, 10, 20) == 1);
    assert(line_is_sent_as_the_software_shows_it(&gfx, 5, 5, 5, 5) == 1);
    assert(line_is_sent_as_the_software_shows_it(&gfx, -50, 60, 300, 60) == 1 && "endpoints are not clipped");
    p8_camera(&gfx, 200, -100);
    assert(line_is_sent_as_the_software_shows_it(&gfx, 210, -90, 250, 10) == 1);
    assert(line_is_sent_as_the_software_shows_it(&gfx, 100, -90, 150, 10) == 0);
    assert(line_is_sent_as_the_software_shows_it(&gfx, 210, -200, 250, -101) == 0);
}

static void test_random_lines_are_sent_when_the_software_shows_them(void) {
    unsigned seed = 777;
    p8_graphics_t gfx;
    for (int i = 0; i < 200; i++) {
        int values[6];
        for (int v = 0; v < 6; v++) {
            seed = seed * 1103515245u + 12345u;
            values[v] = (int)((seed >> 8) % 400) - 120;
        }
        p8_gfx_init(&gfx);
        p8_camera(&gfx, values[4] / 2, values[5] / 2);
        line_is_sent_as_the_software_shows_it(&gfx, values[0], values[1], values[2], values[3]);
    }
}

static void test_a_pset_is_one_pixel_after_the_camera(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(pset_tile_matches_software(&gfx, 0, 0) == 1);
    assert(pset_tile_matches_software(&gfx, 159, 119) == 1);
    assert(pset_tile_matches_software(&gfx, 160, 50) == 0);
    assert(pset_tile_matches_software(&gfx, 50, -1) == 0);
    p8_camera(&gfx, -40, 300);
    assert(pset_tile_matches_software(&gfx, -40, 300) == 1);
    assert(pset_tile_matches_software(&gfx, 80, 360) == 1);
    assert(pset_tile_matches_software(&gfx, -41, 300) == 0);
    assert(pset_tile_matches_software(&gfx, 0, 420) == 0);
}

static void test_a_rect_on_screen_is_its_four_edges(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(rect_tiles_match_software(&gfx, 10, 20, 30, 40) == 4);
    assert(rect_tiles_match_software(&gfx, 30, 40, 10, 20) == 4 && "corners in any order");
}

static void test_a_rect_with_no_inside_is_one_tile(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(rect_tiles_match_software(&gfx, 5, 5, 5, 5) == 1);
    assert(rect_tiles_match_software(&gfx, 5, 5, 40, 5) == 1);
    assert(rect_tiles_match_software(&gfx, 5, 5, 5, 40) == 1);
    assert(rect_tiles_match_software(&gfx, 5, 5, 40, 6) == 1);
    assert(rect_tiles_match_software(&gfx, 5, 5, 6, 40) == 1);
}

static void test_a_rect_is_clipped_to_the_screen_after_the_camera(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_camera(&gfx, 100, -30);
    assert(rect_tiles_match_software(&gfx, 105, -25, 300, 200) == 2 && "only the left and top edges show");
    assert(rect_tiles_match_software(&gfx, 0, 0, 99, 50) == 0);
    assert(rect_tiles_match_software(&gfx, 120, -30, 140, 89) == 4);
}

static void test_a_rectfill_is_one_clipped_tile(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(rectfill_tiles_match_software(&gfx, 10, 20, 30, 40) == 1);
    assert(rectfill_tiles_match_software(&gfx, 30, 40, 10, 20) == 1);
    assert(rectfill_tiles_match_software(&gfx, -500, -500, 500, 500) == 1);
    assert(rectfill_tiles_match_software(&gfx, 160, 0, 170, 10) == 0);
    p8_camera(&gfx, -7, 13);
    assert(rectfill_tiles_match_software(&gfx, -7, 13, -7, 13) == 1);
    assert(rectfill_tiles_match_software(&gfx, -20, 0, -8, 200) == 0);
}

static void test_random_boxes_draw_as_the_software_does(void) {
    unsigned seed = 12345;
    p8_graphics_t gfx;
    for (int i = 0; i < 200; i++) {
        int values[6];
        for (int v = 0; v < 6; v++) {
            seed = seed * 1103515245u + 12345u;
            values[v] = (int)((seed >> 8) % 400) - 120;
        }
        p8_gfx_init(&gfx);
        p8_camera(&gfx, values[4] / 2, values[5] / 2);
        rect_tiles_match_software(&gfx, values[0], values[1], values[2], values[3]);
        rectfill_tiles_match_software(&gfx, values[0], values[1], values[2], values[3]);
    }
}

int main(void) {
    test_a_pset_is_one_pixel_after_the_camera();
    test_a_rect_on_screen_is_its_four_edges();
    test_a_rect_with_no_inside_is_one_tile();
    test_a_rect_is_clipped_to_the_screen_after_the_camera();
    test_a_rectfill_is_one_clipped_tile();
    test_random_boxes_draw_as_the_software_does();
    test_a_line_is_one_segment_between_its_endpoints_after_the_camera();
    test_random_lines_are_sent_when_the_software_shows_them();
    printf("test_psx_tiles: ok\n");
    return 0;
}
