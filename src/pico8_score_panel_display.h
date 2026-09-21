#ifndef PICO8_SCORE_PANEL_DISPLAY_H
#define PICO8_SCORE_PANEL_DISPLAY_H

#include "pico8_class.h"
#include "pico8_fixed.h"

/* Phase 3, Chunk 18 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `pj:ph()`, split out of `pico8_score_panel.c`/`.h` into its
 * own sibling module -- the same "split off the one oversized method,
 * keep the class declaration plus every other method in the main file"
 * precedent `pico8_planet_display.h`'s own top comment already
 * establishes for `jo:fe` (that file's own top comment: "`fe`'s own bulk
 * -- the 'my' closure's context struct, dispatch bridge, and per-row
 * mapper step -- is what re-blew the budget"). `pj:ph()` is, by a wide
 * margin, this chunk's biggest and most involved real method (the
 * end-of-game/"play again?" flow), so it gets the same treatment
 * proactively rather than waiting for a warn-review flag, per this
 * chunk's own explicit "check line counts as you go and split
 * proactively past ~400 lines" instruction.
 *
 * function pj:ph()if fm.hl and not self.gz then
 * fq()js(ob([[
 * fa=15,
 * ra=v(51,100),
 * lq=o(" again? "),
 * ]],{my=_init}))js(ob([[
 * fa=15,
 * ra=v(39,100),
 * lq=o(o(10,7)),
 * ]],de))self.c_,self.gz,self.fa=
 * true,0,13
 * end
 * if self.c_ and kc~=self.ed then
 * local s=ky()local iw=nv(bz,function(st)if st.score then
 * st[3]=tostr(s[st.score])if st.score==0 then
 * st[1]=(fm.hl or "current score").." ("..dk.px..")"end
 * if st.score==6 then
 * for i=1,5 do
 * st[i]=i<=s[6]and 56 or 57
 * end
 * end
 * return {lq(st),st.ra,0.5,fn="ld"}
 * else
 * return st
 * end
 * end)iw.rh=3
 * function iw.lh()
 * return self.c_
 * end
 * self.ed=mh(iw)end
 * end
 * (pico8/lua.lua:870-896, plus the top-level `de={my=function()ju.c_ = not
 * ju.c_ end}` table it needs, pico8/lua.lua:901.)
 *
 * FIRST BLOCK -- `if fm.hl and not self.gz then ... end`: `fm.hl` (a
 * game-over message string, set by some not-yet-ported `hy` method once
 * the game genuinely ends -- read defensively here the same guarded way
 * every other `pico8_fm()` read in this port already is) truthy AND
 * `self.gz` still ABSENT (Lua truthiness on PRESENCE, not on the numeric
 * value -- `self.gz` starts unset, so `not self.gz` is true until this
 * very block sets it) together gate a ONE-SHOT setup that fires exactly
 * once per game-over:
 *   - `fq()` -- Chunk 4's modal backdrop, `pico8_fq_new(NULL)`, no
 *     overrides. Its own constructed instance is discarded (a bare
 *     statement call in the real source), tracked onward only via
 *     `hv`/`cv`/`cg` (`fq`'s own inherited `ke.pq=lt`) -- the same
 *     "statement-level constructor, result simply not captured"
 *     precedent `pj:hk`'s own bare `mh(iw)` call already establishes
 *     (pico8_score_panel.c).
 *   - `js(ob([[fa=15,ra=v(51,100),lq=o(" again? "),]],{my=_init}))` --
 *     builds a small `{my=_init}` overrides table (ONE named field, this
 *     module's own `pico8_ob_table_t*`, built via `pico8_ob_table_new()`+
 *     `pico8_ob_table_set_name(..., "my", pico8_init_field_value())`,
 *     `pico8_score_panel.h`'s own `_init` forward-reference placeholder --
 *     see that header's own top comment for the full trace), passes it as
 *     `ob()`'s own `defaults`/second-argument parameter (`pico8_ob(spec,
 *     overrides)`, Phase 2's own `pico8_ob.c` -- CONFIRMED, not the FIRST
 *     real two-argument `pico8_ob()` call site as a first guess might
 *     assume: `pico8_ob.c`'s own top comment on its `qj(kn,mr)` merge
 *     direction ALREADY cites this EXACT real call site, `js(ob([[...]],
 *     {my=_init}))`, pico8/lua.lua:875, by name and line number as its own
 *     worked evidence for why `defaults` OVERWRITES same-named parsed
 *     fields rather than merely filling gaps -- so this chunk is simply
 *     the first to actually EXERCISE that already-reasoned-through
 *     mechanism, not the first to design it), then constructs a real `js`
 *     button (`pico8_js_new`) from the merged spec. Result discarded, same
 *     "tracked via hv only" treatment as `fq()` above. Freed after use:
 *     both the small `{my=_init}` table and `pico8_ob()`'s own merged
 *     result, matching `pico8_planet_family.c`'s own established
 *     "build overrides via pico8_ob(spec, defaults), free defaults right
 *     after, instantiate, then free the merged overrides too" sequence
 *     exactly (that file's own `pico8_ga_new` call site).
 *   - `js(ob([[fa=15,ra=v(39,100),lq=o(o(10,7)),]],de))` -- the SAME
 *     shape, using the top-level `de` table (built once by this module's
 *     own init, see below) as the merge-in overrides instead of a
 *     freshly-built one-field table. `de`'s own "my" callback IS fully
 *     real and buildable now (unlike `_init`): `function() ju.c_ = not
 *     ju.c_ end` needs only `pico8_ju()` (this chunk's OWN singleton, see
 *     pico8_score_panel.h), so `pico8_pj_de_my_dispatch`
 *     (pico8_score_panel_display.c) is a complete, real implementation,
 *     not a placeholder -- built once, alongside `pj`'s own class, in
 *     this module's own init function (`pico8_score_panel_display_init`,
 *     called by `pico8_score_panel_init()`).
 *   - `self.c_,self.gz,self.fa=true,0,13`: three genuinely per-INSTANCE
 *     field writes -- `self.c_=true` (the score-panel visibility toggle
 *     `de`'s own callback flips), `self.gz=0` (a REAL fixed 0, TRUTHY in
 *     Lua -- this is what makes `pj:kj`'s own `if self.gz then...end`
 *     start firing every frame from here on, kicking off the reveal
 *     curtain), `self.fa=13` (a layer/priority field, read by not-yet-
 *     ported drawing-order code out of this chunk's own scope).
 *
 * SECOND BLOCK -- `if self.c_ and kc~=self.ed then ... end`: whenever the
 * score panel should be showing (`self.c_` true) AND the currently-open
 * message box (`pico8_kc()`, Chunk 8) ISN'T ALREADY the one this `pj`
 * instance itself last built (`self.ed`, a genuinely per-instance field --
 * compared here by RAW POINTER IDENTITY, the direct C equivalent of Lua's
 * own `~=` on two table references), a fresh score-breakdown message box
 * is built:
 *   - `local s=ky()` -- Chunk 12's `pico8_ky()`, a freshly heap-allocated
 *     6-entry `pico8_ob_table_t*` the caller owns; freed at the end of
 *     this block (its own values are copied out, not retained by
 *     reference, into the `st[3]`/icon writes below, so freeing `s` here
 *     is safe).
 *   - `local iw=nv(bz,function(st)...end)` -- maps `bz` (this chunk's own
 *     12-entry data literal, `pico8_score_panel.h`'s `pico8_bz()`,
 *     converted to a `pico8_list_t` via this module's own private
 *     `pico8_score_panel_display_positional_values` copy -- yet another
 *     per-module duplicate of the same "ob_table positional entries ->
 *     list" bridge already used a dozen times elsewhere in this port, kept
 *     separate from `pico8_score_panel.c`'s own identically-shaped copy
 *     per this port's established "duplicated across modules, not shared
 *     across a file boundary for a few-line helper" convention) through
 *     `pico8_nv()` (Chunk 1), with a small ctx struct capturing `s`. The
 *     closure itself (`pico8_pj_ph_map`, pico8_score_panel_display.c)
 *     MUTATES each `st` (one of `bz`'s own shared class-level sub-tables)
 *     directly, in place, exactly the same "class-level table mutated
 *     across calls" pattern `pj.ou`/`pj.ea` already establish elsewhere in
 *     this chunk -- `bz`'s own entries are never copied, only rewritten,
 *     so repeated `pj:ph()` runs simply re-derive the same fields from the
 *     current `s`/`fm`/`dk` state each time:
 *       - Entries with NO `.score` field (`bz`'s own first 5 decorative
 *         `fn="br"` bars) are returned UNCHANGED (`else return st end`).
 *       - Entries WITH a `.score` field (0 is a real NUMBER, hence TRUTHY
 *         in Lua -- `if st.score then` genuinely enters this branch for
 *         `score=0` too, not just 1-6): `st[3]=tostr(s[st.score])` reads
 *         `s`'s own 1-based positional entry at index `st.score`. FOR
 *         `st.score==0` SPECIFICALLY, `s[0]` is genuinely ABSENT (`ky()`'s
 *         own real return only ever sets keys 1 through 6, never 0) --
 *         real Lua's own `tostr(nil)` result for this exact, evidenced
 *         call site is NOT reproduced exactly here: this port's own
 *         `pico8_value_tostr` has no PICO8_VALUE_NIL kind to format (see
 *         `pico8_value.h`'s own top comment: "at least" 5 kinds, no nil),
 *         so the established "absent" filler (`pico8_value_ref(NULL)`)
 *         is passed through instead, which `pico8_value_tostr` already
 *         documents as rendering to `""` (empty string) for a REF kind --
 *         a real, newly-EXERCISED instance of that already-documented,
 *         previously-inert divergence (`pico8_value.h`'s own comment: "no
 *         real call site in scope exercises any of them"), flagged here
 *         rather than silently reproduced as if it were `tostr`'s real,
 *         researched PICO-8 behavior for nil. `st.score==0`'s own extra
 *         `st[1]=(fm.hl or "current score").." ("..dk.px..")"` reads
 *         `dk.px` (Chunk 12's ALREADY-BUILT `pico8_dk()`, guarded against
 *         NULL the same defensive way every other not-yet-reconciled
 *         forward reference in this port already is -- see
 *         `pico8_score_panel.h`'s own top comment on why no SECOND `dk`
 *         accessor is declared here). `st.score==6`'s own extra `for
 *         i=1,5 do st[i]=i<=s[6] and 56 or 57 end` repurposes `st`'s OWN
 *         positional slots 1-5 (this score row's own `o(score=6,ra=v(64,
 *         82))` literal starts with NO positional entries at all) as a
 *         5-icon star-rating row (sprite 56 = filled, 57 = empty),
 *         compared against `s[6]` (`ky()`'s own final 0-5 tier value).
 *         Every scored entry then returns a FRESH small 4-field wrapper
 *         table, `{lq(st),st.ra,0.5,fn="ld"}` -- `lq(st)` is a REAL `lq`
 *         layout-row CONSTRUCTOR CALL (`pico8_lq_new(st)`, Chunk 3's root
 *         `lq` class, pico8_layout.h -- confirmed against `pico8_lq_new`'s
 *         own real signature/`lq`'s own class spec before committing to
 *         this reading, exactly as this chunk's own brief asked: an `lq`
 *         instance is exactly the shape a `bl()`-drawn row needs to become
 *         to be positioned/laid out, matching how `lq` instances are
 *         already constructed elsewhere in this port, e.g. `js:pq`'s own
 *         `self.lq=lq(mv(self,"lq"))`), NOT a field/method call on `st`.
 *         `fn="ld"` selects `pi`'s own already-registered "ld" constructor
 *         key (`pi.ld=lq.ol`, Chunk 3's `pico8_layout.c`) for whatever
 *         future drawing code eventually consumes this wrapper the same
 *         way every other `fn="..."`-tagged row item already is elsewhere
 *         in this port -- out of THIS chunk's own scope to render further.
 *   - `iw.rh=3` / `function iw.lh() return self.c_ end`: real Lua's
 *     `nv()` returns an ordinary Lua table (array part AND hash part in
 *     one), so `iw` can receive these two EXTRA named fields after the
 *     fact -- but this port's own `pico8_nv()` returns a plain
 *     `pico8_list_t` (array-only, no room for named fields at all). This
 *     chunk therefore adds a genuinely NEW small bridge, the mirror image
 *     of the well-established "ob_table positional entries -> list"
 *     conversion: `pico8_score_panel_display_list_to_ob_table` (private,
 *     pico8_score_panel_display.c) builds a FRESH `pico8_ob_table_t*` with
 *     one positional entry per list element, onto which `"rh"`/`"lh"` are
 *     then set as ordinary named fields -- exactly what `mh(iw)`'s own
 *     `overrides` parameter (`const pico8_ob_table_t*`) needs to carry
 *     both the message box's own option ROW (positional) and its `rh`/
 *     `lh` BEHAVIOR fields (named) in the one table real Lua's `iw` always
 *     was. `iw.lh`'s own real closure captures `self` as its OWN upvalue
 *     (not a fresh local like `pj:hk`'s own `t`) -- and since `pj()` is
 *     constructed EXACTLY ONCE in the whole real cart (confirmed by
 *     grepping every real `pj(` call site in pico8/lua.lua: only `eu()`'s
 *     own `fm,ju=hy(),pj()`, pico8/lua.lua:1182), `self` inside `pj:ph()`
 *     is PROVABLY, ALWAYS the same object `pico8_ju()` itself returns --
 *     so `pico8_pj_ph_lh_dispatch` (pico8_score_panel_display.c) reads
 *     `pico8_ju()`'s own "c_" field directly, rather than inventing a
 *     second, redundant static-capture mechanism (`pj:hk`'s own `t`
 *     upvalue, by contrast, has no such existing global accessor to reuse,
 *     which is why THAT one still needs its own dedicated static slot,
 *     pico8_score_panel.c) -- an evidenced simplification, not a guess.
 *   - `self.ed=mh(iw)`: UNLIKE `pj:hk`'s own bare, discarded `mh(iw)`
 *     call, this one's result IS captured -- into `self.ed`, a genuinely
 *     per-instance field, read back next tick by this SAME block's own
 *     `kc~=self.ed` guard. The `iw` table itself (and the `s` breakdown
 *     table) are freed right after this call (matching `pico8_planet_
 *     family.c`'s own "build overrides, instantiate, free overrides"
 *     sequence) -- the nested per-row wrapper tables `iw`'s own positional
 *     entries reference are NOT freed here (they persist as part of the
 *     constructed `mh` instance's own shallow-copied fields, the same "no
 *     GC, entities/their row-item tables simply persist until nothing
 *     needs them any more" convention this whole port already accepts
 *     pervasively for entity lifetimes -- a genuine, accepted per-refresh
 *     leak, not a new gap unique to this chunk, since this block only
 *     re-runs when the box isn't already open, not every frame).
 *
 * `ph` is NOT one of `bm`'s 5 real dispatch names (`bm=["kj","ev","hd",
 * "hk","hb"]`, pico8_entity.c) -- CHECKED DIRECTLY per this chunk's own
 * instruction, not assumed from the name matching `qb:ph`/`nf:ph`/`js:ph`/
 * `iv:ph`'s own arity: `pj`'s own class spec never overrides `ke`'s
 * inherited default `me="ph"` field, so `pj:ph()` IS reached, every real
 * game tick, by `dg()`'s own per-entity `qc[qc.me](qc,qc.t)` dispatch
 * (`pico8_bootstrap.c`'s own `pico8_mv(qc, me_val.as.string, 1, t_arg,
 * pico8_value_ref(NULL), &discard)` call site) -- the EXACT SAME
 * dispatch shape/arity as `iv:ph`/`js:ph`/`qb:ph`/`nf:ph` (argc=1,
 * arg0=`self.t`, unused by the real body, which declares no Lua
 * parameters of its own), simply reached through a DIFFERENT mechanism
 * (`dg()`'s own `me`-field dispatch) than `bm`/`dc()`'s tag-list dispatch
 * -- confirmed by finding `ph`'s own real call site (`dg()`) rather than
 * assuming it matches those other classes' arity merely because the name
 * does. */
