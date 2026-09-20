#ifndef PICO8_PLANET_TRADE_H
#define PICO8_PLANET_TRADE_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_list.h"
#include "pico8_value.h"

/* Phase 3, Chunk 13 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md), FOURTH sub-lot: the `jo`-side of the mutually-recursive trade-
 * propagation cluster this PRD's own "Implementation Decisions" section
 * explicitly refuses to force into a linear order ("The mutually-recursive
 * trade cluster is not force-ordered" -- `jo:cn`, `nr:cn`, `jo:kf`,
 * `mx:kf`, `jo:ib`, `jo:kd`, `trade:kg`, `jc:eb` genuinely call into each
 * other depending on the runtime class of the entity at the OTHER end of a
 * trade route; each is ported in its own class's wave, relying on the
 * ALREADY-BUILT dynamic-dispatch mechanism -- pico8_mv, pico8_entity.h --
 * to resolve the actual call at runtime). This sub-lot ports `jo:kf`,
 * `jo:cn`, `jo:ib` (`jo:kd` already shipped, sub-lot 1). `nr:cn`/`mx:kf`
 * (Chunk 15) and `jc:eb` (Chunk 14) are NOT this sub-lot's job -- every
 * dispatch call this file makes toward them (`sw:eb(self)`, and the
 * dynamically-resolved `:cn`/`:kf` on a runtime-typed counterparty)
 * compiles against `pico8_mv` and resolves to "field not found" today (no
 * `jc`/`nr`/`mx` class registers any of those fields yet), the SAME
 * "compiles, not yet exercisable" treatment every other forward reference
 * in this port already gets (`bg:ldown`'s `mz`/`g_`, `jo:hu`'s
 * `kv[hp.cw]`, `jo:ldown`'s `jc({...})`) -- no fake linear order invented,
 * no subclass special-cased, per this task's own explicit instruction.
 *
 * New module (`pico8_planet_trade.c`/`.h`, not `pico8_planet.c`/
 * `pico8_planet_display.c`): the trade cluster is thematically distinct
 * (trade-route propagation logic, not lifecycle/display) from both
 * existing `jo` modules, and this is exactly the module `pico8_planet.h`'s
 * own sub-lot-1/2 top comments already anticipated ("pico8_planet_trade.c
 * for cn/ib/kf") and `pico8_planet_display.h`'s own sub-lot-3 top comment
 * explicitly left uncreated ("nothing to move there today") -- this sub-
 * lot is the first with real content for it. `jo:kf`/`jo:cn`/`jo:ib`
 * themselves are declared below; their `mv()`-dispatch bridges
 * (`pico8_jo_kf_dispatch`/`pico8_jo_cn_dispatch`/`pico8_jo_ib_dispatch`)
 * stay in `pico8_planet.c` alongside `pico8_jo_init()`'s own class-
 * registration wiring, the SAME "bridge lives where the registration code
 * lives, calls across the file boundary via this header" precedent
 * `pico8_jo_hb_dispatch`/`pico8_jo_gv_dispatch` already established
 * (sub-lot 3) for `pico8_planet_display.h`.
 *
 * *** `jo:ib`'s local closure-shaped `lw` table, and why it needs a real
 * C type here *** -- `jo:ib()` builds, once per connected slipway, a
 * plain (non-`kv:jr`) Lua table `lw={t=trade(...),mu=...,kf=function(self,
 * a)...end}` and hands it, BY REFERENCE, into a dynamically-dispatched
 * `:cn(lw)` call on the OTHER end of that slipway -- which may be `jo:cn`
 * (this file) or `nr:cn` (Chunk 15, which itself builds a shallow copy
 * `gf=mw(lw)` of the SAME shape and recurses). `jo:cn`, in turn, calls
 * `lw:kf(t:kg(self))` -- a plain Lua table-field call on `lw.kf`, the
 * closure literal above, NOT a class-dispatched method call, and
 * genuinely DISTINCT from the `jo:kf` METHOD of the identical short name
 * (`pico8/lua.lua:465-467` vs. `:500-514` -- `pico8_planet.h`'s own top
 * comment already flagged this exact confusion risk for this sub-lot to
 * avoid). Since C has no first-class closures, `lw`'s three real fields
 * (`t`, `mu`, and `kf` -- a closure over the OUTER `ib` list, shared by
 * every `lw` this same `jo:ib()` call builds) are represented here as a
 * concrete, named struct with a plain function-pointer field, NOT the
 * generic `PICO8_VALUE_FUNCTION`/`pico8_value_function_t`-on-an-ob_table
 * technique `jo:fe`'s own "my" closure needed (`pico8_planet_display.c`'s
 * `pico8_jo_fe_my_dispatch`) -- that technique exists specifically because
 * `fe`'s closure had to survive being copied wholesale by
 * `pico8_class_instantiate` (`js(b)`/`pz()`) and be reachable LATER through
 * the fully generic, single-function-pointer `mv()` mechanism. `lw` is
 * never `pico8_class_instantiate`'d (it's a raw local table, not a `kv:jr`
 * instance) and is only ever read back SYNCHRONOUSLY, within the same call
 * chain that built it (`jo:ib` -> `:eb`+`:cn` -> `lw:kf(...)`, and Chunk
 * 15's own `nr:cn` -> `mw(lw)` -> recursive `:cn` -> `gf:kf(...)`) -- a
 * plain named struct + function pointer, exactly this port's own
 * established "closure with upvalues resolved via a context parameter"
 * convention (e.g. `pico8_jo_lz_mw_step`, `pico8_jo_fe_ctx_t`), just with
 * the struct itself doubling as both the "self" the closure is called
 * with AND the context, since that's exactly what `lw:kf(a)`'s own real
 * Lua desugaring (`lw.kf(lw,a)`) already does. `pico8_jo_ib`'s own real
 * closure body (`pico8_jo_ib_kf_closure`, `pico8_planet_trade.c`, static/
 * private -- no other real Lua source ever builds an `lw`-shaped table, so
 * nothing else needs to name it) is the only thing ever assigned to this
 * field; a future `nr:cn`/`mx:kf` (Chunk 15) reads/copies/calls this same
 * struct and function pointer, never redefines its shape. */

