#ifndef PICO8_PLANET_FAMILY_TRADE_H
#define PICO8_PLANET_FAMILY_TRADE_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_planet_trade.h"
#include "pico8_value.h"

/* Phase 3, Chunk 15 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `nr`, `fr`, `mx` -- the two classes this whole port's own
 * mutually-recursive trade-propagation cluster (`jo:cn`/`nr:cn`/`jo:kf`/
 * `mx:kf`/`jo:ib`/`jo:kd`/`trade:kg`/`jc:eb`, first identified as a
 * cluster by this PRD's own "Implementation Decisions" section) has been
 * waiting on since Chunk 13 -- `nr:cn` (overriding `jo:cn`) and `mx:kf`
 * (overriding `jo:kf`) -- plus `nr`'s own three other new methods
 * (`gv`,`ldown`,`hb`) and `fr` (`nr`'s own direct subclass, no new
 * bodies), grouped into this sibling file rather than `pico8_planet_
 * family.c`/`.h` specifically so this chunk's trade-cluster-completion
 * story lives in one place, mirroring `pico8_planet_trade.c`/`.h`'s own
 * precedent of isolating `jo`'s own trade-cluster methods (Chunk 13) into
 * their own module. Once this file's `pico8_nr_cn`/`pico8_mx_kf` land,
 * ALL EIGHT pieces of the cluster are real: `jo:cn`/`jo:kf`/`jo:ib`/
 * `jo:kd` (Chunk 13, `pico8_planet_trade.h`/`pico8_planet.h`), `trade:kg`
 * (Chunk 3, `pico8_trade.h`), `jc:eb` (Chunk 14, `pico8_slipway.h`), and
 * `nr:cn`/`mx:kf` here -- see this file's own `.c` for the disposable
 * integration verification exercising a real subclass override on both
 * sides.
 *
 * Real inheritance, confirmed by reading `nr=qh:jr(...)`/`fr=nr:jr(...)`/
 * `mx=hj:jr(...)` directly: `nr` and `mx` each extend a DIFFERENT parent
 * from `pico8_planet_family.h` (`nr` extends `qh` directly; `mx` extends
 * `hj`, one level deeper) -- `fr` extends `nr`, making `kv(none)->ke->jo
 * ->qh->nr->fr` (5 real ancestor links) and `kv(none)->ke->jo->qh->hj->mx`
 * (also 5) two of the deepest real chains anywhere in this cart, alongside
 * `pm`'s own identical depth (`pico8_planet_family.h`).
 *
 * os=qh:jr([[
 * nu=o("jo","np","prober"),
 * hp=4,px=o("processor",c=13),
 * qh=f,s=100,
 * od=15,
 * hs=0.5,
 * ]])nr=qh:jr([[
 * nu=o("nr","np","prober"),
 * od=10,fp=1,
 * kq=b(-4,-4,5,5),
 * s=170,
 * hz=o(170,170,170,168),
 * dw=0.01,
 * rr="slipgates",
 * ]])function nr:gv(s)
 * return s.qu~=self and #self.oe<3
 * end
 * function nr:ldown()
 * return self:gv({})and jc({qu=self})end
 * function nr:hb()self.s=self.hz[#self.oe+1]
 * end
 * function nr:cn(lw)local t=lw.t
 * for w in all(self.oe)do
 * if lw.mu>0 and not gu(t.rl,w)then
 * local gf=mw(lw)gf.t=t:kg(self,w)w:eb(self):cn(gf)lw.mu=gf.mu
 * end
 * end
 * end
 * fr=nr:jr([[
 * mt=1,
 * rr="infraspace",
 * od=25,
 * s=78,hz=o(78,78,78,76),
 * ]])
 * (pico8/lua.lua:1101-1127; `os`'s own spec is quoted above only because
 * it sits textually between `nr`'s declaration and the module boundary in
 * the real source -- `os` itself is ported in `pico8_planet_family.c`,
 * not here, confirmed by its own `qh:jr(...)` parentage, unrelated to
 * `nr`.) `nr:gv`/`nr:ldown`/`nr:hb`/`nr:cn` are four genuinely NEW method
 * bodies -- see each one's own declaration comment below. `fr` has NO new
 * method bodies and NO field reuse of its own -- confirmed by reading the
 * source in full, exactly the same "just a class declaration extending
 * its own parent" status `os` already has in `pico8_planet_family.h`. */

