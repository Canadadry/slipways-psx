/* The one piece of the raylib presentation/input concern that can be
 * tested: kept out of pico8_raylib_backend.c, which includes raylib.h and
 * can therefore be neither built nor run here, so
 * tests/test_raylib_backend_coords.c links this .c alone. */

#ifndef PICO8_RAYLIB_BACKEND_COORDS_H
#define PICO8_RAYLIB_BACKEND_COORDS_H

/* Converts a window-pixel mouse position to the 128x128 PICO-8 space:
 * divides by `scale` and clamps each axis into [0, 127], so a cursor
 * outside the scaled window -- still reported by raylib while a button is
 * held -- never yields an out-of-range coordinate. A `scale` of 0 or less
 * maps to (0, 0). Pure and raylib-free: plain integers in and out. */
void pico8_raylib_backend_window_to_pico8(int window_x, int window_y, int scale, int *out_x, int *out_y);

#endif