typedef struct pico8_jo_trade_proposal pico8_jo_trade_proposal_t;

/* `function(self,a)if not gu(ib,a)then add(ib,a)self.mu-=1 end end`
 * (pico8/lua.lua:505-509, `jo:ib`'s own inline closure literal) -- see
 * this file's own top comment for the full "why a struct, not the generic
 * mv()-dispatch technique" trace. `self` here is the closure's OWN first
 * parameter (real Lua's own implicit `self` from `lw:kf(a)`'s colon-call
 * sugar), i.e. THIS SAME `pico8_jo_trade_proposal_t`, not the enclosing
 * planet -- matches the real source's own local variable shadowing
 * exactly (the outer `jo:ib`'s own `self` -- the planet -- is never read
 * inside this closure at all; only the captured `ib` upvalue is). */
typedef void (*pico8_jo_trade_proposal_kf_fn)(pico8_jo_trade_proposal_t *self, pico8_value_t a);

/* `local lw={t=trade({...}),mu=mu-#ib,kf=function(self,a)...end}`
 * (pico8/lua.lua:504-510). Three real fields, one-to-one:
 *   - `t`: the trade instance this proposal is currently extending
 *     (`lw.t`, read by `jo:cn`'s own `local t=lw.t`).
 *   - `mu`: the remaining quantity this proposal can still absorb before
 *     it's spent (`lw.mu`, read/compared/decremented by `jo:ib`'s own
 *     `if (lw.mu==0)break`, and by the closure below via `self.mu-=1`).
 *   - `kf`: the closure above, over the SAME `ib` list every `lw` this
 *     `jo:ib()` call builds shares (a real Lua upvalue capture -- stored
 *     here as an explicit `ib` field on the struct itself, rather than
 *     invented context-threading, since every consumer of this type
 *     (`jo:cn` here, `nr:cn`/`mx:kf`'s own future shallow copies, Chunk
 *     15) needs to call `lw:kf(a)` with no separate context parameter of
 *     its own to thread it through -- exactly matching how the struct
 *     itself doubles as the closure's own "self" argument, per this
 *     file's own top comment). */
struct pico8_jo_trade_proposal {
    pico8_instance_t *t;
    pico8_fixed_t mu;
    pico8_jo_trade_proposal_kf_fn kf;
    pico8_list_t *ib;
};

