#ifndef PICO8_SLIPWAY_H
#define PICO8_SLIPWAY_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_value.h"

/* Phase 3, Chunk 14 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `jc`, confirmed by reading pico8/lua.lua directly (`jc=ke:jr(
 * [[...]])`, line 612) to be the SLIPWAY entity -- the trade route a
 * player drags between two colonized planets. `self.qu`/`self.re` are the
 * two `jo`(-family) instances (or, mid-drag, a classless placeholder
 * table) a slipway connects, per pico8_planet.h's own top comment
 * anticipating this class's construction (`jo:ldown`'s own `jc({qu=self})`,
 * Chunk 13). Depends on Chunk 10's `d_`/`dl` (pico8_probe.h), Chunk 13's
 * `jo` (via `self.qu`/`self.re`, and the mutually-recursive trade cluster
 * -- `jc:eb`, this file), and Phase 2's `cl`/`be` (pico8_geom.h).
 *
 * jc=ke:jr([[
 * nu=o("jc"),
 * me="kz",
 * fa=2,
 * fp=f,ik=48,
 * dw=0.004,
 * k_=o(0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1),
 * nh=20,
 * ]])
 * (pico8/lua.lua:612-619.) A `ke` subclass (`ke:jr(...)`), confirmed by
 * reading the source, not assumed -- same tree depth as `bg`/`mh`/`iv`/
 * `g_`/`jo` (all direct `ke` children), nothing structurally new for the
 * class system here. `fp=f` -- `f` is ob()'s own "bare identifier `f`
 * means literal false" token (same convention already confirmed for
 * `g_`'s own `fp=f`, pico8_probe.h), so `fp` genuinely defaults to
 * `false`, not a number -- no real reader of `jc.fp` was found in scope
 * (kept, not guessed at, same treatment `g_`'s own unused `fp` already
 * got). `k_=o(0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1)` is a 20-element
 * positional table of plain fixed numbers -- `jc:kj`'s own `self.k_[self.
 * nh]` indexes it 1-based (`pico8_ob_table_get_index`'s own established
 * convention), matching `self.nh`'s own class default of 20 (the table's
 * own length) and its "reset to 1, count back up to 20" lifecycle (see
 * `pico8_jc_ey`/`pico8_jc_kj` below). `me="kz"` is the icon/animation-
 * state name `dl()`'s own `if self.me=="kz"` branch checks (pico8_probe.h,
 * Chunk 10) -- unlike every OTHER class built so far, `jc` instances are
 * genuinely, always constructed with `me=="kz"` by default (never
 * reassigned by any of this class's own 9 methods), so `dl(self)` (called
 * from `jc:ev` below) is NOT a dead branch here the way it was for every
 * prior class in scope -- this is `dl`'s own FIRST real, non-dead
 * exerciser.
 *
 * New module (`pico8_slipway.c`/`.h`, not `pico8_planet.c`/`pico8_planet_
 * display.c`/`pico8_planet_trade.c` -- a different class entirely, and
 * `pico8_planet.c` is already in the ~430-540 high range per Chunk 13's
 * own final module-size trace): `jc`'s own class declaration plus the 6
 * "core lifecycle/dispatch" methods (`pq`,`hq`,`ey`,`ce`,`eb`,`ev`) live
 * here; the 3 "transit-dot drawing" methods (`ch`,`lg`,`kj`) live in the
 * sibling `pico8_slipway_display.c`/`.h`, proactively split from the very
 * first line of this chunk (not discovered after the fact) -- mirroring
 * Chunk 13's own `pico8_planet.c`/`pico8_planet_display.c` split, and
 * following this session's own instruction to split proactively given
 * Chunk 13's own precedent of a 9-method class landing over threshold
 * when kept in one file. `pico8_class_register_named("jc", ...)` (in
 * `pico8_jc_init()` below) closes, FOR REAL, `jo:ldown`'s own `jc({qu=
 * self})` forward reference (Chunk 13, `pico8_planet.c`'s own `pico8_jo_
 * ldown`) -- the SAME registry mechanism Chunk 7 built for `bg:ldown`'s
 * `mz`/`g_` choice, put to its fourth real use here (after `jo`, `mz`
 * still pending Chunk 16). */

