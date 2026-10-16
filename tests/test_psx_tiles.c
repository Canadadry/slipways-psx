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

enum { CIRCLE_TILES_MAX = 4096 };
static psx_tile_t circle_tiles[CIRCLE_TILES_MAX];
static int circle_tile_count;

static void collect_tile(psx_tile_t tile, void *context) {
    assert(context == circle_tiles);
    assert(circle_tile_count < CIRCLE_TILES_MAX);
    assert((tile.w == 1 || tile.h == 1) && "a circle's tile is a run on one row or one column");
    circle_tiles[circle_tile_count++] = tile;
}

static int circ_tiles_match_software(const p8_graphics_t *gfx, int cx, int cy, int r) {
    p8_vram_init(&software);
    p8_circ(gfx, &software, cx, cy, r, 7);
    circle_tile_count = 0;
    int count = psx_tiles_circ(gfx, cx, cy, r, collect_tile, circle_tiles);
    assert(count == circle_tile_count);
    paint(circle_tiles, count);
    assert_painted_is_the_software_screen();
    unsigned long bytes = 0;
    for (int t = 0; t < count; t++) {
        bytes += circle_tiles[t].w == 1 && circle_tiles[t].h == 1 ? P8_GPU_TILE_1_BYTES : P8_GPU_TILE_BYTES;
    }
    assert(bytes <= p8_draw_cost_circ(gfx, cx, cy, r).bytes && "real bytes must fit the metered");
    return count;
}

static int circfill_spans_match_software(const p8_graphics_t *gfx, int cx, int cy, int r) {
    p8_vram_init(&software);
    p8_circfill(gfx, &software, cx, cy, r, 7);
    psx_tile_t spans[PSX_TILES_CIRCFILL_MAX];
    int count = psx_tiles_circfill(gfx, cx, cy, r, spans);
    assert(count >= 0 && count <= PSX_TILES_CIRCFILL_MAX);
    for (int s = 0; s < count; s++) {
        assert(spans[s].h == 1);
    }
    paint(spans, count);
    assert_painted_is_the_software_screen();
    p8_draw_cost_window_t window;
    p8_draw_cost_frame_begin(&window);
    assert((unsigned long)count * P8_GPU_TILE_BYTES <= p8_draw_cost_circfill(&window, gfx, cx, cy, r, 7).bytes &&
           "real bytes must fit the metered");
    return count;
}

static void test_a_circ_is_the_software_midpoint_circle(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(circ_tiles_match_software(&gfx, 80, 60, 0) == 1);
    assert(circ_tiles_match_software(&gfx, 80, 60, 1) == 4);
    assert(circ_tiles_match_software(&gfx, 80, 60, 9) > 0);
    assert(circ_tiles_match_software(&gfx, 80, 60, 59) > 0);
    assert(circ_tiles_match_software(&gfx, 80, 60, -1) == 0);
    p8_camera(&gfx, 300, -50);
    assert(circ_tiles_match_software(&gfx, 302, -48, 6) > 0 && "clipped by the screen's corner");
    assert(circ_tiles_match_software(&gfx, 380, 160, 150) > 0 && "only arcs show");
    assert(circ_tiles_match_software(&gfx, 280, 10, 19) == 0);
}

static void test_a_circ_is_one_tile_per_run_of_pixels(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(circ_tiles_match_software(&gfx, 80, 60, 2) == 8);
    assert(circ_tiles_match_software(&gfx, 80, 60, 3) == 12);
    assert(circ_tiles_match_software(&gfx, 80, 60, 4) == 16);
    assert(circ_tiles_match_software(&gfx, 80, 60, 9) == 28);
    assert(circ_tiles_match_software(&gfx, 80, 60, 24) == 64);
    assert(circ_tiles_match_software(&gfx, 80, 60, 26) == 68);
}

static void test_random_circles_draw_as_the_software_does(void) {
    unsigned seed = 2024;
    p8_graphics_t gfx;
    int clipped = 0;
    for (int i = 0; i < 200; i++) {
        int values[5];
        for (int v = 0; v < 5; v++) {
            seed = seed * 1103515245u + 12345u;
            values[v] = (int)((seed >> 8) % 281);
        }
        int cx = values[0] - 60;
        int cy = values[1] % 241 - 60;
        int r = values[2] % 71;
        p8_gfx_init(&gfx);
        p8_camera(&gfx, values[3] % 61 - 30, values[4] % 61 - 30);
        circ_tiles_match_software(&gfx, cx, cy, r);
        int sx = cx - gfx.camera_x;
        int sy = cy - gfx.camera_y;
        clipped += sx - r < 0 || sy - r < 0 || sx + r >= P8_SCREEN_WIDTH || sy + r >= P8_SCREEN_HEIGHT;
    }
    assert(clipped > 50 && "the sweep cuts circles at the screen's edges");
}

