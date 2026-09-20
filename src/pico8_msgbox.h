#ifndef PICO8_MSGBOX_H
#define PICO8_MSGBOX_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Phase 3, Chunk 8 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `mh` (the message-box entity), `iv` (a floating damage/credit-
 * change "toast" entity), and the standalone `pz(p,bs)` (the function that
 * actually builds and shows a message box's own row of clickable options).
 * Both classes are real, direct `ke`-descended classes (`mh=ke:jr(...)`,
 * pico8/lua.lua:580; `iv=ke:jr(...)`, pico8/lua.lua:941 -- confirmed by
 * reading the source, not assumed from the PRD's own paraphrase). Grouped
 * in one new module (not pico8_ui_button.c -- 409 lines, already grown by
 * Chunk 5's arity-extension fix; not pico8_class.c/pico8_entity.c/
 * pico8_ui.c -- all at/in the flagged ~430-540-line range already, see
 * docs/portage-progress.md's own module-size checks for Chunks 6/7) since
 * the PRD's own paraphrase ("message-box and floating-notification
 * machinery") already describes them as one cohesive concern, confirmed by
 * reading their real methods below.
 *
 * mh=ke:jr([[
 * nu=o("mh"),
 * fa=14,
 * rh=5,
 * ]])function mh:pq()
 * if (kc)kc.p_=true
 * kc=self
 * end
 * function mh:ev()local t=self.t/self.rh-10
 * if t>=0 and mv(self,"lh")then
 * self.t,t=self.rh*10,0
 * end
 * local nc=-t^3/9
 * bl(self,nc)camera()if t>10.5 then
 * self.p_,kc=true
 * end
 * end
 * (pico8/lua.lua:580-596.)
 *
 * `mh` turns out to be exactly the message-box the PRD's own paraphrase
 * says: `nu=o("mh")` tags every instance "mh" (cg["mh"], read by other
 * classes' own hit-test/UI code, out of this chunk's own scope), `rh=5` is
 * the box's own lifetime-scale ("rate", how many ticks the closing/opening
 * animation takes per `bl()`-driven camera-shake step), and its own row of
 * clickable option buttons lives as the instance's OWN POSITIONAL entries
 * (`self[1]`, `self[2]`, ...) -- set at construction time by whichever
 * caller builds `mh(iw)` (`iw` a list of already-built `js`/plain-data
 * items, e.g. `pj:ph()`'s own real call site, Chunk 18, not yet ported),
 * NOT by `mh`'s own class spec (which has no positional entries at all).
 * This is confirmed by `mh:ev()`'s own `bl(self,nc)` call: `bl(rb,nc)`'s
 * real body is `for e in all(rb)do...end`, and Lua's `all()` on `self`
 * itself (an instance, not a plain list) walks exactly that instance's own
 * array part -- i.e., its own `self[1..n]` positional entries, precisely
 * the row of option buttons a real caller's `mh(iw)` seeds it with.
 *
 * `kc` (a plain mutable global -- see this file's own comment on
 * pico8_kc/pico8_msgbox_set_kc below): `mh:pq()` marks whatever message
 * box was PREVIOUSLY open (`kc`, if any) for removal (`kc.p_=true`, dg()'s
 * own future `if qc.p_ then he(qc) end` self-destruct check, Chunk 6) and
 * then makes itself the new `kc` -- opening a new message box always
 * supersedes/closes whichever one was already open, never lets two coexist.
 * `mh:ev()`'s own `self.p_,kc=true` (line 594) is a real, easy-to-mis-port
 * Lua subtlety worth flagging explicitly: Lua's multiple assignment with
 * FEWER values than targets pads the extra targets with nil, so this is
 * `self.p_=true; kc=nil` -- NOT `self.p_=true; kc=true`. This chunk's own
 * `pico8_mh_ev` (pico8_msgbox.c) ports this literally: the message box
 * marks ITSELF for removal (its own closing animation finished, `t>10.5`)
 * and clears the global `kc` back to "no message box open" in the same
 * statement, since the closing box was necessarily the current `kc`.
 *
 * iv=ke:jr([[
 * fa=9,
 * l=0,v=v(0,-0.34),
 * ]])function iv:pq()self.ra=self.ra or
 * je.ra+mo.ra-v(0,5)end
 * function iv:ph()
 * self.ra+=self.v
 * self.l+=0.1
 * self.p_=self.l>=5
 * end
 * function iv:kj(p)dv(self.l)io(self[1],p.x,p.y,self[2],0.5)dv()end
 * (pico8/lua.lua:941-951.)
 *
 * `iv` turns out to be the PRD's own "floating notification" -- a short-
 * lived text toast that drifts upward (`v=v(0,-0.34)`, a constant upward
 * velocity) and fades out via `dv(self.l)`'s own palette-block selection
 * (`l` climbs 0->5 over its own ~50-tick lifetime, `iv:ph`'s own
 * `self.p_=self.l>=5` self-destructing it once `l` reaches 5). Its own
 * displayed text/color are POSITIONAL entries too, exactly like `mh`'s own
 * option row: `self[1]` (the text, a PICO8_VALUE_STRING) and `self[2]`
 * (the forced color, a PICO8_VALUE_FIXED) -- confirmed by every real
 * `iv(...)` construction site found by grepping pico8/lua.lua
 * (`iv({ra=self.ra,ne(self.kr,10,"$")})`, line 516; `iv({"-no money-",8})`,
 * line 1218; `iv({ne(-jt,10,"$")})`, line 1223): Lua's own table-
 * constructor rule that a call's MULTIPLE return values (here, `ne()`'s
 * own two return values -- the formatted string and its forced color,
 * Chunk 1's pico8_ne) expand into successive POSITIONAL slots when it's
 * the LAST expression in a table literal is exactly what fills `self[1]`/
 * `self[2]` at these real call sites -- not two separate named fields.
 * `iv`'s own spec has no `nu=`/positional entries of its own, so `self.ra`
 * is genuinely absent unless a caller's own overrides set it (e.g. line
 * 516's own `ra=self.ra`), which is exactly what `iv:pq()`'s own `self.ra=
 * self.ra or ...` default-if-absent logic below is for.
 *
 * Both classes are declared with `pico8_class_declare(pico8_ke_class(),
 * ...)`, matching every other real `ke`-descended class so far (nf/js/fq/
 * qb/ui/bg, Chunks 4-7) -- same inheritance depth (ke -> this class, each
 * with its own distinct `pq` separate from `ke.pq=lt`'s inherited auto-
 * registration), nothing structurally new for the entity/dispatch smoke
 * test to cover (see this file's own per-lot-check note in
 * docs/portage-progress.md's Vague 8 section for the full reasoning,
 * including why `mh`'s own `nu=o("mh")` tag-list registration -- the
 * first REAL class spec in this port to set `nu=` -- is NOT new either:
 * Phase 2's own entity/dispatch smoke test, `tests/test_pico8_entity_
 * smoke.c`, already exercises `nu=o("alpha")`/`nu=o("beta")` through this
 * exact same real `pico8_class_declare()` + ob() parsing path). `iv` wires
 * 2 dispatch fields (`ph`, `kj`) and `mh` wires 1 (`ev`) -- fewer than
 * `js`'s own 5 (Chunk 4), so no new dispatch-table-size concern either. */

/* `kc` (pico8/lua.lua's own plain global, set by mh:pq()/read by a future
 * pj:ph(), Chunk 18, per this PRD's own Implementation Decisions,
 * "Singleton globals ownership"): the currently-open `mh` message-box
 * instance, or NULL when none is open. Declared here, alongside `mh`
 * itself, per that same Implementation Decisions text ("kc ... alongside
 * mh, Chunk 8"). Accessor pair mirrors pico8_entity.h's own pico8_mo()/
 * pico8_ui.h's own pico8_je()/pico8_ok() convention (get/set functions,
 * never a raw extern) -- but with ONE deliberate divergence from THAT
 * convention's own setter behavior, flagged explicitly rather than copied
 * blindly: `mo`/`je`/`ok` are singletons this port's own bootstrap code
 * OWNS (built once by pico8_bootstrap.c's pico8_jl(), freed by their own
 * module's shutdown), so their setters free the previous instance before
 * taking ownership of the new one. `kc` is different in kind: it is a
 * plain Lua-style BOOKMARK into whichever `mh` instance happens to be the
 * currently-open message box -- an instance that is (or was) independently
 * `lt()`-registered into `hv`/`cv`/`cg` and owned by whoever constructed it
 * via `mh(...)`, exactly like any other entity in this port (see
 * pico8_class.h's own comment on instance ownership). `pico8_msgbox_set_kc`
 * therefore never frees the previous value it's replacing -- doing so
 * would free a still-potentially-live, still-hv-registered entity out from
 * under the rest of the entity system, a real memory-safety bug this port
 * must not introduce, not a faithful port of anything (real Lua's `kc=nil`/
 * `kc=self` never frees anything either; the old `mh` instance is simply
 * left to be collected once nothing else references it, i.e. once he()
 * removes it from hv/cv/cg -- this port's own equivalent "leaked until its
 * owner frees it" state, the same convention already accepted everywhere
 * else this port lets a field/global be REBOUND rather than mutated in
 * place, e.g. pico8_ui.c's own pico8_ui_cj). A NULL `instance` is the
 * faithful C form of `kc=nil`. */
pico8_instance_t *pico8_kc(void);
void pico8_msgbox_set_kc(pico8_instance_t *instance);

/* Declares the real `mh` class (parent pico8_ke_class()) and wires `ev`
 * onto its defaults table; `mh:pq()` (pico8_mh_pq below) is wired directly
 * as this class's `pq` (pico8_class_set_pq) with NO separate dispatch
 * adapter, unlike nf/js/qb/bg/ui's own pq's -- `mh:pq`'s real body needs no
 * external context (gfx/mem/input) beyond `self` and the `kc` global
 * declared in this same module, so its own signature already matches
 * pico8_class_pq_fn's fixed `void(*)(pico8_instance_t*)` shape exactly. No
 * pico8_pi_init() call needed either: `mh`'s own spec has no `v(...)`/
 * `o(...)` nested constructor call that `pi`'s char-keyed registry would
 * need to resolve (`nu=o("mh")` hits ob()'s own UNREGISTERED-identifier
 * fallback, per this file's own top comment -- `o` itself is never a `pi`
 * key). */
void pico8_mh_init(void);
void pico8_mh_shutdown(void);
pico8_class_t *pico8_mh_class(void);

/* `mh(overrides)` -- the real Lua constructor call (e.g. `mh(iw)`,
 * pico8/lua.lua:869/899, Chunk 18 -- not yet ported). Thin wrapper around
 * pico8_class_instantiate(). `overrides`'s own POSITIONAL entries (see
 * this file's own top comment) become this instance's own `self[1..n]`
 * option row, exactly what `mh:ev()`'s own `bl(self,nc)` (see
 * pico8_mh_ev below) walks. */
pico8_instance_t *pico8_mh_new(const pico8_ob_table_t *overrides);

/* function mh:pq()
 * if (kc)kc.p_=true
 * kc=self
 * end
 * (pico8/lua.lua:584-587.) The literal, directly-callable port -- see this
 * file's own top comment for the full trace. Also directly usable as this
 * class's own pico8_class_pq_fn (see pico8_mh_init above). A NULL `self`
 * is a safe no-op (nothing to become the new `kc`). */
void pico8_mh_pq(pico8_instance_t *self);

/* function mh:ev()local t=self.t/self.rh-10
 * if t>=0 and mv(self,"lh")then
 * self.t,t=self.rh*10,0
 * end
 * local nc=-t^3/9
 * bl(self,nc)camera()if t>10.5 then
 * self.p_,kc=true
 * end
 * end
 * (pico8/lua.lua:588-596.) The literal, directly-callable port. `ev` IS
 * one of `bm`'s 5 dispatch names, so this is also wired as a real
 * PICO8_VALUE_FUNCTION class-default field (pico8_mh_ev_dispatch,
 * pico8_msgbox.c), matching js:ev/ui:ev/bg:kj's own precedent -- dc("ev")'s
 * own dispatch passes argc=1 with `qc.ra` as arg0, unused here (mh:ev's
 * real body never references it).
 *
 * `self.t/self.rh-10`: `t` starts near -10 (self.t starts at ke's own
 * inherited default, 0) and climbs toward/past 0 as `self.t` increments
 * (dg()'s own per-tick `qc.t+=1`, Chunk 6) -- an opening/closing animation
 * timer. `mv(self,"lh")` (a real 0-extra-argument mv() call, Phase 3
 * Chunk 5's own established convention) is an optional per-box "linger/
 * hold" callback (`lh`, never set by `mh`'s own spec, only by a real
 * caller's overrides, e.g. `pj:ph()`'s own `function iw.lh() return
 * self.c_ end`, Chunk 18 -- not yet ported): while it exists AND returns
 * truthy AND the opening animation has already finished (`t>=0`), the
 * closing countdown is held open by resetting `self.t`/`t` back to the
 * start of the "fully open" plateau (`self.rh*10`/`0`) every tick, instead
 * of letting it continue climbing toward the `t>10.5` closing threshold.
 *
 * THIS CHUNK'S OWN RESOLUTION of the Chunk 2-flagged `bl()` call-site gap
 * (pico8_pi.h's own comment on pico8_bl, and this PRD's own Chunk 8
 * paragraph): the real `bl(self,nc)` passes the `mh` INSTANCE ITSELF as
 * `rb`, relying on Lua's `all()` walking that instance's own array part
 * (`self[1..n]`, see this file's own top comment) directly -- not a
 * separate `pico8_list_t` field. Rather than widen `pico8_bl`'s own
 * signature (which correctly takes a `pico8_list_t*` for its two OTHER
 * real call sites, `bl(la(pj.i_,pj.ou))`/`bl(self.rb)`, neither of which
 * has this shape) or add a whole new `pico8_bl`-variant overload for one
 * call site, this chunk adds a small private adapter (pico8_msgbox_
 * positional_values, pico8_msgbox.c) that extracts `self->fields`'s own
 * PICO8_OB_KEY_INDEX entries into a fresh, temporary `pico8_list_t` --
 * exactly the SAME established pattern `pico8_layout.c`'s own (private,
 * static) `pico8_layout_positional_values` already uses for `lq`/`jb`'s
 * own "the whole instance IS a list" shape (Chunk 3), reused here as its
 * own private per-module copy (not exported/shared, matching this port's
 * pervasive "small per-module private helper, not a shared header symbol"
 * convention for a few-line helper, see e.g. pico8_bg.c's own
 * pico8_bg_field_vec) rather than a new, broader mechanism. `pico8_bl`
 * itself (pico8_pi.h/.c, Chunk 2) is NOT touched by this chunk -- its
 * existing `const pico8_list_t *rb` signature is exactly right for all
 * three of ITS OWN real callers once this chunk supplies the missing
 * instance-to-list view at mh:ev's own call site.
 *
 * `t^3` (Lua's `^`, right-associative, HIGHER precedence than unary `-`,
 * which is itself higher precedence than `/` -- per the Lua manual's own
 * operator-precedence table): `-t^3/9` parses as `(-(t^3))/9`, ported as
 * `pico8_fixed_div(pico8_fixed_neg(mul(mul(t,t),t)), 9)`, not `(-t)^3/9`
 * or `-(t^3/9)` (both would be observably different for this specific
 * expression shape whenever t is negative, which it is for most of a
 * message box's own opening animation).
 *
 * Needs `gfx`+`mem` (bl()'s own drawing dispatch + camera()); a NULL
 * `gfx`/`mem` skips the drawing/camera-reset portion only (the closing-
 * timer/kc bookkeeping below still runs), matching this port's own
 * established "NULL gfx/mem is a safe no-op for the drawing part only"
 * convention (e.g. pico8_js_ev's own pico8_jb_ol call). A NULL `self` is a
 * safe no-op for the whole function. */
void pico8_mh_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self);

/* Declares the real `iv` class (parent pico8_ke_class()) and wires `ph`/
 * `kj` onto its defaults table; `iv:pq()` (pico8_iv_pq below) is wired
 * directly as this class's `pq`, same "signature already matches, no
 * adapter needed" reasoning as `mh:pq` above. Calls pico8_pi_init()
 * defensively first (idempotent, matching pico8_mouse_reader.c's/
 * pico8_scroll.c's own established convention): `iv`'s own spec has a
 * nested `v(0,-0.34)` constructor call that needs Chunk 2's char-keyed
 * `'v'` pi registration to already exist, or ob() would silently produce a
 * raw positional table instead of a real pico8_vec_t-backed REF for `v`. */
void pico8_iv_init(void);
void pico8_iv_shutdown(void);
pico8_class_t *pico8_iv_class(void);

/* `iv(overrides)` -- the real Lua constructor call (e.g. `iv({"-no
 * money-",8})`, pico8/lua.lua:1218, Chunk 21 -- not yet ported). Thin
 * wrapper around pico8_class_instantiate(). `overrides`'s own POSITIONAL
 * entries become this instance's own `self[1]` (text)/`self[2]` (color),
 * see this file's own top comment. */
pico8_instance_t *pico8_iv_new(const pico8_ob_table_t *overrides);

/* function iv:pq()self.ra=self.ra or
 * je.ra+mo.ra-v(0,5)end
 * (pico8/lua.lua:944-945.) The literal, directly-callable port: if a
 * caller's own overrides already set `self.ra` (truthy -- see this file's
 * own top comment on real call sites like line 516's `ra=self.ra`), it's
 * kept as-is; otherwise defaults to the mouse-reader singleton `je`'s own
 * screen position plus the camera singleton `mo`'s own position, offset 5
 * pixels up (`v(0,5)`) -- both forward-referenced the same "compiles, read
 * defensively as a zero vector before Chunk 6's real jl() runs" way
 * pico8_ui.c's own `ui:lj()`/`gd()` already read `je`/`mo`. A NULL `self`
 * is a safe no-op. */
void pico8_iv_pq(pico8_instance_t *self);

/* function iv:ph()
 * self.ra+=self.v
 * self.l+=0.1
 * self.p_=self.l>=5
 * end
 * (pico8/lua.lua:946-949.) The literal, directly-callable port: `self.ra
 * +=self.v` is a field REBIND (a fresh heap vector), not an in-place
 * mutation, same reasoning as pico8_scroll.c's own pico8_qb_ph/
 * pico8_ui_button.c's own pico8_js_ph. `self.p_=self.l>=5` reads `self.l`
 * AFTER the preceding `self.l+=0.1` statement has already updated it
 * (evaluation order preserved). `ph` is dispatched via dg()'s own per-tick
 * `qc[qc.me](qc,qc.t)` (ke's own inherited default `me="ph"`, `iv` never
 * overrides it), so this is also wired as a real PICO8_VALUE_FUNCTION
 * class-default field (pico8_iv_ph_dispatch, pico8_msgbox.c), matching
 * nf:ph/qb:ph/js:ph's own precedent -- dg() always passes argc=1 with
 * `qc.t` as arg0, unused here. A NULL `self` is a safe no-op. */
void pico8_iv_ph(pico8_instance_t *self);

/* function iv:kj(p)dv(self.l)io(self[1],p.x,p.y,self[2],0.5)dv()end
 * (pico8/lua.lua:951.) The literal, directly-callable port: selects the
 * palette-fade block for the current `self.l` (Chunk 1's pico8_dv),
 * outlined-text-draws `self[1]` (the toast's text) at `p` in color
 * `self[2]` with the fixed 0.5 centering factor (Chunk 1's pico8_io, the
 * non-default entry point since `a` is never omitted here), then resets
 * the palette block back to 0 (Chunk 1's pico8_dv_default). `p` comes from
 * dc("kj")'s own `qc[nt](qc,qc.ra)` dispatch (arg0 = self.ra, unpacked as a
 * REF-to-vec exactly like js:kj/bg:kj's own precedent, zero-vector fallback
 * if absent/malformed). `kj` IS one of `bm`'s 5 dispatch names, wired as a
 * real PICO8_VALUE_FUNCTION field (pico8_iv_kj_dispatch, pico8_msgbox.c)
 * reading pico8_dispatch_context.h's shared gfx/mem, matching js:kj/bg:kj's
 * own precedent. `self[1]`/`self[2]` are read directly off `self->fields`'
 * own POSITIONAL entries (index 1/2), not through pico8_instance_get_field
 * (a NAMED-key lookup only) -- the same direct-`pico8_ob_table_get_index`
 * convention pico8_layout.c's own `pico8_layout_positional_values`/
 * pico8_entity.c's own `pico8_ho_apply` already use for reading an
 * instance's own positional entries, since `iv`'s class spec never sets
 * index-keyed defaults of its own for a prototype-chain fallback to matter
 * here. A missing/wrong-kind `self[1]`/`self[2]` reads as an empty string/
 * color 0 (the same documented defensive-absent-field convention used
 * throughout this port, e.g. pico8_entity_geom.c's gk/gd/na/ma). A NULL
 * `self`/`gfx`/`mem` is a safe no-op. */
void pico8_iv_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p);

/* function pz(p,bs)if p.ep then
 * bs=la({{lq={p.ep,c=13}}},bs)end
 * rn(p.eo or 62)local ra=p.hi or p.ra
 * local bp=ra+v(0,0)ok:cj()ok.le=nv(bs,function(b)b.ra,b.qf,b.fa=
 * ra,bp,p.ca
 * bp+=p.gq
 * return js(b)end)
 * if (p.ff)ok.le={}
 * end
 * (pico8/lua.lua:280-288.) The literal port of the standalone function
 * that actually POPULATES `ok.le` (the `ui` singleton's own currently-
 * shown row of clickable option icons, read every frame by `ui:lj()`,
 * Chunk 5) -- this is the real mechanism behind every message box's own
 * row of options AND every plain "row of icon buttons" UI moment in the
 * cart (`jo:fe()`/`jo:gh()`, Chunk 13; the difficulty/help screens, Chunks
 * 17/21) -- `p` supplies the row's own shared anchor/behavior fields, `bs`
 * the list of already-mostly-built option items (each a small table with
 * fields like `lq`/`h_`/`my`/`jb`, per every real `bs`-building call site
 * read for this chunk) to finish positioning and turn into real `js`
 * button instances.
 *
 * `p` is read generically (`p.ep`/`p.eo`/`p.hi`/`p.ra`/`p.ca`/`p.gq`/
 * `p.ff`) the same way every other "self-shaped" argument already is in
 * this port -- a `const pico8_instance_t*`, which works whether the real
 * caller passes a genuine class instance (e.g. `jo:fe()`'s own `pz(self,
 * ...)`, Chunk 13) or a plain classless field-bag (e.g. `pz(ha,ha)`, line
 * 997, Chunk 21) via `pico8_instance_get_field`'s own already-established
 * tolerance for `cls==NULL` (pico8_mo()'s own placeholder-instance
 * precedent, pico8_entity.h). None of this chunk's OWN scope has a real
 * caller yet (every real `pz(...)` call site is Chunks 13/17/18/21, not
 * yet ported) -- ported now, callable once those chunks exist, per this
 * whole PRD's "forward references are ported, not deferred" convention.
 *
 * `if p.ep then bs=la({{lq={p.ep,c=13}}},bs) end`: when `p.ep` is present
 * (an extra leading label, e.g. an icon prompt text), a brand-new item
 * table `{lq={p.ep,c=13}}` is PREPENDED (via `la()`, Chunk 1 -- concat
 * order matters: the new item comes first) to a NEW local `bs` (Lua's own
 * `bs=...` here rebinds the LOCAL parameter, never mutating whatever list
 * the caller passed in) -- ported as building a fresh 1-element
 * `pico8_list_t` holding a freshly allocated nested `pico8_ob_table_t`
 * (`{lq=<nested positional-and-c table>}`) and concatenating it with the
 * caller's own `bs` via Chunk 1's pico8_la(), used only for the rest of
 * this function's own body when `p.ep` was present.
 *
 * `rn(p.eo or 62)`: plays the (inert) click/notify sfx, Chunk 1's
 * pico8_rn, `p.eo` defaulting to 62 when absent/falsy.
 *
 * `local ra=p.hi or p.ra`: the row's own shared starting anchor position
 * -- `p.hi` ("hover icon" position, e.g. a specific building's own screen
 * position) if present/truthy, else `p.ra` (the entity's own generic
 * position field), read defensively (absent -> zero vector, this port's
 * pervasive convention) either way.
 *
 * `local bp=ra+v(0,0)`: a genuinely independent COPY of `ra` (Lua's own
 * `+` metamethod always allocates a fresh table) that then advances by
 * `p.gq` (a per-item step vector) once per item inside the closure below
 * -- `bp` is this function's own local, mutated across every closure call,
 * ported here as a plain C local threaded through the `pico8_nv()`
 * callback's own `ctx` pointer (this port's established stand-in for a
 * captured Lua upvalue, matching every other closure-factory in this
 * port, e.g. Chunk 1's pico8_jf/pico8_gn).
 *
 * `ok:cj()`: closes/clears whatever option row was previously showing
 * (Chunk 5's pico8_ui_cj) before building the new one -- `ok` is the `ui`
 * singleton (pico8_ok(), pico8_ui.h, Chunk 6), forward-referenced the same
 * "compiles, safe no-op before it's real" way as every other `ok`/`mo`/
 * `je` read elsewhere in this port; `pico8_ui_cj` already tolerates a NULL
 * `self`.
 *
 * `ok.le=nv(bs,function(b)...end)`: maps every item `b` of `bs` (each
 * expected to be a PICO8_VALUE_REF to a `pico8_ob_table_t*`, matching
 * `pico8_bl`'s own identical convention for the exact same "list of
 * ob()-parsed icon/button spec tables" shape) through a closure that
 * MUTATES `b` in place (`b.ra,b.qf,b.fa=ra,bp,p.ca` -- `b.ra` always set
 * to the SAME shared `ra` table reference for every item, matching Lua's
 * own by-reference table-assignment semantics exactly, not a per-item
 * copy; `b.qf` a fresh per-item snapshot of the CURRENT `bp`; `b.fa` set
 * to `p.ca` only when `p.ca` is actually present -- Lua's own `b.fa=nil`
 * when `p.ca` is absent is a real field REMOVAL, ported as "simply don't
 * write b.fa here", the same "absent stays absent" convention `fq:ldown`'s
 * own `self.p_=mv(self,"my")` already established for an identical shape),
 * advances `bp` by `p.gq` for the NEXT item, and finally constructs
 * (`return js(b)`) a real `js` button instance from the now-fully-
 * positioned `b` -- always contributing a value (never a falsy/nil `nv()`
 * skip, since `js(...)` is always a real, truthy table), stored as the new
 * `ok.le`. The list `pico8_ui_cj()` just built for `ok.le` (an empty
 * heap list) is overwritten here without being freed first -- the SAME
 * accepted "field-rebind leaks the old value" convention `pico8_ui_cj`
 * itself already establishes for its own `self.le={}` rebind (pico8_ui.c),
 * not a new gap this chunk introduces.
 *
 * `if (p.ff) ok.le={}`: when `p.ff` is truthy, the row just built is
 * immediately discarded (rebound to a fresh empty list) -- the individual
 * `js` instances already constructed above are NOT un-lt()'d/un-registered
 * by this (real Lua doesn't either; they simply become unreachable from
 * `ok.le` while remaining registered in `hv`/`cv`/`cg` until something
 * else `he()`s them, or forever if nothing does -- a real, evidenced
 * quirk of the original cart, not a bug this port introduces or fixes).
 *
 * Needs no `gfx`/`mem` of its own: none of `pz()`'s own real Lua body
 * calls a Phase-1 drawing primitive directly (rn()/ok:cj()/nv()/js()'s own
 * construction-time `pq` all need at most `mem`/nothing, threaded through
 * their own established mechanisms, not this function's own parameter
 * list). A NULL `p` is a safe no-op (nothing to read the row's own shared
 * fields from). */
void pico8_pz(const pico8_instance_t *p, const pico8_list_t *bs);

#endif
