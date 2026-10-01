#ifndef PICO8_HUD_H
#define PICO8_HUD_H

#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/mem.h"

/* Fixed-point stays fixed-point across this module's API and is truncated
 * to int only at the pico8_gfx draw call. An omitted Lua argument gets its
 * own entry point (_default, _y1) rather than a sentinel value.
 *
 * PICO-8's numeric builtins take a false or missing argument as 0 instead
 * of erroring the way Lua would; `bor(sget(p,c),c==3 and 0x80)` and the
 * bare `dv()` call sites rely on it. Assumed for `bor` and `flr` only. */

/* Writes the 16 palette blocks (16 source colors each) at 0x5000 that dv()
 * copies from -- PICO-8's trick of encoding palette swaps as sprite-sheet
 * pixels. Color 3's byte gets the 0x80 transparency bit in every block, so
 * activating any block makes color 3 invisible. Takes no gfx. */
void pico8_cu(pico8_mem_t *mem);

/* Activates block `no` of the 16 cu() built, by copying its 16 bytes into
 * the draw palette at 0x5f00 -- overwriting whatever palt() had set there.
 * `no` omitted means block 0. */
void pico8_dv(pico8_mem_t *mem, pico8_fixed_t no);
void pico8_dv_default(pico8_mem_t *mem);

/* Full-width horizontal bar. An explicit `col` also becomes the current
 * pen color (gfx->pen_color), which is what pico8_rk_default then draws
 * with -- the cart has exactly one col-omitted call site, `rk(127-h,127)`
 * in pj:kj. That write is why `gfx` is not const here. */
void pico8_rk(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, pico8_fixed_t y2, int col);
void pico8_rk_y1(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, int col);
void pico8_rk_default(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, pico8_fixed_t y2);

void pico8_eq(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t x1, pico8_fixed_t y1, pico8_fixed_t x2);

/* Outlined text: the same string printed at the 8 neighbouring pixels in
 * black, then once at the center in `c`. band(0,c) is 0 and band(15,c) is
 * c, which is how one loop over `er` draws both. `a` is the optional
 * centering factor, NULL meaning omitted: Lua's `if (a)` never evaluates
 * `a*4*#t` at all, so pico8_io_default must not either. */
void pico8_io(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *t, pico8_fixed_t x, pico8_fixed_t y, int c,
              const pico8_fixed_t *a);
void pico8_io_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *t, pico8_fixed_t x, pico8_fixed_t y,
                       int c);

#endif