void pico8_jc_init(void);
void pico8_jc_shutdown(void);

/* The real `jc` class, built once by pico8_jc_init(). Retrievable via
 * pico8_class_lookup_named("jc") once pico8_jc_init() has run -- NOT via
 * the shared ""-keyed pico8_class_lookup() registry (same reasoning as
 * pico8_jo_class()'s own comment: `jc` genuinely is `ke`-descended, so it
 * never collides into that shared root slot). Returns NULL before
 * pico8_jc_init() (or after pico8_jc_shutdown()). */
pico8_class_t *pico8_jc_class(void);

/* `jc(overrides)` -- the real Lua constructor call (`jo:ldown`'s own
 * `jc({qu=self})`, Chunk 13). Thin wrapper around
 * pico8_class_instantiate(), matching every other class's own established
 * pico8_X_new convention. */
pico8_instance_t *pico8_jc_new(const pico8_ob_table_t *overrides);

/* function jc:pq()rn(50)self.ts,self.hg={},{}
 * end
 * (pico8/lua.lua:620-621.) `jc`'s own extra constructor: `rn(50)` (Chunk
 * 1's inert `pico8_rn` sfx stub -- a real call site, not dead, still a
 * no-op per this port's own permanently-out-of-scope audio decision).
 * `self.ts,self.hg={},{}`: TWO fresh, independent, empty containers --
 * `self.ts` (a list of transit-dot particle records, `pico8_slipway_
 * display.h`'s own `pico8_jc_ch`/`pico8_jc_kj` read/grow/shrink it) as a
 * fresh heap `pico8_list_t`, and `self.hg` (a set of planet-id strings
 * this slipway has already been offered as a hop for -- written by
 * Chunk 12's already-built `bd_route_visit`, `pico8_economy_tiers.c`'s
 * own `w.hg[e.lp[i+1].id]=true`, read by `pico8_jc_ch` below) as a fresh
 * heap `pico8_ob_table_t`. Wired via `pico8_class_set_pq(jc_class,
 * pico8_jc_pq)` -- `jc`'s own DISTINCT leaf-level `pq`, alongside `ke`'s
 * inherited `pq=lt` (both fire, in that order, per `pico8_class_
 * instantiate`'s own documented "leaf-then-ancestor, skip only an exact
 * repeat" dedup rule, pico8_class.c -- the SAME two-pq-in-one-chain shape
 * `jo:pq`/`ke.pq=lt` already established, Chunk 13; `pico8_jc_pq` itself
 * never calls `pico8_lt` directly, exactly like `pico8_jo_pq` doesn't). A
 * NULL `self` is a safe no-op. */
void pico8_jc_pq(pico8_instance_t *self);

