#ifndef PICO8_SLIPWAY_DISPLAY_H
#define PICO8_SLIPWAY_DISPLAY_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"

/* Phase 3, Chunk 14 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `jc`'s own "transit-dot drawing" methods -- `ch`, `lg`, `kj`
 * (pico8/lua.lua:657-681) -- split proactively into this sibling module
 * from `pico8_slipway.c`/`.h` (this class's own "core lifecycle/dispatch"
 * methods) from the very first line of this chunk, mirroring Chunk 13's
 * own `pico8_planet.c`/`pico8_planet_display.c` split and this session's
 * own instruction to split proactively rather than discover an over-size
 * module after the fact. See `pico8_slipway.h`'s own top comment for the
 * full `jc` class trace (spec string, tag, module-split rationale). The
 * dispatch bridge for `kj` (a real `bm` name) lives in `pico8_slipway.c`
 * alongside `pico8_jc_init()`'s own class-registration wiring, the same
 * "bridge lives where the registration code lives, calls across the file
 * boundary via this header" precedent `pico8_jo_hb_dispatch`/`pico8_jo_
 * gv_dispatch` already established for `pico8_planet_display.h` (Chunk
 * 13, sub-lot 3). `ch`/`lg` are never `mv()`/`dc()`-dispatched anywhere in
 * scope (both are called only from `kj`, same-class, same-module) -- no
 * bridge is wired for either, matching `pico8_jo_kd`'s own "known method,
 * no dispatch table" precedent. */

/* function jc:ch(f,t)if rnd()<0.08 and self.hg[t.id]then
 * local d=t.ra-f.ra
 * local v=d:nn()add(self.ts,{ra=f.ra+v:pa(),v=v,l=d:qx()})end
 * end
 * (pico8/lua.lua:657-660.) Random chance (8% per call) of spawning one
 * transit-dot particle from `f` toward `t` -- called twice per draw, once
 * each direction (`jc:kj`'s own `self:ch(self.qu,self.re)self:ch(self.re,
 * self.qu)`), but only ever actually spawns when `self.hg[t.id]` is
 * truthy -- `self.hg` (a set of planet-id strings, `jc:pq`'s own fresh
 * `{}`, written by Chunk 12's already-built `bd_route_visit`'s own `w.hg[
 * e.lp[i+1].id]=true`, pico8_economy_tiers.c) is read here via the SAME
 * polymorphic-key lookup that module's own `set_by_value` already
 * establishes for writing it (`t.id` is a real planet's own `.id` field --
 * a STRING, per `bw()`'s own `jo({id="p"..id,...})`, Chunk 20, not yet
 * built -- but genuinely polymorphic in general, so this file's own
 * private `get_by_value` mirrors `set_by_value`'s own FIXED-vs-STRING
 * dispatch rather than assuming STRING specifically).
 *
 * `local d=t.ra-f.ra local v=d:nn()`: `t`/`f`'s own screen-position
 * vectors (`.ra`, set by `bw()`, not yet built -- absent reads as the
 * zero vector, this port's pervasive convention), `v` the unit direction
 * from `f` toward `t` (`pico8_vec_nn`, Phase 2).
 *
 * `add(self.ts,{ra=f.ra+v:pa(),v=v,l=d:qx()})`: a fresh transit-dot
 * record (a plain `pico8_ob_table_t*`, NOT a `kv:jr` instance -- the
 * EXACT same "particle table" shape/ownership convention Chunk 9's own
 * `ei`/`ez` already established for `self.qy`, pico8_particle.h, just
 * with different field names -- `ra`/`v`/`l` here, `p`/`v`/`a`/`l` there)
 * appended to `self.ts` -- offset PERPENDICULAR to the route (`v:pa()`,
 * Phase 2's `pico8_vec_pa`, a 90-degree rotation) so the dot starts
 * slightly to one side of the direct line, `v` itself as its constant
 * velocity, and `l` (its remaining lifetime, in on-screen pixels) as the
 * FULL route length (`d:qx()`, Phase 2's `pico8_vec_qx`) -- `jc:kj` below
 * decrements `l` by 1 every draw and removes the dot once it reaches 0,
 * so a longer route's dots live proportionally longer, matching a
 * constant on-screen travel speed.
 *
 * A NULL `self`/`f`/`t` reads every field off the NULL one as absent
 * (this port's pervasive convention) rather than crashing; a genuinely
 * absent `self.hg`/`self.ts` (never a real case -- `jc:pq` above always
 * sets both) is a safe no-op. */
void pico8_jc_ch(pico8_instance_t *self, const pico8_instance_t *f, const pico8_instance_t *t);

