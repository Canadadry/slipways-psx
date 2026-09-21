#include "pico8_gfx.h"

#include <stdlib.h>

void pico8_gfx_init(pico8_gfx_t *gfx) {
    gfx->camera_x = 0;
    gfx->camera_y = 0;
    gfx->fillp_pattern = 0;
    gfx->fillp_transparent = 0;
    /* Phase 3, Chunk 18: PICO-8's own documented default pen colour before
     * any real color()/col-taking draw call ever runs is 6 (light grey) --
     * an external, well-known PICO-8-manual fact, not something derived
     * from pico8/lua.lua's own source text (flagged per this port's own
     * convention for such assumptions, e.g. pico8_hud.h's top comment).
     * The one real, evidenced omitted-col call site this default could
     * ever matter for (`pj:kj()`'s own `rk(127-h,127)`) is always preceded,
     * in the SAME function call, by `rk(0,h,0)` (an explicit col=0) on the
     * real source's own immediately preceding line -- so this default is
     * never actually observed by any real call site in this cart; it only
     * matters if some future, not-yet-evidenced call site reads the pen
     * colour before ever setting it explicitly. */
    gfx->pen_color = 6;
}

void pico8_cls(pico8_mem_t *mem) {
    for (int i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        pico8_poke(mem, PICO8_ADDR_SCREEN + i, 0);
    }
}

void pico8_pset(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x, int y, int col) {
    int sx = x - gfx->camera_x;
    int sy = y - gfx->camera_y;
    if (sx < 0 || sx >= PICO8_SCREEN_WIDTH || sy < 0 || sy >= PICO8_SCREEN_HEIGHT) {
        return;
    }
    pico8_poke(mem, PICO8_ADDR_SCREEN + sy * PICO8_SCREEN_WIDTH + sx, (unsigned char)col);
}

void pico8_camera(pico8_gfx_t *gfx, int x, int y) {
    gfx->camera_x = x;
    gfx->camera_y = y;
}

void pico8_camera_reset(pico8_gfx_t *gfx) {
    pico8_camera(gfx, 0, 0);
}

/* See pico8_gfx.h's own comment on this declaration for why the single
 * store is the draw-palette byte in memory rather than a gfx-side flag. */
void pico8_palt(pico8_mem_t *mem, int col, int transparent) {
    int addr;
    unsigned char byte;

    if (col < 0 || col > 15) {
        return;
    }
    addr = PICO8_ADDR_DRAW_PALETTE + col;
    byte = pico8_peek(mem, addr);
    if (transparent) {
        byte = (unsigned char)(byte | PICO8_DRAW_PALETTE_TRANSPARENT_BIT);
    } else {
        byte = (unsigned char)(byte & ~PICO8_DRAW_PALETTE_TRANSPARENT_BIT);
    }
    pico8_poke(mem, addr, byte);
}

int pico8_gfx_is_transparent(const pico8_mem_t *mem, int col) {
    if (col < 0 || col > 15) {
        return 0;
    }
    return (pico8_peek(mem, PICO8_ADDR_DRAW_PALETTE + col) & PICO8_DRAW_PALETTE_TRANSPARENT_BIT) != 0;
}

