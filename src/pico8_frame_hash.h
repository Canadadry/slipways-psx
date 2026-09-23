#ifndef PICO8_FRAME_HASH_H
#define PICO8_FRAME_HASH_H

#include <stdint.h>

/* FNV-1a 64 bits des 8192 octets que cette frame occuperait dans une
 * capture (pico8_screencast.h : pixel 2k en quartet faible, 2k+1 en
 * quartet fort). `pixels` tient PICO8_SCREENCAST_FRAME_PIXELS octets de
 * valeur 0 a 15, ce que rend pico8_screencast_read_frame. */
uint64_t pico8_frame_hash(const unsigned char *pixels);

#endif
