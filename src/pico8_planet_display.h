#ifndef PICO8_PLANET_DISPLAY_H
#define PICO8_PLANET_DISPLAY_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Phase 3, Chunk 13 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md), THIRD sub-lot: PURE MODULE SPLIT (no port-logic change) plus
 * `ldown`/`fe` themselves. `jo`'s own two prior sub-lots (see
 * `pico8_planet.h`'s own top comment for the full class-level trace: the
 * spec string, the PRD's 16-vs-17 method-count discrepancy, the sub-lot
 * 1/2 porting decisions) grew `pico8_planet.c`/`.h` to 767/722 lines --
 * past the ~430-540-line range twice already flagged by `warn-review`
 * (`docs/plan.md`'s "Notes de portage"), and explicitly flagged by
 * sub-lot 2's own write-up (`docs/portage-progress.md`'s "Vague 13"
 * entry) as requiring a split BEFORE this session's own `ldown`/`fe`
 * additions.
 *
 * Split taken (sub-lot 2's own proposed 3-way plan, STEP 1 of this
 * sub-lot's own two-step task): `pico8_planet_display.c`/`.h` (this pair
 * -- `gh`,`gv`, `kj` once a later sub-lot ports it, `hb`),
 * `pico8_planet_trade.c` (`kf`,`cn`,`ib` -- not created yet, nothing to
 * move there today), `pico8_planet.c`/`.h` (the class declaration
 * itself, plus `pq`,`lz`,`kd`,`ll`,`hu`,`hh`). Every trace comment below
 * for `gh`/`gv`/`hb` (the exact Lua source pasted above each function,
 * and every explanatory paragraph under it) is copied VERBATIM from
 * `pico8_planet.h` -- no wording, semantics, or Lua-source text changed,
 * only file location. The only edits made while moving are a handful of
 * now-stale self-references ("this file's own top comment" meaning
 * `pico8_planet.h`'s class-level overview, which stayed behind)
 * corrected to name `pico8_planet.h`/`pico8_planet.c` explicitly instead
 * of "this file" -- each such edit is a pure cross-reference fix, not a
 * change to what is being said.
 *
 * `pico8_jo_hb_dispatch`/`pico8_jo_gv_dispatch` (the `mv()`-dispatch
 * bridges for "hb"/"gv") stay in `pico8_planet.c`, alongside
 * `pico8_jo_init()`'s own class-registration wiring that installs them --
 * they now call the `pico8_jo_hb`/`pico8_jo_gv` declared below across the
 * file boundary, exactly as they already called them as plain sibling
 * functions before the split (`pico8_planet.c` includes this header). No
 * behavioral change: same functions, same call sequence, only some calls
 * now cross a translation-unit boundary via a header declaration instead
 * of a same-file static-visible one.
 *
 * *** STEP 2 REBALANCE: `fe` lands HERE, not in `pico8_planet.c` ***
 * (a deviation from sub-lot 2's own original 3-way plan, made per this
 * session's own explicit "check sizes before deciding" instruction) --
 * after porting `ldown`/`fe` into `pico8_planet.c` as sub-lot 2's plan
 * originally proposed, that file measured 663 lines, still over the
 * ~430-540 range even after the STEP 1 split above (`fe`'s own bulk --
 * the "my" closure's context struct, dispatch bridge, and per-row mapper
 * step -- is what re-blew the budget, ~180 of those 663 lines belong to
 * `fe` alone). Moving `fe` (and ONLY `fe` -- `ldown` itself is small,
 * ~28 lines, and stays in `pico8_planet.c` alongside the rest of `jo`'s
 * own lifecycle methods it's most cohesive with, per this session's own
 * "your call" latitude) here brings `pico8_planet.c` back to 479 lines
 * (comfortably under threshold) at the cost of `pico8_planet_display.c`
 * growing to ~564 lines -- still a net improvement (one file moderately
 * over the range instead of one file well over it), and thematically
 * apt: `fe` is itself a display/panel-building function (it renders the
 * research panel via `pz()`), same family as `gh` (renders the info
 * panel). `pico8_jo_ldown` (`pico8_planet.c`) calls `pico8_jo_fe` across
 * this same file boundary, via this header (already included by
 * `pico8_planet.c`), exactly like the `hb`/`gv` dispatch bridges already
 * do the reverse direction. If `pico8_planet_display.c` itself later
 * crosses the range once `kj` lands, that is a decision for whichever
 * sub-lot ports `kj` to make, not pre-empted here.
 *
 * *** SUB-LOT 5 (this session): `kj` lands here too *** -- exactly where
 * this file's own STEP 1 comment above already anticipated it ("gh,gv,
 * kj once a later sub-lot ports it, hb"). `kj` is the PRD's own explicitly
 * flagged "first real, non-dead `dc('kj')` exercise" method (see this
 * chunk's own PRD text, Testing Decisions section) -- it is one of `bm`'s
 * 5 real dispatch names, so wiring it as a real field on `jo`'s class
 * defaults (`pico8_planet.c`'s own `pico8_jo_kj_dispatch`, alongside
 * `pico8_jo_init()`'s own class-registration wiring, the same "bridge
 * lives with the registration code" precedent `hb`/`gv` already
 * established) is what finally makes `dc("kj")` -- called for real once a
 * frame by `_draw()`, Chunk 22, not yet built -- reach a genuine, non-
 * placeholder `jo` instance's own draw method for the first time in this
 * whole port, alongside `mo:md(ci[o])` (already real since Chunk 4's
 * `qb:md`, wired as `mo`'s own "md" the moment Chunk 6's `pico8_jl()`
 * makes `mo` a real `qb` instance). This module's own `pico8_jo_truthy`
 * (already duplicated here, sub-lot 3) and `pico8_jo_class()` (the public
 * accessor, not the private `pico8_planet.c`-only `g_jo_class`, same
 * precedent `gh`'s own `jo.dz`/`jo.cq` reads already established) are
 * reused for `kj`'s own `jo.ka` class-level read. See `pico8_jo_kj`'s own
 * declaration below for the full per-line trace, including two real,
 * previously-invisible gaps this sub-lot found and fixed (`lq`'s own "ol"
 * field never wired for `mv()`, mirroring the already-logged `hp`'s own
 * "lq" gap; `self.qw` never set anywhere in scope for any `jo`-family
 * instance). (Phase 1's deliberate scope narrowing -- `fillp`'s PICO-8
 * fractional "transparency" flag having no separate representation in this
 * port -- was undone by a Phase 5 bug fix; see `pico8_jo_kj`'s own
 * declaration below.) */

/* function jo:hb()self.lq=jo.ex(self.jy,self.kw.qe,self.kw.mu,#self.by,0
 * )end
 * (pico8/lua.lua:539-540.) Rebuilds this planet's own floating info-icon
 * row (`self.lq`, an `lq` instance -- Chunk 3) from its current wants
 * (`self.jy`, set by `jo:lz` above) and export state (`self.kw.qe`/
 * `self.kw.mu`) and incoming-trade count (`#self.by`, Chunk 13's own
 * `jo:ib()` result field, deferred -- absent/NULL reads as 0, this port's
 * pervasive "missing list counts as empty" convention). Calls
 * `pico8_jo_ex` (`pico8_research_display.h`, Chunk 11, ALREADY BUILT --
 * this is the exact real call site that chunk's own top comment
 * anticipated: "only ONE single Lua GLOBAL FIELD read from within its
 * body, `jo.em`, genuinely forward-references [jo]... That single read
 * ... is NULL today"; `jo.em` resolves for real the moment THIS chunk's
 * own `pico8_class_register_named("jo",...)` call above has run, since
 * `pico8_jo_ex` reads it via `pico8_class_lookup_named("jo")->defaults`)
 * directly (a known function, not `mv()`-routed).
 *
 * `hb` IS one of `bm`'s 5 real dispatch names (see `pico8_planet.h`'s own
 * top comment on the PRD's 16-vs-17 undercount) -- wired as a real
 * `PICO8_VALUE_FUNCTION` field via `pico8_jo_hb_dispatch` (`pico8_planet.c`,
 * a bridge with no extra arguments, matching `hy:lj()`'s own real
 * `e:hb()` call shape, Chunk 12, already built) so any real `jo` instance
 * this sub-lot's own construction produces is reachable through
 * `hy:lj()`'s own `for e in all(cv.hb)do e:hb()end` dispatch loop for
 * real, closing that forward reference. Also directly callable as this
 * plain C function (no other real same-class caller found in scope, but
 * matches this port's established "expose the known function, wire a thin
 * bridge alongside it" pattern regardless). A NULL `self` is a safe
 * no-op. */
void pico8_jo_hb(pico8_instance_t *self);

/* function jo:gh()local bs={{lq=
 * self.px or
 * jo.dz[self.mi]
 * }}
 * bs[1].h_=self.kr
 * and {ne(self.kr).."$",bg=self.kr>=0 and 1}
 * local ji={"receives:"}
 * local jx=jp(self.jy,function(i)for j=1,i.gc do
 * add(ji,i)end
 * return i.gc==0
 * end)if jx[1]then
 * add(bs,{lq=la({"needs:"},jx)})else
 * if self.mi<4 and #self.jy>0 and self.kw.qe~=r.w then
 * add(bs,{lq=la({"wants more:"},self.qh
 * and {self.jy[#self.jy]}
 * or self.jy
 * )})end
 * end
 * add(bs,ji[2]and {lq=ji})if self.kw.mu>0 then
 * local lb=gu(jo.cq,self.kw.qe)and {"makes:",tostr(self.kw.mu),self.kw}
 * or {"exports:",#self.by.."/"..self.kw.mu,self.kw}
 * add(bs,{lq=lb})end
 * for i=2,#bs do
 * add(bs[i].lq,{false,1})end
 * pz(self,bs)end
 * (pico8/lua.lua:424-448.) The planet's own click-to-inspect info panel --
 * builds `bs` (a list of `{lq=...,h_=...}` rows, exactly the shape
 * `pico8_pz`'s own header comment documents, pico8_msgbox.h Chunk 8) and
 * hands it to `pz(self,bs)` (already built) to render.
 *
 * `bs[1]`: the icon row. `self.px or jo.dz[self.mi]` -- `self.px` (an
 * optional per-instance override, truthy-checked via this file's own
 * `pico8_jo_truthy`) if present, else `jo.dz[self.mi]` (the CLASS-level
 * `dz` field -- one of the four `o("unhappy",c=8)`-shaped status tuples
 * from `pico8_planet.h`'s own top comment's class-spec trace -- indexed by
 * `self.mi`, absent-mi reading as "no index", matching real Lua's own
 * safe nil-key table read). `bs[1].h_`: `self.kr and {...}` -- `self.kr`
 * truthy (this port's `pico8_jo_truthy`, so a real fixed 0 still counts)
 * gets a `{text,bg=...}` tuple (`ne(self.kr)` -- `c`/`ku` both omitted,
 * per pico8_collect.h's own documented "only ne()'s first return value is
 * ever used when callers omit `c`" convention -- with a literal "$"
 * appended; `bg=self.kr>=0 and 1`, a real fixed 1 when true, the literal
 * boolean `false` -- stored as a real field value, not omitted -- when
 * false); absent/falsy `self.kr` leaves `h_` as `self.kr` itself (nil).
 *
 * `ji`/`jx`: `jp(self.jy,predicate)`'s own predicate has a real SIDE
 * EFFECT (`for j=1,i.gc do add(ji,i) end`, appending `i` to the captured
 * `ji` list `i.gc` times) alongside its filter condition (`i.gc==0`) --
 * ported via `pico8_jp`'s own `ctx` parameter holding a pointer to the
 * heap `ji` list being built (this file's own private
 * `pico8_jo_gh_jp_step`), not a pure predicate. `i.gc` (a want-record's
 * own "how many trades already fulfill this want" count) is set only by
 * `dh()` (Chunk 12's already-built `pico8_dh`, pico8_economy_tiers.c) --
 * absent here reads as 0 (this port's pervasive convention), which is
 * also real Lua's own correct reading for a want that's never gone
 * through `dh()` yet.
 *
 * `if jx[1]` (jx non-empty, i.e. some want is still fully unfulfilled):
 * adds a `{lq=la({"needs:"},jx)}` row. Else, if `self.mi<4 and
 * #self.jy>0 and self.kw.qe~=r.w` (a young-enough, actually-wanting,
 * non-`r.w`-exporting planet), adds a `{lq=la({"wants more:"},X)}` row,
 * `X` being `{self.jy[#self.jy]}` (a fresh one-item list holding just the
 * LAST want) when `self.qh` is truthy, else the WHOLE `self.jy` list --
 * `self.qh` itself is a plain per-instance data field some planet
 * subtypes' own class specs set (Chunk 15, e.g. `qh=jo:jr([[...,qh=1,...
 * ]])`), absent/falsy on a base `jo`.
 *
 * `add(bs,ji[2]and{lq=ji})`: `ji[2]` is Lua `nil` (not `false`) when the
 * predicate above never appended anything -- `nil and {...}` is `nil`,
 * and `add(t,nil)` is PICO-8's own documented no-op (pico8_collect.h's
 * own pico8_nv comment) -- ported as "only append this row when
 * `ji->count>=2`", not as appending a literal `false` list element.
 *
 * `if self.kw.mu>0`: `gu(jo.cq,self.kw.qe)` -- `jo.cq` (the CLASS-level
 * `o(12,36,24,25)` field) walked via this file's own private
 * `pico8_jo_positional_values` (the same "positional-entries-of-an-
 * ob_table -> pico8_list_t" bridge already established independently by
 * `pico8_research_display_positional_values`/`pico8_layout_positional_
 * values`/`pico8_msgbox_positional_values`, a documented, accepted
 * duplication, not a new pattern) -- picks `{"makes:",tostr(self.kw.mu),
 * self.kw}` when `self.kw.qe` is one of those four constants, else
 * `{"exports:",<concatenated "N/M" string>,self.kw}` (`#self.by` -- Chunk
 * 13's own `jo:ib()` result field, still deferred, absent reads as 0, per
 * `jo:hb`'s own identical precedent -- concatenated with `self.kw.mu` via
 * `tostr()`, this port's established malloc+memcpy/strcat convention,
 * matching `pico8_ne`'s/`pico8_ho_apply`'s own style, see
 * `pico8_planet_display.c`). `self.kw` itself (the whole table) is the
 * row's own THIRD list item, a shared REF to the SAME table `self.kw`
 * already points to, not a copy.
 *
 * `for i=2,#bs do add(bs[i].lq,{false,1}) end`: every row from the SECOND
 * one onward (never `bs[1]`, whose own `lq` is a single icon value, never
 * a list) gets one more trailing `{false,1}` tuple appended to its own
 * `lq` list -- every `lq` value built above by this function (the
 * "needs:"/"wants more:"/`ji`/"makes:"/"exports:" rows) is represented as
 * a heap `pico8_list_t*` REF specifically so this in-place `add()` is
 * possible, matching how `jp`/`la`/`nv` themselves already return
 * `pico8_list_t` values throughout this port.
 *
 * Every intermediate SCRATCH `pico8_list_t` whose own contents get copied
 * elsewhere (the literal `{"needs:"}"/`{"wants more:"}` prefix lists fed
 * into `la()`, and `jx` itself once merged) is released with
 * `pico8_list_free()` right after use, mirroring `pico8_pz`'s own
 * identical `prefix_item`/`merged` cleanup (pico8_msgbox.c) -- but no
 * `pico8_ob_table_t`/`pico8_list_t` that becomes part of the final `bs`
 * structure handed to `pz()` is ever freed here, matching this port's
 * pervasive "the surviving structure leaks, like Lua's own GC-deferred
 * collection" convention. A NULL `self` is a safe no-op (nothing to
 * build a panel for). Read-only on `self` -- no field is written. */
void pico8_jo_gh(const pico8_instance_t *self);

/* function jo:gv(s)if self.me=="ly" then
 * if s.qu==self then
 * self:gh()else
 * return true
 * end
 * end
 * end
 * (pico8/lua.lua:472-478.) "Can an incoming slipway drag `s` land on this
 * planet?" -- real call site: `ui:jz("gv",held)` (pico8_ui.h's own
 * `pico8_ui_jz`, Chunk 5, already built and already `mv()`-dispatching by
 * this exact name). Only colonized planets (`self.me=="ly"`, compared by
 * STRING CONTENT like `jo:gg`) participate at all -- an uncolonized
 * planet falls off the end, returning Lua nil (`pico8_value_ref(NULL)`
 * here). For a colonized planet: if `s.qu` (a slipway's own "already
 * connected from" planet field, Chunk 14's `jc`, not yet built -- read
 * generically via `pico8_instance_get_field`, no `jc` class dependency)
 * IS this same instance (compared by POINTER IDENTITY, matching real
 * Lua's own table-reference `==`), the drag is landing back on its own
 * origin -- shows this planet's own info panel instead (`self:gh()`, this
 * sub-lot's own `pico8_jo_gh` above) and returns nil (no `return`
 * statement follows the `self:gh()` call); otherwise returns `true` (a
 * genuinely different planet -- a valid drop target).
 *
 * Not one of `bm`'s 5 real dispatch names, but still a real `mv()`-routed
 * field (see `pico8_planet.h`'s own top "SUB-LOT 2" comment) -- wired as a
 * `PICO8_VALUE_FUNCTION` bridge (`pico8_jo_gv_dispatch`, pico8_planet.c)
 * threading `mv()`'s own `arg0` through as `s`, the same "field function,
 * not a `cv` auto-registration" treatment `gg`/`hb` already established.
 * Also directly callable as this plain C function. A NULL `self` is a
 * safe no-op (returns Lua nil); a NULL `s` is read the same defensive way
 * every other "self-shaped argument" is elsewhere in this port (`s.qu`
 * absent -> never equal to `self`, so the `else` branch's `return true`
 * is taken, matching what real Lua would do for a `nil.qu` READ... except
 * real Lua would actually ERROR on `nil.qu`; no real call site in scope
 * ever passes a genuinely nil `s` to `gv`, so this divergence is purely
 * defensive, never actually exercised, matching this port's pervasive
 * "wrong-kind/absent argument reads as absent rather than crashing"
 * convention). */
pico8_value_t pico8_jo_gv(const pico8_instance_t *self, const pico8_instance_t *s);

/* function jo:fe()pz(self,nv(bj(self),function(b)local od=b:od()
 * return {lq=mv(b,"lq"),h_=od>0 and {od.."$",c=13},my=function()if fm:lu(od)then
 * self:hu(b)end
 * end,jb=b:jb()}
 * end
 * ))end
 * (pico8/lua.lua:418-423.) Opens the research panel for this planet:
 * builds one info-panel row per available research entry (`bj(self)`,
 * Chunk 11's already-built `pico8_bj`, a list of `hp` instances) and hands
 * the whole list to `pz(self,bs)` (Chunk 8's already-built `pico8_pz`) to
 * render, exactly like `jo:gh()`'s own `pz(self,bs)` call above -- a
 * DIFFERENT list shape, built here instead of there.
 *
 * The real Lua source calls `self:hu` (INSIDE the closure's own body, see
 * pico8_jo_fe_my_dispatch's own comment in pico8_planet_display.c for the
 * full trace) -- `jo:hu` (`pico8_planet.h`'s own `pico8_jo_hu`, sub-lot 2)
 * is already built, so unlike `ldown`'s own `jc` forward reference (see
 * `pico8_planet.h`'s own comment on `pico8_jo_ldown`), NOTHING in `fe`
 * itself is a forward reference to a not-yet-built chunk; the mapper's own
 * per-row construction (`lq`/`h_`/`jb`) and the "my" closure's own
 * representation are this sub-lot's real, novel work -- see
 * pico8_jo_fe_step's and pico8_jo_fe_my_dispatch's own comments
 * (pico8_planet_display.c) for the full per-field trace, including the
 * real gap this sub-lot found and fixed in already-shipped Chunk 11 code
 * (`hp`'s class never wired "lq" as an `mv()`-dispatchable field, needed by
 * this function's own real `mv(b,"lq")` call -- see pico8_research.h's own
 * "CORRECTION" note) and the new closure-representation technique the
 * `my` field's own genuinely-captured-and-later-invoked closure required
 * (C has no first-class closures; see pico8_jo_fe_my_dispatch's own
 * comment for why the established `ctx`-parameter "closure-factory"
 * convention, which only covers SYNCHRONOUS map/filter calls, doesn't by
 * itself cover a closure invoked an arbitrary time later through a
 * generic `mv()`-dispatched field).
 *
 * Not a `bm` name and never `mv()`/`dc()`-routed (the one real call site,
 * `jo:ldown`'s own `self:fe()`, `pico8_planet.c`, is a plain same-class
 * call) -- a known, directly-callable C function, matching `jo:kd`'s own
 * "no dispatch bridge needed" precedent. Lands in THIS module rather than
 * `pico8_planet.c` per this file's own top "STEP 2 REBALANCE" comment
 * (a module-size decision, not a Lua-semantics one -- `fe` has no
 * cross-class dependency that requires it to live here specifically). A
 * NULL `self` is a safe no-op. */
void pico8_jo_fe(pico8_instance_t *self);

/* function jo:kj(p)
 * if (not gd(self))return
 * p=ma(p,self)if self.me=="ll" and rnd(30)<self.t-30 then
 * return
 * end
 * circfill(p.x,p.y,7,0)dv(self.qw)palt(3,false)palt(0,true)spr(self.nk and self.nk.s or self.s or 43,p.x-4,p.y-4,2,2)dv()if self:gg()then
 * circ(p.x,p.y,6,jo.ka[self.mi])spr(self.mi+1,p.x-3,p.y-6)end
 * mv(self.lq,"ol",p+v(0,7),0.5)if self.fo then
 * fillp(0b1010101010101010.1)circfill(p.x,p.y,sqrt(self.fo.dw)*128,2)fillp()self.fo=nil
 * end
 * end
 * (pico8/lua.lua:541-550.) `jo`'s own real "kj" -- the planet's own draw
 * method, one of `bm`'s 5 real dispatch names (`bm=["kj","ev","hd","hk",
 * "hb"]`, pico8_entity.c). Per this chunk's own PRD text (Testing
 * Decisions): the moment this is wired as a real field on a real,
 * `dc()`-registered `jo` instance, `dc("kj")` finally reaches something
 * real and non-dead for the first time in this whole port -- see this
 * file's own top "SUB-LOT 5" comment for the full significance trace, and
 * `pico8_jo_kj_dispatch`'s own comment (`pico8_planet.c`) for the wiring
 * itself.
 *
 * `gd(self)`/`ma(p,self)`: Chunk 1's already-built `pico8_gd`/`pico8_ma`
 * (pico8_entity_geom.h) -- `not gd(self)` early-returns for an
 * off-screen planet (no draw work at all, matching every real call site's
 * own expectation that `kj` never runs for something the camera can't
 * see); `p=ma(p,self)` re-derives the actual screen position AND decays
 * `self.ma` (the particle-halo strength field, `pico8_ma`'s own documented
 * side effect) -- `p` from here on is this ADJUSTED vector, used by every
 * remaining line of the function, never the raw `self.ra` `dc("kj")`
 * itself passed in.
 *
 * `self.me=="ll"`: a STRING-CONTENT comparison (`strcmp`, matching
 * `gg`/`gv`/`ldown`'s own established treatment) against a value `self.me`
 * IS assigned "ll" for real, once, in this cart -- NOT a dead branch (a
 * prior version of this comment claimed it was; that claim was wrong).
 * `jo:hh()` (`pico8_jo_hh`, `pico8_planet.c`, pico8/lua.lua:407-411) does
 * `self.nk=jo.ir()<0.45 and df.ll or jo.oc()` then `self:ks(self.nk.me or
 * "known")`; `df.ll` (pico8/lua.lua:1378, `o("ll",45,0,me="ll")`) has
 * `me="ll"`, and `ke:ks(me)` (pico8/lua.lua:185, the base-class method
 * `jo` inherits) does `self.me,self.t=me,0` -- so roughly 45% of real
 * `jo:hh()` calls DO set `self.me="ll"` for real, a genuinely reachable
 * "lifeless planet" state. It IS still unexercised by any already-ported
 * code path today: `jo.ir`/`jo.oc` (`pico8_planet.c`'s own
 * `g_jo_ir`/`g_jo_oc`) are only ever wired to a real generator by Chunk 20
 * (not yet built), so `pico8_jo_hh`'s own `g_jo_ir != NULL` guard is
 * always false in every test/call path this port currently exercises --
 * `self.me` can genuinely be "ll" (in addition to "unknown"/"known"/"ly",
 * also assigned by `pico8_jo_hh`/`pico8_jo_hu`) once Chunk 20 lands.
 * Ported faithfully regardless, the same "forward references/not-yet-
 * exercised branches are ported, not stripped" treatment this port
 * already gives `jc:ey`'s `stat(19)==50` and `jo:pq`'s own `self.hp`
 * branch (sub-lot 1). `self.t` (this branch's own other read) is a plain,
 * generic per-instance field, read defensively as 0 when absent, this
 * file's own established `field_fixed` helper.
 *
 * `circfill(p.x,p.y,7,0)`: a solid black backing circle, drawn before the
 * sprite so any surrounding halo pixels never bleed through -- `p.x`/`p.y`
 * converted to plain C ints the same `(int)pico8_fixed_to_double(...)`
 * way every other pixel-coordinate conversion in this port already is
 * (`pico8_bg_to_int`/`pico8_ui_to_int`'s own established idiom -- no new
 * named helper introduced here, matching this file's own existing inline
 * style for its handful of int conversions).
 *
 * `dv(self.qw)`: `self.qw` is never set on `jo` itself, but a prior
 * version of this comment's "never set anywhere in this whole cart for
 * ANY jo-family instance" claim was wrong -- Chunk 15's `nl` (a real
 * `jo`-family subclass, `jo->qh->nl`, not yet ported) sets it for real,
 * both as a class-level default (`nl=qh:jr([[...od=20,qw=15,...]])`,
 * pico8/lua.lua:1057-1066) and again as a per-instance override
 * (`nl:ly(t)`'s own `self.qw=8+t%42/6` when `self.mi>=2`). (The only OTHER
 * `qw` occurrences in scope belong to `lq:mn`'s own unrelated local
 * variable and `mz`'s own class, Chunk 16, a wholly different entity.)
 * Until Chunk 15 lands `nl` for real, no real `jo`-family instance this
 * port can actually construct today ever has `qw` set, so `self.qw` still
 * reads as absent for every real call path currently in scope -- a real,
 * currently-correct behavior, but a temporary one, not a permanent
 * property of the cart; Chunk 15's own implementer should NOT read this
 * comment (in its old form) as license to skip wiring `qw` for `nl`. A
 * real Lua `dv(nil)` would error inside `flr()`; this port's own
 * pervasive, already-established "absent numeric field reads as 0"
 * convention (`field_fixed`, this file; `pico8_jo_gh`'s own identical
 * treatment of `self.kr`/`self.mi`) is applied here too -- this call
 * behaves exactly like `pico8_dv_default(mem)`, a documented, non-crashing
 * divergence from what real Lua would do if `self.qw` were ever genuinely
 * absent, exactly the "wrong-kind/absent argument reads as absent rather
 * than crashing" convention `pico8_jo_gv`'s own comment already states
 * verbatim.
 *
 * `palt(3,false)palt(0,true)`/trailing `dv()`: two plain, already-built
 * Phase 1 primitives bracketing the sprite blit (make color 3 opaque,
 * color 0 transparent, for this one sprite only) then reset the draw
 * palette back to default -- no new representation needed.
 *
 * `self.nk and self.nk.s or self.s or 43`: real Lua's `A and B or C or D`
 * idiom, reproduced at full rigor (not shortcut to "if nk present, always
 * use nk.s"): `self.nk.s` is only even consulted when `self.nk` ITSELF is
 * truthy (`pico8_jo_truthy`, this file), and only used when it too is
 * present and PICO8_VALUE_FIXED-kind (any real, evidenced `.s` value is a
 * plain sprite-id number, so a FIXED 0 -- truthy in real Lua, unlike C --
 * still counts, matching `pico8_jo_truthy`'s own semantics); otherwise
 * (nk falsy, OR nk truthy but `.s` itself falsy/absent/wrong-kind) falls
 * through to `self.s` under the exact same FIXED-kind rule, else the
 * literal `43`. `self.nk` itself is a plain `pico8_ob_table_t*` (one of
 * `pico8_df()`'s own entries, or `jo.oc()`'s/`jo.ir()`'s own generator
 * output -- `pico8_jo_hh`'s own established shape, NOT a `kv:jr` instance),
 * so its own "s" field is read via `pico8_ob_table_get_name`, not
 * `pico8_instance_get_field`.
 *
 * `if self:gg()then...end`: `:` method-call sugar (NOT the game's own
 * `mv()` global function, unlike the `mv(self.lq,"ol",...)` line below) --
 * ported as a direct call to the already-built, directly-callable
 * `pico8_jo_gg` (`pico8_planet.c`, sub-lot 1's own "wired as a field AND
 * directly callable" precedent), matching `pico8_jo_gv`'s own identical
 * `self:gh()` treatment. `jo.ka[self.mi]`: a CLASS-level nested-table read
 * (`jo.ka=o(2,3,9,12)`, `pico8_jo_init`'s own class spec string, already
 * wired since sub-lot 1) -- read from `pico8_jo_class()->defaults` (the
 * PUBLIC accessor, not the private `g_jo_class` this file has no access
 * to), the exact same "class-level-not-instance-level read" pattern
 * `pico8_jo_gh`'s own `jo.dz[self.mi]`/`jo.cq` reads already established
 * in this same file (and, further back, Chunk 7's `bg.pl`). An absent or
 * out-of-range `self.mi` index (real Lua's own safe nil-key table read)
 * yields Lua nil here too -- `color` stays 0 (this port's default), never
 * a crash. The second `spr(self.mi+1,...)` call omits `w`/`h`, so it maps
 * to `pico8_spr_default`, not `pico8_spr`.
 *
 * `mv(self.lq,"ol",p+v(0,7),0.5)`: the game's own `mv()` GLOBAL FUNCTION
 * (not `:` sugar this time), so ported through the real, generic
 * `pico8_mv()` mechanism, exactly as this port's own established
 * convention requires (see `pico8_jo_fe`'s own `mv(b,"lq")` treatment,
 * this same file). `self.lq` is an `lq` instance (built by `jo:hb`, sub-
 * lot 1, from `pico8_jo_ex`) -- but `lq`'s class NEVER wired "ol" as an
 * `mv()`-dispatchable field before this sub-lot (its only two previously-
 * known real call sites, `pico8_ui_button.c`'s own `self.jb:ol(...)`/
 * `c:ol(...)`, are both plain `:` sugar, ported as direct calls) -- a real,
 * previously-invisible gap, the exact same shape as `pico8_research.h`'s
 * own already-logged `hp:lq` correction, found and fixed here (see
 * `pico8_layout.h`'s own "CORRECTION" note on `pico8_lq_ol` for the full
 * trace, and `pico8_lq_ol_dispatch`'s own comment, `pico8_layout.c`, for
 * the bridge itself). `p+v(0,7)` is a fresh stack `pico8_vec_t`, passed by
 * address as this call's own `arg0` (safe: `pico8_mv()` is fully
 * synchronous here, unlike `jo:fe`'s own `"my"` closure, which genuinely
 * has to survive past its own building call).
 *
 * `if self.fo then...end`: `self.fo` is a genuine OTHER entity instance
 * (a probe, `g_`'s own class, Chunk 10 -- set by `bv`'s own `self.et.fo=
 * self`, pico8/lua.lua:562), read generically via `pico8_instance_get_field`
 * (not `pico8_ob_table_get_name`, unlike `self.nk` above -- `self.fo` is a
 * real class instance, not a plain data table), truthy-checked the same
 * `pico8_jo_truthy` way as everywhere else in this file. `self.fo.dw` is
 * read the same generic way off that instance (absent reads as 0, this
 * port's pervasive default). `sqrt(self.fo.dw)*128`: `pico8_fixed_sqrt`
 * then `pico8_fixed_mul` by 128, converted to a plain int radius the same
 * inline way as every other pixel coordinate in this function.
 * `fillp(0b1010101010101010.1)`: the binary LITERAL's own integer part is
 * `1010101010101010`=`0xAAAA`; its trailing `.1` (PICO-8's own fixed-point
 * fractional flag, which toggles the pattern's "1" bits from "draw with a
 * second colour" to "leave this pixel untouched" instead) is carried by
 * `pico8_fillp_transparent(gfx,0xAAAAu)` (`pico8_gfx.h`).
 * CORRECTION (Phase 5 bug fix, "hovering a shop button aborts the
 * framebuffer dumper"): until that fix this call passed plain
 * `pico8_fillp(gfx,0xAAAAu)` with the fractional flag simply DROPPED,
 * because the port's fillp model was "a set bit means skip this pixel"
 * unconditionally -- i.e. it modelled only the `.1` case, and had no
 * notion of the second colour at all. That made this one call site
 * accidentally right and every `.1`-less one (`jb:ol`'s own
 * `fillp(0b0000111100001111)rk(om,dn,0x10)`, pico8/lua.lua:319, and
 * `bg:kj`'s own `0b1010010110100101`, :373) wrong; see
 * `pico8_gfx.c`'s own `pico8_fillp_color`. `self.fo=nil`
 * (Lua's own field-clearing assignment) is `pico8_ob_table_set_name(self->
 * fields,"fo",pico8_value_ref(NULL))`, matching this port's established
 * "write nil as REF(NULL)" convention throughout.
 *
 * A NULL `self` is a safe no-op (nothing to draw). `self` is non-const
 * (unlike `pico8_jo_gh`'s own read-only treatment): `pico8_ma`'s own
 * documented side effect (decaying `self.ma`) and this function's own
 * final `self.fo=nil` write both mutate the instance. */
void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p);

#endif
