#ifndef PICO8_UI_H
#define PICO8_UI_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 5 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `ui`, a real `ke`-descended class (`ui=ke:jr(...)`,
 * pico8/lua.lua:212 -- confirmed by reading the source, matching the PRD's
 * own later text identifying it as the click/panel dispatcher, Chunk 6's
 * future `ok=ui()`). Its own new module (not pico8_ui_button.c, already at
 * 383 lines -- see docs/portage-progress.md's own module-size check; not
 * pico8_entity.c, already at/over the flagged ~430-540-line threshold),
 * per this chunk's own "leave headroom" instruction.
 *
 * ui=ke:jr([[
 * ql=58,
 * fa=15,
 * le=o(),
 * hw=b(10,10,118,118),
 * ]])function ui:lj()local le=self.le
 * self.qa=je.ra+mo.ra
 * self:gw()if #le==0 or gu(le,self.o_)then
 * self:bh("l")end
 * self:bh("r")if je.l.jn and self.le==le then
 * self:cj()end
 * self.gj=self.o_
 * end
 * function ui:ev()spr(self.rheld and 9 or self.o_ and self.o_.ql or self.ql,je.ra.x,je.ra.y)end
 * function ui:gw()self.ic,self.o_={}
 * for e in all(cv.hd)do
 * if mv(e,"hd",self.qa,je.ra)then
 * ft(self.ic,e.fa,e)self.o_=e
 * end
 * end
 * (self.gj or {}).m_=false
 * (self.o_ or {}).m_=true
 * poke(0x5f80,self.o_
 * and self.o_.fg
 * or max(peek(0x5f80)-1,0))end
 * function ui:bh(bt)local b,ni=
 * je[bt],bt.."held"if b.jn then
 * local pw=self:jz(bt.."down",self.qa,je.ra)if pw and type(pw.kn)=="table" then
 * self[ni]=pw.kn
 * end
 * end
 * local held=self[ni]
 * mv(held,"hq",self.qa,je.ra)if b.hm and held then
 * local nw=self:jz("gv",held)mv(held,"ey",nw and nw.kv)self[ni]=nil
 * end
 * end
 * function ui:jz(rm,...)for mf=15,0,-1 do
 * for h in all(self.ic[mf])do
 * local kn=mv(h,rm,...)if kn then
 * return {kv=h,kn=kn}
 * end
 * end
 * end
 * end
 * function ui:cj()nv(self.le,he)self.le={}
 * end
 * (pico8/lua.lua:212-257.)
 *
 * Class spec: `ql=58` (the default idle cursor sprite id), `fa=15` (draw
 * layer), `hw=b(10,10,118,118)` (a qz() hit box via Chunk 1/2's char-keyed
 * `'b'` pi constructor, pico8_geom.h -- parsed but never read by any of
 * this chunk's own 6 methods, so it's ported as data only), and `le=o()`.
 * `le`'s own real consumers (`gu()`/`nv()`, pico8_collect.h) both need a
 * `pico8_list_t*`, not the raw, untyped `pico8_ob_table_t*` ob()'s generic
 * "identifier(...)" fallback would naively produce for an unregistered
 * `o(...)` call -- the exact same "the naive ob()-parsed shape isn't the
 * type real code actually needs" situation Chunk 1 already hit for a
 * `v(...)` field (pico8_entity_geom.h's own top comment) and Chunk 4 hit
 * for `qb.pc` -- resolved the same way: pico8_ui_init() overrides the
 * class's own "le" default, right after declaring it, with a properly-
 * typed empty heap `pico8_list_t*` REF (freeing the raw table ob() built
 * for it, which nothing else ever sees). Needs pico8_pi_init() first
 * (idempotent), same real ordering hazard as nf/qb/js's own `v(...)`/`b(...)`
 * fields (see pico8_mouse_reader.c's own pico8_nf_init() comment).
 *
 * ui has NO own `pq` -- like qb, it purely inherits ke's own `pq=lt`
 * (auto-registration), needing no pico8_class_set_pq() call.
 *
 * Of the 6 methods, only `ev` is one of bm's 5 dispatch names (kj/ev/hd/
 * hk/hb), so only `ev` is wired as a real PICO8_VALUE_FUNCTION dispatch
 * field (found via dc("ev")/mv(), exactly like js:ev). `lj`/`gw`/`bh`/
 * `jz`/`cj` are ordinary Lua method-call-sugar (`self:gw()` etc.) on a
 * statically-known `self` of this exact class -- never routed through
 * mv()/dc()'s generic name-based dispatch -- so each is ported as a plain,
 * directly-callable C function instead, matching this port's own
 * established "internal same-class calls skip the generic mechanism"
 * precedent (e.g. pico8_js_kj's own direct calls into pico8_lq_ol_default).
 * `ui:lj()` itself is called directly too, once per frame, by the real
 * `_update60()` (`dg()ok:lj()`, pico8/lua.lua:1187, Chunk 22 -- not yet
 * ported): this chunk's own pico8_ui_lj() takes `mem` as an explicit
 * parameter for exactly this reason, rather than reading it from
 * pico8_dispatch_context.h's shared ambient context (unlike `ev`, which
 * genuinely IS reached through the generic mechanism and so DOES use that
 * shared context, matching js:ev/js:kj's own precedent).
 *
 * `je`/`mo` (the future mouse-reader and camera singletons) are both
 * forward-referenced here, ported not deferred, per this PRD's own
 * established convention. `mo` already has a placeholder accessor
 * (pico8_mo(), pico8_entity.h, built in Phase 2 for dc()'s own literal
 * `mo:md(ci[o])` line) -- reused here as-is. `je` has no such placeholder
 * anywhere yet; this chunk adds one (pico8_je(), below), mirroring
 * pico8_mo()'s own exact shape (a single, bare, classless
 * pico8_instance_t* with no fields set, so every `je.<field>` read in
 * this chunk's own methods reads as absent and falls back to its own
 * documented default -- a zero vector for `.ra`, false for a missing
 * button-state field) -- NOT placed in pico8_entity.c (already at/over
 * the flagged ~430-540-line threshold, so this chunk deliberately doesn't
 * grow it further). Real construction of `je` (a genuine `nf()` instance)
 * is explicitly Chunk 6's own job (jl() completion, this PRD's own
 * singleton-ownership decision) -- whichever module Chunk 6 lands in must
 * either replace this accessor's own backing global with a real `nf`
 * instance (mirroring exactly how pico8_mo()'s own bare placeholder is
 * expected to become a real qb() instance) or explicitly redirect this
 * header's declaration there; either way, Chunk 6 must not create a
 * SECOND, competing `je` accessor alongside this one. Not exercised by
 * anything in this chunk's own scope (no real `_update60()`/dg() exists
 * yet to call pico8_ui_lj() for real) -- the same "compiles, not yet
 * meaningfully testable" forward-reference treatment used throughout this
 * PRD. */