/* function jc:hq(mp)self.re=ok.o_.gv and ok.o_ or {ra=mp,p=mp/128}
 * self.od=
 * dj(self.qu,self.re)self.mt=
 * self.qu.mt or self.re.mt
 * self.et=self:ce()if self.et then
 * self.et.fo=
 * self
 * end
 * self.fa=
 * self.mt and 2 or 4
 * end
 * (pico8/lua.lua:622-632.) The per-frame drag handler -- an mv()-routed,
 * NOT `bm`-name field (`ui:bh()`'s own `mv(held,"hq",self.qa,je.ra)`,
 * pico8_ui.h, Chunk 5, already built -- `bg_hq_dispatch`/`bv_dispatch`'s
 * own identical precedent), called every frame a freshly-`jo:ldown`'d
 * `jc` is held by the mouse.
 *
 * `self.re=ok.o_.gv and ok.o_ or {ra=mp,p=mp/128}`: `ok.o_` (`pico8_ok()`,
 * Chunk 6 -- the ui singleton's own currently-hovered entity, `ui:gw()`'s
 * own `self.o_=e`) is read defensively (a NULL `pico8_ok()`, or a NULL/
 * absent `o_`, reads as "no hover", the same "field access on a possibly-
 * nil value reads as absent rather than crashing" divergence this whole
 * port already applies pervasively -- real Lua would error indexing
 * `nil.gv` if `ok.o_` is itself nil, a real, reachable case (no entity
 * under the mouse this frame) this port deliberately does not reproduce
 * as a crash). `.gv` is `jo`'s own real `PICO8_VALUE_FUNCTION` field
 * (Chunk 13) -- checking it for presence/truthiness is this source's own
 * way of asking "is the hovered entity a `jo`(-family) planet", since a
 * function value is always truthy in Lua and no other class in scope ever
 * defines a `gv` field. When truthy, `self.re` becomes `ok.o_` itself (the
 * real planet, by reference); otherwise a BRAND NEW classless table
 * (`cls=NULL`, matching `pico8_bv`'s own `{}`-as-`pico8_instance_t`
 * convention, pico8_probe.c) holding `ra=mp` (the raw mouse-position
 * value, by reference, same "field-value copy, REF identity preserved"
 * treatment `pico8_bv`'s own `self.ra=mp` already established) and
 * `p=mp/128` (a fresh heap vector).
 *
 * `self.od=dj(self.qu,self.re)`: Chunk 12's already-built `pico8_dj`
 * (pico8_economy_formulas.h) -- trade-route base cost between the two
 * endpoints, tolerant of `self.re` being the classless placeholder above
 * (reads its own `.p` field the same generic way either way).
 *
 * `self.mt=self.qu.mt or self.re.mt`: `.mt` is never set by `jo`'s own
 * class spec, nor by any subclass ported so far -- only Chunk 15's `fr`
 * (`fr=nr:jr([[mt=1,...]])`, pico8/lua.lua:1123-1127, not yet built) ever
 * sets it for real. A genuine forward reference, ported literally (full
 * `A or B` semantics -- the whole value of whichever side is truthy,
 * preserved as-is, not narrowed) -- compiles and is mechanically ready,
 * simply not yet meaningfully exercised until Chunk 15 lands, the same
 * treatment every other forward reference in this port already gets.
 *
 * `self.et=self:ce()`: a plain, same-class direct call (`pico8_jc_ce`
 * below -- `:ce()` has no other real caller anywhere in scope, so no
 * dispatch bridge is wired for it, matching `pico8_jo_kd`'s own "known
 * method, no dispatch table" precedent). `if self.et then self.et.fo=self
 * end`: only when `self:ce()`'s result is genuinely truthy (never the
 * `false` a classless `{qs=...}` table can't produce -- `ce()` never
 * returns a bare boolean) does the resolved blocker/target get its own
 * `.fo` ("found by") field pointed back at this slipway, the SAME `.fo`
 * convention `pico8_bv`'s own `self.et.fo=self` already established
 * (Chunk 10) -- `jc:kj` (pico8_slipway_display.h) reads it back via the
 * same `(self.et or self.fo)` halo check `jo:kj` already established for
 * its own `self.fo`.
 *
 * `self.fa=self.mt and 2 or 4`: unconditionally REBINDS the draw-layer
 * field the class spec also defaults to `fa=2` -- since `self.mt` is
 * always absent/falsy in this chunk's own reachable scope (see above),
 * this always resolves to `4` today, faithfully reproduced rather than
 * simplified away (a future `fr`-connected slipway will genuinely see
 * `self.fa=2` once Chunk 15 lands).
 *
 * A NULL `self` is a safe no-op for the whole function. */
void pico8_jc_hq(pico8_instance_t *self, pico8_value_t mp);

