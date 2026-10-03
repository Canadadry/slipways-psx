/* PICO-8's sixteen default palette colors, defined once for every program
 * that turns screen memory into pixels, today the interactive driver
 * (src/platform/raylib_backend.c).
 *
 * These are PICO-8's published default palette values. PICO-8 transparency
 * (palt()) is already resolved by p8/gfx.c before a pixel reaches screen
 * memory, so alpha is always opaque.
 *
 * Byte triplets rather than a `Color`: a `Color` would force raylib.h on
 * this file and take it out of `make unit`, while forty-eight hex bytes are
 * exactly where a typo only shows by eye, in a window.
 * tests/test_palette.c asserts them one by one; each raylib file
 * builds its `Color` where it uses it. */

#ifndef P8_PALETTE_H
#define P8_PALETTE_H

/* The sixteen indices PICO-8 screen memory can hold. */
#define P8_PALETTE_COLORS 16

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} p8_rgb_t;

/* P8_PALETTE_RGB[index] is the color of that index, 0 to 15. */
extern const p8_rgb_t P8_PALETTE_RGB[P8_PALETTE_COLORS];

#endif