void pico8_ui_init(void);
void pico8_ui_shutdown(void);

/* The real `ui` class, built once by pico8_ui_init(). Mirrors
 * pico8_scroll.h's own pico8_qb_class() convention (not retrievable via
 * pico8_class_lookup(), same shared "" registry-slot reasoning). Returns
 * NULL before pico8_ui_init() (or after pico8_ui_shutdown()). */
pico8_class_t *pico8_ui_class(void);

/* `ui(overrides)` -- the real Lua constructor call (Chunk 6's future
 * `ok=ui()`). Thin wrapper around pico8_class_instantiate(). */
pico8_instance_t *pico8_ui_new(const pico8_ob_table_t *overrides);

/* je -- see this file's own top comment for the full rationale. Built/
 * freed by pico8_ui_init()/pico8_ui_shutdown(), like pico8_mo() is by
 * pico8_entity_init()/_shutdown(). */
pico8_instance_t *pico8_je(void);

/* function ui:lj()local le=self.le
 * self.qa=je.ra+mo.ra
 * self:gw()if #le==0 or gu(le,self.o_)then
 * self:bh("l")end
 * self:bh("r")if je.l.jn and self.le==le then
 * self:cj()end
 * self.gj=self.o_
 * end
 * (pico8/lua.lua:217-224.) The literal, directly-callable port: called
 * once per frame by the real `_update60()` (`ok:lj()`, Chunk 22, not yet
 * ported), so `mem` is passed explicitly here rather than read from the
 * shared pico8_dispatch_context.h singleton (see this file's own top
 * comment). A NULL `self` is a safe no-op. */
void pico8_ui_lj(pico8_mem_t *mem, pico8_instance_t *self);

/* function ui:ev()spr(self.rheld and 9 or self.o_ and self.o_.ql or self.ql,je.ra.x,je.ra.y)end
 * (pico8/lua.lua:225.) The literal, directly-callable port: `ev` IS one of
 * bm's 5 dispatch names, so this is also wired as a real
 * PICO8_VALUE_FUNCTION class-default field (pico8_ui_ev_dispatch,
 * pico8_ui.c), reading pico8_dispatch_context.h's shared gfx/mem exactly
 * like js:ev/js:kj. A NULL `self`/`gfx`/`mem` is a safe no-op. */
void pico8_ui_ev(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self);

/* function ui:gw()self.ic,self.o_={}
 * for e in all(cv.hd)do
 * if mv(e,"hd",self.qa,je.ra)then
 * ft(self.ic,e.fa,e)self.o_=e
 * end
 * end
 * (self.gj or {}).m_=false
 * (self.o_ or {}).m_=true
 * poke(0x5f80,self.o_
 * and self.o_.fg
 * or max(peek(0x5f80)-1,0))end
 * (pico8/lua.lua:226-236.) The literal, directly-callable port: THE real
 * 2-argument `mv(e,"hd",self.qa,je.ra)` call site that forces this chunk's
 * own pico8_mv()/pico8_value_function_t arity extension (pico8_value.h).
 * `self.ic` (the per-layer hit-test bucket table `ft()` builds here) is
 * represented the same way dc()'s own per-layer buckets are: a
 * pico8_ob_table_t indexed by `e.fa`, each index entry a heap
 * pico8_list_t* REF (pico8_collect.h's own established "a bucket is a
 * heap-allocated pico8_list_t" convention) -- consumed by pico8_ui_jz()
 * below. A NULL `self`/`mem` is a safe no-op. */