static void test_a_circfill_is_one_span_per_row_of_the_software_disc(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(circfill_spans_match_software(&gfx, 80, 60, 0) == 1);
    assert(circfill_spans_match_software(&gfx, 80, 60, 1) == 3);
    assert(circfill_spans_match_software(&gfx, 80, 60, 9) == 19);
    assert(circfill_spans_match_software(&gfx, 80, 60, 61) == P8_SCREEN_HEIGHT);
    assert(circfill_spans_match_software(&gfx, 80, 60, -1) == 0);
    p8_camera(&gfx, -20, 400);
    assert(circfill_spans_match_software(&gfx, -18, 515, 7) == 12 && "clipped by the screen's corner");
    assert(circfill_spans_match_software(&gfx, 60, 380, 19) == 0);
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

static void test_a_slipway_bundle_is_sent_segment_by_segment_as_the_software_shows_it_at_every_edge(void) {
    static const int cameras[][2] = { { 0, 0 }, { 37, -21 }, { -64, 90 } };
    p8_graphics_t gfx;
    int sent = 0;
    int dropped = 0;
    for (int c = 0; c < 3; c++) {
        p8_gfx_init(&gfx);
        p8_camera(&gfx, cameras[c][0], cameras[c][1]);
        for (int x = -37; x <= P8_SCREEN_WIDTH + 5; x += 14) {
            for (int y = -20; y <= P8_SCREEN_HEIGHT + 5; y += 13) {
                int wx = x + cameras[c][0];
                int wy = y + cameras[c][1];
                p8_segment_t bundle[4] = {
                    { wx - 1, wy - 1, wx + 29, wy + 13 }, { wx - 1, wy + 2, wx + 29, wy + 16 },
                    { wx + 2, wy - 1, wx + 32, wy + 13 }, { wx + 2, wy + 2, wx + 32, wy + 16 },
                };
                for (int i = 0; i < 4; i++) {
                    int count = line_is_sent_as_the_software_shows_it(&gfx, bundle[i].x1, bundle[i].y1, bundle[i].x2, bundle[i].y2);
                    sent += count;
                    dropped += 1 - count;
                }
            }
        }
    }
    assert(sent > 0 && dropped > 0 && "the sweep crosses every edge");
}

static void test_points_are_placed_one_by_one_as_the_software_shows_them_at_every_edge(void) {
    static const int cameras[][2] = { { 0, 0 }, { 37, -21 }, { -64, 90 } };
    p8_graphics_t gfx;
    int placed = 0;
    int dropped = 0;
    for (int c = 0; c < 3; c++) {
        p8_gfx_init(&gfx);
        p8_camera(&gfx, cameras[c][0], cameras[c][1]);
        for (int x = -2; x < P8_SCREEN_WIDTH + 2; x += 3) {
            for (int y = -2; y < P8_SCREEN_HEIGHT + 2; y += P8_SCREEN_HEIGHT + 3 - 4 * (x & 1)) {
                int count = pset_tile_matches_software(&gfx, x + cameras[c][0], y + cameras[c][1]);
                placed += count;
                dropped += 1 - count;
            }
        }
    }
    assert(placed > 0 && dropped > 0 && "the sweep crosses every edge");
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
    test_points_are_placed_one_by_one_as_the_software_shows_them_at_every_edge();
    test_a_slipway_bundle_is_sent_segment_by_segment_as_the_software_shows_it_at_every_edge();
    test_a_pset_is_one_pixel_after_the_camera();
    test_a_rect_on_screen_is_its_four_edges();
    test_a_rect_with_no_inside_is_one_tile();
    test_a_rect_is_clipped_to_the_screen_after_the_camera();
    test_a_rectfill_is_one_clipped_tile();
    test_random_boxes_draw_as_the_software_does();
    test_a_line_is_one_segment_between_its_endpoints_after_the_camera();
    test_random_lines_are_sent_when_the_software_shows_them();
    test_a_circ_is_the_software_midpoint_circle();
    test_a_circ_is_one_tile_per_run_of_pixels();
    test_random_circles_draw_as_the_software_does();
    test_a_circfill_is_one_span_per_row_of_the_software_disc();
    printf("test_psx_tiles: ok\n");
    return 0;
}