/* function jc:ey(re)if not re
 * or not d_(self)then
 * if (stat(19)==50)rn(-1)
 * self.p_=true
 * return
 * end
 * self.nh,self.re=1,re
 * self.qu:kf(self)re:kf(self)fm:nx(1)end
 * (pico8/lua.lua:633-640.) The "commit this drag to a target planet"
 * handler -- an mv()-routed, NOT `bm`-name field (`ui:bh()`'s own `mv(
 * held,"ey",nw and nw.kv)`, argc=1 -- the PRD's own Problem Statement
 * cites this EXACT call site as its evidence for `pico8_mv`'s 1-argument
 * arity, Chunk 5).
 *
 * `if not re or not d_(self)then`: real Lua's own `or` short-circuit --
 * `d_(self)` (Chunk 10's already-built, directly-callable `pico8_d_`,
 * pico8_probe.h -- its own header comment already names THIS exact call
 * site: "also called DIRECTLY as a plain function by jc:ey(), its own
 * boolean RESULT genuinely consumed") is called ONLY when `re` is
 * genuinely present, ported as C's own `||` short-circuit over an
 * explicit `re==NULL` check first (never evaluating `pico8_d_` when `re`
 * is NULL, matching the real short-circuit exactly, not just relying on
 * incidental C evaluation order for a struct-returning call).
 *
 * Failure branch: `if (stat(19)==50)rn(-1)` -- Chunk 1's `pico8_stat`
 * (pico8_input.h) documents every index outside 32/33/34 as returning 0,
 * so this is dead-but-compiled code (`0==50` is always false), the same
 * "already-documented stat() fallback makes this branch permanently dead,
 * ported literally anyway" treatment the PRD's own Implementation
 * Decisions section explicitly calls out for this EXACT branch (User
 * Story 8). `self.p_=true`: the probe/slipway self-destruct flag (`dg()`'s
 * own future `if qc.p_ then he(qc) end`, Chunk 6, already built). `return`
 * (implicit nil) ends the function here on this branch.
 *
 * Success branch: `self.nh,self.re=1,re` -- `self.nh` (the fade-in tick
 * counter `jc:kj`'s own `self.k_[self.nh]` indexes, class default 20)
 * resets to the genuine Lua integer 1 (`PICO8_VALUE_FIXED`, not a
 * boolean), and `self.re` is REBOUND from whatever `jc:hq` last set it to
 * (a real planet or the classless placeholder) to the now-confirmed real
 * target `re`. `self.qu:kf(self)`/`re:kf(self)`: two genuinely
 * `mv()`-dispatched cross-class calls (the receiver's concrete class --
 * `jo` or a future `mx` override, Chunk 15 -- isn't statically known any
 * more here than at `jo:ib`'s own identical `w:eb(self):cn(lw)` call
 * shape, `pico8_planet_trade.h`), registering this now-committed slipway
 * onto BOTH endpoints' own `.oe` connection lists (`jo:kf`, Chunk 13,
 * already built and reachable) -- this is HALF of what finally makes the
 * mutually-recursive trade cluster end-to-end exercisable through a real
 * `jc` instance (see this file's own top comment and `pico8_jc_eb` below
 * for the other half). `fm:nx(1)`: a DIRECT call (`pico8_hy_nx(pico8_fm(),
 * ...)`, NOT `pico8_mv()`) -- `hy` (Chunk 12) is never subclassed anywhere
 * in the real cart, so there is no runtime-polymorphic receiver to resolve
 * here, matching `pico8_economy.h`'s own explicit, documented convention
 * for every `hy` method ("none ... are ever called via mv()/dc() ...
 * ported as direct, known C functions") -- see `pico8_probe.c`'s own
 * `pico8_d_` for the CORRECTION this chunk's own reading of that
 * convention also applied there (a stale `pico8_mv()` call left over from
 * before `hy` existed, found while building this exact end-to-end path).
 * Pays the flat cost of establishing a slipway.
 *
 * A NULL `self` is a safe no-op for the whole function; real Lua's own
 * "extra call arguments are silently discarded" semantics cover the case
 * where `mv()`'s own `argc` doesn't match what this function reads (it
 * only ever reads its own declared `re` parameter). */
void pico8_jc_ey(pico8_instance_t *self, pico8_instance_t *re);

