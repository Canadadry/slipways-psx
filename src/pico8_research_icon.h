#ifndef PICO8_RESEARCH_ICON_H
#define PICO8_RESEARCH_ICON_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Phase 3, Chunk 19 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): the `ie` class -- the buildable-research-station icon shown on
 * the galaxy map (one small clickable "build a qh/nl/os/hj/mx/nr/fr here"
 * marker, revealed one at a time as `fm.hl`/tech progress allows). Extends
 * `ke` directly (Chunk 3), same depth as nf/js/fq/qb/bg/jo/mh/jc/g_/mz/pj/
 * iv.
 *
 * ie=ke:jr([[
 * kq=b(0,0,9,8),
 * fa=12,
 * id=0,
 * ra=v(999,999),
 * eh=v(118,120),
 * ]])ie.hd=gk
 * function ie:hb()if not self.kh and oj(self.ke.rr)then
 * self.ra,self.kh=
 * ie.gy,true
 * ie.gy-=v(10,0)
 * end
 * end
 * function ie:ldown(mp)
 * ie.id+=1
 * return self.ke({id="s"..ie.id
 * })end
 * function ie:kj(p)if self.kh then
 * palt(3,false)spr(self.ke.s,p.x,p.y)if self.m_ then
 * dl(self.ke)self.jb:ol(0,0)end
 * palt(3,true)end
 * end
 * (pico8/lua.lua:919-937.)
 *
 * `ie(overrides)` is always constructed with TWO real fields not in `ie`'s
 * own class spec above -- `ke` (one of the 7 real `jo`-descended planet-
 * upgrade CLASSES qh/os/nr/fr/nl/mx/hj, NOT an instance of one) and `jb` (a
 * real `jb`-class tooltip-box INSTANCE) -- confirmed by reading `eu(nj)`'s
 * own real call site (pico8/lua.lua, Chunk 20/22's own scope, not ported
 * here): `local gx={qh,os,nr,fr,nl,mx,hj} for i,b in pairs(gx)do
 * ie({ke=b,jb=jb(dr[i])}) end`. So every real `ie` instance's own
 * `self.ke` field holds a CLASS VALUE (a Lua table with no instance
 * fields of its own -- classes and instances are the exact same kind of
 * table in real Lua, both reachable through the same `.field` syntax and
 * the same `__index` metatable-chain fallback), never a `pico8_instance_t`.
 *
 * *** THE `self.ke`-AS-CLASS REPRESENTATION PROBLEM, AND ITS RESOLUTION ***
 * This port's `pico8_instance_t*` and `pico8_class_t*` are two
 * STRUCTURALLY UNRELATED C layouts (pico8_class.h: `{pico8_ob_table_t
 * *fields; pico8_class_t *cls;}` vs `{char *name; pico8_class_t *parent;
 * pico8_ob_table_t *defaults; pico8_class_pq_fn pq;}`) -- exactly the
 * representation mismatch Chunk 16 already hit and solved for `mz.jj`
 * (pico8_wormhole.h's own "SELF.ET REPRESENTATION" section, read directly
 * before writing this module): every real Lua field read on `self.ke`
 * (`self.ke.rr` in `hb`, `self.ke.s` in `kj`, and `dl(self.ke)`'s own
 * internal `self.me`/`self.et`/`self.od` reads) needs the SAME class-chain
 * `__index` fallback `pico8_instance_get_field` already implements for
 * instances, but `self.ke` genuinely IS a class here, not an instance, so
 * it cannot be passed to `pico8_instance_get_field` directly.
 *
 * RESOLVED THE SAME WAY Chunk 16 resolved `mz.jj`: every real `self.ke.X`
 * read in this module goes through a CLASSLESS `pico8_instance_t` WRAPPER
 * (`ie_ke_wrapper()`, pico8_research_icon.c) whose `cls` is `self.ke`
 * itself and whose `fields` is a fresh EMPTY table (never borrowed from
 * anywhere, unlike `mz.jj`'s wrapper -- there is no per-instance data to
 * borrow here, since `self.ke` is a bare class with no fields of its own
 * beyond its class-level defaults). `pico8_instance_get_field`'s own
 * class-chain walk (own fields first -- empty here, correctly falls
 * through -- then `cls`'s own defaults, then `cls->parent`'s, and so on up
 * to `jo`/`ke`) then resolves `self.ke.rr`/`self.ke.s`/`self.ke.me`/etc.
 * exactly the way real Lua's own `__index` chain would on the class table
 * itself. UNLIKE `mz.jj`'s wrapper (one single, class-lifetime-persistent
 * object, since `mz.jj` is always the SAME one shared table for every
 * `mz` instance), THIS wrapper cannot be built once and reused for every
 * `ie` instance: different `ie` instances hold DIFFERENT classes in their
 * own `self.ke` field (a `qh`-backed icon vs an `hj`-backed one, etc.), so
 * `ie_ke_wrapper()` builds a small, transient, STACK-allocated
 * `pico8_instance_t` fresh on every call, pointed at a single shared,
 * never-written, module-static empty `pico8_ob_table_t` (safe to share
 * across every call since `pico8_instance_get_field` never writes through
 * `fields` -- only ever reads it). No heap allocation, no accessor to
 * expose, no shutdown-time teardown needed -- the wrapper's whole lifetime
 * is the one C statement that uses it, a narrower, simpler case than
 * `mz.jj`'s own necessarily-persistent, exposed, heap-allocated wrapper.
 *
 * *** IS `dl(self.ke)` PROVABLY A NO-OP TODAY? NO -- CONFIRMED BY DIRECT
 * EVIDENCE, NOT ASSUMED *** `pico8_dl`'s (pico8_probe.c) own very first
 * real check is `self.me=="kz"`, early-returning a no-op otherwise. Every
 * one of the 7 real classes `self.ke` can ever hold (`qh`, `os`, `nr`,
 * `fr`, `nl`, `mx`, `hj`) was checked DIRECTLY by grepping every one of
 * their own real class specs (pico8_planet_family.c/pico8_planet_family_
 * trade.c): `qh`'s own spec sets `me="kz"` explicitly
 * (`"hp=1,px=o(\"laboratory\",c=13),\nme=\"kz\",dq=\"ly\",\n..."`, this
 * file's own `pico8_planet_family.c`); NONE of `os`/`nr`/`fr`/`nl`/`mx`/
 * `hj` ever set their own `me` field (confirmed by reading every one of
 * their own specs line by line -- no `me=` anywhere in any of them), and
 * every one of the 7 is `qh` itself or a (possibly multi-level) subclass
 * of it (`jo -> qh -> {nl, os, hj -> {pm(not in gx), mx}, nr -> fr}` --
 * `pm` is excluded from `gx`, but every other one of the 7 real `gx`
 * members chains back to `qh`), so EVERY ONE of the 7 real classes `self.
 * ke` can hold resolves `me` to the SAME inherited class-level default,
 * `"kz"`, through the class-chain walk -- NOT `ke`'s own root default
 * `"ph"` (`pico8_ke.c`'s `me="ph"`), which would only be reached if
 * nothing along the chain ever overrode it. So `dl(self.ke)`'s own first
 * check (`self.me=="kz"`) is ALWAYS TRUE for every real `self.ke` this
 * cart ever constructs an `ie` with -- the function does NOT early-return,
 * and its body genuinely executes further.
 *
 * Tracing what it does next, still with `self` = the classless wrapper
 * (`cls=self.ke`, empty own fields): `self.et` is read next -- `self.ke`
 * (a bare class) never has an "et" field anywhere in its own class-level
 * defaults chain (`et` is always written per-INSTANCE, by `bv()`, never as
 * a class-spec field), so this reads as absent/falsy, taking `dl`'s own
 * `elseif self.od then` branch. `self.od` (the tech/research-project cost
 * this planet-upgrade class needs, e.g. `qh`'s own `od=15`, `hj`'s own
 * `od=60`) IS a real class-level default field every one of the 7 classes
 * sets directly on themselves (confirmed by rereading each spec: `od=15`
 * `qh`, `od=20` `nl`, `od=15` `os`, `od=60` `hj`, `od=40` `mx`, `od=10`
 * `nr`, `od=25` `fr`) -- so `dl`'s own `t=self.od.."$"` branch fires for
 * real, and `io(t or "",mp.x,mp.y-8,8,0.5)` draws e.g. "15$" near the
 * mouse. So `dl(self.ke)` is a REAL, DELIBERATE, EVIDENCED call here --
 * NOT the inert no-op a shallower reading (assuming `me` always stays at
 * `ke`'s own root default) would predict: it draws the CLASS-LEVEL build
 * cost of whichever planet-upgrade `self.ke` names, as a tooltip near the
 * mouse, while `self.m_` (some "currently being placed/previewed" mode
 * flag, set elsewhere, out of this chunk's own scope to trace further) is
 * truthy. This finding is the OPPOSITE of what a surface reading of
 * `pico8_probe.h`'s own "`me` never becomes `kz` outside `mz`" framing
 * (written before `qh`'s own class-level `me="kz"` was re-checked here)
 * might suggest -- ported anyway, exactly as written, per this whole
 * port's own "a real call site is ported even when its current behavior
 * turns out to be surprising" discipline; nothing here is skipped or
 * special-cased away because the finding contradicts an earlier chunk's
 * own narrower framing.
 *
 * *** `self.ra=ie.gy` -- SNAPSHOT, NOT ALIAS, CONFIRMED BY HOW THIS PORT
 * REPRESENTS VECTOR ARITHMETIC *** `ie.gy` is a CLASS-LEVEL field (see
 * `ie.id`/`ie.gy` note below) holding a `v(x,y)` vector -- represented, per
 * this port's own established convention (`pico8_entity_geom.h`'s own top
 * comment, first established by `gk`), as a `PICO8_VALUE_REF` pointing to
 * a heap-allocated `pico8_vec_t`. `self.ra,self.kh=ie.gy,true` is a plain
 * Lua value COPY: `self.ra` becomes a `pico8_value_t` holding THE SAME REF
 * (the same heap `pico8_vec_t*`) `ie.gy` currently holds -- a real,
 * deliberate ALIAS at the moment of assignment, exactly mirroring real
 * Lua's own "assigning a table value copies the reference, both names
 * point at the same table" semantics (this is NOT a bug to avoid; it is
 * the literally correct reading of this exact statement). The SEPARATE
 * following statement, `ie.gy-=v(10,0)`, does NOT retroactively change
 * what `self.ra` already captured, because this port's own vector
 * arithmetic is PURE: `pico8_vec_sub` (pico8_vec.c) computes and returns a
 * brand-new `pico8_vec_t` BY VALUE, mutating neither of its operands --
 * confirmed by reading its own current body directly (`return pico8_vec_v
 * (pico8_fixed_sub(a.x,b.x), pico8_fixed_sub(a.y,b.y));`, no write through
 * either `a` or `b`). Every other class-level/instance vector field this
 * port has ever mutated in place (`pico8_bv`'s own `self.p`, `pico8_pj_kj`
 * rebinding fixed fields the same way) follows the identical convention:
 * compute the new value, heap-allocate a FRESH `pico8_vec_t` for it, and
 * REBIND the field's own `pico8_value_t` slot to the new heap pointer --
 * never write through the OLD heap pointer in place. `ie.gy-=v(10,0)` is
 * ported the same way here (see `pico8_ie_hb` below): a fresh heap vector
 * is allocated for `ie.gy`'s new value, and `ie.gy`'s own class-defaults
 * slot is rebound to point at it -- the OLD heap vector `self.ra` already
 * captured a REF to is left completely untouched, so `self.ra`'s own
 * snapshot survives the following decrement exactly as real Lua's
 * (non-mutating) vector `-` metamethod guarantees. The ALTERNATIVE reading
 * rejected here -- mutating `*old_heap_vec` in place for `-=`, which WOULD
 * retroactively corrupt `self.ra`'s already-captured snapshot -- would
 * contradict both real Lua's own vector-subtraction semantics (`km:__sub`
 * always returns a fresh `v(...)`, Phase 2) and this port's own pervasive,
 * already-shipped "rebind, never mutate in place" convention for vector
 * fields, so it is not the reading used here.
 *
 * `ie.gy`/`ie.id`: plain CLASS-LEVEL fields on `ie`'s own `defaults` table,
 * read/written directly (`pico8_ob_table_get_name`/`pico8_ob_table_set_
 * name(g_ie_class->defaults, ...)`), the same established "read/write a
 * class's own default field directly" idiom `pj.ea`/`pj.gs`/`pj.cr`/`pj.
 * i_`/`pj.ou` already use (pico8_score_panel.h, Chunk 18) -- not a new
 * mechanism. `ie.id` starts at its own real class-spec default (`id=0`
 * above) and is a plain incrementing counter (`ie:ldown`'s own `ie.id+=
 * 1`), used only to build a fresh, distinguishing `"id"` STRING override
 * (`"s"..ie.id`) for the NEWLY CONSTRUCTED `self.ke(...)` instance -- a
 * different, per-instance "id" field on a DIFFERENT object than `ie`'s own
 * class-level counter, a naming coincidence in the real source, not a
 * conflict. `ie.gy` is NOT one of `ie`'s own 5 class-spec fields above
 * (`kq,fa,id,ra,eh`) -- confirmed by rereading the spec line by line -- it
 * starts genuinely ABSENT (no writer anywhere in this chunk's own scope)
 * until Chunk 22's real `_init()` (`ie.gy,kc=ie.eh`, a real Lua multi-
 * assignment with fewer values than names, padding `kc` with nil) sets it
 * for the very first time. Every read of `ie.gy` in this chunk's own scope
 * (`ie:hb`) is therefore guarded the same defensive way every other
 * genuinely-not-yet-set forward-referenced field in this port already is
 * (e.g. `pj.ed`'s own class-level `1` default before Chunk 18's own real
 * `_init()`-adjacent `kc` reconciliation, or `fm`'s own not-yet-real
 * fields throughout Chunks 10-11): a raw VALUE read (`pico8_value_ref
 * (NULL)` if absent, for the exact-copy `self.ra,self.kh=ie.gy,true`
 * assignment) and a separate DEFENSIVE VEC read (zero vector if absent,
 * for the `-=` arithmetic itself, matching e.g. `pico8_probe_field_vec`'s
 * own established "absent vector field reads as (0,0)" convention) -- no
 * new forward-reference accessor pair needed, since `ie.gy` is a plain
 * class-level field, not a singleton (the same "just read it defensively"
 * resolution this port already gave `dk`, Chunk 18). Real Lua itself would
 * actually ERROR on `ie.gy-=v(10,0)` while `ie.gy` is still nil (`attempt
 * to perform arithmetic on a nil value`) -- but every real runtime path
 * that can ever reach a live `ie:hb()` call passes through `_init()`
 * first (which builds every `ie` instance in the first place, via `eu()`),
 * so `ie.gy` is always already real by the time `ie:hb()` is ever actually
 * dispatched for real; this chunk's own defensive zero-vector fallback is
 * therefore, like so many other forward references in this port, a
 * "compiles safely today, not yet meaningfully exercised until Chunk 22
 * lands" situation, not a real gap the finished cart will ever hit. */
void pico8_research_icon_init(void);
void pico8_research_icon_shutdown(void);

/* The real `ie` class, built once by pico8_research_icon_init(). Registers
 * itself under the shared ""-keyed pico8_class_lookup() slot (`ie`'s own
 * spec never sets an `fz`/`nu` field beyond the plain data fields above --
 * confirmed by reading the spec line by line), like most other classes in
 * this port. Returns NULL before pico8_research_icon_init() (or after
 * pico8_research_icon_shutdown()). */
pico8_class_t *pico8_ie_class(void);

/* `ie(overrides)` -- the real Lua constructor call (`eu(nj)`'s own
 * `ie({ke=b,jb=jb(dr[i])})`, Chunk 20/22, not ported here). Thin wrapper
 * around pico8_class_instantiate(). */
pico8_instance_t *pico8_ie_new(const pico8_ob_table_t *overrides);

/* `ie.hd=gk` -- the EXACT SAME "field reused from Chunk 1's standalone
 * `gk`" pattern already established by `jo.hd=gk` (`pico8_jo_hd_dispatch`,
 * pico8_planet.c), `js.hd=gk` (`pico8_js_hd_dispatch`, pico8_ui_button.c)
 * and `fv.hd=gk` (`pico8_fv_hd_dispatch`, pico8_help.c) -- a plain field
 * assignment, NOT a `function ie:hd()...end` definition. `hd` IS one of
 * `bm`'s 5 real dispatch names (`bm=["kj","ev","hd","hk","hb"]`,
 * pico8_entity.c), wired as a real `PICO8_VALUE_FUNCTION` field on `ie`'s
 * own class defaults (`pico8_ie_hd_dispatch`, this file's own `.c`),
 * replicating `pico8_jo_hd_dispatch`'s exact shape: read `argc`/`arg0`/
 * `arg1` through to `pico8_gk`'s own real 2-argument signature exactly. */

/* function ie:hb()if not self.kh and oj(self.ke.rr)then
 * self.ra,self.kh=
 * ie.gy,true
 * ie.gy-=v(10,0)
 * end
 * end
 * (pico8/lua.lua:923-928.) "Reveal" this icon (make it visible/hoverable)
 * once its own prerequisite tech (`self.ke.rr`, whichever planet-upgrade
 * class this icon builds -- e.g. `nr`'s own `rr="slipgates"`, `hj`'s own
 * `rr="ascension"`; `qh`/`os` never set `rr` anywhere in their own chain,
 * so `self.ke.rr` is genuinely absent/nil for them, and `oj(nil)` is
 * UNCONDITIONALLY true per `oj`'s own `not px` short-circuit,
 * `pico8_tech_gate.h`) is unlocked (`oj`, Chunk 11's `pico8_oj`, already
 * internally guards `pico8_fm()!=NULL` -- called directly here, no extra
 * guard needed) -- but only once (`not self.kh`, `self.kh` a "already
 * claimed a reveal position" per-INSTANCE flag this function itself sets).
 * See this file's own top comment for the full `self.ke`-as-class-wrapper
 * trace (`self.ke.rr`'s own read) and the `self.ra=ie.gy`
 * snapshot-vs-alias trace (the multi-assignment plus the following `-=`).
 * `bm` real dispatch name; needs no `gfx`/`mem` (no drawing). A NULL
 * `self` is a safe no-op. */
void pico8_ie_hb(pico8_instance_t *self);

/* function ie:ldown(mp)
 * ie.id+=1
 * return self.ke({id="s"..ie.id
 * })end
 * (pico8/lua.lua:929-933.) Left-click on a revealed icon: increments `ie`'s
 * own class-level counter `ie.id`, then CONSTRUCTS AND RETURNS a fresh
 * instance of whichever real planet-upgrade class `self.ke` names (a real
 * class VALUE, not a name -- `pico8_class_instantiate` called directly on
 * it, no `pico8_class_lookup_named` needed, unlike `jo:ldown`'s own
 * `jc({...})`, which forward-references a class only reachable BY NAME),
 * overriding just its own fresh `"id"` field with a newly built string
 * (`"s"..ie.id`, this module's own private two-piece concat helper,
 * matching `pj_concat2`'s own established per-module-duplicate
 * convention) -- the caller (`ui:bh()`, out of this chunk's own scope)
 * holds onto the returned instance as a placement-in-progress, the same
 * "return a fresh entity to hold as a drag" shape `bg:ldown`'s own
 * `g_()`/`mz()` call and `jo:ldown`'s own `jc({qu=self})` call already
 * establish. `mp` (the click's own world-space mouse position) is
 * declared, per this port's literal-signature-fidelity convention, but
 * genuinely unused by the real body (matching `bg:ldown`'s own unused
 * `mp`). `ldown` is NOT one of `bm`'s 5 real dispatch names (confirmed
 * directly against `bm`'s own literal, `pico8_entity.c`) -- reached only
 * through `mv()`'s own unrestricted by-name field lookup, matching `bg:
 * ldown`'s own identical "not bm, still a real mv()-routed
 * PICO8_VALUE_FUNCTION field" precedent (`ui:bh()`'s own real `self:jz(bt
 * .."down",self.qa,je.ra)` call site, `bt="l"`, argc=2). Returns
 * `pico8_value_ref(NULL)` if `self.ke` is absent/wrong-kind (defensive;
 * every real `ie` instance always has a real `self.ke` class by
 * construction, so this never actually fires on a real instance, matching
 * this port's pervasive "defend against a case no real call site
 * produces" convention). A NULL `self` reads every field as absent,
 * matching every other defensive read in this port. */
pico8_value_t pico8_ie_ldown(pico8_instance_t *self, pico8_value_t mp);

/* function ie:kj(p)if self.kh then
 * palt(3,false)spr(self.ke.s,p.x,p.y)if self.m_ then
 * dl(self.ke)self.jb:ol(0,0)end
 * palt(3,true)end
 * end
 * (pico8/lua.lua:934-937.) Draws this icon, but ONLY once it has been
 * "revealed" (`self.kh`, set by `ie:hb` above) -- a genuinely-absent/false
 * `self.kh` makes the WHOLE function a no-op, exactly mirroring the real
 * source's own single enclosing `if self.kh then...end`. `palt(3,false)`/
 * `palt(3,true)` (`pico8_palt`, `pico8_gfx.h`) temporarily disables color 3
 * as the sprite-sheet transparent key while drawing this icon's own sprite
 * (`self.ke.s`, whichever planet-upgrade class's own sprite id --
 * `pico8_spr_default`, no `w`/`h` scale, matching every other single-tile
 * `spr(id,x,y)` call already ported in this port) at `p` (the position
 * argument -- `dc("kj")`'s own real `qc[nt](qc,qc.ra)` call shape means
 * this is always `self.ra`, the exact snapshot `ie:hb` captured off `ie.
 * gy` -- see this file's own top comment's snapshot-vs-alias trace).
 * `if self.m_ then dl(self.ke) self.jb:ol(0,0) end`: while some
 * "currently being placed/hovered" mode flag `self.m_` (set elsewhere,
 * out of this chunk's own scope) is truthy, ALSO draws `dl(self.ke)`'s own
 * build-cost tooltip (see this file's own top comment's full `dl(self.
 * ke)`-is-not-a-no-op trace) and this icon's own tooltip box
 * (`self.jb:ol(0,0)`, `pico8_jb_ol`, `pico8_layout.h` -- `self.jb`, unlike
 * `self.ke`, really is stored as a real `jb`-class INSTANCE ref, an
 * ordinary `pico8_instance_get_field` read, no representation trick
 * needed). `bm` real dispatch name (`dc("kj")`-reachable); needs `gfx`+
 * `mem` (every real draw call here needs both) -- a NULL `gfx`/`mem` is a
 * safe no-op for the WHOLE function (nothing to draw with), matching `pj:
 * kj`'s own established convention; a NULL/falsy-`kh` `self` is likewise a
 * safe no-op, checked first. */
void pico8_ie_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p);

#endif