void pico8_ui_gw(pico8_mem_t *mem, pico8_instance_t *self);

/* function ui:bh(bt)local b,ni=
 * je[bt],bt.."held"if b.jn then
 * local pw=self:jz(bt.."down",self.qa,je.ra)if pw and type(pw.kn)=="table" then
 * self[ni]=pw.kn
 * end
 * end
 * local held=self[ni]
 * mv(held,"hq",self.qa,je.ra)if b.hm and held then
 * local nw=self:jz("gv",held)mv(held,"ey",nw and nw.kv)self[ni]=nil
 * end
 * end
 * (pico8/lua.lua:237-247.) The literal, directly-callable port. `bt` is
 * "l" or "r" (self's own two real call sites, `ui:lj()` above).
 * `type(pw.kn)=="table"` is ported the same way `type(fn)=="function"`
 * already is (pico8_entity.c's own mv()): `kn.kind==PICO8_VALUE_REF`.
 * `self[ni]=pw.kn`/`self[ni]=nil` (`ni` a dynamically-computed field name,
 * "lheld"/"rheld") need no special mechanism -- pico8_ob_table_set_name
 * already takes an arbitrary runtime string key; "unset" is represented
 * by writing pico8_value_ref(NULL), this port's own established "no real
 * Lua value" filler, matching how absence is read back everywhere else
 * (kind==REF && ref!=NULL for "truthy"). FLAGGED, not exercised by any
 * real call site in this chunk's own scope (js/fq's own `ldown` -- the
 * only ldown/rdown implementations that exist yet -- return a boolean,
 * never a table, so `self[ni]` is never actually assigned today): once a
 * future class's own ldown/rdown implementation genuinely returns a
 * table, `held` (read back from `self[ni]`) is cast to
 * `pico8_instance_t*` for the `mv(held,"hq",...)` call below, which is
 * only correct if that table genuinely is a class instance -- the same
 * kind of representation gap this whole PRD already flags rather than
 * silently guesses at (e.g. jo:hu's `kv[hp.cw]`). A NULL `self` is a safe
 * no-op. */
void pico8_ui_bh(pico8_instance_t *self, const char *bt);

/* function ui:jz(rm,...)for mf=15,0,-1 do
 * for h in all(self.ic[mf])do
 * local kn=mv(h,rm,...)if kn then
 * return {kv=h,kn=kn}
 * end
 * end
 * end
 * end
 * (pico8/lua.lua:248-255.) The literal, directly-callable port. Real
 * Lua's `{kv=h,kn=kn}` return table has no dedicated C struct here --
 * ported as the established "written 1/0 + out params" convention
 * (pico8_ob.c's own pico8_fi, this port's pervasive idiom for "a value
 * that may or may not be produced"): returns 1 and writes *out_kv and *out_kn
 * if some `h` in some non-empty `self.ic[mf]` bucket (mf descending from
 * 15 to 0) dispatches `rm` (via pico8_mv(), threading `argc`/`arg0`/`arg1`
 * straight through -- this function's own second real 0-/1-/2-argument
 * mv()-forwarding call site, alongside ui:bh()'s own direct ones) to a
 * TRUTHY result (`kn` non-nil, non-false -- ported as "pico8_mv() found
 * the field AND the result isn't PICO8_VALUE_BOOL false", mirroring every
 * other Lua truthiness check in this port); returns 0 (no match in any
 * bucket) otherwise, mirroring Lua's own implicit "falls off the end,
 * returns nothing" (nil). A NULL `self` is a safe "no match" (0). */
int pico8_ui_jz(pico8_instance_t *self, const char *rm, int argc, pico8_value_t arg0, pico8_value_t arg1,
                pico8_instance_t **out_kv, pico8_value_t *out_kn);

/* function ui:cj()nv(self.le,he)self.le={}
 * end
 * (pico8/lua.lua:256-257.) The literal, directly-callable port: `nv(self.
 * le,he)` maps self.le through he() (Chunk 10's pico8_he, pico8_entity.h)
 * -- he() has no return statement (always Lua nil), so nv()'s own
 * `kp(e) or nil` contributes NOTHING to the mapped list for every element
 * (pico8_collect.h's own established "kp writes *out_value, returns 1/0"
 * convention, here always returning 0) -- the mapped list itself is
 * discarded immediately; only the per-element pico8_he() SIDE EFFECT
 * (removing each entity from hv/cv/cg) is what this call is actually for.
 * `self.le={}` rebinds self.le to a fresh, empty heap pico8_list_t* REF,
 * the same representation this file's own top comment already establishes
 * for `le`'s class-spec default. A NULL `self` is a safe no-op. */
void pico8_ui_cj(pico8_instance_t *self);

#endif
