#ifndef DEVTOOLS_CAPTURE_COMPARE_H
#define DEVTOOLS_CAPTURE_COMPARE_H

#include <stddef.h>
#include <stdint.h>

/* A GPU frame capture ("psx-vram 320 240 16\n" then takeScreenShot()'s bytes: 16-bit little-endian
 * VRAM pixels, red in bits 0-4) against the PC frame of the same replay frame, both brought to
 * 160x120 PICO-8 colour indices. */

enum {
    CAPTURE_COMPARE_SCREEN_WIDTH = 160,
    CAPTURE_COMPARE_SCREEN_HEIGHT = 120,
    CAPTURE_COMPARE_PIXEL_BYTES = 320 * 240 * 2,
    CAPTURE_COMPARE_OFF_PALETTE = 16,
    CAPTURE_COMPARE_ERROR_CAP = 160,
    CAPTURE_COMPARE_REPORT_CAP = 320,
    CAPTURE_COMPARE_DIFF_MAP_BYTES = CAPTURE_COMPARE_SCREEN_HEIGHT * (CAPTURE_COMPARE_SCREEN_WIDTH + 1),
    CAPTURE_COMPARE_PPM_HEADER_BYTES = 15,
    CAPTURE_COMPARE_PPM_BYTES = CAPTURE_COMPARE_PPM_HEADER_BYTES + 960 * 240 * 3
};

typedef struct {
    /* 0..15, or CAPTURE_COMPARE_OFF_PALETTE. */
    unsigned char index[CAPTURE_COMPARE_SCREEN_HEIGHT][CAPTURE_COMPARE_SCREEN_WIDTH];
    uint16_t raw[CAPTURE_COMPARE_SCREEN_HEIGHT][CAPTURE_COMPARE_SCREEN_WIDTH];
    int nonuniform;
    int offpalette;
} capture_compare_image_t;

typedef struct {
    int differ;
    int near1;
    int near2;
    int far;
    /* bbox_x0 < 0 when nothing differs. */
    int bbox_x0, bbox_y0, bbox_x1, bbox_y1;
    int offpalette;
    int nonuniform;
} capture_compare_metrics_t;

/* Returns 1, or 0 with error naming what was found and what was expected (header or size). Each
 * 2x2 block's top-left pixel is the downscaled one; a block not uniform is counted. */
int capture_compare_decode(const unsigned char *bytes, size_t size, capture_compare_image_t *image,
                           char error[CAPTURE_COMPARE_ERROR_CAP]);

/* pc: 160x120 indices 0..15, row-major (the screencast reader's frame). */
capture_compare_metrics_t capture_compare_measure(const unsigned char *pc, const capture_compare_image_t *psx);

void capture_compare_report_line(char line[CAPTURE_COMPARE_REPORT_CAP], const char *label, int frame,
                                 const char *mask_names, int calls, const capture_compare_metrics_t *metrics);

/* 120 lines of 160 characters: '.' where equal, the PSX index's hex digit where not, '?' off-palette. */
void capture_compare_diff_map(const unsigned char *pc, const capture_compare_image_t *psx,
                              char map[CAPTURE_COMPARE_DIFF_MAP_BYTES]);

/* Binary P6, 960x240: PC, PSX and diff panels, each x2; the diff panel is dark grey where equal. */
void capture_compare_image(const unsigned char *pc, const capture_compare_image_t *psx,
                           unsigned char ppm[CAPTURE_COMPARE_PPM_BYTES]);

#endif