/* function jo:kf(sw)add(self.oe,sw)
 * if (sw:eb(self).fx)self.fx=true
 * self:kd()end
 * (pico8/lua.lua:465-467.) Registers a newly-established slipway `sw`
 * (a `jc` instance, Chunk 14 -- not yet built) onto this planet's own
 * connection list (`self.oe`, a REF `pico8_list_t*`, always present --
 * `jo:pq()`, sub-lot 1, always seeds it as a fresh empty list). Real call
 * sites (`jc:ey`'s own `self.qu:kf(self)re:kf(self)`, Chunk 14) call this
 * on a receiver whose CONCRETE class isn't statically known (any
 * `jo`/subclass) -- genuinely `mv()`-dispatched, matching this file's own
 * top comment on why "kf" (the outer registration method, DISTINCT from
 * `jo:ib`'s own local closure FIELD of the identical short name) is part
 * of this cluster: `mx:kf` (Chunk 15) overrides it, so resolving the
 * actual method to run needs the receiver's real runtime class.
 *
 * `sw:eb(self).fx`: `sw:eb(self)` is ALSO a genuine `mv()` dispatch --
 * `eb` is only ever defined by `jc:eb(jo)` (Chunk 14, not yet built:
 * `return jo==self.qu and self.re or self.qu`, i.e. "the planet at the
 * OTHER end of this slipway from `jo`") -- ported against `pico8_mv`
 * exactly like every other forward-referenced cross-class dispatch in
 * this cluster; `pico8_mv` returns 0 (field genuinely absent) today, so
 * the whole `if` is skipped, matching real Lua's own short-circuit if
 * `sw:eb` itself were nil (never evidenced as a real case -- no real
 * call site in scope ever passes a `sw` with no `eb` method once `jc`
 * exists). When it DOES resolve (Chunk 14+), reads the counterparty
 * planet's own `.fx` field (truthy, this file's own `pico8_jo_truthy`
 * copy) and, if set, marks `self.fx=true` too (a "this planet touches an
 * autoassembled/`fx`-flagged network" propagation, matching `jo:hu`'s own
 * `self.fx=true` write on the colonization branch).
 *
 * `self:kd()`: a plain, known same-class call (`pico8_jo_kd`,
 * `pico8_planet.h`, sub-lot 1 -- every real `:kd()` call site in scope,
 * including this one, is same-class, never itself `mv()`/`dc()`-routed).
 *
 * Wired as a real `PICO8_VALUE_FUNCTION` field ("kf",
 * `pico8_jo_kf_dispatch`, `pico8_planet.c`) so `jc:ey`'s own polymorphic
 * `qu:kf(self)`/`re:kf(self)` calls resolve to this (or `mx:kf`'s own
 * override) at runtime -- AND directly callable as this plain C function,
 * since `mx:kf` (Chunk 15) calls the BASE implementation directly via
 * `jo.kf(self,sw)` (a real Lua "explicit super-call", `pico8/lua.lua:
 * 1056`), bypassing dispatch on purpose. A NULL `self` is a safe no-op. */
void pico8_jo_kf(pico8_instance_t *self, pico8_instance_t *sw);

