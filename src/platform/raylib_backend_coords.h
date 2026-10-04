/* The one piece of the raylib presentation and input concern that can be
 * tested: kept out of raylib_backend.c, which includes raylib.h and is
 * therefore neither built by `make unit` nor run (only `make raylib` builds
 * it), so tests/test_raylib_backend_coords.c links this file alone. */

#ifndef PLATFORM_RAYLIB_BACKEND_COORDS_H
#define PLATFORM_RAYLIB_BACKEND_COORDS_H

#include "p8/vram.h"

/* Converts a window-pixel mouse position to the PICO-8 screen space:
 * divides by `scale` and clamps x into [0, P8_SCREEN_WIDTH - 1] and y into
 * [0, P8_SCREEN_HEIGHT - 1], so a cursor
 * outside the scaled window -- still reported by raylib while a button is
 * held -- never yields an out-of-range coordinate. A `scale` of 0 or less
 * maps to (0, 0). Pure and raylib-free: plain integers in and out. */
void raylib_backend_window_to_screen(int window_x, int window_y, int scale, int *out_x, int *out_y);

#endif