/* function jc:lg(l,h,s,c)local qu,re=
 * self.qu.ra+qm(1.5,self.t/126),self.re.ra+qm(1.5,self.t/176)for dx=l,h,s do
 * for dy=l,h,s do
 * line(qu.x+dx,qu.y+dy,re.x+dx,re.y+dy,c)end
 * end
 * end
 * (pico8/lua.lua:661-666.) Draws the slipway's own dashed/thick line
 * between its two endpoints, at a small, slowly-time-varying polar jitter
 * offset (`qm(1.5,self.t/126)`/`qm(1.5,self.t/176)`, Phase 2's `pico8_vec_
 * qm` -- two DIFFERENT periods for the two endpoints, giving the line a
 * subtle organic wobble rather than a rigid straight segment) -- called
 * twice per draw with different `(l,h,s,c)` tuples (`jc:kj`'s own `self:
 * lg(-1,2,3,bc)` for a thick, sparse "border" pass, then `self:lg(0,1,1,c)`
 * for a thin, dense "fill" pass), drawing one `line()` per `(dx,dy)` pair
 * in the nested Lua numeric for-loops (`for dx=l,h,s do for dy=l,h,s do
 * ...end end`, ported as an ordinary ascending C `for` with step `s`,
 * matching every real call site's own positive-step literals -- no
 * general negative/zero-step handling is added speculatively, the same
 * "port what's evidenced" discipline this whole port already applies).
 *
 * `self.t`: `dg()`'s own per-tick counter, present on every `ke`-
 * descended instance (Chunk 6, already built) -- read here the same
 * defensive "absent numeric field reads as 0" way as everywhere else in
 * this port.
 *
 * A NULL `self`/`gfx`/`mem` is a safe no-op (a NULL `gfx`/`mem` skips
 * every `line()` call; a NULL `self` returns before doing anything). */
void pico8_jc_lg(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_fixed_t l,
                  pico8_fixed_t h, pico8_fixed_t s, int c);