/* function jc:ce()local sp,dp=self.qu.p,self.re.p
 * if #(sp-dp)>ov("space folding",0.5,1.08)then
 * return {qs="-too far-"}
 * end
 * for w in all(self.qu.oe)do
 * if (w:eb(self.qu)==self.re)return w
 * end
 * end
 * local ds=la(jp(cg.np,function(n)
 * return be(sp,dp,n.p,0.0032)end),jp(cg.jc,function(sw)
 * return sw.mt==self.mt and
 * cl(sp,dp,sw.qu.p,sw.re.p)end))
 * return ds[1]
 * end
 * (pico8/lua.lua:641-653.) "What, if anything, blocks (or already
 * satisfies) a slipway between `self.qu` and `self.re`?" -- called only
 * from `pico8_jc_hq` above (`self:ce()`, a plain same-class call, no
 * dispatch bridge needed, matching `pico8_jo_kd`'s own precedent).
 *
 * `if #(sp-dp)>ov("space folding",0.5,1.08)then return {qs="-too far-"}
 * end`: `#(sp-dp)` is the SQUARED distance (`pico8_vec_len_sq`, PICO-8's
 * `#` on a `km` vector -- Phase 2's own established convention, NOT the
 * Euclidean distance); `ov(...)` (Chunk 11's already-built `pico8_ov`,
 * pico8_tech_gate.h) resolves to its own `lk` argument (0.5) until Chunk
 * 12's own `fm.invented` genuinely lists "space folding" -- narrowed to a
 * `pico8_fixed_t` (defaulting to 0, this port's pervasive "absent/wrong-
 * kind numeric field reads as 0" convention, in the unevidenced case its
 * result isn't `PICO8_VALUE_FIXED` -- never actually reached here, since
 * both of `pico8_ov`'s own real arguments at this call site are already
 * fixed literals). On overflow, returns a BRAND NEW classless table
 * (`cls=NULL`, the SAME representation `pico8_jc_hq`'s own placeholder
 * `{ra=mp,p=mp/128}` above uses) holding one named field, `qs="-too
 * far-"` (a plain C string literal, never freed -- matching every other
 * literal-string `pico8_value_string(...)` call already in this port,
 * e.g. `pico8_ov(pico8_value_string("space folding"),...)` itself).
 *
 * `for w in all(self.qu.oe)do if (w:eb(self.qu)==self.re)return w end
 * end`: `self.qu.oe` (Chunk 13's already-built connection list) -- for
 * each already-connected slipway `w`, dispatches `w:eb(self.qu)`
 * (`pico8_mv(w,"eb",...)`, `pico8_jc_eb` below -- `w` is always a real
 * `jc` instance here, found via `cg.jc`-registered `oe` membership, but
 * genuinely `mv()`-dispatched anyway per this cluster's own established
 * "no receiver-class special-casing" rule) and compares the result to
 * `self.re` by REFERENCE (`pico8_value_equal`'s own `PICO8_VALUE_REF`
 * pointer-identity rule, the correct tool here -- these are real table/
 * instance identities, not interned strings) -- "does an existing
 * slipway from `self.qu` already reach `self.re`?"; if so, reuse it
 * (`return w`) rather than proposing a duplicate.
 *
 * `local ds=la(jp(cg.np,fn1),jp(cg.jc,fn2))return ds[1]`: builds the
 * combined "what's in the way" list -- `fn1` (`be(sp,dp,n.p,0.0032)`,
 * Phase 2's already-built `pico8_geom_be`) keeps every known planet
 * (`cg.np`, Chunk 13) lying near the segment `sp`-`dp` (including `self.
 * qu`/`self.re` THEMSELVES is not specially excluded -- `be`'s own
 * `l_>0.01 and l_<0.99` range already excludes a segment's own endpoints
 * naturally, since the projection fraction is exactly 0 or 1 there, so
 * no extra filtering is added, matching this source's own literal
 * shape); `fn2` (`sw.mt==self.mt and cl(sp,dp,sw.qu.p,sw.re.p)`, Phase
 * 2's already-built `pico8_geom_cl`) keeps every existing slipway (`cg.
 * jc`) sharing this one's own `.mt` group (`pico8_value_equal` -- a REF-
 * (NULL)-vs-REF(NULL) comparison correctly reads as EQUAL when both are
 * absent, exactly mirroring real Lua's own `nil==nil` -- no `.mt`-
 * specific string-content gap here, since these are `PICO8_VALUE_REF`
 * comparisons, not `PICO8_VALUE_STRING`) AND geometrically crossing the
 * new route -- this naturally never matches `self` itself even though
 * `self` is already `cg.jc`-registered by the time this runs (`jc:pq()`'s
 * own `lt()` self-registration fires at construction): `cl(sp,dp,sp,dp)`
 * (comparing a segment against ITSELF) always resolves false, since
 * `pico8_geom_ee(p1,p2,p1)`'s own cross product is exactly zero (`c-b`
 * term vanishes), giving `PICO8_EE_ON_LINE`, which `cl`'s own `oq1 and
 * ...` chain treats as falsy and short-circuits on -- no special-casing
 * needed, the same "the geometry itself excludes this" reasoning this
 * comment documents rather than working around defensively. `pico8_la`
 * concatenates (`fn1`'s matches first, then `fn2`'s, in order); `ds[1]`
 * is the first (or nil if both empty).
 *
 * A NULL `self` returns `pico8_value_ref(NULL)` (nothing to check). */
