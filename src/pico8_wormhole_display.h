#ifndef PICO8_WORMHOLE_DISPLAY_H
#define PICO8_WORMHOLE_DISPLAY_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_vec.h"

/* Phase 3, Chunk 16 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `mz`'s own "kj" (draw) method -- see pico8_wormhole.h's own top
 * comment for the full class-level trace (the real Lua source for every
 * `mz` method, the bm-vs-mv()-routed-vs-pq classification, and the
 * "SELF.ET REPRESENTATION" finding). Split into its own display-concern
 * sibling module, matching `pico8_planet.c`/`pico8_planet_display.c`'s
 * already-established precedent: `pico8_mz_kj`'s own body lives here,
 * while its `pico8_mz_kj_dispatch` bridge stays in `pico8_wormhole.c`
 * alongside `pico8_wormhole_init()`'s own class-registration wiring (the
 * exact same "bridge lives with the registration code, body lives in the
 * display sibling" split `pico8_jo_kj`/`pico8_jo_kj_dispatch` already
 * established, `pico8_planet_display.h`'s own top comment). This one
 * function does not by itself justify a split purely on module-size
 * grounds (`pico8_wormhole.c` stays comfortably under the ~430-540-line
 * flagged range even with `kj` folded in) -- it is split anyway to keep
 * this port's own "draw methods live in a `_display` sibling" convention
 * uniform across every multi-method class this phase has ported so far
 * (`jo`/`pico8_planet_display.c`, and this module's own single-function
 * mirror of that same shape), rather than special-casing `mz` as the one
 * exception with its own draw method inlined into the main class file. */