/* function jc:kj()if gd(self.qu)or gd(self.re)then
 * local jv=
 * self.k_[self.nh]
 * if (self.nh<20)self.nh+=1
 * local c,bc,tc=
 * (self.et or self.fo)and 2 or self.mt and 0 or jv,self.mt and jv or 0,self.mt and 5 or 13
 * self:lg(-1,2,3,bc)self:lg(0,1,1,c)for _,t in pairs(self.ts)do
 * t.ra+=t.v
 * t.l-=1
 * if (t.l<=0)del(self.ts,t)
 * local x,y=t.ra.x,t.ra.y
 * rectfill(x,y,x+1,y+1,0)pset(x,y,tc)end
 * self:ch(self.qu,self.re)self:ch(self.re,self.qu)end
 * self.fo=nil
 * end
 * (pico8/lua.lua:667-681.) `jc`'s own draw method -- a real `bm` dispatch
 * name (`bm=["kj","ev","hd","hk","hb"]`, pico8_entity.c), wired via a new
 * `pico8_jc_kj_dispatch` bridge (`pico8_slipway.c`, alongside `pico8_jc_
 * init()`'s own class-registration wiring -- the same "bridge lives with
 * the registration code" precedent `pico8_jo_kj_dispatch` already
 * established, pico8_planet.c). Unlike `jo:kj`, this method declares NO
 * Lua parameters of its own -- `dc("kj")`'s own `entity[nt](entity,
 * entity.ra)` second argument is simply discarded here (real Lua's own
 * "extra call arguments are silently dropped" semantics), so no `p`
 * parameter/`pico8_ma` call is needed the way `jo:kj` needed one.
 *
 * `if gd(self.qu)or gd(self.re)then ... end self.fo=nil`: only draws
 * anything when at least one endpoint is on screen (Chunk 1's `pico8_gd`,
 * NULL-safe -- `self.qu`/`self.re` absent both read as invisible); `self.
 * fo=nil` (this file's own "halo" flag, set by `pico8_jc_hq`/`pico8_bv`
 * pointing another entity's `.fo` back at this slipway) is cleared
 * UNCONDITIONALLY at the very end, whether or not the visibility gate
 * passed -- the SAME "clear once per draw regardless of the visibility
 * gate" shape `pico8_jo_kj`'s own identical `self.fo=nil` placement
 * already established (pico8_planet_display.c).
 *
 * `local jv=self.k_[self.nh]`: `self.k_` (the class's own 20-element
 * fade-in color table, `pico8_slipway.h`'s own top comment) read via the
 * SAME plain `self.k_` field lookup real Lua uses here (unlike `jo:kj`'s
 * own explicit `jo.ka[...]` class-table reference -- this source literally
 * says `self.k_`, so a generic `pico8_instance_get_field(self,"k_",...)`
 * chain-walk, which resolves to the SAME class-default table since no
 * instance ever overrides it, is the exact-literal port here, not a
 * simplification). `self.nh` (reset to 1 by `jc:ey`, defaulting to the
 * class's own 20 before any commit) indexes it 1-based (`pico8_ob_table_
 * get_index`'s established convention). `if (self.nh<20)self.nh+=1`: the
 * fade-in counter climbs back to 20 over 19 draws after a commit,
 * saturating there.
 *
 * `local c,bc,tc=(self.et or self.fo)and 2 or self.mt and 0 or jv,self.mt
 * and jv or 0,self.mt and 5 or 13`: a Lua multi-assignment of THREE
 * independent `A and B or C [or D]` expressions (`and` binds tighter than
 * `or`) -- ported as three separate ternary chains, ALL at full rigor
 * (a truthy `self.mt` genuinely selects the literal fixed `0`/`5` here,
 * never falling through to `jv`/`13`, since a real Lua fixed 0 IS truthy,
 * this port's own pervasively-documented rule): `c` is `2` while a target/
 * halo is set (`self.et`/`self.fo`), else `0` if `self.mt` (a future `fr`-
 * network flag, Chunk 15) is set, else the fade-in color `jv`; `bc` is
 * `jv` if `self.mt`, else `0`; `tc` (the transit-dot color) is `5` if
 * `self.mt`, else `13`. `self.mt` is always absent/falsy in this chunk's
 * own reachable scope (see `pico8_slipway.h`'s own top comment on `self.
 * mt`'s own forward-reference status), so `c`/`bc`/`tc` always resolve to
 * `(self.et or self.fo) and 2 or jv`, `0`, `13` today -- compiles and is
 * mechanically ready for the `self.mt`-truthy branches regardless, the
 * same "forward reference ported, not deferred" treatment every other
 * such case in this port already gets.
 *
 * `self:lg(-1,2,3,bc)self:lg(0,1,1,c)`: the two draw passes `pico8_jc_lg`
 * above documents.
 *
 * `for _,t in pairs(self.ts)do...end`: real Lua's own PLAIN `pairs()`
 * (not `all()`) over an array-shaped table -- ported the SAME "plain
 * forward index walk over the list's live count/items, no compensation
 * for a mid-loop `del()` shift" way `pico8_dg`'s own `for _,qc in
 * pairs(hv)do...end` and Chunk 9's own `pico8_ei`'s identical `for _,p in
 * pairs(self.qy)do...end` loop already established (both documented,
 * deliberate `pairs()`-vs-`all()` divergences, not bugs). Per element `t`
 * (a plain `pico8_ob_table_t*`, this file's own `pico8_jc_ch` above):
 * `t.ra+=t.v` REBINDS `t`'s own `.ra` field to a brand-new heap vector
 * (Lua's `+=` on a whole-vector field goes through `km`'s own `__add`
 * metamethod, producing a NEW table, then assigns it back -- NOT an
 * in-place mutation of the pointee, unlike e.g. `pico8_na`'s own `self.
 * ra.y+=...`); `t.l-=1` decrements the remaining-lifetime field; `if (t.l
 * <=0)del(self.ts,t)` removes the (one-line-if, no `then`/`end` needed)
 * expired dot from `self.ts` -- but, UNLIKE `pico8_ei`'s own `if/else`
 * shape, this source has NO `else`: `local x,y=t.ra.x,t.ra.y rectfill(...)
 * pset(...)` runs UNCONDITIONALLY right after, drawing the dot ONE LAST
 * TIME at its final position even on the very frame it gets removed --
 * ported as a plain sequential `if (del)... ; (draw unconditionally)`,
 * not an if/else, to preserve this exact (and genuinely different from
 * `pico8_ei`'s own shape) one-more-frame-of-drawing behavior.
 *
 * `self:ch(self.qu,self.re)self:ch(self.re,self.qu)`: the two random-
 * chance transit-dot spawns, one per direction.
 *
 * A NULL `self` is a safe no-op for the whole function; a NULL `gfx`/
 * `mem` skips every actual draw call while still running the rest of the
 * bookkeeping (`self.nh`/`t.ra`/`t.l`/`del()`), matching this port's own
 * established "drawing calls tolerate a NULL gfx/mem, state updates don't
 * need to" convention (`pico8_ei`'s own identical treatment). */
void pico8_jc_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self);

#endif