pico8_value_t pico8_jc_ce(pico8_instance_t *self);

/* function jc:eb(jo)
 * return jo==self.qu and self.re or self.qu
 * end
 * (pico8/lua.lua:654-656.) "The planet at the OTHER end of this slipway
 * from `jo_instance`" -- THE method that completes the other half of the
 * mutually-recursive trade cluster `pico8_planet_trade.h`'s own top
 * comment already names by name (`jo:kf`'s `sw:eb(self).fx`, `jo:ib`'s
 * `w:eb(self):cn(lw)`, both Chunk 13, already built and, until this
 * chunk, always resolving to "field not found"). The Lua parameter is
 * named `jo` (shadowing the CLASS name `jo` within this function's own
 * scope only -- the same shadowing precedent `pico8_jo_lz`'s own `hp`
 * parameter already established, pico8_planet.h) -- renamed
 * `jo_instance` here for the same reason.
 *
 * Ported at FULL `A and B or C` rigor, not simplified to a plain
 * ternary: real Lua's own gotcha (a truthy `A` with a FALSY `B` falls
 * through to `C`) is preserved via an explicit truthiness check on
 * `self.re`, even though every real call site in scope only ever
 * observes `self.re` genuinely set (by `jc:hq`/`jc:ey` above) by the
 * time `eb` is dispatched -- matching this port's own established "port
 * the exact idiom, don't assume away a case just because it's
 * unevidenced" discipline (`pico8_jo_gg`'s own identical rationale,
 * pico8_planet.h).
 *
 * Wired as a real `PICO8_VALUE_FUNCTION` field ("eb", `pico8_jc_eb_
 * dispatch`, `pico8_slipway.c`) -- NOT one of `bm`'s 5 real dispatch
 * names -- closing, for real, `jo:kf`'s and `jo:ib`'s own forward
 * references to `sw:eb`/`w:eb` the moment any real `jc` instance exists.
 * Also directly callable as this plain C function (`jc:ce`'s own `w:eb(
 * self.qu)` call above uses it exactly this way, via `pico8_mv`, matching
 * the cluster's own "always route through mv(), never special-case the
 * receiver" rule even for a same-module call). A NULL `self` reads
 * `self.qu`/`self.re` as absent, returning `pico8_value_ref(NULL)`. */
pico8_value_t pico8_jc_eb(const pico8_instance_t *self, const pico8_instance_t *jo_instance);

/* function jc:ev()
 * if (self.qu~=self.re)dl(self)
 * end
 * (pico8/lua.lua:682-684.) `ev` IS one of `bm`'s 5 real dispatch names
 * (`bm=["kj","ev","hd","hk","hb"]`, pico8_entity.c) -- wired with the
 * same "class-default PICO8_VALUE_FUNCTION field, real bridge, real `cv`
 * auto-registration at lt() time" treatment `jo:hb`/`jo:hk`/`g_:ev=dl`
 * already established, closing a future `dc("ev")`'s own dispatch loop
 * for real the moment any real `jc` instance is `lt()`'d.
 *
 * `self.qu~=self.re`: pointer/reference inequality (`self.qu`/`self.re`
 * compared by IDENTITY, matching every other Lua `==`/`~=` on a table in
 * this port) -- true whenever `self.re` is anything other than the exact
 * same instance as `self.qu` (absent/nil before the first `jc:hq()` call,
 * the classless drag placeholder, or a genuinely different target planet
 * all satisfy this; only a degenerate same-planet self-loop, `self.qu==
 * self.re`, skips the call). `dl(self)` (Chunk 10's already-built,
 * directly-callable `pico8_dl`, pico8_probe.h) is called UNCONDITIONALLY
 * on that inequality -- this is `dl`'s own FIRST real, non-dead exercise
 * in this whole port (see this file's own top comment: every `jc`
 * instance's own `me` field genuinely stays `"kz"`, the one state `dl`
 * itself actually draws for, unlike every prior class in scope).
 *
 * A NULL `self` is a safe no-op. */
void pico8_jc_ev(pico8_instance_t *self);

#endif