void pico8_planet_family_trade_init(void);
void pico8_planet_family_trade_shutdown(void);

/* `nr`, built once by pico8_planet_family_trade_init(). No `fz` field, so
 * -- same convention as pico8_qh_class()/pico8_nl_class()/etc.
 * (pico8_planet_family.h) -- registered only into the shared ""-keyed
 * pico8_class_lookup() registry, automatically, via pico8_class_declare()
 * itself; no separate pico8_class_register_named() call. */
pico8_class_t *pico8_nr_class(void);
pico8_instance_t *pico8_nr_new(const pico8_ob_table_t *overrides);

/* `fr`, built once by pico8_planet_family_trade_init(). Same registration
 * convention as pico8_nr_class() above (no `fz` field). */
pico8_class_t *pico8_fr_class(void);
pico8_instance_t *pico8_fr_new(const pico8_ob_table_t *overrides);

/* `mx`, built once by pico8_planet_family_trade_init(). Same registration
 * convention as pico8_nr_class() above -- confirmed by reading `mx`'s own
 * spec: no `fz` field (unlike `pm`/`ga`, `pico8_planet_family.h`), so
 * `jo:hu`'s own `kv[hp.cw]` dynamic reclass can never resolve to `mx`
 * (only `pm`/`ga` are real, evidenced reclass targets in this whole
 * cart). */
pico8_class_t *pico8_mx_class(void);
pico8_instance_t *pico8_mx_new(const pico8_ob_table_t *overrides);

/* function nr:gv(s)
 * return s.qu~=self and #self.oe<3
 * end
 * (pico8/lua.lua:1109-1111.) `nr`'s own override of `jo:gv`
 * (`pico8_jo_gv`, `pico8_planet_display.h`) -- a DIFFERENT question from
 * the base class's own ("can an incoming slipway drag land on this
 * uncolonized-vs-colonized planet?" vs. `nr`'s own "is this slipgate not
 * already the drag's origin, and does it still have room for one more of
 * its own at-most-3 connections?"). `s.qu~=self`: pointer-identity
 * inequality (`s.qu`, a slipway's own "connected from" planet field, read
 * generically via `pico8_instance_get_field` -- no `jc` class dependency,
 * same treatment `pico8_jo_gv` already established for this exact field).
 * `#self.oe<3`: `self.oe` (this planet's own connection list, `jo:pq`/
 * `jo:kf`, Chunks 13/`pico8_planet_trade.h`) compared by COUNT, not by
 * membership. Real Lua's `and` on two genuine boolean comparisons always
 * yields a real boolean (never nil) -- ported as `pico8_value_bool`,
 * never `pico8_value_ref(NULL)`, for BOTH outcomes (unlike `pico8_jo_gv`,
 * whose own uncolonized-planet branch falls off the end into a real nil).
 * Wired as a real `PICO8_VALUE_FUNCTION` field ("gv", overriding `jo`'s
 * own "gv" for any real `nr`/`fr` instance via the ordinary class-chain
 * fallback -- no special-casing needed). Also directly callable as this
 * plain C function (`nr:ldown`'s own `self:gv({})` below calls it this
 * way, a same-class known call, matching `pico8_jo_kd`'s own precedent).
 * A NULL `self` is a safe no-op (returns `pico8_value_ref(NULL)`, the
 * same "nothing to check" convention `pico8_jo_gv` uses for its own NULL-
 * self case, even though this function's own REAL two branches never
 * produce nil); a NULL `s` reads `s.qu` as absent (never equal to
 * `self`), matching `pico8_jo_gv`'s own identical defensive treatment of
 * a possibly-NULL `s`. */
pico8_value_t pico8_nr_gv(const pico8_instance_t *self, const pico8_instance_t *s);

