/* Phase 5, Etape 1, Chunk 3 (docs/prd/triage/phase5-etape1-raylib-
 * interactive-backend.md): the one meaningfully testable piece of the
 * raylib presentation/input-capture concern, per the PRD's own Testing
 * Decisions -- split into its own raylib-free header/source pair
 * (rather than living inside pico8_raylib_backend.c/.h, which include
 * raylib.h) so tests/test_pico8_raylib_backend_coords.c can #include
 * this header and link this .c alone, with no raylib dependency at all,
 * keeping `make test` exactly as fast and window-free as it is today. */

#ifndef PICO8_RAYLIB_BACKEND_COORDS_H
#define PICO8_RAYLIB_BACKEND_COORDS_H

/* Converts a live window-pixel mouse position to the 128x128 PICO-8
 * coordinate space: divides by `scale` (the fixed integer window scale,
 * e.g. PICO8_RAYLIB_BACKEND_SCALE) and clamps the result into [0, 127]
 * in each axis, so a cursor position outside the scaled window (still
 * reported by raylib while the button is held, or right at the window's
 * far edge) never produces an out-of-range or negative PICO-8
 * coordinate. `scale` <= 0 is treated as "no signal" and maps to (0, 0)
 * pre-clamp, since a real backend never configures a non-positive
 * scale. Pure and raylib-free: plain integers in, plain integers out. */
void pico8_raylib_backend_window_to_pico8(int window_x, int window_y, int scale, int *out_x, int *out_y);

#endif
