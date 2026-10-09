#include "devtools/capture_compare.h"

#include "p8/palette.h"
#include "platform/psx_colour.h"

#include <stdio.h>
#include <string.h>

enum {
    W = CAPTURE_COMPARE_SCREEN_WIDTH,
    H = CAPTURE_COMPARE_SCREEN_HEIGHT,
    CAPTURE_WIDTH = 2 * W,
    SCREEN_PIXELS = W * H,
    PANEL_WIDTH = 2 * W,
    IMAGE_WIDTH = 3 * PANEL_WIDTH,
    DIFF_EQUAL_GREY = 0x30
};

static const char EXPECTED_HEADER[] = "psx-vram 320 240 16";

static unsigned pixel_at(const unsigned char *pixels, int x, int y) {
    const unsigned char *at = pixels + ((size_t)y * CAPTURE_WIDTH + (size_t)x) * 2;
    return (unsigned)at[0] | (unsigned)at[1] << 8;
}

static unsigned char index_of(unsigned raw) {
    for (int i = 0; i < P8_PALETTE_COLORS; i++) {
        if ((psx_colour_rgb555(i) & 0x7FFFu) == (raw & 0x7FFFu)) return (unsigned char)i;
    }
    return CAPTURE_COMPARE_OFF_PALETTE;
}

int capture_compare_decode(const unsigned char *bytes, size_t size, capture_compare_image_t *image,
                           char error[CAPTURE_COMPARE_ERROR_CAP]) {
    const unsigned char *newline = memchr(bytes, '\n', size < 64 ? size : 64);
    if (newline == NULL) {
        snprintf(error, CAPTURE_COMPARE_ERROR_CAP, "no header line, expected '%s'", EXPECTED_HEADER);
        return 0;
    }
    size_t header_length = (size_t)(newline - bytes);
    if (header_length != strlen(EXPECTED_HEADER) || memcmp(bytes, EXPECTED_HEADER, header_length) != 0) {
        snprintf(error, CAPTURE_COMPARE_ERROR_CAP, "header '%.*s', expected '%s'", (int)header_length,
                 (const char *)bytes, EXPECTED_HEADER);
        return 0;
    }
    size_t expected_size = header_length + 1 + CAPTURE_COMPARE_PIXEL_BYTES;
    if (size != expected_size) {
        snprintf(error, CAPTURE_COMPARE_ERROR_CAP, "size %lu, expected %lu", (unsigned long)size,
                 (unsigned long)expected_size);
        return 0;
    }
    const unsigned char *pixels = newline + 1;
    image->nonuniform = 0;
    image->offpalette = 0;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            unsigned raw = pixel_at(pixels, 2 * x, 2 * y);
            if (pixel_at(pixels, 2 * x + 1, 2 * y) != raw || pixel_at(pixels, 2 * x, 2 * y + 1) != raw
                || pixel_at(pixels, 2 * x + 1, 2 * y + 1) != raw) {
                image->nonuniform++;
            }
            image->raw[y][x] = (uint16_t)raw;
            image->index[y][x] = index_of(raw);
            if (image->index[y][x] == CAPTURE_COMPARE_OFF_PALETTE) image->offpalette++;
        }
    }
    return 1;
}

static int pc_has_within(const unsigned char *pc, int x, int y, int distance, unsigned char colour) {
    for (int ny = y - distance; ny <= y + distance; ny++) {
        for (int nx = x - distance; nx <= x + distance; nx++) {
            if (nx < 0 || ny < 0 || nx >= W || ny >= H) continue;
            if (pc[ny * W + nx] == colour) return 1;
        }
    }
    return 0;
}

capture_compare_metrics_t capture_compare_measure(const unsigned char *pc, const capture_compare_image_t *psx) {
    capture_compare_metrics_t m;
    memset(&m, 0, sizeof m);
    m.bbox_x0 = -1;
    m.offpalette = psx->offpalette;
    m.nonuniform = psx->nonuniform;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            unsigned char colour = psx->index[y][x];
            if (colour == pc[y * W + x]) continue;
            m.differ++;
            if (pc_has_within(pc, x, y, 1, colour)) {
                m.near1++;
            } else if (pc_has_within(pc, x, y, 2, colour)) {
                m.near2++;
            } else {
                m.far++;
            }
            if (m.bbox_x0 < 0) {
                m.bbox_x0 = m.bbox_x1 = x;
                m.bbox_y0 = m.bbox_y1 = y;
            }
            if (x < m.bbox_x0) m.bbox_x0 = x;
            if (x > m.bbox_x1) m.bbox_x1 = x;
            if (y > m.bbox_y1) m.bbox_y1 = y;
        }
    }
    return m;
}