/* function nr:ldown()
 * return self:gv({})and jc({qu=self})end
 * (pico8/lua.lua:1112-1113.) `nr`'s own override of `jo:ldown`
 * (`pico8_jo_ldown`, `pico8_planet.h`). `self:gv({})`: a same-class direct
 * call to `pico8_nr_gv` above (real Lua's own colon-call sugar on `self`,
 * ported as a direct C call per this port's own "known same-class method,
 * no dispatch table" convention, matching `pico8_jc_hq`'s own `self:ce()`
 * precedent) with a genuinely THROWAWAY classless `{}` argument -- built
 * and freed the same "malloc + fresh empty pico8_ob_table_t + cls=NULL"
 * way `pico8_bv`'s own term-3 `{}` placeholder already established
 * (`pico8_probe.c`), except freed right after this one synchronous call
 * (unlike `pico8_bv`'s own throwaway, which gets STORED into `self.et`
 * and so must survive) -- `pico8_nr_gv` never stores its own `s` argument
 * anywhere, only reads `s.qu` once, so nothing outlives this call. `and
 * jc({qu=self})`: real Lua's own full `A and B` semantics -- if `gv`'s own
 * result is falsy (a real `false`, per `pico8_nr_gv`'s own comment above),
 * the WHOLE expression's value is that SAME falsy value, `jc(...)` is
 * never even evaluated; only when `gv` is truthy does this construct a
 * fresh `jc` instance via `pico8_class_lookup_named("jc")` (Chunk 14's
 * already-built registration) with `qu=self`, the exact same "compiles
 * against the generic named-class registry, no class name guessed"
 * pattern `pico8_jo_ldown`'s own identical `jc({qu=self})` call already
 * established (`pico8_planet.c`) -- copied here verbatim, not
 * reimplemented differently. A NULL `self` returns `pico8_value_ref(NULL)`
 * (nothing to check `gv` against). */
pico8_value_t pico8_nr_ldown(pico8_instance_t *self);

/* function nr:hb()self.s=self.hz[#self.oe+1]
 * end
 * (pico8/lua.lua:1114-1115.) `nr`'s own override of the real `bm`-name
 * "hb" dispatch field (`jo:hb`, `pico8_jo_hb`, `pico8_planet.h`) -- a
 * DIFFERENT body from the base class's own (`jo.ex(...)`-driven research
 * icon), reusing `self.oe`'s own count (`jo:pq`/`jo:kf`, same as `nr:gv`
 * above) to pick a sprite from `self.hz` (`nr`'s own class-level
 * `hz=o(170,170,170,168)`, or `fr`'s own override `hz=o(78,78,78,76)`,
 * read via `pico8_instance_get_field`, chain-fallback-aware -- picks up
 * whichever the real instance's own concrete class defines) at the
 * 1-based Lua index `#self.oe+1` (`pico8_ob_table_get_index`, this port's
 * own established 1-based convention, `pico8_ob.h`) -- always in range
 * 1..4 for any real `nr`/`fr` instance in this cart's own reachable
 * scope, since `nr:gv` above already caps `self.oe`'s own count below 3
 * before a new connection can ever be accepted, and `hz` always has
 * exactly 4 positional entries; an out-of-range/absent lookup still
 * defensively reads as fixed 0 rather than crashing, this port's
 * pervasive established convention, matching `jo:kj`'s own identical
 * `jo.ka[self.mi]` treatment (`pico8_planet_display.h`). Auto-registers
 * into `cv["hb"]` at `lt()` time (the SAME "field present -> cv-
 * registered" mechanism `jo.hb` already exercises -- a data change, one
 * more class defining this same `bm` name, not a new mechanism). A NULL
 * `self` is a safe no-op. */
void pico8_nr_hb(pico8_instance_t *self);

