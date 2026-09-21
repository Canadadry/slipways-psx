#ifndef PICO8_PLANET_FAMILY_H
#define PICO8_PLANET_FAMILY_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Phase 3, Chunk 15 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): the planet-subtype family that extends `jo` (Chunk 13) --
 * `qh`, `nl`, `os`, `hj`, `pm`, `ga` live here; the two classes most
 * directly tied to the mutually-recursive trade cluster this chunk
 * completes (`nr`, whose `nr:cn` overrides `jo:cn`, and its own subclass
 * `fr`; `mx`, whose `mx:kf` overrides `jo:kf` via `hj`) live in the sibling
 * `pico8_planet_family_trade.c`/`.h` instead -- see that file's own top
 * comment for the trade-cluster-completion trace.
 *
 * Real inheritance, confirmed by reading every `X:jr(...)` declaration
 * directly (not assumed from the PRD's own prose, which lists the family
 * but doesn't state ga's own parent):
 *   jo -> qh -> nl
 *            -> os
 *            -> hj -> pm
 *                  -> mx  (pico8_planet_family_trade.h)
 *            -> nr -> fr  (pico8_planet_family_trade.h)
 *   jo -> ga
 * `ga=jo:jr([[...]])` (pico8/lua.lua:1067) extends `jo` DIRECTLY, not
 * `hj` -- despite reusing `hj`/`g_`'s own particle helpers (`f_`/`ez`/`ei`,
 * Chunks 9-10) the same way `hj` does, `ga` is its own separate branch off
 * `jo`, one level shallower than `hj`'s own particle-emitting subtree.
 *
 * qh=jo:jr([[
 * nu=o("qh","jo","np","prober"),
 * hp=1,px=o("laboratory",c=13),
 * me="kz",dq="ly",
 * hf=f,qh=1,
 * od=15,fp=2,
 * dw=0.025,
 * mi=1,
 * s=174,
 * ba=o(28,18,20,21,22,48),
 * ]])qh.hq=bv
 * qh.ly=na
 * qh.ev=dl
 * qh.ey=d_
 * (pico8/lua.lua:905-918.) NO new method bodies -- per the PRD's own
 * framing, pure field-reuse wiring onto a freshly `jo:jr()`-declared
 * class. `hq=bv`/`ev=dl`/`ey=d_` reuse Chunk 10's already-built
 * `pico8_bv`/`pico8_dl`/`pico8_d_` VERBATIM -- the exact same already-
 * exported dispatch bridges `g_` itself already wires onto its own class
 * defaults (`pico8_bv_dispatch`/`pico8_dl_dispatch`/`pico8_d__dispatch`,
 * `pico8_probe.h`) are reused HERE too, unmodified, no new adapter code
 * needed for these three. `ev` IS one of `bm`'s 5 real dispatch names
 * (`bm=["kj","ev","hd","hk","hb"]`) -- reached by a future `dc("ev")` in
 * addition to ordinary `mv()`, the same "field present -> cv-registered"
 * treatment `g_.ev=dl` already established; `hq`/`ey` are NOT `bm` names,
 * mv()-routed only (same `ui:bh()` drag/commit call sites `g_.hq=bv`/
 * `g_.ey=d_` already serve).
 *
 * `qh.ly=na` (Chunk 1's already-built `pico8_na`, `pico8_entity_geom.h`)
 * is the one field-reuse this file's own reading confirms is genuinely
 * LOAD-BEARING, not dead: `dg()`'s own per-tick dispatch loop
 * (`pico8_bootstrap.c`'s `pico8_dg`, pico8/lua.lua:173-181 `local
 * fn=qc[qc.me] if fn then fn(qc,qc.t) end`) calls `pico8_mv(qc,
 * qc.me,1,qc.t,...)` every tick -- i.e. it dispatches BY NAME using
 * whatever string `self.me` currently holds. `qh`'s own default `dq=
 * "ly"` plus `d_`'s own `self:ks(self.dq or "ph")` (Chunk 10) means a
 * successfully-committed `qh`-family instance's own `me` becomes exactly
 * "ly" -- so `dg()`'s own per-tick dispatch resolves to THIS SAME field,
 * calling whichever `ly` implementation the instance's real class chain
 * provides (`qh.ly=na` here, or `nl:ly`'s own override below) with
 * `argc=1`, `arg0=self.t`. `na` has no existing dispatch bridge to reuse
 * (no earlier class ever wired it as a field) -- a small NEW bridge
 * (`pico8_qh_ly_dispatch`, this file's own `.c`) is genuinely needed, but
 * it is pure dispatch plumbing around the unchanged `pico8_na` body, the
 * same category of "new glue, not a new method body" `pico8_jo_hd_
 * dispatch` already established for `jo.hd=gk` (Chunk 13) -- not a
 * violation of this class's own "no new method bodies" status.
 *
 * nl=qh:jr([[
 * nu=o("jo","np","prober"),
 * rr="trade league",qh=f,nl=1,
 * hp=6,s=68,px=o("trading hub",c=13),
 * od=20,qw=15,
 * ba=o(28,48,18,20,21,22),
 * ]])function nl:ly(t)na(self,t)if self.mi>=2 then
 * self.qw=8+t%42/6
 * end
 * end
 * (pico8/lua.lua:1057-1066.) `nl`'s own real override of `qh.ly=na` --
 * see `pico8_nl_ly` below for the full trace. `self.mi` is always
 * present as a real `PICO8_VALUE_FIXED` for any real `nl` instance (`nl`
 * never overrides `mi` itself, so it falls through the class chain to
 * `qh`'s own default `mi=1`) -- the unguarded `if self.mi>=2` (no `and
 * self.mi` truthy pre-check, unlike `hj:kj` below) never observes a
 * genuinely-absent `mi` in this chunk's own real scope, so this is ported
 * as a plain fixed comparison, this port's pervasive "absent numeric
 * field reads as 0" fallback covering the (unevidenced) defensive case.
 *
 * os=qh:jr([[
 * nu=o("jo","np","prober"),
 * hp=4,px=o("processor",c=13),
 * qh=f,s=100,
 * od=15,
 * hs=0.5,
 * ]])
 * (pico8/lua.lua:1095-1100.) NO new method bodies and NO field reuse --
 * just a class declaration extending `qh`, confirmed by reading the
 * source in full: no `os.XXX=YYY` line and no `function os:...` anywhere
 * in the file.
 *
 * hj=qh:jr([[
 * nu=o("hj","jo","np","prober"),
 * rr="ascension",
 * hp=2,qh=f,
 * px=o("ascension nr",c=13),
 * od=60,
 * s=166,
 * py=0.12,ki=9,
 * pn=5,pk=1,
 * fc=0.5,
 * k_=o(1,13,12,12,12,13,13,5,1,1,1),
 * du=7.5,
 * qo=0.98,ro=-0.07,
 * qg=0.07,q_=0.98,
 * ]])hj.pq=f_
 * hj.mm=g_.mm
 * function hj:kj(p)if self.mi and self.mi>=2 then
 * ei(self)end
 * jo.kj(self,p)end
 * (pico8/lua.lua:998-1016.) `pq=f_` reuses Chunk 9's already-built
 * `pico8_f_` DIRECTLY as `hj`'s own extra constructor
 * (`pico8_class_set_pq(hj_class, pico8_f_)`, no adapter -- `pico8_f_`
 * already has exactly `pico8_class_pq_fn`'s shape, the same "signature
 * already matches" precedent `pico8_particle.h`'s own top comment already
 * confirmed for `g_.pq=f_`). `mm=g_.mm` reuses Chunk 10's already-built
 * `pico8_g__mm` DIRECTLY as `hj`'s own "mm" field value (`pico8_ob_
 * table_set_name(hj_class->defaults,"mm",pico8_value_function(pico8_g__
 * mm))`) -- `pico8_g__mm`'s own body reads every constant it needs
 * (`pn`,`du`,`py`,`ki`,`pk`,`t`) off `self` GENERICALLY (never off any
 * `g_`-specific global or class pointer), so reusing it verbatim for an
 * `hj` instance picks up `hj`'s OWN class-level values for those fields
 * (confirmed by comparing `hj`'s own spec above against `g_`'s, Chunk 10
 * -- genuinely different numbers, same formula), exactly the real Lua
 * semantics of assigning the SAME function value onto a second class.
 *
 * `hj:kj` is a genuinely NEW method body -- see `pico8_hj_kj` below.
 *
 * pm=hj:jr([[
 * fz="pm",
 * nu=o("jo","np","prober"),
 * hp=3,px=o("protostar",c=13),
 * me="ly",s=98,
 * py=0.06,ki=9,
 * pn=8,pk=f,
 * ng=v(1,0),
 * du=2,
 * fc=1,
 * k_=o(1,2,4,9,10,7),
 * qo=1.02,ro=-0.0525,
 * qg=0.0525,q_=1.02,
 * ]])function pm:pq()ga(ob([[
 * qo=0.97,ro=-0.1255,
 * qg=0.1255,q_=0.97,
 * k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1),
 * iu=-0.009,py=9,ms=1.5,
 * ]],{ra=self.ra+self.ng}))end
 * (pico8/lua.lua:1017-1035.) `pm`'s own class spec explicitly sets
 * `fz="pm"` -- confirmed by reading `jo:hu`'s own dynamic reclass
 * (`pico8_planet.c`'s `pico8_jo_hu`, Chunk 13) that ONLY `pm`/`ga` ever
 * set a real `fz` field in this whole cart, the exact fact `jo:hu`'s own
 * header comment already anticipated ("only reachable once Chunk 15
 * builds a real fz-named class"). `pm:pq()` is a genuinely NEW method
 * body -- see `pico8_pm_pq` below.
 *
 * ga=jo:jr([[
 * fz="ga",
 * nu=o(),
 * hf=f,
 * by=o(),
 * kq=b(0,0,-1,-1),
 * fa=3,
 * fc=0,
 * qo=0.9,ro=0,
 * qg=0,q_=0.9,
 * k_=o(1,2,4,9,9,10,10,7,7,7,7,7),
 * iu=-0.06,py=6,ms=3,
 * ]])function ga:pq()f_(self)ez(self,140)mo.ma=3
 * end
 * function ga:mm()local qd=rnd(1.5)+0.5
 * if (rnd()<0.6)qd=flr(qd/0.5)*0.5
 * local p=qm(qd,rnd())
 * return {p=p,v=p*self.ms,a=p*self.iu,l=rnd(self.py)+self.py
 * }
 * end
 * function ga:kj(p)local pt
 * for i=1,3 do
 * pt=self.qy[i]
 * if pt then
 * circ(p.x,p.y,pt.p:qx(),self.k_[flr(pt.l/2)])end
 * end
 * ei(self)self.p_=not pt
 * end
 * (pico8/lua.lua:1067-1094.) `ga`'s own `nu=o()` is a genuinely EMPTY
 * positional table -- overriding `jo`'s own `nu=o("jo","np","prober")`
 * class default entirely (chain fallback stops at the FIRST class from
 * the leaf up that defines a field, so `ga`'s own empty override wins
 * outright, never merged with `jo`'s): a real `ga` instance is never
 * registered into `cg["jo"]`/`cg["np"]`/`cg["prober"]` at all, despite
 * being `jo`-descended -- confirmed by re-reading `pico8_lt`
 * (`pico8_entity.c`), whose own `for t in all(e.nu) do...end` loop simply
 * never executes when `e.nu` resolves to a zero-length table (an ordinary
 * consequence of the SAME already-tested list-iteration loop, not a new
 * mechanism -- see this file's own "entity/dispatch coverage" note in
 * `docs/portage-progress.md`). `pq`/`mm`/`kj` are three genuinely NEW
 * method bodies -- see `pico8_ga_pq`/`pico8_ga_mm`/`pico8_ga_kj` below. */

void pico8_planet_family_init(void);
void pico8_planet_family_shutdown(void);

/* `qh`, built once by pico8_planet_family_init(). `qh`'s own spec never
 * sets an `fz` field, so -- exactly like `bg`/`mh`/`iv`/`js`/`fq`/`qb`/`ui`
 * before it (none of which register under Chunk 7's named forward-
 * reference registry either) -- it is registered ONLY into the shared
 * ""-keyed `pico8_class_lookup()` registry, automatically, as a plain
 * side effect already built into every `pico8_class_declare()` call
 * (`pico8_class.c`'s own `pico8_class_register(cls->name, cls)`, `cls-
 * >name` defaulting to "" when no `fz` field is present) -- no separate
 * `pico8_class_register_named()` call is made or needed for `qh`, since
 * no real forward reference in this cart's scope ever looks "qh" up by
 * name (confirmed by re-reading `jo:hu`'s own `kv[hp.cw]` dynamic
 * reclass, `pico8_planet.c` -- the ONLY real consumer of that specific
 * registry, and it only ever resolves a class that sets a real `fz`
 * field, i.e. `pm`/`ga` below, never `qh`). Returns NULL before
 * pico8_planet_family_init() (or after pico8_planet_family_shutdown()). */
pico8_class_t *pico8_qh_class(void);
pico8_instance_t *pico8_qh_new(const pico8_ob_table_t *overrides);

/* `nl`, built once by pico8_planet_family_init(). Same registration
 * convention as pico8_qh_class() above (no `fz` field, no named-registry
 * entry). */
pico8_class_t *pico8_nl_class(void);
pico8_instance_t *pico8_nl_new(const pico8_ob_table_t *overrides);

/* `os`, built once by pico8_planet_family_init(). Same registration
 * convention as pico8_qh_class() above (no `fz` field, no named-registry
 * entry). */
pico8_class_t *pico8_os_class(void);
pico8_instance_t *pico8_os_new(const pico8_ob_table_t *overrides);

/* `hj`, built once by pico8_planet_family_init(). Same registration
 * convention as pico8_qh_class() above (no `fz` field, no named-registry
 * entry). */
pico8_class_t *pico8_hj_class(void);
pico8_instance_t *pico8_hj_new(const pico8_ob_table_t *overrides);

/* `pm`, built once by pico8_planet_family_init(). Registered under BOTH
 * pico8_class_lookup_named("pm") (Chunk 7's registry) AND the shared
 * ""-keyed pico8_class_lookup() registry -- `pm`'s own spec sets a real
 * `fz="pm"` field, which is what `jo:hu`'s own `kv[hp.cw](...)` dynamic
 * reclass (Chunk 13) needs to find it by name. This is the FIRST real
 * class in this whole port to make that forward reference resolve to
 * something other than NULL. */
pico8_class_t *pico8_pm_class(void);
pico8_instance_t *pico8_pm_new(const pico8_ob_table_t *overrides);

/* `ga`, built once by pico8_planet_family_init(). Same dual registration
 * as pico8_pm_class() above (`ga`'s own spec also sets a real `fz="ga"`
 * field) -- the SECOND, and per this file's own top comment the only
 * OTHER, real `fz`-named class `jo:hu` can ever resolve to. */
pico8_class_t *pico8_ga_class(void);
pico8_instance_t *pico8_ga_new(const pico8_ob_table_t *overrides);

/* function nl:ly(t)na(self,t)if self.mi>=2 then
 * self.qw=8+t%42/6
 * end
 * end
 * (pico8/lua.lua:1063-1066.) See this file's own top comment for the full
 * `dg()`-per-tick-dispatch trace confirming "ly" is genuinely load-bearing
 * (not dead code). Calls Chunk 1's already-built `pico8_na` directly (same
 * signature, `(self,t)`, no adapter needed for the call itself) THEN, only
 * when `self.mi>=2` (always present as FIXED for a real `nl` instance,
 * inherited from `qh`'s own `mi=1` default when `nl` itself doesn't
 * override it), rebinds `self.qw` (an instance-level field write,
 * regardless of where `qw`'s class-level default of 15 came from) to
 * `8+(t%42)/6` -- `pico8_fixed_mod` (this chunk's own new primitive,
 * `pico8_fixed.h`) ported literally, not approximated with `t-flr(t/42)*
 * 42` reimplemented inline. Wired as a real `PICO8_VALUE_FUNCTION` field
 * ("ly", `pico8_nl_ly_dispatch`, this file's own `.c`) on `nl`'s own class
 * defaults, overriding `qh.ly=na` for any real `nl` instance (the class-
 * chain fallback naturally prefers the leaf's own field over the
 * inherited one, no special-casing needed) -- reached by `dg()`'s own
 * per-tick `qc[qc.me](qc,qc.t)` dispatch exactly like `qh.ly=na` is. A
 * NULL `self` is a safe no-op (matches `pico8_na`'s own established
 * convention). */
void pico8_nl_ly(pico8_instance_t *self, pico8_fixed_t t);

/* function hj:kj(p)if self.mi and self.mi>=2 then
 * ei(self)end
 * jo.kj(self,p)end
 * (pico8/lua.lua:1014-1016.) `hj`'s own override of the real `bm`-name
 * "kj" dispatch field -- unlike `nl:ly` above, this condition IS guarded
 * by a truthy pre-check (`self.mi and ...`, not just `self.mi>=2`): a real
 * `hj` instance's own `mi` field is written only by `jo:hu`'s dynamic
 * reclass (Chunk 13, `pico8_jo_hu`'s own `self.mi=...` on the FRESH
 * reclassed instance, not on `hj`'s own class spec, which never sets
 * `mi` itself) -- genuinely absent (nil) until that reclass runs, so the
 * `self.mi and` short-circuit is load-bearing here, not decorative.
 * `ei(self)` (Chunk 9's already-built `pico8_ei`, explicit `gfx`/`mem`,
 * `pico8_particle.h`) only fires once `self.mi>=2`; `jo.kj(self,p)`
 * (Chunk 13's already-built `pico8_jo_kj`, `pico8_planet_display.h`) is
 * an EXPLICIT SUPER-CALL, always run unconditionally afterward -- ported
 * as a direct C call bypassing `mv()`/`dc()` dispatch entirely, the same
 * "real Lua explicit super-call" treatment already confirmed for `mx:kf`'s
 * own `jo.kf(self,sw)` (`pico8_planet_trade.h`'s own top comment). Wired
 * as a real `PICO8_VALUE_FUNCTION` field ("kj", `pico8_hj_kj_dispatch`,
 * this file's own `.c`) on `hj`'s own class defaults, overriding `jo`'s
 * own "kj" for any real `hj`/`pm`/`mx` instance (chain fallback) -- ALSO
 * auto-registers into `cv["kj"]` at `lt()` time (the same "field present
 * -> cv-registered" mechanism `jo.kj`/`g_.kj=ei` already exercise, a
 * DATA change -- one more class defining this same `bm` name -- not a new
 * MECHANISM). A NULL `self`/`gfx`/`mem` is a safe no-op (matches
 * `pico8_jo_kj`'s/`pico8_ei`'s own established convention). */
void pico8_hj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p);

/* function pm:pq()ga(ob([[
 * qo=0.97,ro=-0.1255,
 * qg=0.1255,q_=0.97,
 * k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1),
 * iu=-0.009,py=9,ms=1.5,
 * ]],{ra=self.ra+self.ng}))end
 * (pico8/lua.lua:1030-1035.) `pm`'s own extra constructor (`pico8_class_
 * pq_fn`-shaped, set via `pico8_class_set_pq(pm_class, pico8_pm_pq)`, NOT
 * a `mv()`-routed field): spawns one fresh `ga` instance (a particle-burst
 * effect, this file's own `pico8_ga_new`) positioned at `self.ra+self.ng`
 * (`self.ra` -- this planet's own position, defensively zero-vector if
 * absent, same convention as every other `.ra` read in this port;
 * `self.ng`, `pm`'s own class default `v(1,0)`, an offset), with a
 * per-spawn OVERRIDE spec merged in via Chunk 2's already-built `pico8_ob`
 * (`qo`/`ro`/`qg`/`q_`/`k_`/`iu`/`py`/`ms` -- overriding `ga`'s own class
 * defaults for THIS one spawned instance only, `defaults` winning over
 * the parsed spec on the shared `ra` key per `pico8_ob`'s own documented
 * merge direction, `pico8_ob.h`). The freshly-`ga`-constructed instance
 * (via `pico8_class_instantiate`'s own automatic leaf-`pq`-plus-inherited-
 * `ke.pq=lt` chain, self-registering it into `hv`/`cv`/`cg` exactly as any
 * other real `lt()`'d entity) is never stored anywhere -- real Lua's own
 * `ga(...)` call site here discards its own return value too, matching
 * this port's own established "GC-deferred" convention already documented
 * for `jo:ib`'s own discarded per-iteration `lw.t` (`pico8_planet_trade.h`)
 * -- the spawned `ga` stays alive and reachable ONLY through `hv`/`cg`,
 * same as every other self-registering entity in this port. A NULL `self`
 * is a safe no-op. */
void pico8_pm_pq(pico8_instance_t *self);

/* function ga:pq()f_(self)ez(self,140)mo.ma=3
 * end
 * (pico8/lua.lua:1079-1080.) `ga`'s own extra constructor: `f_(self)`
 * (Chunk 9's already-built `pico8_f_`) then `ez(self,140)` (Chunk 9's
 * already-built `pico8_ez`, spawning up to 140 particles via `self:mm()`
 * -- resolving to `pico8_ga_mm` below, since `ga`'s own class already has
 * "mm" wired by the time any real `ga` instance is constructed), then
 * `mo.ma=3` -- a DIRECT write onto the global camera/palette singleton's
 * own "ma" field (`pico8_mo()`, `pico8_entity.h`), the same `mo->fields`
 * write convention `pico8_bg_hq`'s own `mo->fields` write already
 * established (`pico8_bg.c`) -- a no-op (guarded) before Chunk 6's `jl()`
 * ever replaces `pico8_mo()`'s own classless placeholder with a real `qb`
 * instance. Wired via `pico8_class_set_pq(ga_class, pico8_ga_pq)`. A NULL
 * `self` is a safe no-op. */
void pico8_ga_pq(pico8_instance_t *self);

/* function ga:mm()local qd=rnd(1.5)+0.5
 * if (rnd()<0.6)qd=flr(qd/0.5)*0.5
 * local p=qm(qd,rnd())
 * return {p=p,v=p*self.ms,a=p*self.iu,l=rnd(self.py)+self.py
 * }
 * end
 * (pico8/lua.lua:1081-1086.) `ga`'s own particle-shape generator --
 * genuinely different from `g_:mm`'s (`pico8_probe.h`'s `pico8_g__mm`):
 * a random length `qd` (biased 60% of the time to snap to the nearest
 * 0.5 multiple below it, `flr(qd/0.5)*0.5`) at a fully random angle
 * (`rnd()`, no angular-slice biasing unlike `g_:mm`'s own `pn`-sliced
 * angle), PLUS a real "a" (acceleration) field this particle shape
 * produces that `g_:mm`'s own shape never does (`p*self.iu`, a per-frame
 * inward/outward pull `pico8_ei`'s own `if (p.a)p.v+=p.a` already reads,
 * Chunk 9) -- confirming `pico8_particle.h`'s own top-comment prediction
 * that a future `mm` might use the OPTIONAL "a" field its own particle-
 * representation convention already reserved for exactly this case.
 * Wired as a real `PICO8_VALUE_FUNCTION` field ("mm", directly -- this
 * function already has `pico8_value_function_t`'s exact shape, no
 * separate dispatch-bridge wrapper needed, unlike `pico8_hj_kj`/
 * `pico8_ga_kj`, which need `gfx`/`mem`) on `ga`'s own class defaults.
 * `argc`/`arg0`/`arg1` unused (the real `ga:mm()` declares no Lua
 * parameters beyond `self`, matching `g_:mm`'s own identical shape). A
 * NULL `self` reads every field as its documented default (0), matching
 * `pico8_g__mm`'s own established convention for the same case. */
pico8_value_t pico8_ga_mm(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* function ga:kj(p)local pt
 * for i=1,3 do
 * pt=self.qy[i]
 * if pt then
 * circ(p.x,p.y,pt.p:qx(),self.k_[flr(pt.l/2)])end
 * end
 * ei(self)self.p_=not pt
 * end
 * (pico8/lua.lua:1087-1094.) `ga`'s own draw method -- a real `bm`-name
 * dispatch field, wired the same way `pico8_hj_kj_dispatch` is. Draws (up
 * to) the first 3 live particles of `self.qy` (a `PICO8_VALUE_REF` to a
 * heap `pico8_list_t*`, Chunk 9's own particle-representation convention
 * -- indexed 1-based, `self.qy[i]` for `i=1,2,3`, read as a direct C array
 * access `qy->items[i-1]` when `i<=qy->count`, since `pico8_list_t` is a
 * plain growable array, not an opaque type) as expanding circles centered
 * on the DISPATCHED screen position `p` (the function's OWN parameter,
 * shadowing each particle record's own unrelated "p" field, `pt.p`, the
 * particle's stored position-magnitude vector -- `pt.p:qx()`, `pico8_vec_
 * qx`, becomes the circle's own radius) colored by `self.k_[flr(pt.l/2)]`
 * (a class-level positional lookup, `pico8_ob_table_get_index` on `ga`'s
 * own class defaults, absent/out-of-range reading as color 0 -- the same
 * defensive convention `jo:kj`'s own `jo.ka[self.mi]` lookup already
 * established, `pico8_planet_display.h`). After the loop, `ei(self)`
 * (Chunk 9's `pico8_ei`, the shared physics/decay/draw step) always runs;
 * `self.p_=not pt` uses whatever `pt` was left holding by the LAST loop
 * iteration (`i=3`, real Lua's own `local pt` declared OUTSIDE the loop,
 * never reset between iterations) -- once the 3rd particle slot no longer
 * exists (`self.qy` has shrunk to fewer than 3 live particles, `pico8_ei`'s
 * own `del()`-on-death), `pt` is NULL and `self.p_` becomes real Lua
 * `true` (a genuine `not nil`), scheduling this whole burst entity for
 * removal via `dg()`'s own `if qc.p_ then he(qc) end`. A NULL
 * `self`/`gfx`/`mem` is a safe no-op. */
void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p);

#endif
