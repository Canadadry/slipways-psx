#ifndef PICO8_HUD_H
#define PICO8_HUD_H

#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"

/* Phase 3, Chunk 1, second sub-lot: cu, dv, rk, eq, io -- drawing-
 * composition helpers built directly on Phase 1's pico8_gfx/pico8_mem
 * primitives (poke/sget/rectfill/rect/spr/print/memcpy), plus io's own
 * private data literal `er`. Every numeric parameter that is a genuine
 * PICO-8 (fixed-point) value stays pico8_fixed_t through this module's own
 * API, truncated to a plain `int` only at the actual Phase 1 draw-call
 * boundary (pico8_gfx.h's own int-pixel-coordinate convention) -- mirrors
 * how main.c's own demo bridges the same two domains.
 *
 * Two genuinely unsourced, PICO-8-specific leniency assumptions this
 * module relies on (flagged, not silently assumed, matching
 * pico8_fixed.h's own precedent for e.g. sqrt's truncation direction):
 * PICO-8's own numeric built-ins (flr, bor, ...) are widely understood to
 * default a missing/non-number argument to 0 rather than erroring the way
 * vanilla Lua would (this is why `dv()`/`cu()`'s own `c==3 and 0x80`
 * -- which evaluates to plain `false`, not a number, when c~=3 -- and
 * `dv()`'s real no-argument call sites, e.g. `_draw()`'s bare `dv()`,
 * don't crash in the working original cart). This port bakes that
 * assumption in as "false/omitted numeric argument coerces to 0" for
 * exactly the two builtins (`bor`, `flr`) that need it here. */

/* function cu()local a=0x5000
 * for p=0,15 do
 * for c=0,15 do
 * poke(a,bor(sget(p,c),c==3 and 0x80))
 * a+=1
 * end
 * end
 * end
 * Builds the 16 (color-highlight variant) x 16 (source color) palette
 * remap table at 0x5000 that dv() below copies slices of into the active
 * draw palette (0x5f00) -- the classic PICO-8 "encode palette swaps as
 * sprite-sheet pixel data" trick. No pico8_gfx_t dependency (only
 * poke/sget), unlike every other function in this module. */
void pico8_cu(pico8_mem_t *mem);

/* function dv(no)memcpy(0x5f00,0x5000+shl(flr(no),4),16)end
 * Selects one of cu()'s 16 precomputed palette blocks (block `no`, each 16
 * bytes) and copies it into the active draw palette. `no` omitted (every
 * `dv()` call site with no argument, e.g. `_draw()`'s own bare `dv()`)
 * defaults to 0 -- see this file's own top comment on why. */
void pico8_dv(pico8_mem_t *mem, pico8_fixed_t no);
void pico8_dv_default(pico8_mem_t *mem);