void pico8_pj_ph(pico8_instance_t *self);

/* function pj:hk(yr)local iw,pp=
 * pj.ea,pj.gs[3426-yr]
 * iw[2][1],iw[3]=
 * "year "..yr,pp or nil
 * if pp then
 * local t=90
 * function iw.lh()
 * t-=1
 * return t>0
 * end
 * end
 * mh(iw)
 * end
 * (pico8/lua.lua:856-869.) MOVED HERE from `pico8_score_panel.c`/`.h`
 * once that file's own line count crossed the ~430-540-line range this
 * port's own warn-review threshold flags (docs/portage-progress.md's
 * Vague 18 entry has the exact before/after tally) -- `pj:hk`'s own
 * dispatch bridge (`pico8_pj_hk_dispatch`) stays in `pico8_score_panel.c`,
 * alongside `pj`'s own class-registration wiring, calling across to this
 * function exactly as it already did as a same-file sibling before the
 * move -- the SAME "dispatch bridge lives with the registration code,
 * function lives in the sibling module" precedent `pico8_jo_hb_dispatch`/
 * `pico8_jo_gv_dispatch` (`pico8_planet.c`, calling into `pico8_planet_
 * display.h`) already establishes; no wording/semantics changed by the
 * move, only file location.
 *
 * `iw[3]=pp or nil` runs UNCONDITIONALLY every call (a real Lua
 * statement, not inside the `if pp` block) -- ported as an if/else that
 * either sets index 3 to `pp` or REMOVES it outright
 * (`pico8_ob_table_remove_index`, new this chunk, see pico8_ob.h's own
 * comment on it) -- this is the ordinary, exercised path (real gameplay's
 * own `yr` countdown genuinely toggles `pp` between truthy and falsy
 * across successive years, confirmed by working through `pj.gs`'s own
 * 5-entry shape against `3426-yr`), not a rare edge case, so simply
 * "skipping the write" (this port's usual treatment for a real Lua
 * nil-write with no previously-live value to take away) would leave a
 * stale, previously-real `iw[3]` behind -- a genuine, observable
 * divergence this chunk's own new `pico8_ob_table_remove_index` avoids.
 * The `iw.lh` closure setup, by contrast, genuinely has NO else branch in
 * the real source -- a stale `pj.ea.lh` from an earlier truthy `pp` is
 * faithfully left in place (not cleared) whenever a later call's `pp` is
 * falsy, exactly mirroring the real Lua's own `if pp then...end` with no
 * `else iw.lh=nil`.
 *
 * `iw.lh`'s own captured `t` upvalue is represented as a SINGLE
 * module-level static int (`g_pj_hk_lh_t`, pico8_score_panel_display.c),
 * not a per-call heap allocation the way `pico8_gen.h`'s own `jf`/`gn`
 * closures are -- a deliberately narrower adaptation of that same
 * "closure -> heap/static state" precedent, evidenced correct here
 * because `pj.ea.lh` is a SINGLE class-level field, wholesale-replaced by
 * every `pj:hk()` call that sets it at all, so only ONE such closure is
 * ever "the current one" a real `mv(mh_instance,"lh")` call could reach --
 * unlike `jf`/`gn`'s own factories, which can produce multiple
 * independently-live closures simultaneously.
 *
 * `mh(iw)`'s own return value is discarded (a bare statement in the real
 * source -- `pj:ph()`'s own `self.ed=mh(iw)` is the DIFFERENT call site
 * that captures it) -- the constructed instance stays alive via `mh:pq`'s
 * own `kc=self`/`ke.pq=lt` self-registration into `hv`/`cv`/`cg`, matching
 * `bg:ldown`'s own "statement-level constructor call, result simply not
 * captured, still tracked by hv" precedent. `pj:hk`'s own declared `yr`
 * parameter IS genuinely read (unlike `hb`/`kj`'s own ignored dispatch
 * arguments). */
void pico8_pj_hk(pico8_fixed_t yr);

/* `de={my=function()ju.c_ = not ju.c_ end}` (pico8/lua.lua:901, the
 * top-level table right after `pj`'s own methods) -- see this file's own
 * top comment for the full trace on why it lives here (its only real use
 * is inside `pj:ph()`) and why, unlike `_init`, it needs no forward-
 * reference placeholder at all: `pico8_pj_de_my_dispatch`
 * (pico8_score_panel_display.c) is a complete, real implementation today,
 * since everything it needs (`pico8_ju()`) is this SAME chunk's own new
 * singleton. Built once by `pico8_score_panel_display_init()` below
 * (called by `pico8_score_panel_init()`, pico8_score_panel.c), freed by
 * `pico8_score_panel_display_shutdown()`. */
void pico8_score_panel_display_init(void);
void pico8_score_panel_display_shutdown(void);

#endif