/* function nr:cn(lw)local t=lw.t
 * for w in all(self.oe)do
 * if lw.mu>0 and not gu(t.rl,w)then
 * local gf=mw(lw)gf.t=t:kg(self,w)w:eb(self):cn(gf)lw.mu=gf.mu
 * end
 * end
 * end
 * (pico8/lua.lua:1116-1122.) `nr`'s own override of `jo:cn`
 * (`pico8_jo_cn`, `pico8_planet_trade.h`) -- THE `nr:cn` half of the
 * mutually-recursive trade cluster this whole chunk completes. Unlike
 * `jo:cn`'s own want-list bookkeeping, `nr:cn` never touches `self.jy` at
 * all -- a slipgate's real job here is to FORWARD an in-transit proposal
 * onward through every OTHER slipway it's connected to (`self.oe`),
 * extending the trade chain by one more hop through itself each time,
 * rather than consuming the proposal locally.
 *
 * `local t=lw.t`: the trade instance this proposal is currently
 * extending, same field as `jo:cn`'s own identical `local t=lw.t`.
 *
 * `for w in all(self.oe)do if lw.mu>0 and not gu(t.rl,w)then ... end end`:
 * for each OTHER slipway `w` this slipgate connects to (`self.oe`, same
 * field `nr:gv`/`nr:hb` above already read) -- only while the INCOMING
 * proposal `lw` still has quantity left (`lw.mu>0`, this port's own
 * `pico8_jo_trade_proposal_t.mu` field, `pico8_planet_trade.h`) AND `w`
 * isn't already part of this trade's own route (`gu(t.rl,w)`, `t.rl` a
 * `PICO8_VALUE_REF` list field on the trade instance -- `pico8_gu`,
 * Chunk 1, the ordinary pointer-identity list-membership helper, the
 * CORRECT tool here since `t.rl` holds `jc`/slipway instances compared by
 * reference, not `trade` instances needing `pico8_trade_eq`'s own
 * semantic equality -- this is NOT the gap `pico8_jo_ib`'s own `gu(ib,a)`
 * needed `gu_trade` for, a genuinely different comparison target).
 *
 * `local gf=mw(lw)gf.t=t:kg(self,w)w:eb(self):cn(gf)lw.mu=gf.mu`: `mw(lw)`
 * -- real Lua's own shallow-table-copy (Chunk 1's `pico8_mw`, which
 * operates on a `pico8_ob_table_t*`) has no meaning for `lw`'s own C
 * representation here, since `pico8_jo_trade_proposal_t` (`pico8_planet_
 * trade.h`) is a plain named struct, not an `ob_table` -- ported as a
 * literal C struct-copy-by-value (`pico8_jo_trade_proposal_t gf = *lw;`),
 * the exact same "shallow copy, sub-objects shared by reference" outcome
 * real Lua's own `mw()` produces for a plain table (its `t`/`kf`/`ib`
 * fields are copied AS POINTERS, never deep-cloned -- `gf.ib` therefore
 * still points at the SAME shared `ib` list every proposal spawned by the
 * original `jo:ib()` call shares, and `gf.kf` is still the SAME closure
 * function pointer). `gf.t=t:kg(self,w)`: extends the trade chain by
 * THIS planet and slipway `w` -- Chunk 3's already-built `pico8_trade_kg`
 * (`pico8_trade.h`), called with BOTH real arguments this time (`self`
 * AND `w`), unlike `jo:cn`'s own single-argument `t:kg(self)` (which
 * always passes a NULL `jc` argument, `pico8_planet_trade.c`'s own
 * documented choice) -- `nr:cn` is the one real call site in this whole
 * cart that ever supplies BOTH of `kg`'s own parameters for real.
 * `w:eb(self):cn(gf)`: the cluster's own recursive dispatch -- `w:eb
 * (self)` (`pico8_mv`, Chunk 14's `jc:eb`) resolves the planet at the far
 * end of slipway `w`, then `:cn(gf)` (`pico8_mv` again, resolving to
 * `jo:cn` OR, if that far planet is ITSELF an `nr`/`fr`, back to `nr:cn`
 * -- a REAL, evidenced same-function recursion this cluster's own design
 * anticipates) is dispatched on it with the freshly-extended copy `gf`.
 * `lw.mu=gf.mu`: copies the REMAINING quantity back from the (possibly
 * mutated, via `gf`'s own shared `kf` closure decrementing `gf.mu`
 * through `self.mu-=1` inside `pico8_jo_ib_kf_closure`) copy `gf` back
 * onto the ORIGINAL incoming proposal `lw` -- so quantity consumed by
 * ANY hop along this forwarding chain is visible to every earlier hop
 * still holding a reference to the SAME original `lw`.
 *
 * Wired as a real `PICO8_VALUE_FUNCTION` field ("cn", overriding `jo`'s
 * own "cn" for any real `nr`/`fr` instance) -- `pico8_nr_cn_dispatch`
 * (this file's own `.c`) unwraps `mv()`'s own `arg0` back into a
 * `pico8_jo_trade_proposal_t*`, the SAME shape `pico8_jo_cn_dispatch`
 * already establishes (`pico8_planet.c`). A NULL `self` or NULL `lw` is a
 * safe no-op. */
void pico8_nr_cn(pico8_instance_t *self, pico8_jo_trade_proposal_t *lw);

