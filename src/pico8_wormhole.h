#ifndef PICO8_WORMHOLE_H
#define PICO8_WORMHOLE_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Phase 3, Chunk 16 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): the `mz` class -- a wormhole/probe-arrival event entity,
 * pico8/lua.lua:752-796, read directly (not guessed at from the PRD's own
 * paraphrase):
 *
 * mz=ke:jr([[
 * nu=o("mz"),
 * me="kz",dw=0,
 * dq="pr",
 * spawn_snd=52,ik=49,
 * oi=0,
 * od=3,fp=1,
 * fa=5,
 * kk=o(7,7,6,13,5,1,1,1,0),
 * jj=o(qs="-too far-"),
 * ]])mz.hq=bv
 * mz.ev=dl
 * function mz:pq()rn(53)self.ox=jp(hv,function(p)
 * return p.fx
 * end)end
 * function mz:hq(...)bv(self,...)local nq
 * nq,self.ij=ij(self.p,self.ox)if nq>ov("space folding",0.19,0.27)then
 * self.et=mz.jj
 * end
 * end
 * function mz:kz(t)self.oi=ov("space folding",29,35)*
 * (min(t/15,1)+sin(t/40)*0.1)end
 * function mz:ey()self.p_=
 * self.t<15 or not d_(self)end
 * function mz:pr(t)local sr=self:ej(t)/128+0.031
 * for pt in all(cg.jo)do
 * if pt.me=="unknown"and (pt.p-self.p):qx()<sr then
 * pt:hh()end
 * end
 * self.p_=t>25
 * end
 * function mz:kj(p)if self.me=="kz" then
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
 * function mz:ej(t)
 * return min(sqrt(t/15),1)*self.oi
 * end
 * (pico8/lua.lua:752-796.)
 *
 * `nu=o("mz")`: a SINGLE tag, "mz" only -- unlike `g_`'s own `nu=o("g_",
 * "prober")` (pico8_probe.h), a real `mz` instance registers ONLY into
 * `cg["mz"]`, never `cg["prober"]` or any planet/probe tag. `me="kz"` is
 * the initial icon/state name (the wormhole's own "approaching" visual);
 * `dq="pr"` is the state `d_`'s own `self:ks(self.dq or "ph")` transitions
 * to once a target is committed (see pico8_probe.h's own top comment on
 * `pico8_ke_ks`/`pico8_d_` -- `ke:ks(me)` is a plain state-name+timer
 * reset, NOT dynamic dispatch by name; the dispatch itself happens
 * separately and generically, every tick, in `pico8_dg()`, see below).
 * `dw=0` (vs. `g_`'s own `dw=0.025`) feeds `bv`'s own `nq<self.dw`
 * near-planet snap check (pico8_probe.h/`pico8_bv`) -- with `dw=0`, that
 * branch only ever fires for an exact (never-evidenced) position
 * coincidence, a real, faithfully-preserved behavioral difference from
 * `g_`, not an oversight. `od=3,fp=1` feed `d_`'s own `fm:lu(self.od)`/
 * `fm:nx(self.fp or 0)` calls (a wormhole "costs" a flat 3 of whatever
 * `fm:lu`/`fm:nx` gate/charge, and always reports `1` back through `fp`,
 * unlike `g_`'s own `fp=f` which reports `false`/0 -- both still forward-
 * referencing the still-placeholder `fm`, per pico8_probe.h's own already-
 * documented `fm` trace, unaffected by this chunk). `kk`/`jj` are read by
 * `kj`/`hq` below, each documented at its own point of use.
 *
 * DISPATCH MECHANISM, PER-FIELD CLASSIFICATION (checked against the real
 * mechanisms this port already ships, not assumed):
 *
 *   - `kz`/`pr` are reached by `pico8_dg()`'s OWN generic per-tick
 *     self.me-named dispatch (`pico8_bootstrap.c`: `if
 *     pico8_instance_get_field(qc,"me",&me_val)... pico8_mv(qc,
 *     me_val.as.string,1,t_arg,...)`, mirroring the real `local
 *     fn=qc[qc.me] if fn then fn(qc,qc.t) end`). This mechanism is
 *     ALREADY WIRED and unconditionally live for every entity in `hv` --
 *     `mz:kz`/`mz:pr` need only be registered as ordinary
 *     `PICO8_VALUE_FUNCTION` fields named "kz"/"pr" on `mz`'s own class
 *     defaults, with a `(self,argc,arg0,arg1)`-shaped bridge that reads
 *     `arg0` as `t` (argc=1 every real call, matching `pico8_dg`'s own
 *     `pico8_mv(qc, me_val.as.string, 1, t_arg, pico8_value_ref(NULL),
 *     &discard)` call shape) -- unlike `pico8_nf_ph_dispatch`
 *     (`pico8_mouse_reader.c`), whose own real body never references its
 *     own second Lua parameter and so ignores `arg0` entirely, these two
 *     bridges genuinely READ `arg0` as `t` and forward it, since `kz(t)`/
 *     `pr(t)` both use it directly.
 *   - `ev` (`mz.ev=dl`, pure field-reuse of Chunk 10's `pico8_dl`) IS one
 *     of `bm`'s 5 real dispatch names ("kj","ev","hd","hk","hb",
 *     `pico8_entity.c`'s own `bm` literal) -- reached automatically by a
 *     future `dc("ev")` (Chunk 22) IN ADDITION to plain `mv()` lookup,
 *     wired here by reusing the EXISTING `pico8_dl_dispatch`
 *     (`pico8_probe.h`) verbatim, no new glue. See this file's own "SELF.ET
 *     REPRESENTATION" section further down for a real, evidenced
 *     discrepancy this reuse surfaces and how it is resolved.
 *   - `hq` (`mz.hq=bv` PLUS `function mz:hq(...)...end` -- a genuine
 *     OVERRIDE, not a bare field-reuse: `mz`'s own `hq` body starts by
 *     calling `bv(self,...)` directly, then does MORE work of its own) is
 *     NOT a `bm` name -- reached only through `mv()`'s own unrestricted
 *     field lookup, at the real call site already read for Chunk 10
 *     (`ui:bh()`'s own `mv(held,"hq",self.qa,je.ra)`, argc=2). Wired here
 *     as a new, `mz`-specific `pico8_mz_hq_dispatch` bridge (this file's own
 *     new work, following `pico8_bg_hq_dispatch`'s established 2-arg-bridge
 *     shape, `pico8_bg.c`) around a new `pico8_mz_hq` body that itself calls
 *     `pico8_bv` directly (a known, already-built function -- not a second,
 *     duplicated reimplementation of `bv`'s own logic).
 *   - `pq` (`function mz:pq()...end`) is the class system's own dedicated
 *     "extra constructor" slot (`pico8_class_pq_fn`, `pico8_class.h`),
 *     wired via `pico8_class_set_pq(g_mz_class, pico8_mz_pq)` -- never a
 *     `bm`/`mv()`/`dc()`-routed field at all, same treatment as `g_.pq=f_`
 *     (`pico8_probe.h`) except this body is genuinely NEW, not reused.
 *   - `ey` (`function mz:ey()...end`) is NOT a `bm` name and is a
 *     GENUINELY NEW method body (NOT `mz.ey=d_` -- there is no such field-
 *     reuse assignment anywhere in `mz`'s own real source, unlike `g_.ey=
 *     d_`) -- reached only through `mv()`'s unrestricted lookup, at the
 *     real call site already read for Chunk 10 (`ui:bh()`'s own
 *     `mv(held,"ey",nw and nw.kv)`, argc=1, unused by this body). `mz:ey`'s
 *     own body itself CALLS `d_` (`pico8_d_`, `pico8_probe.h`) directly as
 *     a plain function -- exactly the call `pico8_probe.h`'s own
 *     `pico8_d_` documentation already anticipated ("also called DIRECTLY
 *     as a plain function by `jc:ey()`... and by `mz:ey()`/`mz:hq()`
 *     (Chunk 16)").
 *   - `ej` (`function mz:ej(t)...end`) is a plain SAME-CLASS helper, never
 *     `bm`/`mv()`/`dc()`-routed at all -- its only real call sites are
 *     `self:ej(t)`/`self:ej(self.t-dt)` from `mz:pr`/`mz:kj` THEMSELVES.
 *     Ported as a direct, known, already-in-this-module C function
 *     (`pico8_mz_ej`), called directly by `pico8_mz_pr`/`pico8_mz_kj`, the
 *     exact same "known, single, already-built implementation, no dispatch
 *     table needed" treatment `pico8_ke_ks` already established
 *     (`pico8_ke.h`).
 *   - `kj` (`function mz:kj(p)...end`) IS one of `bm`'s 5 real dispatch
 *     names -- `mz`'s own draw method. Lands in `pico8_wormhole_display.h`/
 *     `.c` (this module's own display-concern sibling, matching
 *     `pico8_planet.c`/`pico8_planet_display.c`'s established split
 *     precedent), with its `pico8_mz_kj_dispatch` bridge staying HERE
 *     alongside `pico8_wormhole_init()`'s own class-registration wiring --
 *     the exact same "bridge lives with the registration code, body lives
 *     in the display sibling" split `pico8_jo_kj`/`pico8_jo_kj_dispatch`
 *     already established (`pico8_planet_display.h`'s own top comment).
 *     Real call convention confirmed against `dc`'s own literal source
 *     (pico8/lua.lua's `nt=="kj"` branch, `qc[nt](qc,qc.ra)`): `kj` is
 *     called with `p=self.ra` (a vec), matching `pico8_jo_kj`'s own
 *     identical `(gfx,mem,self,p)` signature shape exactly.
 *
 * *** SELF.ET REPRESENTATION -- A REAL, EVIDENCED FINDING, NOT A GUESS ***
 * `mz:hq`'s own `self.et=mz.jj` assigns a CLASS-LEVEL plain data table
 * (`mz.jj`, i.e. `mz`'s own `jj=o(qs="-too far-")` spec field, a bare
 * `pico8_ob_table_t*` once parsed -- NOT a `kv:jr` class instance) directly
 * onto `self.et`. But `self.et` is ALSO written by the REUSED `bv`
 * (`pico8_bv`, `pico8_probe.c`) with THREE possible other shapes (a real
 * `cg.np`/`cg.jc` member instance, a classless `{}` throwaway instance, or
 * literal `false`) -- and `pico8_probe.h`'s own top comment on `bv`
 * explicitly settled, for THAT function's own three cases, that "`self.et`
 * ALWAYS holds a `pico8_instance_t*` REF regardless of which term produced
 * it", specifically so `dl`'s own `self.et.qs` read (`pico8_dl`,
 * `pico8_probe.c`) can uniformly call `pico8_instance_get_field` on it.
 *
 * Reading `pico8_dl`'s ACTUAL current C body (not guessed at) confirms it
 * does exactly that, unconditionally: `pico8_instance_t *et_inst =
 * (et_val.kind == PICO8_VALUE_REF) ? (pico8_instance_t *)et_val.as.ref :
 * NULL; ... pico8_instance_get_field(et_inst, "qs", &qs_val)`. `mz.ev=dl`
 * wires this EXACT function as `mz`'s own "ev" field, reachable by a
 * future `dc("ev")` on any real `mz` instance. If `self.et` were set
 * directly to `mz.jj`'s own raw `pico8_ob_table_t*` (the reading this
 * chunk's own PRD text initially suggests -- "just store whatever kind of
 * REF mz.jj's own value actually is... into self.et as-is"), that cast
 * would reinterpret a `pico8_ob_table_t` (`{pico8_ob_entry_t *entries; int
 * count; int capacity;}`, `pico8_ob.h`) as a `pico8_instance_t`
 * (`{pico8_ob_table_t *fields; pico8_class_t *cls;}`, `pico8_class.h`) --
 * two STRUCTURALLY UNRELATED layouts. `et_inst->fields` would actually read
 * `mz.jj`'s own `entries` pointer (a `pico8_ob_entry_t*`, wrong pointer
 * type) as if it were a `pico8_ob_table_t*`, and `pico8_instance_get_field`
 * would then dereference THAT as a table, walking further into whatever
 * garbage/adjacent memory `mz.jj`'s own `count`/`capacity` ints happen to
 * alias on this platform -- undefined behavior, not a graceful "field
 * absent" no-op. This is a REAL representation mismatch, confirmed by
 * reading `pico8_dl`'s own shipped body directly (not assumed either way),
 * exactly the kind of discrepancy this chunk's own instructions asked to be
 * found and resolved with evidence rather than guessed past.
 *
 * RESOLUTION (this module's own new decision, scoped narrowly to this one
 * assignment): `mz.jj` is wrapped in a single, `mz`-class-lifetime-lived,
 * CLASSLESS `pico8_instance_t` (`cls=NULL`, matching `bv`'s own established
 * "throwaway/classless table -> classless instance" convention) whose own
 * `->fields` pointer IS `mz.jj`'s own real `pico8_ob_table_t*` -- BORROWED,
 * not copied, mirroring how real Lua's `self.et=mz.jj` shares the SAME
 * table object every other reader of `mz.jj` also sees (`mz.jj` is one
 * persistent Lua table, built once at class-declaration time, not a fresh
 * `{}` literal created anew per assignment the way `bv`'s own third
 * `and{}` term is) -- so this wrapper is built ONCE, in
 * `pico8_wormhole_init()`, and reused for every `self.et=mz.jj` assignment
 * across every `mz` instance's whole lifetime, never reallocated per call
 * (unlike `bv`'s own genuinely-fresh-per-call `{}` throwaway). With this
 * wrapper, `self.et` uniformly holds a real `pico8_instance_t*` REF exactly
 * as `pico8_dl`'s own existing, unmodified code already assumes -- ZERO
 * changes needed to `pico8_probe.c`/`pico8_dl` itself, and
 * `pico8_instance_get_field(et_inst,"qs",...)` correctly finds `mz.jj`'s
 * own real "qs"="-too far-" entry through the wrapper's borrowed `fields`,
 * producing the intended "-too far-" tooltip. `pico8_mz_jj_wrapper()`
 * (below) is this module's own private accessor for it; `pico8_wormhole_
 * shutdown()` frees ONLY the wrapper `pico8_instance_t` struct itself (a
 * plain `free()`, not `pico8_instance_free()`, which would incorrectly
 * also free the BORROWED `->fields` table that `mz`'s own class defaults
 * still own for the class's whole lifetime -- matching this port's
 * pervasive "nested class-spec structures are never explicitly deep-freed"
 * convention already established for `bg.pl`/`jo.dz`/`jo.cq`/`jo.ka`/etc.). */

void pico8_wormhole_init(void);
void pico8_wormhole_shutdown(void);

/* The real `mz` class, built once by pico8_wormhole_init(). Registers
 * itself under BOTH `pico8_class_lookup_named("mz")` (Chunk 7's own
 * registry, `pico8_class.h` -- this is what finally makes `bg:ldown`'s own
 * `(#(cg.g_ or {})>0 and mz or g_)()` forward reference resolve for `mz`
 * too, alongside `g_`'s own already-real resolution since Chunk 10) and the
 * shared ""-keyed `pico8_class_lookup()` registry (`mz`'s own spec never
 * sets an `fz` field, so it collides there exactly as documented, not
 * separately handled -- same as every other unnamed `:jr()` class).
 * Returns NULL before `pico8_wormhole_init()` (or after `pico8_wormhole_
 * shutdown()`). */
pico8_class_t *pico8_mz_class(void);

/* `mz(overrides)` -- the real Lua constructor call. Thin wrapper around
 * pico8_class_instantiate(). */
pico8_instance_t *pico8_mz_new(const pico8_ob_table_t *overrides);

/* This module's own private "mz.jj wrapped as a classless instance"
 * accessor -- see this file's own top "SELF.ET REPRESENTATION" section for
 * the full trace of why this exists and how it is built/freed. Not part of
 * the Lua original (a pure C representation-bridging device); exposed here
 * (rather than kept `static` in pico8_wormhole.c) only so
 * pico8_wormhole_display.c's own `pico8_mz_kj` -- which independently reads
 * `self.et`'s truthiness, though it never needs to walk through to `.qs`
 * itself -- can, if ever needed, recognize this exact wrapper pointer;
 * today `pico8_mz_kj` only checks `self.et`'s truthiness generically and
 * does not special-case it, so this accessor is not yet actually consulted
 * from that file, but is exposed on the same "make the real representation
 * inspectable across this module's own file split" principle the split
 * itself already relies on. Returns NULL before `pico8_wormhole_init()`. */
pico8_instance_t *pico8_mz_jj_wrapper(void);

/* function mz:pq()rn(53)self.ox=jp(hv,function(p)
 * return p.fx
 * end)end
 * (pico8/lua.lua:762-764.) `mz`'s own extra constructor: plays sfx 53
 * (`pico8_rn`, Chunk 1's inert stub, `pico8_audio.h`) then builds
 * `self.ox` -- the list of every CURRENTLY LIVE entity (`hv`, `pico8_hv()`,
 * `pico8_entity.h`) whose own `.fx` field is truthy. Grepping every real
 * `.fx` write site in `pico8/lua.lua` confirms this collects colonized
 * planets (`jo:hu`'s own `self.fx=true`), trade-linked slipways (`jc`'s own
 * `if(sw:eb(self).fx)self.fx=true`), and any live `g_` probe instance
 * (`g_`'s own class-level default `fx=1`, always truthy) -- i.e. every
 * "already-established, already-reachable" point in the current game state
 * a wormhole's own nearest-neighbor search (`mz:hq`, below) can target.
 * `jp`'s own predicate (`pico8_jp_predicate_fn`, `pico8_collect.h`) reads
 * each candidate's own "fx" field via `pico8_instance_get_field` (every
 * `hv` element is a real `pico8_instance_t*` REF, `pico8_entity.h`'s own
 * documented `hv` shape -- never a raw table) and applies this port's own
 * established truthy rule (falsy only for `PICO8_VALUE_BOOL(0)` or
 * `PICO8_VALUE_REF(NULL)`; a real FIXED 0 -- never actually produced by any
 * real `.fx` writer above, all of which write `true`/`1` -- would still
 * count as truthy here, matching this port's pervasive convention).
 *
 * The result (a fresh, heap-backed `pico8_list_t` from `pico8_jp`) is
 * stored via the SAME "malloc a `pico8_list_t`, copy the by-value result
 * in, store its REF" pattern already established by `pico8_trade.c`'s own
 * `pico8_trade_kg` (`new_lp`/`new_rl`) -- `self.ox` OWNS this heap list for
 * the rest of the instance's own lifetime (never freed here, matching this
 * port's pervasive "the surviving structure leaks, like Lua's own
 * GC-deferred collection" convention; `pq` itself is only ever invoked
 * once per instance, at construction time, by `pico8_class_instantiate`'s
 * own pq-walk, so there is no "re-run leaks the old one" concern to
 * resolve here at all -- unlike, say, a per-frame field rebuild would be).
 *
 * Wired via `pico8_class_set_pq(g_mz_class, pico8_mz_pq)` -- matches
 * `pico8_class_pq_fn`'s own `void(*)(pico8_instance_t*)` signature exactly,
 * no adapter needed (the same "already the right shape" treatment
 * `pico8_f_`'s own reuse for `g_.pq=f_`/`hj.pq=f_` already established,
 * `pico8_particle.h`). A NULL `self` is a safe no-op. */
void pico8_mz_pq(pico8_instance_t *self);

/* function mz:hq(...)bv(self,...)local nq
 * nq,self.ij=ij(self.p,self.ox)if nq>ov("space folding",0.19,0.27)then
 * self.et=mz.jj
 * end
 * end
 * (pico8/lua.lua:765-770.) A genuine OVERRIDE of `bv` (`mz.hq=bv` PLUS this
 * own `function mz:hq`, both present in the real source -- Lua's own
 * `function mz:hq(...)` REBINDS the "hq" field a second time, so the
 * `mz.hq=bv` line 3 statements earlier is entirely superseded the moment
 * this definition runs; only THIS body is ever actually reachable on a
 * real `mz` instance). Called every frame the wormhole event is being
 * dragged (`ui:bh()`'s own `mv(held,"hq",self.qa,je.ra)`, same real call
 * site/argument-order precedent as `g_.hq=bv`/`qh.hq=bv`, `pico8_probe.h`).
 *
 * `bv(self,...)`: calls the already-built `pico8_bv` (`pico8_probe.h`)
 * directly with `self`/`mp`/`rp` forwarded as-is -- this sets `self.ra`,
 * `self.p` and a FIRST candidate `self.et` exactly as documented on
 * `pico8_bv`'s own declaration, entirely unmodified by this override.
 *
 * `local nq nq,self.ij=ij(self.p,self.ox)`: a SECOND, independent nearest-
 * neighbor search (`pico8_ij`, `pico8_entity_geom.h`) over `self.ox` (this
 * instance's own `pq`-built list of "already-established" `.fx`-truthy
 * points, above) using `self.p` (just set by `bv`, a heap vector).
 * `self.ij` is a real INSTANCE FIELD NAME here (note this shares its
 * spelling with the free function `ij` -- real Lua allows this since a
 * table field and a global function live in entirely separate namespaces;
 * C has no such collision at all, since `self->fields`'s "ij" entry and
 * the C function `pico8_ij` are simply different symbols) -- written with
 * whatever `pico8_ij`'s own `*out_nearest` produced (a real `pico8_
 * instance_t*` REF, since every `self.ox` member is itself a real
 * instance, per `pico8_mz_pq`'s own comment above -- never `mz.jj`'s own
 * raw-table representation, so no wrapper is needed for THIS particular
 * field write, only for `self.et` below).
 *
 * `if nq>ov("space folding",0.19,0.27)then self.et=mz.jj end`: `ov`
 * (Chunk 11's already-built `pico8_ov`, `pico8_tech_gate.h`) computes a
 * "too far" SQUARED-distance threshold that widens from 0.19 to 0.27 once
 * the "space folding" tech is unlocked (today, with `fm` still a
 * placeholder with no "invented" list at all, `pico8_oj`'s own `gu(fm.
 * invented,px)` always reads false, so this always evaluates to the lower
 * 0.19 bound -- the same "compiles, not yet exercised past its own
 * documented default" status every other `ov`/`oj`/`fm`-touching call in
 * this port already has, `pico8_tech_gate.h`'s own top comment). If the
 * nearest `.ox` candidate is farther than that threshold, `self.et` is
 * OVERWRITTEN (regardless of whatever `bv` itself just set it to,
 * including a real target `bv` may have found) with `mz.jj` -- see this
 * file's own top "SELF.ET REPRESENTATION" section for the full trace of
 * how this is represented (the class-lifetime-cached wrapper instance,
 * `pico8_mz_jj_wrapper()`) and why. If `nq<=` the threshold, `self.et`
 * keeps whatever `bv` already set it to (no `else` branch in the real
 * source -- ported as a plain "only write on the `if` branch" C
 * conditional, no `else` needed).
 *
 * Wired as a NEW `pico8_mz_hq_dispatch` bridge (`pico8_wormhole.c`,
 * following `pico8_bg_hq_dispatch`'s own established 2-arg-bridge shape)
 * on `mz`'s own class defaults under "hq" -- NOT a `bm` name, `mv()`-routed
 * only, same classification `g_.hq=bv` already established. A NULL `self`
 * is a safe no-op (via `pico8_bv`'s own existing NULL guard; this
 * function's own remaining work also reads every field defensively). */
void pico8_mz_hq(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp);

/* function mz:kz(t)self.oi=ov("space folding",29,35)*
 * (min(t/15,1)+sin(t/40)*0.1)end
 * (pico8/lua.lua:771-772.) The wormhole's own "kz" (initial/approaching)
 * state tick, dispatched every frame by `pico8_dg()`'s own generic
 * self.me-named mechanism (see this file's own top comment on the `kz`/
 * `pr` dispatch classification) while `self.me=="kz"` (the class default,
 * until `d_` transitions it to `self.dq`="pr" once a target is committed).
 * Grows `self.oi` (the expanding-circle radius `mz:kj` draws, below) from
 * 0 up toward `ov("space folding",29,35)` (the same tech-gated "space
 * folding" threshold as `mz:hq` above, but a DIFFERENT pair of bounds
 * here -- 29/35 vs. 0.19/0.27 -- confirmed by reading the real source
 * literally rather than assuming the two `ov()` calls share one constant;
 * today, `fm` still a placeholder, this always evaluates to the lower `29`
 * bound), following an ease-in-then-wobble curve: `min(t/15,1)` ramps
 * linearly from 0 to 1 over the first 15 ticks then holds at 1 (`min`
 * ported as a plain C ternary on the raw `pico8_fixed_t`, matching
 * `pico8_ma`'s/`pico8_gi`'s own identical `max(...)` precedent,
 * `pico8_entity_geom.h`/`pico8_tech_gate.h` -- no shared `min` helper
 * exists anywhere in this port yet either, same "not a third independent
 * occurrence yet" reasoning), plus a small persistent `sin(t/40)*0.1`
 * wobble (`pico8_fixed_sin`, never damped out, matching the real source's
 * own lack of an envelope on this term). A NULL `self` is a safe no-op. */
void pico8_mz_kz(pico8_instance_t *self, pico8_fixed_t t);

/* function mz:ey()self.p_=
 * self.t<15 or not d_(self)end
 * (pico8/lua.lua:773-774.) A GENUINELY NEW `ey` body (see this file's own
 * top comment's dispatch classification -- NOT `mz.ey=d_`, no such
 * assignment exists in the real source). Real call site: `ui:bh()`'s own
 * `mv(held,"ey",nw and nw.kv)` (argc=1, its own single extra argument never
 * referenced by this body, matching `g_.ey=d_`'s own identical unused-
 * argument treatment, `pico8_d__dispatch`).
 *
 * `self.t<15 or not d_(self)`: Lua's `A or B` idiom where BOTH operands are
 * genuine booleans, not table-valued idiom terms (unlike `bv`'s own `A and
 * B or C and D or E and F` chain, `pico8_probe.h`) -- `A` (`self.t<15`,
 * `self.t` being `dg()`'s own per-tick counter, `ke`'s own default 0) is a
 * real boolean; when `A` is true, Lua's `or` short-circuits and the WHOLE
 * expression's value is `true` itself (never evaluating `d_(self)` at
 * all -- ported as a genuine early-exit, not merely evaluate-and-discard,
 * so `d_`'s own real side effects, e.g. `fm:lu`/`fm:nx`/`rn`, `self.p_=
 * true`/`self:ks(...)`, are correctly skipped entirely during the
 * wormhole's own first 15 ticks); when `A` is false, the expression's
 * value becomes `not d_(self)` (`pico8_d_`, `pico8_probe.h`, called
 * DIRECTLY as a plain function -- exactly the call that file's own header
 * comment on `pico8_d_` already anticipated by name, "also called...by
 * `mz:ey()`/`mz:hq()` (Chunk 16)" -- `not` applies this port's own
 * established truthy rule to `pico8_d_`'s own result, which is always
 * either `PICO8_VALUE_BOOL(1)` (target committed) or `pico8_value_ref
 * (NULL)` (Lua nil, the implicit-return "else" branch) -- so `not` is
 * always the LOGICAL NEGATION of "did `d_` succeed").
 *
 * `self.p_=...`: writes a real Lua boolean (never Lua nil -- both operands
 * of this specific `or` are booleans, unlike `bv`'s own table-valued `and/
 * or` chain) onto `self.p_`, the self-destruct flag `pico8_dg()`'s own
 * `if qc.p_ then he(qc) end` reads every tick (`pico8_bootstrap.c`) -- so a
 * wormhole self-destructs either before its own 15th tick has even STARTED
 * committing to a target, or the instant `d_` itself fails to commit one
 * (mirroring `g_`'s own probe self-destructing on the same `d_`-failure
 * path, `pico8_d_`'s own `self.p_=true` else-branch write). Wired as a NEW
 * `pico8_mz_ey_dispatch` bridge (`pico8_wormhole.c`) on `mz`'s own class
 * defaults under "ey" -- NOT a `bm` name, `mv()`-routed only. A NULL `self`
 * is a safe no-op. */
void pico8_mz_ey(pico8_instance_t *self);

/* function mz:pr(t)local sr=self:ej(t)/128+0.031
 * for pt in all(cg.jo)do
 * if pt.me=="unknown"and (pt.p-self.p):qx()<sr then
 * pt:hh()end
 * end
 * self.p_=t>25
 * end
 * (pico8/lua.lua:775-781.) The wormhole's own "pr" (post-commit) state
 * tick -- `self.dq="pr"` (`mz`'s own class default) is exactly what `d_`'s
 * own `self:ks(self.dq or "ph")` transitions `self.me` to once a target is
 * committed, so `pico8_dg()`'s own generic self.me-named dispatch (this
 * file's own top comment) reaches THIS function every tick from then on,
 * in place of `kz` above.
 *
 * `local sr=self:ej(t)/128+0.031`: a SAME-CLASS direct call to `self:ej`
 * (`pico8_mz_ej`, below -- a known, single, already-in-this-module
 * implementation, no dispatch table, matching `pico8_ke_ks`'s own
 * established "direct call" treatment) converts the current expanding-
 * circle radius (in screen-pixel-ish units, per `mz:kj`'s own `circ(...,
 * self.oi,...)` usage) back down to the SAME `/128` map-tile scale every
 * other map-coordinate quantity in this cart already uses (matching
 * `pico8_bv`'s own identical `mp/128` scale-down, `pico8_probe.c`), plus a
 * small constant pixel-to-map-unit fudge (`+0.031`) -- `sr` is therefore a
 * map-space "reveal radius" that grows over time as the wormhole's own
 * visual expands.
 *
 * `for pt in all(cg.jo)do if pt.me=="unknown"and(pt.p-self.p):qx()<sr then
 * pt:hh()end end`: walks every `jo`-tagged (and subtype) planet instance
 * (`cg.jo`, `pico8_cg("jo")`, `pico8_entity.h` -- real since Chunk 13),
 * revealing (`pt:hh()`, `pico8_jo_hh`, `pico8_planet.h`, already built)
 * every still-`"unknown"` planet (`pt.me`, compared by STRING CONTENT,
 * matching `jo:gg`/`jo:gv`/`bg:ldown`'s own established `strcmp`
 * treatment) that lies within `sr` map-units of the wormhole's own current
 * position (`(pt.p-self.p):qx()`, real EUCLIDEAN distance -- `pico8_vec_
 * sub` then `pico8_vec_qx`, `pico8_vec.h`'s own sqrt-of-squared-length
 * method -- NOT the squared-distance `#(...)`/`pico8_vec_len_sq` shortcut
 * `ij`/`bv` use elsewhere, confirmed by reading this exact line: `:qx()`,
 * not `:rg(...)` or bare `#`). `pt:hh()` is called directly (`:` method-
 * call sugar on a concrete, already-known-class receiver, same treatment
 * `jo:gv`'s own `self:gh()` call already established, `pico8_planet_
 * display.h`) -- NOT through `mv()`, since `hh` is never itself a
 * dynamically-named field access here.
 *
 * `self.p_=t>25`: a real Lua boolean (a plain comparison, never a table-
 * valued `and/or` idiom) -- the wormhole self-destructs 25 ticks after
 * entering the "pr" state (i.e. `self.t` resets to 0 on the `self:ks("pr")`
 * transition, per `ke:ks`'s own documented `self.me,self.t=me,0`, so `t>25`
 * here measures ticks-since-commit, not ticks-since-spawn).
 *
 * Wired as a NEW `pico8_mz_pr_dispatch` bridge (`pico8_wormhole.c`,
 * argc=1, `arg0`=`t`, same shape as `kz`'s own bridge above) on `mz`'s own
 * class defaults under "pr". A NULL `self` is a safe no-op. */
void pico8_mz_pr(pico8_instance_t *self, pico8_fixed_t t);

/* function mz:ej(t)
 * return min(sqrt(t/15),1)*self.oi
 * end
 * (pico8/lua.lua:794-796.) A plain, SAME-CLASS-only helper (never `bm`/
 * `mv()`/`dc()`-routed -- its only two real call sites are `self:ej(t)`/
 * `self:ej(self.t-dt)` from `mz:pr`/`mz:kj` themselves, this file's own top
 * comment). Computes a "reveal front" radius at elapsed time `t`: a
 * `sqrt`-eased ramp from 0 to 1 over the first 15 ticks (`min(sqrt(t/15),
 * 1)`, ported the same plain-C-ternary way as `mz:kz`'s own `min` above),
 * scaled by `self.oi` (the SAME expanding-circle radius `mz:kz` itself
 * grows every tick) -- so `mz:ej`'s own result is always bounded by
 * `self.oi`, reaching it exactly once `t>=15`. Ported as a direct,
 * known C function (`pico8_mz_ej`), called directly by `pico8_mz_pr`
 * and `pico8_mz_kj` (`pico8_wormhole_display.c`) -- no dispatch bridge. A
 * NULL `self` reads `self.oi` as its documented default (0), matching this
 * port's pervasive defensive convention. */
pico8_fixed_t pico8_mz_ej(pico8_instance_t *self, pico8_fixed_t t);

#endif