/* function jo:cn(lw)local t=lw.t
 * if t.qe==r.e
 * and not self.jh then
 * for i in all(self.jy)do
 * if i.gc==0 and i.qe~=r["?"]then
 * del(self.jy,i)break
 * end
 * end
 * self.jy,self.jh=
 * la({{qe=r.e}},self.jy),1
 * end
 * for i in all(self.jy)do
 * if i.qe==r["?"]and gu(self.ba,t.qe)then
 * i.qe=t.qe
 * end
 * if t.qe==i.qe and i.qe~=r["?"]then
 * lw:kf(t:kg(self))
 * return
 * end
 * end
 * end
 * (pico8/lua.lua:479-499.) "Does this planet's own want-list have room for
 * the resource `lw.t.qe` is proposing?" -- the OTHER real half of the
 * cluster: called on a runtime-typed receiver (`w:eb(self):cn(lw)`,
 * `jo:ib` below, and `nr:cn`'s own recursive `w:eb(self):cn(gf)`, Chunk
 * 15) via `pico8_mv`, resolving to this base implementation or `nr:cn`'s
 * override depending on the counterparty's real class.
 *
 * `t.qe==r.e and not self.jh`: `r` (Chunk 1's already-built `pico8_r()`,
 * `pico8_stash.h`) is the resource-catalog table decoded from
 * `unstash()`; `r.e`/`r["?"]` are read by NAME each call (`pico8_
 * ob_table_get_name`, matching `pico8_planet_display.c`'s own identical
 * `r.w` lookup precedent) rather than cached, since this module owns no
 * persistent state of its own. Compared via `pico8_value_equal` (this
 * port's own established "compare against an `r`-table entry" convention,
 * same file/precedent) -- `r.e`/`r["?"]`/`self.kw.qe`-shaped values are
 * plain fixed numbers (confirmed by their real spec-string siblings, e.g.
 * `k_=o(28,20,21,22)`), so exact-value comparison is correct, not a
 * pointer-identity gap. `not self.jh`: `self.jh` is a genuinely per-
 * instance field (no `jo` class-spec default sets it -- `pico8_planet.c`'s
 * own class-spec trace confirms this), read via this file's own
 * `pico8_jo_truthy` copy (documented duplication, same rationale as every
 * other module-private copy of this exact helper already in this port).
 * "First time this planet is ever offered energy (`r.e`), and it hasn't
 * already special-cased that": removes at most ONE currently-unfulfilled
 * (`i.gc==0`, absent-`gc` reading as 0 per this port's own established
 * convention -- see `jo:gh`'s own identical `i.gc` precedent,
 * `pico8_planet_display.h`), non-wildcard (`i.qe~=r["?"]`) want
 * (`del(self.jy,i)`, `pico8_list_del`, Phase 2) to make room, THEN
 * prepends a fresh `{qe=r.e}` want ahead of every existing one
 * (`la({{qe=r.e}},self.jy)`, `pico8_la`, Chunk 1) and sets `self.jh=1` (a
 * genuine Lua integer literal, `pico8_value_fixed`, NOT the boolean
 * `true`) so this injection only ever happens once per planet.
 *
 * Second loop (always runs, whether or not the block above did): for each
 * want `i` still on `self.jy` -- if it's still a wildcard (`i.qe==r["?"]`)
 * AND this planet's own accepted-resource list (`self.ba`, a CLASS-level
 * `o(...)`-built positional table on the subclasses that define one, e.g.
 * `nl.ba=o(28,48,18,20,21,22)` -- absent on a bare `jo`, read defensively
 * as "accepts nothing" via this file's own private positional-values
 * bridge, the same documented-duplicate pattern already independently
 * established by `pico8_jo_positional_values`/`pico8_research_display_
 * positional_values`/etc.) actually includes the incoming `t.qe`, the
 * wildcard is filled in (`i.qe=t.qe`, a real in-place mutation of the
 * shared want-record table, matching real Lua's own by-reference table
 * semantics). Then, if THIS want (now possibly just-filled) genuinely
 * matches the incoming resource (`t.qe==i.qe and i.qe~=r["?"]`) --
 * extends the trade chain by this planet (`t:kg(self)`, Chunk 3's already-
 * built `pico8_trade_kg`, called DIRECTLY, not `mv()`-dispatched: `trade`
 * has no subclass anywhere in the real source, so unlike `jo`'s own
 * cluster methods, no receiver-class ambiguity ever needs resolving here
 * -- matching `pico8_trade.h`'s own design, which already built `kg` as a
 * plain function for exactly this reason) and hands the result to the
 * PROPOSAL's own closure (`lw:kf(...)`, a direct call through `lw->kf`,
 * NOT `mv()` -- see this file's own top comment on why `lw` is a plain
 * struct, never a dispatched entity) -- then returns immediately (a
 * genuine Lua `return`, stopping after the FIRST matching want, never
 * checking any want beyond it).
 *
 * `gu(self.ba,t.qe)`'s own list-membership test is `pico8_gu` (Chunk 1),
 * safe on a NULL/absent `self.ba` (returns false, matching real Lua's own
 * safe `all(nil)`) -- NOT the trade-instance-aware membership gap
 * `pico8_trade.h`'s own top comment flags (that gap is `jo:ib`'s own
 * `gu(ib,a)`, below, comparing `trade` instances specifically -- `self.ba`
 * holds plain resource numbers, never `trade` instances, so the ordinary
 * `pico8_gu` is the CORRECT tool here, not the gap).
 *
 * Not one of `bm`'s 5 real dispatch names, but a genuine `mv()`-routed
 * field for the reason above (dynamic receiver-class resolution) --
 * `pico8_jo_cn_dispatch` (`pico8_planet.c`) unwraps `mv()`'s own `arg0`
 * back into a `pico8_jo_trade_proposal_t*`. A NULL `self` or NULL `lw` is
 * a safe no-op. */
void pico8_jo_cn(pico8_instance_t *self, pico8_jo_trade_proposal_t *lw);

