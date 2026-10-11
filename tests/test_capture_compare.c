#include "devtools/capture_compare.h"

#include "platform/psx_colour.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define HEADER "psx-vram 320 240 16\n"

enum { HEADER_BYTES = sizeof HEADER - 1, W = CAPTURE_COMPARE_SCREEN_WIDTH, H = CAPTURE_COMPARE_SCREEN_HEIGHT };

static unsigned char capture[HEADER_BYTES + CAPTURE_COMPARE_PIXEL_BYTES];
static unsigned char pc[H * W];
static capture_compare_image_t psx;
static char error[CAPTURE_COMPARE_ERROR_CAP];

static void put_raw(int x, int y, unsigned value) {
    unsigned char *at = capture + HEADER_BYTES + ((size_t)y * 2 * W + (size_t)x) * 2;
    at[0] = (unsigned char)(value & 0xFF);
    at[1] = (unsigned char)(value >> 8);
}

/* One downscaled pixel: its whole 2x2 block. */
static void put_block(int x, int y, unsigned value) {
    for (int dy = 0; dy < 2; dy++) {
        for (int dx = 0; dx < 2; dx++) put_raw(2 * x + dx, 2 * y + dy, value);
    }
}

static void fill_capture(int index) {
    memcpy(capture, HEADER, HEADER_BYTES);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) put_block(x, y, psx_colour_rgb555(index));
    }
}

static void fill_pc(int index) {
    memset(pc, index, sizeof pc);
}

static int decode(void) {
    error[0] = '\0';
    return capture_compare_decode(capture, sizeof capture, &psx, error);
}

static void test_a_bad_header_or_size_is_refused_naming_both(void) {
    fill_capture(0);
    memcpy(capture, "psx-vram 320 240 24\n", HEADER_BYTES);
    assert(!decode());
    assert(strstr(error, "psx-vram 320 240 24") != NULL && strstr(error, "psx-vram 320 240 16") != NULL);

    fill_capture(0);
    memcpy(capture, "psx-vram 640 480 16\n", HEADER_BYTES);
    assert(!decode());

    fill_capture(0);
    assert(!capture_compare_decode(capture, sizeof capture - 2, &psx, error));
    assert(strstr(error, "153618") != NULL && strstr(error, "153620") != NULL);

    assert(!capture_compare_decode((const unsigned char *)"no header at all", 16, &psx, error));
}

static void test_a_uniform_capture_of_each_colour_maps_to_its_index(void) {
    for (int index = 0; index < 16; index++) {
        fill_capture(index);
        assert(decode());
        assert(psx.nonuniform == 0 && psx.offpalette == 0);
        assert(psx.index[0][0] == index && psx.index[H - 1][W - 1] == index && psx.index[57][93] == index);
    }
}

static void test_the_mask_bit_is_ignored_and_red_is_the_low_bits(void) {
    fill_capture(0);
    put_block(3, 4, psx_colour_rgb555(8) & 0x7FFF);
    assert(decode());
    assert(psx.index[4][3] == 8 && psx.offpalette == 0);
    put_block(3, 4, 0x8000 | 31 << 10 | 9 << 5);
    assert(decode());
    assert(psx.index[4][3] == CAPTURE_COMPARE_OFF_PALETTE && "blue and red swapped is no palette colour");
}

static void test_an_off_palette_pixel_and_a_non_uniform_block_are_counted(void) {
    fill_capture(1);
    put_block(10, 20, 0x8000 | 1);
    put_raw(2 * 30 + 1, 2 * 40 + 1, psx_colour_rgb555(2));
    assert(decode());
    assert(psx.offpalette == 1);
    assert(psx.index[20][10] == CAPTURE_COMPARE_OFF_PALETTE);
    assert(psx.nonuniform == 1);
    assert(psx.index[40][30] == 1 && "the top-left pixel is the downscaled one");
}

static capture_compare_metrics_t measure(void) {
    assert(decode());
    return capture_compare_measure(pc, &psx);
}

static void test_equal_frames_differ_nowhere(void) {
    fill_capture(5);
    fill_pc(5);
    capture_compare_metrics_t m = measure();
    assert(m.differ == 0 && m.near1 == 0 && m.near2 == 0 && m.far == 0);
    assert(m.bbox_x0 < 0);
    char line[CAPTURE_COMPARE_REPORT_CAP];
    capture_compare_report_line(line, "cls", 1, "cls", 1, &m);
    assert(strcmp(line, "cls f1 mask cls calls 1 differ 0 (0.0\xe2\x80\xb0) near1 0 near2 0 far 0 bbox - offpalette 0 "
                        "nonuniform 0") == 0);
}