/* function rk(y1,y2,...)rectfill(0,y1,127,y2 or y1,...)end
 * Full-width horizontal filled bar. `y2` omitted defaults to `y1`
 * (pico8_rk_y1, mirroring pico8_gfx.h's own pico8_spr/pico8_spr_default
 * two-entry-point convention for an optional trailing Lua argument). The
 * trailing `...` (col) is NEVER omitted at either real call site this
 * sub-lot itself needs (`rk(om,dn,0x10)`, `rk(0,h,0)`) -- the one real
 * col-omitted call site in the whole cart (`rk(127-h,127)`, pj:kj,
 * Chunk 18) relies on PICO-8's "col omitted -> last color drawn" state,
 * which this port's pico8_gfx_t has no field for (Phase 1 never needed
 * it). This WAS left as a real, evidenced, but explicitly DEFERRED gap
 * for Chunk 18 to resolve (by extending pico8_gfx_t with real
 * current-color state) rather than guessed at here with no real consumer
 * yet to verify against.
 *
 * RESOLVED, Phase 3 Chunk 18: pico8_gfx_t now carries a `pen_color` field
 * (pico8_gfx.h) for exactly this. `gfx` is no longer `const` here (it was
 * before Chunk 18, when this function never wrote through it) since a real
 * `col` argument now RECORDS itself into `gfx->pen_color` as a side
 * effect, matching real PICO-8's own documented "an explicit col argument
 * to a draw call also becomes the new current pen colour" behavior for
 * every draw primitive that takes one -- confirmed against both of this
 * function's own existing real callers (pico8_layout.c's `pico8_rk(gfx,
 * mem, om, dn, 0x10)`, pico8_pi.c's own `bl()`-driven dispatch), both of
 * which already hold a non-const `pico8_gfx_t*` at their own call site, so
 * this signature change needed no caller-side workaround. Deliberately
 * narrow (see pico8_gfx.h's own comment on `pen_color`): only pico8_rk
 * itself updates/reads this field; pico8_rectfill/pico8_rect/etc. are
 * untouched, since no other real call site in this cart is evidenced to
 * need the "col omitted" case. pico8_rk_default (new) is the third real
 * entry point, for the one evidenced col-omitted call site
 * (`rk(127-h,127)`, pj:kj) -- it reads whatever `gfx->pen_color` currently
 * holds (6, PICO-8's own documented default pen colour, until some
 * explicit-col draw call updates it -- see pico8_gfx_init's own comment)
 * rather than taking a `col` parameter at all, mirroring
 * pico8_dv_default/pico8_io_default's own "omitted argument -> no
 * parameter for it at all, not a sentinel value" convention.
 *
 * CORRECTION (found during Chunk 18's own verification): a prior version
 * of this comment claimed "a NULL `gfx` is a safe no-op ... pico8_rectfill's
 * own NULL handling covers the rest" -- false, confirmed by a real ASan/
 * UBSan repro (`pico8_rk(NULL, mem, 0, 5, 3)` crashes, "member access
 * within null pointer", pico8_gfx.c's own pico8_fillp_bit_set -- named
 * pico8_fillp_skips when that repro was run). Neither
 * `pico8_rectfill` nor `pico8_pset` (pico8_gfx.c, both pre-existing,
 * unmodified here) guard their own `gfx` parameter against NULL at all --
 * matching this port's pervasive "the CALLER must guard NULL, not the
 * callee" convention (see e.g. pico8_ob_table_get_name/get_index's own
 * unguarded contract). No real call site in this cart's own scope ever
 * passes a NULL `gfx` to `pico8_rk`, so this was never a live bug -- just
 * an inaccurate safety claim, corrected here rather than adding a guard
 * nothing evidences a need for. */
void pico8_rk(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, pico8_fixed_t y2, int col);
void pico8_rk_y1(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, int col);
void pico8_rk_default(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, pico8_fixed_t y2);

/* function eq(x1,y1,x2)rect(x1,y1,x2+1,y1+12,0)rectfill(x1+1,y1+1,x2,y1+11,5)spr(70,x1,y1,1,2)spr(71,x2-2,y1,1,2)end
 * A help-button frame: a 1px black outline, a gray fill, and two 8x16
 * sprites (70/71) for the button's own left/right icon halves. */
void pico8_eq(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t x1, pico8_fixed_t y1, pico8_fixed_t x2);

/* function io(t,x,y,c,a)
 * if (a)x-=a*4*#t
 * for _,d in pairs(er)do
 * print(t,x+d.x,y+d.y,band(d.c,c))end
 * end
 * Outlined text: draws `t` at 8 surrounding 1px offsets in black (er's own
 * `c=0` entries) then once more at the center in color `c` (er's own
 * `c=15` entry -- band(15,c)==c for any c in 0..15, passing it through
 * unchanged; band(0,c)==0 always, forcing black). `a` (an optional
 * centering factor) offsets x left by `a*4*#t` (`#t` the string's own
 * character count) before drawing -- omitted (`pico8_io_default`) skips
 * this entirely, matching `if (a)`'s own Lua truthiness short-circuit
 * (never evaluating `a*4*#t` at all when `a` is nil). `er`
 * (pico8/lua.lua:143-152's `ob([[o(x=-1,y=-1,c=0),...]])`) is this
 * function's own private data -- ported as a plain fixed C array of
 * {dx,dy,c} triples (not re-run through pico8_ob()), the same "purely
 * constant, no cv-shaped bookkeeping to preserve" reasoning
 * pico8_entity.c's own `bm` array already used for an identical case. */
void pico8_io(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *t, pico8_fixed_t x, pico8_fixed_t y, int c,
              const pico8_fixed_t *a);
void pico8_io_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *t, pico8_fixed_t x, pico8_fixed_t y,
                       int c);

#endif