/* function jo:ib()local ib=self.by or {}
 * local qe,mu=
 * self.kw.qe,self.kw.mu
 * for w in all(self.oe)do
 * local lw={t=trade({lp={self},rl={w},qe=qe
 * }),mu=mu-#ib,kf=function(self,a)if not gu(ib,a)then
 * add(ib,a)
 * self.mu-=1
 * end
 * end
 * }
 * if (lw.mu==0)break
 * w:eb(self):cn(lw)end
 * return ib
 * end
 * (pico8/lua.lua:500-514.) Rebuilds this planet's own list of accepted
 * incoming trade ROUTES (`ib`, a list of freshly-extended `trade`
 * instances, one per slipway that ends up accepting a proposal -- exactly
 * the shape `pico8_economy_tiers.c`'s own already-built, already-forward-
 * referencing `bd_route_visit` expects: `pico8_mv(pt,"ib",0,...,&by_val)`
 * then `pt.fields["by"]=by_val`, Chunk 12, confirmed by reading that
 * file's own top comment -- this sub-lot is the exact call site that
 * comment anticipated). `jo:ib()` itself never writes `self.by` (only its
 * real caller does, `bd()`'s own `pt.by=pt:ib()`, `pico8/lua.lua:1269`,
 * Chunk 12 already built) -- matches the real source precisely.
 *
 * `local ib=self.by or {}`: reuses the EXISTING `self.by` list object (if
 * present and truthy -- this file's own `pico8_jo_truthy`-style REF/NULL
 * check) rather than copying it, so `add(ib,a)` below mutates it in
 * place; a genuinely absent `self.by` gets a BRAND NEW empty heap list
 * instead (the returned `ib` either way is what the caller reassigns onto
 * `self.by`, matching real Lua's own semantics regardless of which case
 * applies).
 *
 * `qe,mu=self.kw.qe,self.kw.mu`: read once, from `self.kw` (always a REF
 * `pico8_ob_table_t*` -- either the class-level `kw=o(mu=0)` default or a
 * per-instance table `jo:lz` built), unchanged for the rest of this call.
 *
 * `for w in all(self.oe)do...end`: `self.oe` (this planet's own connected-
 * slipway list, `jo:pq`/`jo:kf` above) -- for each slipway `w`, builds a
 * fresh trade PROPOSAL `lw` (this file's own `pico8_jo_trade_proposal_t`,
 * see its own comment above and this file's top comment for the full
 * closure-representation trace) directly via `pico8_trade_new` (a literal
 * `trade({...})` table-constructor call, NOT `:kg()` -- there is no prior
 * trade to extend yet, this IS the first hop) with one-element `lp`/`rl`
 * heap lists (`{self}`/`{w}`, matching `pico8_trade_kg`'s own established
 * "build a one-element list, hand its REF to `pico8_ob_table_set_name`,
 * never free the list itself (it's now owned by the new instance's own
 * fields)" convention) -- STACK-allocated (not heap), unlike `jo:fe`'s own
 * "my" closure context, since nothing needs `lw` to outlive this
 * synchronous call chain (`:eb`+`:cn`, and whatever further recursion
 * `nr:cn`/`mx:kf` do with it, Chunk 15, all complete before this loop
 * iteration ends -- matching real Lua's own local-variable scoping for
 * `lw` exactly).
 *
 * `mu-#ib`: recomputed EVERY iteration (not once, unlike `qe`/`mu`
 * themselves) since `#ib` can grow mid-loop (an earlier iteration's own
 * `lw:kf(a)` call may have `add(ib,a)`'d a new route already).
 *
 * `if (lw.mu==0)break`: stops building further proposals once this
 * planet's own remaining export quantity is exhausted -- the ALREADY-
 * CONSTRUCTED `lw.t` trade instance for this one iteration is simply never
 * used further (leaked, matching this port's pervasive "GC-deferred"
 * convention for a table real Lua would also just let become garbage).
 *
 * `w:eb(self):cn(lw)`: the cluster's own two chained forward-referenced
 * dispatches -- `w:eb(self)` (`pico8_mv(w,"eb",1,REF(self),...)`, Chunk
 * 14's `jc:eb`, not yet built) resolving the OTHER end of slipway `w`,
 * then `:cn(lw)` (`pico8_mv` again, THIS planet's own `jo:cn` above or a
 * future `nr:cn` override, Chunk 15) on whatever that resolves to. Either
 * `pico8_mv` call returning "not found" (today's real, evidenced case --
 * no `jc` class exists yet) simply skips the rest of that iteration's own
 * body, matching real Lua's own "nothing happens" if `w:eb` genuinely had
 * no such method (never a real case once Chunk 14 lands).
 *
 * `return ib`: hands back the (possibly just-built, possibly-reused,
 * possibly-grown) list for the real caller (`bd()`, Chunk 12) to assign
 * onto `self.by`.
 *
 * Not one of `bm`'s 5 real dispatch names, but the exact `mv()`-routed
 * field `pico8_economy_tiers.c`'s own `bd_route_visit` already calls by
 * name (`pico8_jo_ib_dispatch`, `pico8_planet.c`) -- wrapping this
 * function's own `pico8_list_t*` return in a `PICO8_VALUE_REF`, matching
 * that forward reference's own documented expectation exactly. Also
 * directly callable as this plain C function. A NULL `self` returns NULL
 * (nothing to build a route list for). */
pico8_list_t *pico8_jo_ib(pico8_instance_t *self);

#endif
