#ifndef PICO8_GFX_H
#define PICO8_GFX_H

#include "pico8_mem.h"

#define PICO8_SCREEN_WIDTH 128
#define PICO8_SCREEN_HEIGHT 128

#define PICO8_SPRITE_SIZE 8
#define PICO8_SPRITE_SHEET_WIDTH 128
#define PICO8_SPRITE_SHEET_HEIGHT 128
#define PICO8_MAP_WIDTH 128
#define PICO8_MAP_HEIGHT 64

typedef struct {
    int camera_x;
    int camera_y;
    unsigned char transparent[16];
    unsigned int fillp_pattern;
    /* Phase 3, Chunk 18 (docs/prd/triage/phase3-etape1-function-by-function-
     * c-port.md): PICO-8's own "current pen colour" state -- real PICO-8's
     * draw functions that take an optional trailing `col` argument use this
     * remembered colour when `col` is omitted, and (per the PICO-8 manual's
     * own documented behavior for `color()`/every col-taking draw call)
     * updates it to whatever `col` WAS explicitly given otherwise. This
     * port only needed a plain `int col` parameter everywhere until this
     * chunk's own `pj:kj()` (pico8_score_panel.c) hit the ONE real
     * evidenced omitted-`col` call site in the whole cart (`rk(127-h,127)`,
     * pico8/lua.lua:855, right after `rk(0,h,0)` on the previous real
     * source line -- see pico8_hud.h's own pre-existing, pre-Chunk-18
     * comment on pico8_rk that already flagged this exact gap and named
     * this chunk as the one to resolve it). Deliberately narrow: only
     * pico8_rk() (pico8_hud.c) reads/writes this field today -- no other
     * draw primitive in this port (rectfill/rect/circ/print/...) has ever
     * been evidenced to need the "col omitted" case, so none of them are
     * touched here; widening this to a fully general "every draw call
     * updates the current pen colour" mechanism is left for whichever
     * future chunk's own real Lua source actually needs a SECOND
     * omitted-col call site (not speculative generality built now). */
    int pen_color;
} pico8_gfx_t;

void pico8_gfx_init(pico8_gfx_t *gfx);

void pico8_cls(pico8_mem_t *mem);

void pico8_pset(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x, int y, int col);

void pico8_camera(pico8_gfx_t *gfx, int x, int y);

void pico8_camera_reset(pico8_gfx_t *gfx);

void pico8_palt(pico8_gfx_t *gfx, int col, int transparent);

int pico8_gfx_is_transparent(const pico8_gfx_t *gfx, int col);

void pico8_line(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col);

void pico8_rect(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col);

void pico8_rectfill(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col);

void pico8_circ(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int r, int col);

void pico8_circfill(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int r, int col);

void pico8_fillp(pico8_gfx_t *gfx, unsigned int pattern);

void pico8_fillp_reset(pico8_gfx_t *gfx);

unsigned char pico8_sget(const pico8_mem_t *mem, int x, int y);

void pico8_spr(const pico8_gfx_t *gfx, pico8_mem_t *mem, int n, int x, int y, int w, int h);

void pico8_spr_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, int n, int x, int y);

void pico8_mset(pico8_mem_t *mem, int x, int y, int val);

void pico8_map(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cel_x, int cel_y, int sx, int sy, int cel_w, int cel_h);

/* Each glyph occupies a 3x5-pixel bitmap left-aligned in a 4-pixel-wide
 * advance cell (column 3 and the 6th row are always blank, giving 1px of
 * horizontal spacing and matching the "characters 4px apart" metric real
 * PICO-8 uses); see pico8_gfx.c for the covered character set and the
 * placeholder glyph drawn for anything outside it. */
#define PICO8_FONT_GLYPH_WIDTH 3
#define PICO8_FONT_GLYPH_HEIGHT 5
#define PICO8_FONT_CELL_WIDTH 4

void pico8_print(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *str, int x, int y, int col);

#endif