/* function mz:kj(p)if self.me=="kz" then
 * if not self.et then
 * circ(p.x,p.y,self.oi,1)end
 * local cp=self.ij.ra
 * line(cp.x,cp.y,p.x,p.y,1)spr(7,p.x-4,p.y-4)else
 * for dt=0,8,2 do
 * if self.t>dt then
 * local sr=self:ej(self.t-dt)circ(p.x,p.y,sr,self.kk[flr(sr/self.oi*8)])end
 * end
 * end
 * end
 * (pico8/lua.lua:782-793.) `mz`'s own draw method -- one of `bm`'s 5 real
 * dispatch names, called with `p=self.ra` (a vec), matching `dc`'s own
 * literal `qc[nt](qc,qc.ra)` call convention and `pico8_jo_kj`'s own
 * identical `(gfx,mem,self,p)` signature shape (`pico8_planet_display.h`).
 *
 * `if self.me=="kz" then ... else ... end`: a STRING-CONTENT comparison
 * (`strcmp`, matching `jo:gg`/`jo:gv`/`bg:ldown`/`pico8_dl`'s own
 * established treatment) -- while the wormhole is still in its initial
 * "approaching"/aiming state (`self.me` starts at `"kz"`, `mz`'s own class
 * default, until `d_`'s own `self:ks(self.dq or "ph")` -- `dq="pr"` --
 * transitions it once a target is committed), one visual is drawn; once
 * committed (any OTHER `self.me` value -- in practice always `"pr"`, since
 * that is the only state `d_` ever transitions `mz` into), a different one
 * is drawn instead.
 *
 * `"kz"` BRANCH: `if not self.et then circ(p.x,p.y,self.oi,1) end` draws
 * the expanding "approaching" ring (`self.oi`, grown every tick by `mz:kz`,
 * `pico8_wormhole.c`) UNLESS a target has already been found (`self.et`,
 * set by `mz:hq`'s own call into `bv` -- this port's own established
 * truthy rule, falsy only for `PICO8_VALUE_BOOL(0)`/`PICO8_VALUE_REF
 * (NULL)`, applied identically to `pico8_dl`'s/`pico8_bv`'s own `self.et`
 * truthiness checks). `local cp=self.ij.ra line(cp.x,cp.y,p.x,p.y,1)
 * spr(7,p.x-4,p.y-4)` then run UNCONDITIONALLY (outside that inner `if`,
 * still inside the `"kz"` branch): `self.ij` is the nearest `.ox` candidate
 * `mz:hq`'s own `ij(self.p,self.ox)` call found (`pico8_wormhole.c`'s own
 * `pico8_mz_hq`) -- ALWAYS a real `pico8_instance_t*` REF or Lua-nil
 * (`self.ox` only ever holds real `hv` entity instances, per `pico8_mz_pq`'s
 * own comment, `pico8_wormhole.h` -- never `mz.jj`'s own raw-table
 * representation, so no wrapper is needed to read `self.ij.ra` safely
 * here), so `cp` (its own `.ra` render-position field, read the same
 * defensive-zero-vector way as every other absent-vector-field read in
 * this port) is well-defined even before `hq` has ever run once (reading
 * as the zero vector then). A line is drawn from that nearest reachable
 * point toward the wormhole's own current draw position `p`, plus a sprite
 * 7 icon centered on `p` (`spr(7,...)` with no `w`/`h`, mapping to
 * `pico8_spr_default`, not `pico8_spr` -- matching `pico8_dl`'s own
 * identical `spr(31,...)` no-`w`/`h` treatment).
 *
 * OTHER-STATE BRANCH: `for dt=0,8,2 do if self.t>dt then local sr=self:ej
 * (self.t-dt)circ(p.x,p.y,sr,self.kk[flr(sr/self.oi*8)])end end` draws up
 * to 5 concentric "ripple" rings (`dt`=0,2,4,6,8), each only once
 * `self.t>dt` (so fewer rings show in the first few ticks after the
 * "pr" transition, more as `self.t` grows), each at its own radius
 * `self:ej(self.t-dt)` (`pico8_mz_ej`, a direct same-class call -- this
 * file's own only real call site alongside `pico8_mz_pr`'s identical one,
 * `pico8_wormhole.c`) and its own color, indexed into `self.kk`
 * (`kk=o(7,7,6,13,5,1,1,1,0)`, `mz`'s own class default, a 9-entry
 * POSITIONALLY-1-INDEXED Lua table -- resolved via the normal instance
 * field-chain fallback since no real `mz` instance ever overrides `kk`
 * per-instance) by `flr(sr/self.oi*8)`. This index genuinely ranges 0..8
 * as `sr` sweeps from 0 up to `self.oi` (`sr/self.oi` sweeps 0..1, times 8
 * sweeps 0..8) -- INCLUDING THE REAL, FAITHFULLY-REPRODUCED EDGE CASE that
 * index 0 itself (reached exactly when `sr==0`, i.e. whenever `self.t==dt`
 * for one of the 5 `dt` values) reads `self.kk[0]`, a POSITION THE REAL
 * LUA TABLE NEVER SETS (its own entries start at index 1) -- so this is a
 * genuine, evidenced moment where the real cart itself passes Lua `nil` as
 * `circ`'s own color argument, not a porting gap. Ported via `pico8_ob_
 * table_get_index` (`pico8_ob.h`, the exact same 1-based, no-renumbering
 * indexing the real Lua table itself uses) with a `color=0` default on a
 * failed/wrong-kind lookup -- the same "absent numeric class-level lookup
 * defaults to 0" convention `pico8_jo_kj`'s own identical `jo.ka[self.mi]`
 * read already established (`pico8_planet_display.c`'s own `int color=0;`
 * default), not a new decision made here. `self.oi`'s own possible-0 value
 * (never yet set on a freshly-instantiated `mz`, before `mz:kz` has ever
 * run once) is divided into unconditionally, inheriting `pico8_fixed_div`'s
 * own documented zero-divisor saturation rather than a special-cased
 * guard, matching `pico8_vec_nn`'s own identical divide-by-`qx()`-of-zero
 * precedent (`pico8_vec.h`).
 *
 * A NULL `self` is a safe no-op (nothing to draw); a NULL `gfx`/`mem`
 * skips every actual draw call while still reading fields (mirroring
 * `pico8_jo_kj`'s/`pico8_dl`'s own identical `gfx!=NULL&&mem!=NULL` guard
 * convention) -- though in practice this function reads no field it
 * wouldn't otherwise need regardless of `gfx`/`mem`, so the guard here is
 * purely around the drawing primitive calls themselves. */
void pico8_mz_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p);

#endif