void capture_compare_report_line(char line[CAPTURE_COMPARE_REPORT_CAP], const char *label, int frame,
                                 const char *mask_names, int calls, const capture_compare_metrics_t *m) {
    char bbox[48];
    if (m->bbox_x0 < 0) {
        snprintf(bbox, sizeof bbox, "-");
    } else {
        snprintf(bbox, sizeof bbox, "%d,%d-%d,%d", m->bbox_x0, m->bbox_y0, m->bbox_x1, m->bbox_y1);
    }
    long tenths = (long)m->differ * 10000L / SCREEN_PIXELS;
    snprintf(line, CAPTURE_COMPARE_REPORT_CAP,
             "%s f%d mask %s calls %d differ %d (%ld.%ld\xe2\x80\xb0) near1 %d near2 %d far %d bbox %s offpalette %d "
             "nonuniform %d",
             label, frame, mask_names, calls, m->differ, tenths / 10, tenths % 10, m->near1, m->near2, m->far, bbox,
             m->offpalette, m->nonuniform);
}

void capture_compare_diff_map(const unsigned char *pc, const capture_compare_image_t *psx,
                              char map[CAPTURE_COMPARE_DIFF_MAP_BYTES]) {
    static const char HEX[] = "0123456789abcdef";
    char *at = map;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            unsigned char colour = psx->index[y][x];
            if (colour == pc[y * W + x]) {
                *at++ = '.';
            } else {
                *at++ = colour == CAPTURE_COMPARE_OFF_PALETTE ? '?' : HEX[colour];
            }
        }
        *at++ = '\n';
    }
}

static void put_rgb(unsigned char *pixels, int panel, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    for (int dy = 0; dy < 2; dy++) {
        for (int dx = 0; dx < 2; dx++) {
            unsigned char *at = pixels + ((size_t)(2 * y + dy) * IMAGE_WIDTH + (size_t)(panel * PANEL_WIDTH + 2 * x + dx)) * 3;
            at[0] = r;
            at[1] = g;
            at[2] = b;
        }
    }
}

/* Off-palette pixels show their own colour, widened back to 8 bits. */
static void put_psx(unsigned char *pixels, int panel, int x, int y, const capture_compare_image_t *psx) {
    unsigned char colour = psx->index[y][x];
    if (colour != CAPTURE_COMPARE_OFF_PALETTE) {
        p8_rgb_t rgb = P8_PALETTE_RGB[colour];
        put_rgb(pixels, panel, x, y, rgb.red, rgb.green, rgb.blue);
        return;
    }
    unsigned raw = psx->raw[y][x];
    put_rgb(pixels, panel, x, y, (unsigned char)((raw & 31) << 3), (unsigned char)((raw >> 5 & 31) << 3),
            (unsigned char)((raw >> 10 & 31) << 3));
}

void capture_compare_image(const unsigned char *pc, const capture_compare_image_t *psx,
                           unsigned char ppm[CAPTURE_COMPARE_PPM_BYTES]) {
    memcpy(ppm, "P6\n960 240\n255\n", CAPTURE_COMPARE_PPM_HEADER_BYTES);
    unsigned char *pixels = ppm + CAPTURE_COMPARE_PPM_HEADER_BYTES;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            p8_rgb_t rgb = P8_PALETTE_RGB[pc[y * W + x] & 0x0F];
            put_rgb(pixels, 0, x, y, rgb.red, rgb.green, rgb.blue);
            put_psx(pixels, 1, x, y, psx);
            if (psx->index[y][x] == pc[y * W + x]) {
                put_rgb(pixels, 2, x, y, DIFF_EQUAL_GREY, DIFF_EQUAL_GREY, DIFF_EQUAL_GREY);
            } else {
                put_psx(pixels, 2, x, y, psx);
            }
        }
    }
}
