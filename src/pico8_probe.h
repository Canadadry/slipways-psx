#ifndef PICO8_PROBE_H
#define PICO8_PROBE_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Phase 3, Chunk 10 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): the standalone handlers `bv`, `dl`, `d_` (pico8/lua.lua:552-
 * 576) plus the `g_` class itself (pico8/lua.lua:708-731) -- the probe
 * entity the player launches from `bg:ldown` (Chunk 7) to explore toward a
 * wormhole/planet. Confirmed by reading the real source (`g_=ke:jr(...)`,
 * line 708) to be exactly what the PRD's own paraphrase says. New module
 * (not pico8_entity.c/pico8_ui.c/pico8_msgbox.c -- all at/over the flagged
 * ~430-540-line range, see docs/portage-progress.md's Vague 6/8 checks; not
 * pico8_particle.c/.h either, Chunk 9's own module, since `g_` REUSES that
 * chunk's already-built `f_`/`ei` verbatim rather than defining new bodies
 * for them -- adding a class declaration here would mix "shared particle
 * helpers with no class of their own" with "the first class that actually
 * uses them", two distinct concerns per this port's own module-per-concern
 * convention).
 *
 * g_=ke:jr([[
 * nu=o("g_","prober"),
 * fx=1,
 * dq="ly",
 * od=0,fp=f,
 * fa=2,
 * dw=0.025,
 * py=0.06,ki=9,
 * pn=3,pk=1,
 * fc=1,
 * k_=o(1,1,5,13,6),
 * qo=1.02,ro=-0.0525,
 * qg=0.0525,q_=1.02,
 * du=2,
 * ]])g_.hq=bv
 * g_.ey=d_
 * g_.ev=dl
 * g_.pq=f_
 * g_.kj=ei
 * function g_:mm()local a=(flr(rnd(self.pn))+rnd(0.5))/self.pn+self.t*0.001
 * local p=qm(self.du+rnd(),a)local v=p*self.py
 * return {p=p,v=self.pk and v:pa()or v,l=self.ki
 * }
 * end
 * (pico8/lua.lua:708-731.)
 *
 * `nu=o("g_","prober")` tags every instance with BOTH "g_" (read by
 * `bg:ldown`'s own `cg.g_` check, Chunk 7) and "prober" (no real reader
 * found in this chunk's own scope -- kept, not guessed at). `fp=f` -- `f`
 * is ob()'s own "bare identifier `f` means literal false" token (see
 * pico8_ob.c's own comment on `sc!="f"`), so `fp` genuinely DEFAULTS TO
 * `false`, not to a number -- `d_`'s own `self.fp or 0` below only takes
 * that `false` branch until a real caller (Chunk 12+, out of scope) ever
 * overrides `fp` with a real value.
 *
 * WHICH FIELDS ARE REAL `bm`-DISPATCH NAMES VS PLAIN mv()-ROUTED FIELDS
 * (per this chunk's own instruction to check this explicitly, not assume
 * it): `bm`'s real 5 names (pico8_entity.c's own `bm` literal) are
 * "kj","ev","hd","hk","hb". Of `g_`'s 5 field assignments:
 *   - `kj=ei` and `ev=dl` ARE `bm` names -- reached automatically by
 *     `dc("kj")`/`dc("ev")` (a future `_draw()`, Chunk 22) in addition to
 *     ordinary `mv()` lookup, exactly like `bg:kj`/`js:ev`'s own precedent.
 *   - `hq=bv` and `ey=d_` are NOT `bm` names -- reached ONLY through `mv()`'s
 *     own unrestricted by-name field lookup, at real call sites already read
 *     for Chunk 5 (`ui:bh()`'s `mv(held,"hq",self.qa,je.ra)`, argc=2 --
 *     this PRD's own Problem Statement even cites `ui:bh()`'s sibling
 *     `mv(held,"ey",nw and nw.kv)`, argc=1, as its own evidence for the
 *     1-argument `mv()` arity), the exact same treatment `bg:ldown`/`bg:
 *     rdown`/`bg:hq`'s own "NOT bm names but still real PICO8_VALUE_FUNCTION
 *     fields, dispatched via mv()'s unrestricted lookup" precedent already
 *     established (pico8_bg.h).
 *   - `pq=f_` is neither a `bm` name nor an `mv()`-routed field at all: `pq`
 *     is the class system's own dedicated "extra constructor" slot
 *     (pico8_class_pq_fn, pico8_class.h), invoked automatically by
 *     pico8_class_instantiate() at construction time, never through dc()/
 *     mv(). Since Chunk 9's own `f_` already has EXACTLY `pico8_class_pq_fn`'s
 *     signature, this wires as a direct `pico8_class_set_pq(g_class,
 *     pico8_f_)` call, no adapter -- see pico8_particle.h's own top comment,
 *     confirmed here rather than re-derived.
 *   `g_:mm` (this chunk's own genuinely NEW method, not a field-assignment
 *   reuse) is likewise not a `bm` name; it is reached only through `ez`'s own
 *   `self:mm()` -> pico8_mv(self,"mm",0,...) call (Chunk 9), so it is wired
 *   as a real PICO8_VALUE_FUNCTION field ("mm") on `g_`'s own class defaults,
 *   the same "mv()-routed, non-bm field" treatment as `hq`/`ey` above.
 *
 * DOUBLE UNDERSCORE, DELIBERATE, NOT A TYPO: `g_`'s own accessor family
 * below is named `pico8_g__init`/`pico8_g__shutdown`/`pico8_g__class`/
 * `pico8_g__new` -- the exact minified class name ("g_") plus this port's
 * own established `_init`/`_shutdown`/`_class`/`_new` accessor suffixes
 * (pico8_bg_class/pico8_mh_class/pico8_qb_class's own precedent), which
 * happens to concatenate into a double underscore. This is fully legal C99
 * (the reserved-identifier rule only forbids identifiers that START with
 * two underscores, or with an underscore followed by an uppercase letter --
 * "pico8_g__class" starts with 'p'), and keeping the exact minified name as
 * the accessor's own root (rather than inventing a different-looking base,
 * e.g. "probe") is what lets a reader searching for "g_" find every one of
 * its own C symbols directly, the same way "pico8_bg_class" is found by
 * searching "bg". The FILE this lives in is still named for the class's
 * real-world role (`pico8_probe.c`/`.h`, "the probe entity" -- this PRD's
 * own paraphrase), matching pico8_msgbox.c/pico8_scroll.c's own precedent
 * of a thematic file name distinct from the C symbols' own literal-minified-
 * name roots.
 *
 * `fm` FORWARD REFERENCE (`bv`'s `not ui.hw(rp)` aside, `d_`'s own
 * `fm:lu(self.od)`/`fm:nx(self.fp or 0)`): `fm` (Chunk 12's `hy` economy
 * singleton) does not exist yet. Unlike `bg:ldown`'s `mz`/`g_` forward
 * reference (a DYNAMIC choice between two possible class NAMES, resolved
 * via the new pico8_class_lookup_named() registry, pico8_class.h), `fm` is
 * a single, specific, already-named Lua GLOBAL VARIABLE -- structurally the
 * exact same shape as Chunk 2/4's own `mo`/`je` forward references before
 * Chunk 6 made them real: a bare classless PLACEHOLDER instance the calling
 * code can safely read/dispatch-through today (via ordinary `pico8_mv()`
 * lookup, which already tolerates "field not found on a classless instance"
 * as its own documented "absent" case), reconciled to a REAL `hy()` instance
 * later. Following pico8_mo()/pico8_je()/pico8_ok()/pico8_kc()'s own get/set
 * accessor-pair convention exactly (never a raw extern):
 *   pico8_instance_t *pico8_fm(void);
 *   void pico8_probe_set_fm(pico8_instance_t *instance);
 * `pico8_fm()` is built as a classless placeholder (`cls=NULL`, a fresh
 * empty `fields` table -- byte-for-byte the same construction
 * pico8_entity.c's own pico8_entity_init() already uses for `g_mo`) inside
 * `pico8_probe_init()` below, freed by `pico8_probe_shutdown()`. This
 * accessor pair is declared HERE (not pico8_entity.c/pico8_ui.c/
 * pico8_msgbox.c, this chunk's own instruction: all three are at/over the
 * flagged ~430-540-line range already) -- the same "new chunk's own module
 * hosts a forward-referenced singleton's placeholder when every existing
 * candidate module is already oversized" precedent Chunk 5 already
 * established for `pico8_je()` living in its own new `pico8_ui.c` rather
 * than the already-oversized `pico8_entity.c`.
 *
 * *** CHUNK 12 RECONCILIATION NOTE, mirroring Chunk 5's own `pico8_je()`
 * placeholder note verbatim in spirit *** -- when Chunk 12 builds the real
 * `hy` class and its `fm=hy()` construction (per this PRD's own
 * "Singleton globals ownership" Implementation Decision, alongside `eu()`,
 * Chunk 22's own module), it must call `pico8_probe_set_fm(pico8_class_
 * instantiate(hy_class, NULL))` (or equivalent) to REPLACE this module's
 * own placeholder -- `pico8_fm()`'s own declaration/signature here never
 * changes, so every existing real caller (`pico8_d_` below, and any future
 * one) keeps working unmodified, exactly like `pico8_entity_set_mo()`'s own
 * reconciliation of `pico8_mo()`. Chunk 12 must NOT declare a second,
 * competing `pico8_fm()` accessor anywhere else -- this is the one and only
 * home for it. */

void pico8_probe_init(void);
void pico8_probe_shutdown(void);

/* The real `g_` class, built once by pico8_probe_init(). Registers itself
 * under BOTH pico8_class_lookup_named("g_") (per Chunk 7's own registry,
 * pico8_class.h -- this is what finally makes `bg:ldown`'s forward
 * reference to `g_` resolve for real) and the shared ""-keyed
 * pico8_class_lookup() registry (like every other unnamed `:jr()` class so
 * far -- `g_`'s own spec never sets an `fz` field, so it collides there
 * exactly as documented, not separately handled). Returns NULL before
 * pico8_probe_init() (or after pico8_probe_shutdown()). */
pico8_class_t *pico8_g__class(void);

/* `g_(overrides)` -- the real Lua constructor call (`bg:ldown`'s own
 * `g_()`, no overrides, pico8/lua.lua:377). Thin wrapper around
 * pico8_class_instantiate(). */
pico8_instance_t *pico8_g__new(const pico8_ob_table_t *overrides);

/* `fm` -- see this file's own top comment for the full forward-reference
 * trace and the Chunk 12 reconciliation note. */
pico8_instance_t *pico8_fm(void);
void pico8_probe_set_fm(pico8_instance_t *instance);

/* function bv(self,mp,rp)self.ra,self.p=mp,mp/128
 * local nq,c=ij(self.p,cg.np)self.et=
 * nq<self.dw and c or
 * jp(cg.jc,function(sw)
 * return be(sw.qu.p,sw.re.p,self.p,0.0032)end)[1]or
 * not ui.hw(rp)and {}
 * if (self.et)self.et.fo=self
 * end
 * (pico8/lua.lua:552-559.) Called every frame the probe is being dragged
 * (`g_.hq=bv`, an mv()-routed non-bm field -- see this file's own top
 * comment): `mp` is the world-space mouse position (`self.qa`), `rp` the
 * raw screen-space one (`je.ra`), matching `bg:hq`'s own established
 * argument-order precedent for this exact `mv(held,"hq",self.qa,je.ra)`
 * call shape.
 *
 * `self.ra,self.p=mp,mp/128`: `self.ra` is REBOUND to the exact same
 * pico8_value_t `mp` the caller passed (a field-value copy, matching
 * `bg:rdown`'s own `self.mq=rp` precedent -- `mp`'s own REF identity/
 * lifetime is preserved, not deep-copied); `self.p` becomes a FRESH heap
 * vector, `mp`'s own vector value divided by the fixed constant 128 (world-
 * to-map-tile-scale, matching every other `/128` scale factor already seen
 * in this cart's own map-coordinate conventions).
 *
 * `local nq,c=ij(self.p,cg.np)`: nearest-neighbor search (Chunk 1's
 * pico8_ij) over `cg.np` (pico8_cg("np") -- a planet tag this port hasn't
 * built a real writer for yet, Chunk 13's `jo` class; empty/NULL today, so
 * `nq` stays ij()'s own sentinel-derived huge distance and `c` stays nil,
 * matching the "compiles, not yet meaningfully exercised" treatment every
 * other forward-referenced tag list already gets in this port).
 *
 * `self.et=nq<self.dw and c or jp(cg.jc,fn)[1]or not ui.hw(rp)and{}`: Lua's
 * `A and B or C and D or E and F` groups as `(A and B) or (C and D) or (E
 * and F)` (`and` binds tighter than `or`) -- three short-circuited terms,
 * evaluated left to right, the first TRUTHY one (Lua's own nil/false-only-
 * falsy rule) becomes `self.et`:
 *   1. `nq<self.dw and c` -- if the nearest `cg.np` entity is within
 *      `self.dw` (0.025 default), the term's value IS `c` itself (which can
 *      itself be falsy/nil if `cg.np` was empty -- Lua's `and` returns its
 *      second operand's own value, even if falsy, whenever the first is
 *      truthy, so a truthy `nq<self.dw` with a nil `c` correctly falls
 *      through to term 2, not treated as "term 1 matched").
 *   2. `jp(cg.jc,fn)[1]` -- Chunk 1's pico8_jp filtering `cg.jc` (a slipway
 *      tag, Chunk 14's `jc` class -- empty today) by the closure `function
 *      (sw)return be(sw.qu.p,sw.re.p,self.p,0.0032)end` (Chunk 1's pico8_
 *      geom_be, testing whether `self.p` lies near the segment between the
 *      two planets `sw` connects, `sw.qu`/`sw.re` -- both `jc`-class fields
 *      that don't exist until Chunk 14 either; read defensively as absent,
 *      same "compiles, not yet exercisable" treatment), then takes the
 *      FIRST match (index 1) or nil if none.
 *   3. `not ui.hw(rp) and {}` -- `ui.hw` (the `ui` CLASS's own `hw=b(10,10,
 *      118,118)` spec field, a qz() hit-box predicate -- read directly off
 *      pico8_ui_class()'s own `defaults` table, matching `bg:pq`'s own
 *      precedent for reading a class's own default field rather than an
 *      instance's) tested against `rp`: if `rp` is OUTSIDE the main UI
 *      panel, `not ui.hw(rp)` is true and the term's value is a BRAND NEW
 *      empty Lua table (`{}`); if `rp` IS inside the panel, the term's value
 *      is `false` itself (Lua's `and` short-circuits without ever building
 *      `{}` when its first operand is already falsy) -- i.e. this specific
 *      term can genuinely produce a real, observable `false`, not just nil,
 *      when nothing else matched and the click was inside the UI panel. If
 *      `pico8_ui_class()` itself is NULL (ui not yet built) or its own `hw`
 *      field is absent/wrong-kind, `ui.hw(rp)` is treated as false (the
 *      same "missing predicate reads as not-satisfied" divergence already
 *      established by `gk`'s own `e.kq`-absent case, pico8_entity_geom.h).
 *   A THROWAWAY LUA `{}` HAS NO METATABLE, i.e. no class -- exactly the
 *   representation Phase 2 already chose for `pico8_mo()`'s own classless
 *   placeholder. So this port represents term 3's `{}` the SAME way: a
 *   fresh, classless `pico8_instance_t` (cls=NULL, empty fields table), not
 *   a bare `pico8_ob_table_t*`. This is a deliberate, documented choice
 *   (not evidenced by any single real Lua line, since C has no single
 *   "any table" type the way Lua does) that lets `self.et` ALWAYS hold a
 *   `pico8_instance_t*` REF regardless of which of the three terms produced
 *   it (a real `cg.np`/`cg.jc` member, or this throwaway), so `if (self.et)
 *   self.et.fo=self` below can uniformly write through `->fields` either
 *   way, without a second, incompatible "raw table" representation to
 *   special-case.
 *
 * `if (self.et)self.et.fo=self`: only when `self.et` is genuinely truthy
 * (a real target OR the `{}` throwaway -- NOT term 3's own possible bare
 * `false`) does the target get its own `.fo` ("found by") field set back to
 * this probe -- read later by `dl`'s own `self.et.qs` lookup (a target's
 * status text) and, per Chunk 9's own particle-header comment, by other not-
 * yet-ported code. A NULL `self` is a safe no-op for the whole function. */
void pico8_bv(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp);

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape to
 * pico8_bv's own signature; `ui:bh()`'s own `mv(held,"hq",self.qa,je.ra)`
 * dispatch passes argc=2 every frame the probe is held, matching `bg_hq_
 * dispatch`'s own identical precedent (pico8_bg.c). `bv` itself returns
 * nothing (nil) in the real source, so this bridges to `pico8_value_ref
 * (NULL)`, the same "void real function -> ref(NULL) bridge" convention
 * `pico8_bg_hq_dispatch`/`pico8_bg_kj_dispatch` already established. */
pico8_value_t pico8_bv_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* function dl(self)if self.me=="kz" then
 * local mp,t=je.ra
 * if self.et then
 * t=self.et.qs
 * if (not t)spr(31,mp.x-3,mp.y-8)
 * elseif self.od then
 * t=self.od.."$"end
 * io(t or "",mp.x,mp.y-8,8,0.5)end
 * end
 * (pico8/lua.lua:560-567.) A generic "draw a small tooltip near the mouse"
 * helper, NOT specific to probes despite living alongside them here --
 * confirmed by grepping every real call site: `g_.ev=dl` (this chunk, a
 * REAL `bm`-name field, auto-dispatched by a future `dc("ev")`), but also
 * `dl(self)` called DIRECTLY as a plain function by `jc:hh()` (Chunk 14,
 * pico8/lua.lua:683) and `dl(self.ke)` by `qh:ph()` (Chunk 15, line 938) --
 * on entities that are never `g_` instances. So, exactly like Chunk 9's
 * `ei`/`pico8_ei`+`pico8_ei_dispatch` split, this ports as BOTH a plain,
 * directly-callable function (explicit `gfx`/`mem` parameters, needed for
 * `spr()`/`io()`) and a `pico8_value_function_t`-shaped bridge reading the
 * shared per-frame dispatch context (pico8_dispatch_context.h) for `g_.ev`'s
 * own real dc()/mv() dispatch path.
 *
 * `if self.me=="kz" then ... end`: only draws anything when the entity's
 * own icon/state name (`ke:ks`'s own `self.me`, Chunk 3) is literally "kz"
 * -- a state this chunk's own scope never sets (only `mz:kz`, Chunk 16,
 * does), so this whole function is a documented no-op for every instance
 * constructible in this chunk's own scope, the same "compiles, not yet
 * meaningfully exercised" status as every other forward-reference-adjacent
 * function in this PRD.
 *
 * `local mp,t=je.ra`: `mp` is the mouse-reader singleton's own screen
 * position (`pico8_je()`, Chunk 6 -- read defensively as a zero vector
 * before Chunk 6 ever runs, same treatment as every other `je`/`mo` read in
 * this port); `t` starts nil (Lua's own multi-assignment with fewer values
 * than names pads the rest with nil, the SAME subtlety `mh:pq`'s own
 * `self.p_,kc=true` already documents).
 *
 * `if self.et then t=self.et.qs if (not t)spr(31,mp.x-3,mp.y-8)end elseif
 * self.od then t=self.od.."$" end`: if a drag target is set (`self.et`,
 * from `bv` above -- read here via the SAME "classless-instance-or-real-
 * entity" REF representation `bv` establishes), `t` becomes the target's
 * own `.qs` field (a status string, Chunk 13+, not yet built -- absent
 * reads as nil here); if THAT is itself falsy/absent, a placeholder "?"
 * icon (sprite 31) is drawn instead, and `t` stays nil. Otherwise (no
 * `self.et` at all), if `self.od` (the research-project code/cost this
 * probe is chasing, default fixed 0 -- see this file's own top comment on
 * why 0 is TRUTHY in Lua, unlike `fp`'s own `f`/false default) is present,
 * `t` becomes `tostring(self.od)` (Lua's `..` auto-coerces a number
 * operand to its decimal string; ported by reusing `pico8_value_tostr`,
 * pico8_value.h, rather than duplicating number-to-string formatting)
 * concatenated with "$".
 *
 * `io(t or "",mp.x,mp.y-8,8,0.5)`: outlined-text-draws whatever `t` ended
 * up as (or an empty string if it's still nil), 8px above the mouse, in
 * color 8, with `io`'s own 0.5 centering factor (Chunk 1's pico8_io, the
 * non-default entry point since `a` is never omitted here).
 *
 * A NULL `self`/`gfx`/`mem` is a safe no-op (the `gfx`/`mem`-needing draw
 * calls are simply skipped; `self`'s own field reads all tolerate a NULL
 * instance already via `pico8_instance_get_field`'s own established
 * convention, but a NULL `self` still returns immediately since there is
 * nothing to check `self.me` on). */
void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self);

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape to
 * pico8_dl's own signature, reading gfx/mem from the shared dispatch
 * context -- the same split as `pico8_ei`/`pico8_ei_dispatch` (pico8_
 * particle.c). This is what `g_.ev=dl` wires as a real `bm`-name field
 * (`ev`); a future `dc("ev")`'s own `entity.ra` second argument (`argc`/
 * `arg0`/`arg1`) is never read, matching real Lua silently dropping an
 * excess argument to a function declared with fewer parameters (`dl` only
 * declares `self`). */
pico8_value_t pico8_dl_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* function d_(self)if not self.et
 * and fm:lu(self.od)then
 * self:ks(self.dq or "ph")fm:nx(self.fp or 0)rn(self.ik or 54)
 * return true
 * else
 * rn(57)self.p_=true
 * end
 * end
 * (pico8/lua.lua:569-576.) The probe's "commit to a target" click handler
 * (`g_.ey=d_` -- NOT a `bm` name, mv()-routed only, see this file's own top
 * comment; also called DIRECTLY as a plain function by `jc:ey()` (Chunk 14,
 * `if not d_(self)then`, its own boolean RESULT genuinely consumed) and by
 * `mz:ey()`/`mz:hq()` (Chunk 16) -- so, like `bv`/`dl` above, this needs a
 * real, meaningful return VALUE (true, or Lua's own implicit nil on the
 * else branch), not just a side effect, hence a single plain signature
 * shared by both calling conventions (no split needed here: `d_` needs no
 * `gfx`/`mem` of its own, so there is no adapter-shape mismatch to resolve
 * the way `dl`/`ei` have).
 *
 * `if not self.et and fm:lu(self.od)then ... else ... end`: Lua's `and`
 * short-circuits -- `fm:lu(self.od)` is called ONLY when `self.et` is
 * falsy/absent (no drag target already committed); this port preserves
 * that exactly (does not call through to `fm`'s own "lu" field at all when
 * `self.et` is truthy), not merely evaluate-and-discard.
 *
 * `fm:lu(self.od)` / `fm:nx(self.fp or 0)`: real Lua method-call sugar on
 * the SINGLE global `fm` (Chunk 12's `hy` economy singleton -- "can afford
 * this project's cost" / "spend it").
 *
 * *** CORRECTION (Chunk 14, docs/prd/triage/phase3-etape1-function-by-
 * function-c-port.md) ***: this paragraph originally routed both calls
 * through `pico8_mv()`, reasoned below as HISTORY (still accurate for why
 * that choice was made AT THE TIME): since `hy`'s own class didn't exist
 * yet when this chunk (10) was written and `lu`/`nx` are ordinary Lua
 * method-call-sugar names (never written as literal `mv(...)` in the real
 * source), this was routed through the same generic `pico8_mv()` dynamic
 * field-lookup mechanism Chunk 9's own `ez`'s `self:mm()` already uses for
 * an identical "genuinely dynamic method call on a not-yet-fully-resolved
 * self" shape (pico8_particle.h's own top comment) -- NOT `pico8_ke_ks`'s
 * own "known, single, already-built implementation" direct-call treatment,
 * since `hy:lu`/`hy:nx` didn't exist to call directly yet at the time.
 * Once Chunk 12 landed, its own `pico8_economy.h` top comment settled the
 * real convention explicitly for every `hy` method, including these two by
 * name: never `bm`/mv()/dc()-routed, always direct C calls (`hy` is never
 * subclassed anywhere in the real cart, so there's no runtime-polymorphic
 * receiver to resolve, unlike `jo:kf`/`jo:cn`'s own genuine cluster-wide
 * dispatch) -- this port's own code here was never updated to match, so
 * `pico8_mv(fm,"lu"/"nx",...)` kept silently resolving "field not found"
 * even after `hy` became real, since `hy`'s own class correctly never
 * wires "lu"/"nx" as dispatch fields at all. Found and fixed by Chunk 14
 * (`pico8_jc_ey`'s own real end-to-end trade-cluster exercise, the first
 * real caller this stale routing actually blocked) -- `pico8_d_` below now
 * calls `pico8_hy_lu`/`pico8_hy_nx` directly, matching Chunk 12's
 * documented convention; see `pico8_d_`'s own updated in-body comment for
 * the full trace, and `docs/plan.md`'s "Notes de portage" for this port's
 * general record of such found-in-shipped-code gaps. `self.od` is passed
 * as-is (whatever kind it holds, narrowed to fixed with a 0 default per
 * this file's own top comment's pervasive convention); `self.fp or 0`
 * reproduces Lua's own `or` fallback (default `false`, per `fp=f` -- so
 * this fallback is taken by default until a real caller overrides `fp`).
 *
 * `self:ks(self.dq or "ph")`: unlike `fm:lu`/`fm:nx` above, `ks` IS a
 * single, already-built, known implementation on every `ke`-descended
 * instance (`pico8_ke_ks`, Chunk 3) -- called DIRECTLY, the same "known
 * method, no dispatch table" treatment `pico8_ke.h`'s own top comment
 * already establishes for every real `self:ks(...)` call site in this
 * port. `self.dq` defaults to the string "ly" per `g_`'s own class spec;
 * the `or "ph"` fallback only matters if a future caller ever clears it.
 *
 * `rn(self.ik or 54)` / `rn(57)`: Chunk 1's pico8_rn (inert sfx stub),
 * `self.ik` (never set by `g_`'s own class spec -- absent today, so this
 * always takes the `or 54` fallback in THIS chunk's own scope, faithfully
 * reproduced rather than simplified away since a future caller could still
 * set it).
 *
 * `self.p_=true`: the else branch's own self-destruct flag (`dg()`'s own
 * future `if qc.p_ then he(qc) end`, Chunk 6) -- the probe removes itself
 * once its target commit fails (target already claimed, or `fm:lu` says
 * "can't afford").
 *
 * Return value: `true` (PICO8_VALUE_BOOL) on the success branch, matching
 * the real explicit `return true`; the else branch has NO `return`
 * statement, so real Lua implicitly returns nil here -- ported as this
 * port's own established `pico8_value_ref(NULL)` "nil" filler convention
 * (e.g. `ui:bh()`'s own `self[ni]=nil`), NOT `pico8_value_bool(0)` (this
 * port never uses BOOL(false) to stand in for a genuine Lua nil elsewhere,
 * keeping the nil/false distinction real Lua itself preserves). A NULL
 * `self` is a safe no-op, returning `pico8_value_ref(NULL)` (nothing to
 * act on). */
pico8_value_t pico8_d_(pico8_instance_t *self);

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape to
 * pico8_d_'s own signature; `ui:bh()`'s own `mv(held,"ey",nw and nw.kv)`
 * dispatch passes argc=1 (unused -- `d_`'s real body never references a
 * second Lua parameter of its own). Returns `pico8_d_`'s own result
 * directly, the same "return the real function's own result" precedent
 * `pico8_bg_ldown_dispatch` already established for a non-void real
 * function. */
pico8_value_t pico8_d__dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* function g_:mm()local a=(flr(rnd(self.pn))+rnd(0.5))/self.pn+self.t*0.001
 * local p=qm(self.du+rnd(),a)local v=p*self.py
 * return {p=p,v=self.pk and v:pa()or v,l=self.ki
 * }
 * end
 * (pico8/lua.lua:727-731.) `g_`'s own particle-shape generator -- the FIRST
 * real definer of `mm` anywhere in this port's scope (Chunk 9's `ez`'s own
 * `self:mm()` call has had nothing to find until now, per pico8_particle.h's
 * own "mm forward-reference status, confirmed inert today" note). Wired as
 * a real PICO8_VALUE_FUNCTION field ("mm") on `g_`'s own class defaults --
 * mv()-routed like `hq`/`ey` above, not a `bm` name either.
 *
 * `a=(flr(rnd(self.pn))+rnd(0.5))/self.pn+self.t*0.001`: a pseudo-random
 * angle (in PICO-8 "turns"), biased into `self.pn` (3 default) equal
 * angular slices via `flr(rnd(self.pn))` (a random slice index 0..pn-1)
 * plus a random offset within that slice (`rnd(0.5)`), normalized back to
 * turns (`/self.pn`), then slowly rotated over time by `self.t*0.001`
 * (`self.t`, dg()'s own per-tick counter, Chunk 6).
 *
 * `p=qm(self.du+rnd(),a)`: a polar vector (Chunk 2's pico8_vec_qm) at angle
 * `a`, length `self.du+rnd()` (`self.du`=2 default, `rnd()`=`rnd(1)`, i.e. a
 * random extra 0..1 length).
 *
 * `v=p*self.py`: `p` scaled down to a much smaller velocity (`self.py`=0.06
 * default) -- `p` itself becomes the particle's own initial POSITION
 * offset, `v` its velocity.
 *
 * `return {p=p,v=self.pk and v:pa()or v,l=self.ki}`: the particle table
 * itself, in the exact shape Chunk 9's own `pico8_ei` already expects
 * (pico8_particle.h's own "particle representation" convention -- a REF to
 * a heap `pico8_ob_table_t` with named "p"/"v"/"l" fields, no "a" field
 * here, matching this literal source's own omission of one). `self.pk`
 * (1/truthy default) selects whether `v` is rotated 90 degrees
 * (`pico8_vec_pa`, Chunk 2's own `qx:pa`) before being stored -- true by
 * `g_`'s own default spec, giving the probe's own particle trail its
 * circular-orbit look rather than a straight outward trail.
 *
 * A NULL `self` reads every field as its documented default (0/false via
 * this file's own private field-reader helpers, pico8_probe.c), matching
 * this port's pervasive defensive convention; `argc`/`arg0`/`arg1` are
 * unused (the real `g_:mm()` declares no Lua parameters of its own beyond
 * `self`, and `ez`'s own real call site passes none). */
pico8_value_t pico8_g__mm(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

#endif
