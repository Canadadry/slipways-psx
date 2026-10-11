#ifndef PLATFORM_PSX_COLOUR_H
#define PLATFORM_PSX_COLOUR_H

#include <stdint.h>

/* The GPU's colour of a PICO-8 palette index (0..15): each 8-bit component shifted right by 3,
 * red in bits 0-4, mask bit 0x8000 set so that black is not the transparent 0x0000. */
uint16_t psx_colour_rgb555(int index);

/* A CLUT entry for one draw-palette byte: 0x0000 (transparent) when bit 0x80 is set, else the
 * colour of its low nibble. */
uint16_t psx_colour_clut_entry(unsigned char draw_palette_byte);

#endif