void pico8_line(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col) {
    int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int x = x1;
    int y = y1;

    for (;;) {
        pico8_pset(gfx, mem, x, y, col);
        if (x == x2 && y == y2) {
            break;
        }
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

/* fillp's 16-bit pattern is a 4x4 grid of bits, read most-significant-bit
 * first, row-major from top-left: bit 15 is (row 0, col 0), bit 0 is
 * (row 3, col 3) -- the PICO-8 manual's own "P is a bitfield in reading
 * order starting from the highest bit" table. fillp(0) (the
 * default/reset value) has every bit clear.
 * Only *fill* operations (rectfill, circfill) consult this: every
 * fillp(...)-bracketed call site found in pico8/lua.lua surrounds a fill
 * call and explicitly resets the pattern with fillp() before any
 * unfilled line/rect/circ call, so those never need to respect it. */
static int pico8_fillp_bit_set(const pico8_gfx_t *gfx, int x, int y) {
    int row = ((y % 4) + 4) % 4;
    int col = ((x % 4) + 4) % 4;
    int bit_index = 15 - (row * 4 + col);
    return (gfx->fillp_pattern >> bit_index) & 1;
}

/* The fill pattern is a TWO-COLOUR pattern, and the colour parameter of a
 * fill carries both of its colours: "To specify a second colour for the
 * pattern, use the high bits of any colour parameter" (PICO-8 manual,
 * FILLP). So the pattern's clear bits draw `col`'s low nibble and its set
 * bits draw `col`'s high nibble -- unless the pattern's own transparency
 * flag is set, in which case "the second colour is not drawn" and the set
 * bits leave the screen untouched (returned here as -1).
 *
 * This is why the colour never reaches the framebuffer unmasked: PICO-8's
 * screen is 4 bits per pixel, so only a 0..15 index can ever be stored,
 * and the cart's one two-nibble colour literal -- `rk(om,dn,0x10)` in
 * jb:ol (pico8/lua.lua:319) -- means "colour 0 and colour 1", not
 * "colour 16". pico8_pset() is deliberately left unmasked so that
 * src/pico8_dump.c's own assert(pixel <= 0x0f) keeps catching any future
 * primitive that stores a bogus index. */
static int pico8_fillp_color(const pico8_gfx_t *gfx, int x, int y, int col) {
    if (!pico8_fillp_bit_set(gfx, x, y)) {
        return col & 0x0F;
    }
    if (gfx->fillp_transparent) {
        return -1;
    }
    return (col >> 4) & 0x0F;
}

static int pico8_min(int a, int b) {
    return a < b ? a : b;
}

static int pico8_max(int a, int b) {
    return a < b ? b : a;
}

void pico8_rect(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col) {
    int xmin = pico8_min(x1, x2);
    int xmax = pico8_max(x1, x2);
    int ymin = pico8_min(y1, y2);
    int ymax = pico8_max(y1, y2);

    for (int x = xmin; x <= xmax; x++) {
        pico8_pset(gfx, mem, x, ymin, col);
        pico8_pset(gfx, mem, x, ymax, col);
    }
    for (int y = ymin; y <= ymax; y++) {
        pico8_pset(gfx, mem, xmin, y, col);
        pico8_pset(gfx, mem, xmax, y, col);
    }
}

static void pico8_circ_plot8(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int dx, int dy, int col) {
    pico8_pset(gfx, mem, cx + dx, cy + dy, col);
    pico8_pset(gfx, mem, cx - dx, cy + dy, col);
    pico8_pset(gfx, mem, cx + dx, cy - dy, col);
    pico8_pset(gfx, mem, cx - dx, cy - dy, col);
    pico8_pset(gfx, mem, cx + dy, cy + dx, col);
    pico8_pset(gfx, mem, cx - dy, cy + dx, col);
    pico8_pset(gfx, mem, cx + dy, cy - dx, col);
    pico8_pset(gfx, mem, cx - dy, cy - dx, col);
}

void pico8_circ(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int r, int col) {
    if (r < 0) {
        return;
    }
    int x = r;
    int y = 0;
    int err = 0;

    while (x >= y) {
        pico8_circ_plot8(gfx, mem, cx, cy, x, y, col);
        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

static void pico8_circfill_span(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int x2, int y, int col) {
    int xmin = pico8_min(x1, x2);
    int xmax = pico8_max(x1, x2);

    for (int x = xmin; x <= xmax; x++) {
        int pattern_col = pico8_fillp_color(gfx, x, y, col);
        if (pattern_col >= 0) {
            pico8_pset(gfx, mem, x, y, pattern_col);
        }
    }
}

void pico8_circfill(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int r, int col) {
    if (r < 0) {
        return;
    }
    int x = r;
    int y = 0;
    int err = 0;

    while (x >= y) {
        pico8_circfill_span(gfx, mem, cx - x, cx + x, cy + y, col);
        pico8_circfill_span(gfx, mem, cx - x, cx + x, cy - y, col);
        pico8_circfill_span(gfx, mem, cx - y, cx + y, cy + x, col);
        pico8_circfill_span(gfx, mem, cx - y, cx + y, cy - x, col);
        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void pico8_fillp(pico8_gfx_t *gfx, unsigned int pattern) {
    gfx->fillp_pattern = pattern & 0xFFFFu;
    gfx->fillp_transparent = 0;
}

void pico8_fillp_transparent(pico8_gfx_t *gfx, unsigned int pattern) {
    gfx->fillp_pattern = pattern & 0xFFFFu;
    gfx->fillp_transparent = 1;
}

void pico8_fillp_reset(pico8_gfx_t *gfx) {
    pico8_fillp(gfx, 0);
}

unsigned char pico8_sget(const pico8_mem_t *mem, int x, int y) {
    if (x < 0 || x >= PICO8_SPRITE_SHEET_WIDTH || y < 0 || y >= PICO8_SPRITE_SHEET_HEIGHT) {
        return 0;
    }
    return pico8_peek(mem, PICO8_ADDR_SPRITE_SHEET + y * PICO8_SPRITE_SHEET_WIDTH + x);
}

/* Sprite index n's top-left pixel sits at sheet coordinate
 * ((n % 16) * 8, (n / 16) * 8): the sheet is 128x128 pixels, laid out as
 * 16x16 blocks of 8x8-pixel sprites (128/8 = 16 per row). w/h are in
 * sprite-size (8px) multiples, so a wide/tall sprite just reads a larger
 * contiguous pixel rectangle from the sheet, spilling into neighboring
 * sprite slots exactly like real PICO-8's multi-cell sprites do.
 *
 * Real hardware's own `spr()` treats a source pixel as transparent when
 * the CURRENT draw palette byte for that color
 * (`mem[PICO8_ADDR_DRAW_PALETTE+col]`, `pal()`'s own backing memory) has
 * its high bit set -- the one, single store, which real PICO-8's `palt()`
 * writes into as well (`pico8_palt`/`pico8_gfx_is_transparent` above; this
 * port used to keep a second, gfx-side flag array OR'd onto this byte,
 * see pico8_gfx.h's own comment on pico8_palt for the bug that caused).
 * This cart never calls a plain, unconditional
 * `palt(3,true)`; instead `cu()`/`dv()` (pico8_hud.c) bake
 * that exact bit into every palette block they build/activate (`bor(sget(
 * p,c),c==3 and 0x80)`), and `_draw()`'s own bare `dv()` re-activates it
 * every single frame -- the cart's real, literal mechanism for making
 * color 3 (not color 0) the default "invisible" placeholder everywhere,
 * including the title screen's own `spr(192,0,11,16,4)` logo blit
 * (`fv:kj`/`bl`, pico8_help.c) whose sprite art uses color 3 as its own
 * background fill. Checking only `pico8_gfx_is_transparent` here (as
 * before this fix) leaves that mem-side bit permanently unconsulted, so
 * every `spr()` draws color 3 as a literal opaque dark green -- this was
 * the exact cause of a real, human-reported "solid green band behind the
 * SLIPWAYS logo" bug on the title screen. Deliberately scoped to `spr()`
 * only, not every draw primitive (pset/rect/circ/print/...): it is the
 * one primitive with a real, evidenced, currently-broken call site; see
 * this function's own top comment in pico8_gfx.h for why widening this to
 * every primitive (a full `pal()` remap, not just its transparency bit)
 * is left for a future chunk with its own evidenced need. */
void pico8_spr(const pico8_gfx_t *gfx, pico8_mem_t *mem, int n, int x, int y, int w, int h) {
    const int sprites_per_row = PICO8_SPRITE_SHEET_WIDTH / PICO8_SPRITE_SIZE;
    int sheet_x = (n % sprites_per_row) * PICO8_SPRITE_SIZE;
    int sheet_y = (n / sprites_per_row) * PICO8_SPRITE_SIZE;
    int pixel_w = w * PICO8_SPRITE_SIZE;
    int pixel_h = h * PICO8_SPRITE_SIZE;

    for (int dy = 0; dy < pixel_h; dy++) {
        for (int dx = 0; dx < pixel_w; dx++) {
            unsigned char col = pico8_sget(mem, sheet_x + dx, sheet_y + dy);
            if (pico8_gfx_is_transparent(mem, col)) {
                continue;
            }
            pico8_pset(gfx, mem, x + dx, y + dy, col);
        }
    }
}

void pico8_spr_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, int n, int x, int y) {
    pico8_spr(gfx, mem, n, x, y, 1, 1);
}

/* Map cell addressing mirrors pico8_assets.c's map_cell_sink: cel rows
 * 0-31 live in the base map region, rows 32-63 spill into the shared
 * sprite-sheet/map region (pico8/map.json's cart data is 128x64 cells,
 * larger than the base 128x32 map region). Kept in sync by hand rather
 * than shared code, since pico8_assets.c's sink is private to its own
 * flat-JSON-array loading loop. */
#define PICO8_MAP_BASE_ROWS 32

static int pico8_map_cell_addr(int x, int y) {
    if (x < 0 || x >= PICO8_MAP_WIDTH || y < 0 || y >= PICO8_MAP_HEIGHT) {
        return -1;
    }
    if (y < PICO8_MAP_BASE_ROWS) {
        return PICO8_ADDR_SPRITE_MAP_SHARED + y * PICO8_MAP_WIDTH + x;
    }
    return PICO8_ADDR_MAP + (y - PICO8_MAP_BASE_ROWS) * PICO8_MAP_WIDTH + x;
}

void pico8_mset(pico8_mem_t *mem, int x, int y, int val) {
    int addr = pico8_map_cell_addr(x, y);
    if (addr < 0) {
        return;
    }
    pico8_poke(mem, addr, (unsigned char)val);
}

static unsigned char pico8_map_cell_get(const pico8_mem_t *mem, int x, int y) {
    int addr = pico8_map_cell_addr(x, y);
    if (addr < 0) {
        return 0;
    }
    return pico8_peek(mem, addr);
}

void pico8_map(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cel_x, int cel_y, int sx, int sy, int cel_w, int cel_h) {
    for (int r = 0; r < cel_h; r++) {
        for (int c = 0; c < cel_w; c++) {
            unsigned char sprite_n = pico8_map_cell_get(mem, cel_x + c, cel_y + r);
            /* "Sprite 0 is taken to mean empty and is not drawn" -- PICO-8
             * manual, MAP(). (Its opt-out, POKE(0x5F36,0x8), is never poked
             * by this cart, so it isn't implemented here.) The rule is
             * map()'s alone: spr(0,...) still draws sprite 0. It matters
             * because THIS cart's sprite 0 is not artwork at all but one
             * quadrant of the 16x16 pixel block `cu()` reads as its palette
             * table (pico8/lua.lua:134-140), while `bg:pq()` writes cell
             * value 0 for most of the sky (3 of `bg.pl`'s 15 entries are
             * literally 0, pico8/lua.lua:362-371) -- drawing it tiled that
             * palette block over the whole galaxy background. */
            if (sprite_n == 0) {
                continue;
            }
            pico8_spr(gfx, mem, sprite_n, sx + c * PICO8_SPRITE_SIZE, sy + r * PICO8_SPRITE_SIZE, 1, 1);
        }
    }
}

/* Hand-authored bitmap font (see pico8_gfx.h for cell/glyph metrics).
 * Each row is a 3-bit value, MSB = leftmost column of the glyph's 3x5
 * bitmap (bit 2 = col 0, bit 1 = col 1, bit 0 = col 2).
 *
 * Real PICO-8's font bitmap isn't present anywhere in the extracted
 * `pico8/` cart assets (it's part of the PICO-8 runtime, not the cart --
 * see docs/prd/triage/phase1-etape1-pico8-interface-software-rendering.md's
 * "Further Notes"), so this table is a minimal, hand-authored invention,
 * not a reproduction of PICO-8's real glyphs -- legible enough to spot
 * one character from another in a headless text dump, nothing more.
 *
 * Coverage: space, digits 0-9, letters a-z AND A-Z, and the punctuation
 * ! $ % ' ( ) + , - . / : = ? -- originally derived by grepping
 * pico8/lua.lua's string literals actually reachable from print()/io()
 * call sites (see this chunk's implementation notes). That scan missed
 * `ef`, the month-name table, whose entries are written as decimal
 * escapes rather than plain literals (`ef=ob("\"\74\65\78\",...")`,
 * pico8/lua.lua:797 -- \74\65\78 is "JAN"), so uppercase went uncovered
 * and the top menu bar's date drew three solid placeholder blocks instead
 * of a month: a real, human-reported bug. Upper- and lowercase share one
 * bitmap each, for the same reason 'u'/'v' do (below) -- this table only
 * has to answer "which character is this", and at 3x5 there is no room
 * for a second, case-distinguishing shape. Any still-uncovered character
 * draws PICO8_FONT_ROWS_PLACEHOLDER, a solid 3x5 block -- deliberately
 * distinct from every covered glyph and from the blank space glyph, so a
 * missing-character bug is visually obvious in a text dump rather than
 * silently reading as whitespace.
 *
 * 'u' and 'v' intentionally share the same bitmap: at 3x5 there isn't
 * room to distinguish them meaningfully, and this table doesn't need to
 * be legible beyond "which of the covered characters is this". */
static const unsigned char PICO8_FONT_ROWS_PLACEHOLDER[PICO8_FONT_GLYPH_HEIGHT] = {7, 7, 7, 7, 7};
static const unsigned char PICO8_FONT_ROWS_SPACE[PICO8_FONT_GLYPH_HEIGHT] = {0, 0, 0, 0, 0};

static const unsigned char PICO8_FONT_ROWS_0[PICO8_FONT_GLYPH_HEIGHT] = {7, 5, 5, 5, 7};
static const unsigned char PICO8_FONT_ROWS_1[PICO8_FONT_GLYPH_HEIGHT] = {2, 6, 2, 2, 7};
static const unsigned char PICO8_FONT_ROWS_2[PICO8_FONT_GLYPH_HEIGHT] = {7, 1, 7, 4, 7};
static const unsigned char PICO8_FONT_ROWS_3[PICO8_FONT_GLYPH_HEIGHT] = {7, 1, 7, 1, 7};
static const unsigned char PICO8_FONT_ROWS_4[PICO8_FONT_GLYPH_HEIGHT] = {5, 5, 7, 1, 1};
static const unsigned char PICO8_FONT_ROWS_5[PICO8_FONT_GLYPH_HEIGHT] = {7, 4, 7, 1, 7};
static const unsigned char PICO8_FONT_ROWS_6[PICO8_FONT_GLYPH_HEIGHT] = {7, 4, 7, 5, 7};
static const unsigned char PICO8_FONT_ROWS_7[PICO8_FONT_GLYPH_HEIGHT] = {7, 1, 1, 1, 1};
static const unsigned char PICO8_FONT_ROWS_8[PICO8_FONT_GLYPH_HEIGHT] = {7, 5, 7, 5, 7};
static const unsigned char PICO8_FONT_ROWS_9[PICO8_FONT_GLYPH_HEIGHT] = {7, 5, 7, 1, 7};

static const unsigned char PICO8_FONT_ROWS_A[PICO8_FONT_GLYPH_HEIGHT] = {2, 5, 7, 5, 5};
static const unsigned char PICO8_FONT_ROWS_B[PICO8_FONT_GLYPH_HEIGHT] = {4, 4, 6, 5, 6};
static const unsigned char PICO8_FONT_ROWS_C[PICO8_FONT_GLYPH_HEIGHT] = {3, 4, 4, 4, 3};
static const unsigned char PICO8_FONT_ROWS_D[PICO8_FONT_GLYPH_HEIGHT] = {1, 1, 3, 5, 3};
static const unsigned char PICO8_FONT_ROWS_E[PICO8_FONT_GLYPH_HEIGHT] = {7, 4, 6, 4, 7};
static const unsigned char PICO8_FONT_ROWS_F[PICO8_FONT_GLYPH_HEIGHT] = {7, 4, 6, 4, 4};
static const unsigned char PICO8_FONT_ROWS_G[PICO8_FONT_GLYPH_HEIGHT] = {3, 4, 5, 5, 3};
static const unsigned char PICO8_FONT_ROWS_H[PICO8_FONT_GLYPH_HEIGHT] = {5, 5, 7, 5, 5};
static const unsigned char PICO8_FONT_ROWS_I[PICO8_FONT_GLYPH_HEIGHT] = {7, 2, 2, 2, 7};
static const unsigned char PICO8_FONT_ROWS_J[PICO8_FONT_GLYPH_HEIGHT] = {1, 1, 1, 5, 2};
static const unsigned char PICO8_FONT_ROWS_K[PICO8_FONT_GLYPH_HEIGHT] = {5, 5, 6, 5, 5};
static const unsigned char PICO8_FONT_ROWS_L[PICO8_FONT_GLYPH_HEIGHT] = {4, 4, 4, 4, 7};
static const unsigned char PICO8_FONT_ROWS_M[PICO8_FONT_GLYPH_HEIGHT] = {5, 7, 7, 5, 5};
static const unsigned char PICO8_FONT_ROWS_N[PICO8_FONT_GLYPH_HEIGHT] = {5, 6, 5, 5, 5};
static const unsigned char PICO8_FONT_ROWS_O[PICO8_FONT_GLYPH_HEIGHT] = {2, 5, 5, 5, 2};
static const unsigned char PICO8_FONT_ROWS_P[PICO8_FONT_GLYPH_HEIGHT] = {6, 5, 6, 4, 4};
static const unsigned char PICO8_FONT_ROWS_Q[PICO8_FONT_GLYPH_HEIGHT] = {2, 5, 5, 3, 1};
static const unsigned char PICO8_FONT_ROWS_R[PICO8_FONT_GLYPH_HEIGHT] = {6, 5, 6, 5, 5};
static const unsigned char PICO8_FONT_ROWS_S[PICO8_FONT_GLYPH_HEIGHT] = {3, 4, 2, 1, 6};
static const unsigned char PICO8_FONT_ROWS_T[PICO8_FONT_GLYPH_HEIGHT] = {7, 2, 2, 2, 2};
static const unsigned char PICO8_FONT_ROWS_U[PICO8_FONT_GLYPH_HEIGHT] = {5, 5, 5, 5, 2};
static const unsigned char PICO8_FONT_ROWS_W[PICO8_FONT_GLYPH_HEIGHT] = {5, 5, 5, 7, 5};
static const unsigned char PICO8_FONT_ROWS_X[PICO8_FONT_GLYPH_HEIGHT] = {5, 5, 2, 5, 5};
static const unsigned char PICO8_FONT_ROWS_Y[PICO8_FONT_GLYPH_HEIGHT] = {5, 5, 2, 2, 2};
static const unsigned char PICO8_FONT_ROWS_Z[PICO8_FONT_GLYPH_HEIGHT] = {7, 1, 2, 4, 7};

static const unsigned char PICO8_FONT_ROWS_BANG[PICO8_FONT_GLYPH_HEIGHT] = {2, 2, 2, 0, 2};
static const unsigned char PICO8_FONT_ROWS_DOLLAR[PICO8_FONT_GLYPH_HEIGHT] = {2, 7, 2, 7, 2};
static const unsigned char PICO8_FONT_ROWS_PERCENT[PICO8_FONT_GLYPH_HEIGHT] = {5, 1, 2, 4, 5};
static const unsigned char PICO8_FONT_ROWS_APOSTROPHE[PICO8_FONT_GLYPH_HEIGHT] = {2, 2, 0, 0, 0};
static const unsigned char PICO8_FONT_ROWS_LPAREN[PICO8_FONT_GLYPH_HEIGHT] = {1, 2, 2, 2, 1};
static const unsigned char PICO8_FONT_ROWS_RPAREN[PICO8_FONT_GLYPH_HEIGHT] = {4, 2, 2, 2, 4};
static const unsigned char PICO8_FONT_ROWS_PLUS[PICO8_FONT_GLYPH_HEIGHT] = {0, 2, 7, 2, 0};
static const unsigned char PICO8_FONT_ROWS_COMMA[PICO8_FONT_GLYPH_HEIGHT] = {0, 0, 0, 2, 4};
static const unsigned char PICO8_FONT_ROWS_MINUS[PICO8_FONT_GLYPH_HEIGHT] = {0, 0, 7, 0, 0};
static const unsigned char PICO8_FONT_ROWS_PERIOD[PICO8_FONT_GLYPH_HEIGHT] = {0, 0, 0, 0, 2};
static const unsigned char PICO8_FONT_ROWS_SLASH[PICO8_FONT_GLYPH_HEIGHT] = {1, 1, 2, 4, 4};
static const unsigned char PICO8_FONT_ROWS_COLON[PICO8_FONT_GLYPH_HEIGHT] = {0, 2, 0, 2, 0};
static const unsigned char PICO8_FONT_ROWS_EQUALS[PICO8_FONT_GLYPH_HEIGHT] = {0, 7, 0, 7, 0};
static const unsigned char PICO8_FONT_ROWS_QUESTION[PICO8_FONT_GLYPH_HEIGHT] = {6, 1, 2, 0, 2};

static const unsigned char *pico8_font_glyph_rows(char c) {
    switch (c) {
        case ' ': return PICO8_FONT_ROWS_SPACE;
        case '0': return PICO8_FONT_ROWS_0;
        case '1': return PICO8_FONT_ROWS_1;
        case '2': return PICO8_FONT_ROWS_2;
        case '3': return PICO8_FONT_ROWS_3;
        case '4': return PICO8_FONT_ROWS_4;
        case '5': return PICO8_FONT_ROWS_5;
        case '6': return PICO8_FONT_ROWS_6;
        case '7': return PICO8_FONT_ROWS_7;
        case '8': return PICO8_FONT_ROWS_8;
        case '9': return PICO8_FONT_ROWS_9;
        case 'A': case 'a': return PICO8_FONT_ROWS_A;
        case 'B': case 'b': return PICO8_FONT_ROWS_B;
        case 'C': case 'c': return PICO8_FONT_ROWS_C;
        case 'D': case 'd': return PICO8_FONT_ROWS_D;
        case 'E': case 'e': return PICO8_FONT_ROWS_E;
        case 'F': case 'f': return PICO8_FONT_ROWS_F;
        case 'G': case 'g': return PICO8_FONT_ROWS_G;
        case 'H': case 'h': return PICO8_FONT_ROWS_H;
        case 'I': case 'i': return PICO8_FONT_ROWS_I;
        case 'J': case 'j': return PICO8_FONT_ROWS_J;
        case 'K': case 'k': return PICO8_FONT_ROWS_K;
        case 'L': case 'l': return PICO8_FONT_ROWS_L;
        case 'M': case 'm': return PICO8_FONT_ROWS_M;
        case 'N': case 'n': return PICO8_FONT_ROWS_N;
        case 'O': case 'o': return PICO8_FONT_ROWS_O;
        case 'P': case 'p': return PICO8_FONT_ROWS_P;
        case 'Q': case 'q': return PICO8_FONT_ROWS_Q;
        case 'R': case 'r': return PICO8_FONT_ROWS_R;
        case 'S': case 's': return PICO8_FONT_ROWS_S;
        case 'T': case 't': return PICO8_FONT_ROWS_T;
        case 'U': case 'u': return PICO8_FONT_ROWS_U;
        case 'V': case 'v': return PICO8_FONT_ROWS_U; /* deliberately shared with u, see comment above */
        case 'W': case 'w': return PICO8_FONT_ROWS_W;
        case 'X': case 'x': return PICO8_FONT_ROWS_X;
        case 'Y': case 'y': return PICO8_FONT_ROWS_Y;
        case 'Z': case 'z': return PICO8_FONT_ROWS_Z;
        case '!': return PICO8_FONT_ROWS_BANG;
        case '$': return PICO8_FONT_ROWS_DOLLAR;
        case '%': return PICO8_FONT_ROWS_PERCENT;
        case '\'': return PICO8_FONT_ROWS_APOSTROPHE;
        case '(': return PICO8_FONT_ROWS_LPAREN;
        case ')': return PICO8_FONT_ROWS_RPAREN;
        case '+': return PICO8_FONT_ROWS_PLUS;
        case ',': return PICO8_FONT_ROWS_COMMA;
        case '-': return PICO8_FONT_ROWS_MINUS;
        case '.': return PICO8_FONT_ROWS_PERIOD;
        case '/': return PICO8_FONT_ROWS_SLASH;
        case ':': return PICO8_FONT_ROWS_COLON;
        case '=': return PICO8_FONT_ROWS_EQUALS;
        case '?': return PICO8_FONT_ROWS_QUESTION;
        default: return PICO8_FONT_ROWS_PLACEHOLDER;
    }
}

void pico8_print(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *str, int x, int y, int col) {
    int cx = x;
    for (const char *p = str; *p != '\0'; p++) {
        const unsigned char *rows = pico8_font_glyph_rows(*p);
        for (int row = 0; row < PICO8_FONT_GLYPH_HEIGHT; row++) {
            for (int gx = 0; gx < PICO8_FONT_GLYPH_WIDTH; gx++) {
                int bit_index = PICO8_FONT_GLYPH_WIDTH - 1 - gx;
                if ((rows[row] >> bit_index) & 1) {
                    pico8_pset(gfx, mem, cx + gx, y + row, col);
                }
            }
        }
        cx += PICO8_FONT_CELL_WIDTH;
    }
}

void pico8_rectfill(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col) {
    int xmin = pico8_min(x1, x2);
    int xmax = pico8_max(x1, x2);
    int ymin = pico8_min(y1, y2);
    int ymax = pico8_max(y1, y2);

    for (int y = ymin; y <= ymax; y++) {
        for (int x = xmin; x <= xmax; x++) {
            int pattern_col = pico8_fillp_color(gfx, x, y, col);
            if (pattern_col >= 0) {
                pico8_pset(gfx, mem, x, y, pattern_col);
            }
        }
    }
}