static void test_shifted_pixels_are_near_and_a_wrong_colour_is_far(void) {
    fill_capture(0);
    fill_pc(0);
    pc[50 * W + 50] = 7;
    put_block(51, 50, psx_colour_rgb555(7));
    pc[10 * W + 10] = 8;
    put_block(12, 10, psx_colour_rgb555(8));
    put_block(100, 100, psx_colour_rgb555(9));
    capture_compare_metrics_t m = measure();
    /* (50,50) and (10,10) turned black, next to black PC pixels: near1 with (51,50). */
    assert(m.differ == 5);
    assert(m.near1 == 3);
    assert(m.near2 == 1 && "colour 8 two pixels away on PC");
    assert(m.far == 1 && "colour 9 nowhere near on PC");
    assert(m.bbox_x0 == 10 && m.bbox_y0 == 10 && m.bbox_x1 == 100 && m.bbox_y1 == 100);
    char line[CAPTURE_COMPARE_REPORT_CAP];
    capture_compare_report_line(line, "line", 194, "cls,line", 3, &m);
    assert(strcmp(line, "line f194 mask cls,line calls 3 differ 5 (0.2\xe2\x80\xb0) near1 3 near2 1 far 1 bbox "
                        "10,10-100,100 offpalette 0 nonuniform 0") == 0);
}

static void test_the_diff_map_marks_differing_pixels_with_the_psx_colour(void) {
    fill_capture(0);
    fill_pc(0);
    put_block(0, 0, psx_colour_rgb555(12));
    put_block(159, 119, 0x8000 | 1);
    assert(decode());
    static char map[CAPTURE_COMPARE_DIFF_MAP_BYTES];
    capture_compare_diff_map(pc, &psx, map);
    assert(sizeof map == 120 * 161);
    for (int y = 0; y < H; y++) assert(map[y * (W + 1) + W] == '\n');
    assert(map[0] == 'c' && map[1] == '.');
    assert(map[119 * (W + 1) + 159] == '?');
    int dots = 0;
    for (size_t i = 0; i < sizeof map; i++) dots += map[i] == '.';
    assert(dots == W * H - 2);
}

static void test_the_image_is_a_p6_of_three_panels(void) {
    fill_capture(0);
    fill_pc(7);
    assert(decode());
    static unsigned char ppm[CAPTURE_COMPARE_PPM_BYTES];
    capture_compare_image(pc, &psx, ppm);
    const char *header = "P6\n960 240\n255\n";
    assert(memcmp(ppm, header, strlen(header)) == 0);
    assert(sizeof ppm == strlen(header) + 960 * 240 * 3);
    const unsigned char *pixels = ppm + strlen(header);
    assert(pixels[0] == 0xFF && pixels[1] == 0xF1 && pixels[2] == 0xE8 && "the PC panel in colour 7");
    const unsigned char *psx_panel = pixels + 320 * 3;
    assert(psx_panel[0] == 0 && psx_panel[1] == 0 && psx_panel[2] == 0);
    const unsigned char *diff_panel = pixels + 640 * 3;
    assert(diff_panel[0] == 0 && "a differing pixel shows the PSX colour");
    fill_pc(0);
    capture_compare_image(pc, &psx, ppm);
    assert(diff_panel[0] == diff_panel[1] && diff_panel[0] > 0 && diff_panel[0] < 0x80 && "dark grey where equal");
}

int main(void) {
    test_a_bad_header_or_size_is_refused_naming_both();
    test_a_uniform_capture_of_each_colour_maps_to_its_index();
    test_the_mask_bit_is_ignored_and_red_is_the_low_bits();
    test_an_off_palette_pixel_and_a_non_uniform_block_are_counted();
    test_equal_frames_differ_nowhere();
    test_shifted_pixels_are_near_and_a_wrong_colour_is_far();
    test_the_diff_map_marks_differing_pixels_with_the_psx_colour();
    test_the_image_is_a_p6_of_three_panels();
    printf("test_capture_compare: ok\n");
    return 0;
}