/* function mx:kf(sw)local cp=sw:eb(self)for i in all(cp.jy)do
 * if i.gc==0
 * and gu(self.ht,i.qe)then
 * self.kw.qe,self.ht=
 * i.qe,{}
 * end
 * end
 * jo.kf(self,sw)end
 * (pico8/lua.lua:1049-1056.) `mx`'s own override of `jo:kf`
 * (`pico8_jo_kf`, `pico8_planet_trade.h`) -- THE `mx:kf` half of the
 * mutually-recursive trade cluster, confirmed real by Chunk 13's own
 * sub-lot-4 finding (`pico8_planet_trade.h`'s own top comment: "`mx:kf`
 * ... does its own thing then explicitly super-calls `jo.kf(self,sw)`
 * ... already accounted for in `jo:kf`'s dual-callability design").
 *
 * `local cp=sw:eb(self)`: `sw:eb(self)` (`pico8_mv`, Chunk 14's `jc:eb`)
 * resolves the planet at the OTHER end of the newly-established slipway
 * `sw` from `self`'s own perspective -- `cp` (a `pico8_instance_t*`, "the
 * COUNTERPARTY planet").
 *
 * `for i in all(cp.jy)do if i.gc==0 and gu(self.ht,i.qe)then self.kw.qe,
 * self.ht=i.qe,{} end end`: `cp.jy` (the counterparty's own want-list,
 * the SAME field `jo:cn`/`jo:gh` already read, `pico8_planet_trade.h`/
 * `pico8_planet_display.h`) -- for each want-record `i` (a plain
 * `pico8_ob_table_t*`, not an instance, same representation `jo:cn`'s
 * own `i` already uses): if it's currently unfulfilled (`i.gc==0`,
 * absent-`gc` reading as 0, the same established convention `jo:cn`'s
 * own identical `i.gc` check already uses) AND the resource it wants is
 * one `self` (this `mx` instance) is configured to synthesize
 * (`gu(self.ht,i.qe)` -- `self.ht`, `mx`'s own class-level positional
 * `ht=o(28,20,21,22)`, or a PER-INSTANCE empty override this same method
 * writes below, read via this file's own private `positional_values`
 * bridge + `pico8_gu`, the same documented-duplicate pattern already
 * established by `pico8_jo_cn`'s own identical `gu(self.ba,t.qe)` check)
 * -- `self.kw.qe,self.ht=i.qe,{}` REBINDS this `mx` instance's OWN
 * production target (`self.kw`, a REF `pico8_ob_table_t*` -- always
 * present, `jo`'s own class-level `kw=o(mu=0)` default or a per-instance
 * table `jo:lz` built, `pico8_planet.h`) to the counterparty's own
 * requested resource `i.qe`, AND clears `self.ht` to a BRAND NEW, empty
 * `pico8_ob_table_t` (an instance-level field write on `self->fields`,
 * overriding whatever `self.ht` resolved to before -- `mx`'s own class-
 * level `ht` default stays untouched for every OTHER `mx` instance) --
 * "once this synthesizer commits to producing a resource a neighbor
 * actually wants, stop offering to synthesize anything else." Real Lua's
 * own multi-assignment (`a,b=x,y`) evaluates BOTH right-hand values
 * before either assignment lands -- ported as reading `i.qe` into a local
 * BEFORE mutating `self.ht`, matching `mh:pq`'s own already-established
 * "evaluate RHS first" precedent for this exact Lua idiom.
 *
 * `jo.kf(self,sw)`: the EXPLICIT SUPER-CALL Chunk 13's own sub-lot-4
 * already anticipated -- a direct C call to `pico8_jo_kf` (`pico8_planet_
 * trade.h`), bypassing `mv()`/`dc()` dispatch entirely, run
 * UNCONDITIONALLY after the loop above (registers `sw` onto `self.oe`,
 * propagates `.fx`, calls `self:kd()` -- see `pico8_jo_kf`'s own
 * declaration comment for the full trace, unchanged here).
 *
 * Wired as a real `PICO8_VALUE_FUNCTION` field ("kf", overriding `jo`'s
 * own "kf" -- via `hj`, since `mx` extends `hj`, which itself never
 * overrides "kf" -- for any real `mx` instance). A NULL `self` or NULL
 * `sw` is a safe no-op (matches `pico8_jo_kf`'s own established
 * convention for the same case). */
void pico8_mx_kf(pico8_instance_t *self, pico8_instance_t *sw);

#endif
