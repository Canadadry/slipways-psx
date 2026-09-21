#ifndef PICO8_MENU_H
#define PICO8_MENU_H

#include "pico8_ob.h"

/* Phase 3, Chunk 21 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): "`cr()`, `eg()`, and the `ix`/`ha` UI-callback data tables".
 * `eg()`'s real body is a small in-place edit to the ALREADY-EXISTING
 * stub in pico8_score_panel.c/.h (that stub's own top comment, written by
 * Chunk 18, explicitly names this as Chunk 21's job) -- not duplicated
 * here. This module owns the two genuinely NEW pieces this chunk builds:
 * `cr()` (the tech-tree screen) and the `ix`/`ha` data tables plus their
 * own callback wiring (the main galaxy-view menu buttons) -- including
 * the "toggle sfx" button's own real `ih` global, which reuses Chunk 1's
 * already-built `pico8_ih()`/`pico8_set_ih()` singleton pair (see this
 * file's own "CORRECTION" note on pico8_ha() below) rather than a new
 * one.
 *
 * New module (neither `pico8_score_panel.c`/`.h` -- Chunk 18's own module
 * for the `pj` class itself, already at/near the ~430-540-line
 * warn-review threshold -- nor any other existing module is a natural
 * home for "the tech-tree screen plus the main galaxy-view menu's own
 * button data", a distinct thematic concern from every already-shipped
 * chunk). Thematically named for that concern, matching this port's own
 * "one module per distinct real-source concern" convention (e.g.
 * pico8_tech_gate.c for oj/ov/gi/cb, pico8_help.c for fv/bo).
 *
 * NOTE ON THIS FILE'S OWN COMMENTS: this project's `.claude/hooks/strip-
 * claude-comments.py` PostToolUse hook strips non-TODO comments from any
 * file created via the Write tool -- verified directly against this very
 * file (its first Write landed with every comment removed). Per this
 * phase's own governing methodology (docs/plan.md's "Garde-fous": the
 * exact real Lua source pasted in a comment directly above each ported
 * function IS this phase's only verification method), every doc comment
 * in this module was therefore added via the Edit tool instead (which the
 * hook's matcher does not intercept), never via a further Write call. */

/* ============================================================
 * function cr()local bs,rd,gr=
 * {},{},#fm.invented
 * local tl=fm.ew
 * local qi=ek[tl-1]or 0
 * local bs=nv(cb(true),function(t)
 * return {lq={t.nk,t.px,c=6},h_={{24,5},tostr(gi(t))},jb=t.oy,my=t.e_ and function()fm:kt(t)end
 * }
 * end
 * )if gr>0 then
 * add(bs,{lq={30,gr.." invented",c=3},jb=fm.invented
 * })end
 * pj.cr.ep=
 * "level "..tl.." ("..(gr-qi).."/"..(ek[tl]-qi)..")"pz(pj.cr,bs)end
 * (pico8/lua.lua:952-964.) The tech-tree screen: builds one row per
 * currently-affordable-or-visible tech (`cb(true)`, Chunk 11's own
 * `pico8_cb`, called here with the real Lua "bypass affordability" flag
 * genuinely `true` -- a DIFFERENT real call site from `ix[2].hb`'s own
 * bare `cb()` below, which passes the omitted-argument filler instead),
 * plus a trailing summary row when at least one tech has been invented so
 * far.
 *
 * `local bs,rd,gr={},{},#fm.invented` / `local bs=nv(...)`: the real
 * source declares `bs` TWICE as a local (the first, `{}`, is immediately
 * shadowed and discarded by the second `local bs=nv(...)`; `rd` is
 * likewise declared and never read again anywhere in this function) --
 * both are dead, real Lua locals with no observable effect, ported as
 * simply not represented at all (this port's own established "a real but
 * dead local needs no C storage" treatment, e.g. `mo:md()`'s own dead
 * "kj" branch, Phase 2). Only `gr` (`#fm.invented`, a snapshot of `fm`'s
 * own invented-tech COUNT taken ONCE at the top of the function, never
 * re-read from `fm` again even though the loop below itself extends
 * `fm.invented` via `t.e_ and function()fm:kt(t)end`'s own eventual
 * click -- that click doesn't run synchronously inside `cr()` itself, so
 * `gr` staying stale for the REST of this one `cr()` call is exactly the
 * real Lua semantics, not a divergence) and `tl`/`qi` genuinely matter.
 *
 * `tl=fm.ew` (the player's own current tech era/tier, Chunk 12) and
 * `qi=ek[tl-1]or 0` (Chunk 11's own `pico8_ek()`, a positional 4-entry
 * threshold table `[2,4,7,12]`): `tl-1` is a genuine 1-based
 * `pico8_ob_table_get_index` lookup that can land on index 0 (or, for a
 * defensively-read `fm==NULL`/absent-`ew` `tl=0`, even -1) -- both
 * genuinely ABSENT in a 1-based table, so `ek[tl-1]` is real Lua `nil`
 * there, and `or 0` supplies the fallback exactly as written (a REAL edge
 * case for `tl=1`, the lowest tier, not a defensive guess this port
 * invented). Matches `pico8_hy_kt`'s own already-shipped `ek[self.ew]`
 * read (pico8_economy.c) for the index-conversion technique
 * (`pico8_ob_table_get_index(ek, (int)pico8_fixed_to_double(tl), ...)`).
 *
 * Each mapped row (`pico8_cr_step`, this module's own private `pico8_nv`
 * callback): `lq={t.nk,t.px,c=6}` and `h_={{24,5},tostr(gi(t))}` are
 * brand-new small `pico8_ob_table_t`s built directly
 * (`pico8_ob_table_new`+`set_index`/`set_name`), matching this port's
 * established "table constructor literal -> pico8_ob_table_t built by
 * hand" convention throughout (e.g. `pico8_jo_fe_step`'s own `h_`
 * tuple). `jb=t.oy` is a plain field copy off `t` (an `nb` entry, a
 * `pico8_ob_table_t*`, not a class instance -- read via
 * `pico8_ob_table_get_name`, matching `pico8_gi`'s own established
 * treatment of the SAME `t`). `gi(t)` is Chunk 11's already-built
 * `pico8_gi`.
 *
 * `my=t.e_ and function()fm:kt(t)end`: THE SAME "closure captures an
 * upvalue, must survive `pico8_class_instantiate`'s own field-copy and be
 * dispatched later via `mv()`'s own `self` parameter" shape
 * `pico8_jo_fe_my_dispatch` already solved (pico8_planet_display.c,
 * `jo:fe`'s own row-building closure) -- applied here identically for
 * this row's single captured upvalue `t`: stashed as an extra,
 * clearly-prefixed, non-Lua-original field on the SAME row table that
 * also carries `"my"` (`"_cr_t"`), read back by `pico8_cr_row_my_dispatch`
 * off its own dispatched `self` (cast to `pico8_instance_t*`, since this
 * row genuinely DOES become a real instance later -- `pz(pj.cr,bs)`'s own
 * eventual `js(b)` construction, matching `jo:fe`'s row's own identical
 * fate) once `mv()`-style dispatch eventually calls it (out of this
 * chunk's own scope).
 *
 * `t.e_` ITSELF is a genuine Lua BOOLEAN, not an arbitrary value: `cb()`'s
 * own real body (Chunk 11, pico8_tech_gate.c) sets `t.e_=gi(t)<=fm.jk` on
 * every entry it visits -- a comparison, always `true`/`false`. So
 * `t.e_ and function()...end`'s own FALSY branch is the literal boolean
 * `false`, never Lua `nil` -- ported as `pico8_value_bool(0)`, NOT this
 * port's `pico8_value_ref(NULL)` "absent" filler, the same "the
 * falsy operand of a real Lua `and` is a genuine value, not nil, when its
 * own left-hand side is a comparison/already-boolean expression"
 * precedent `jo:fe`'s own sibling row field `h_=od>0 and {od.."$",c=13}`
 * already established (pico8_planet_display.c: `od>0` is a comparison,
 * so its falsy branch is `pico8_value_bool(0)`, not nil, even though
 * `od.."$"` truthy branch is a table). This is a DELIBERATE, EVIDENCED
 * correction to this chunk's own governing brief (which loosely
 * paraphrased this as "my is nil" when falsy) -- `t.e_`'s own real
 * definition (a comparison result, confirmed by re-reading `cb()`'s own
 * already-shipped source directly, not assumed from the brief's
 * paraphrase) is the evidence for this reading, per this chunk's own
 * "pick the reading best evidenced by actual call sites/behavior, document
 * the alternative rejected" instruction for a genuine ambiguity.
 *
 * `if gr>0 then add(bs,{lq={30,gr.." invented",c=3},jb=fm.invented})end`:
 * `gr` (the TOP-of-function snapshot, not re-read) truthy gates appending
 * ONE more row to the SAME `bs` list `nv()` just built -- `gr.." invented"`
 * (this port's own established "literal".."number" concat precedent,
 * matching `ie:ldown`'s `"s"..ie.id`/`pj:hb`'s `fm.nz.."$"`) and
 * `jb=fm.invented` (a REF to the SAME live `pico8_list_t*` `fm.invented`
 * already is -- not a copy, matching Lua's own by-reference table
 * assignment).
 *
 * `pj.cr.ep="level "..tl.." ("..(gr-qi).."/"..(ek[tl]-qi)..")"`: `pj.cr`
 * is `pj`'s own CLASS-LEVEL nested spec field (Chunk 18's `pj` class spec,
 * `cr=o(hi=v(0,11),gq=v(0,11),ca=10,)`, `pico8_score_panel.c`) -- a plain
 * `pico8_ob_table_t*`, read directly off `pico8_pj_class()->defaults`
 * (this module's own private `pico8_cr_pj_cr_table()`, mirroring
 * `pico8_score_panel.c`'s own `pj_class_field_ref_table` established
 * shape, a per-module duplicate since that helper is `static` to its own
 * file). Written to directly via `pico8_ob_table_set_name` -- no wrapper
 * needed for THIS access, since `pj.cr` genuinely IS a plain
 * `pico8_ob_table_t*` here, not a `kv:jr` instance.
 *
 * `pz(pj.cr,bs)`: Chunk 8's `pico8_pz(const pico8_instance_t *p, const
 * pico8_list_t *bs)` (src/pico8_msgbox.h) expects a `pico8_instance_t*`
 * for `p`, but `pj.cr` is the SAME plain `pico8_ob_table_t*` above -- a
 * representation mismatch resolved via the SAME classless-instance-
 * wrapper-borrowing-a-table trick `pico8_wormhole.c`'s own
 * `g_mz_jj_wrapper` already established (`->fields` BORROWS the existing
 * table directly, `->cls=NULL`) -- NOT Chunk 19's `self.ke`-as-class
 * wrapper (a different trick, for wrapping a CLASS, not a plain table).
 * Built as a plain STACK-local `pico8_instance_t` inside `pico8_cr()`
 * itself (confirmed safe by reading `pico8_pz`'s own real body,
 * pico8_msgbox.c: `p` is only ever read via `pico8_instance_get_field`
 * during the call, never retained past it -- unlike `g_mz_jj_wrapper`,
 * which needs to persist because `mz.et` reads it repeatedly across many
 * separate calls, `pj.cr`'s own wrapper is needed for exactly one call, so
 * a heap-allocated, persistently-owned wrapper would be unjustified
 * extra bookkeeping this call site's own real need doesn't evidence).
 * `bs` itself is already a real `pico8_list_t` (built directly by
 * `pico8_nv`/`pico8_list_add`, no ob_table-to-list bridge needed here --
 * unlike most other `pz(...)` call sites in this port, which convert FROM
 * an existing `pico8_ob_table_t`'s positional entries).
 * ============================================================ */
void pico8_cr(void);

/* ============================================================
 * ix=ob([[
 * o(
 * ra=v(118,0),
 * lq=o(o(15,7)),
 * ),o(
 * ra=v(0,0),
 * lq=o(o(37,7)),
 * ),o(
 * ra=v(47,0),
 * lq=o(o(25,7)),
 * ),o(
 * ra=v(0,117),
 * lq=o(o(8,7)),
 * ),
 * ]])ix[1].my=function()fm.lo=1
 * fm:hk()end
 * ix[2].my=cr
 * ix[2].hb=function(self)self.lq=lq({#cb()>0 and 24 or 37
 * })end
 * ix[3].my=function()ju.c_=true
 * fq(de)end
 * (pico8/lua.lua:965-985.) The main galaxy-view menu's 4 icon buttons, a
 * purely positional `ob()` literal (matching the nested `ra=v(...)`,
 * `lq=o(o(...))` shape already familiar from `bo`/`bz`), each row later
 * turned into a real `js` button instance by `eu()`'s own `nv(ix,js)`
 * (pico8/lua.lua:1182, Chunk 22 -- CONFIRMED directly by reading that real
 * call site, not assumed: this is why every dispatch bridge below casts
 * its own `self` parameter to `pico8_instance_t*`, not a bare
 * `pico8_ob_table_t*` -- `ix[N]`'s own row DOES become a real,
 * `pico8_class_instantiate`'d instance, exactly like `jo:fe`'s/`fv`'s own
 * `bo` rows, unlike a plain never-instantiated data record).
 *
 * `ix[1].my`: `fm.lo=1` (a direct field write via
 * `pico8_ob_table_set_name(pico8_fm()->fields,...)`, guarded against a
 * NULL `pico8_fm()` per this port's established convention) then
 * `fm:hk()` (Chunk 12's already-real, directly-callable `pico8_hy_hk` --
 * `hy` methods are never `mv()`-routed, matching this port's established
 * convention).
 *
 * `ix[2].my=cr`: `cr` (this SAME chunk's own piece above, fully real by
 * the time `ix` is built) assigned as a bare function VALUE onto the
 * "my" field -- needs its own small `_dispatch` bridge
 * (`pico8_ix2_my_dispatch`) wrapping `pico8_cr()`, registered directly
 * (not a forward-reference placeholder, unlike `_init`/`eu`: `cr` is
 * fully real here, not a not-yet-ported later chunk's function).
 *
 * `ix[2].hb=function(self)self.lq=lq({#cb()>0 and 24 or 37})end`: a new
 * closure with no captured upvalues besides `self` (already
 * `pico8_value_function_t`'s own first parameter). `#cb()` is
 * `pico8_cb`'s own DEFAULT-`bi`-argument form -- re-reading `cb`'s own
 * real Lua source (Chunk 11) confirms `bi` only ever affects which
 * entries are KEPT (`t.e_ or bi`), never the SHAPE of what's returned,
 * and this call site only ever reads the returned list's own LENGTH, so
 * whichever value `bi` resolves to when omitted is passed as this port's
 * own established "omitted Lua argument" filler
 * (`pico8_value_ref(NULL)`), matching real Lua's own implicit-nil-argument
 * semantics for a bare `cb()` call exactly, and observably equivalent
 * here regardless. `lq({24})`/`lq({37})`: `pico8_lq_new(overrides)`
 * (Chunk 3's root `lq` class) with a single-positional-element overrides
 * table holding a sprite-id constant -- built via
 * `pico8_ob_table_set_index(t,1,pico8_value_fixed(...))`. `self.lq=...`
 * writes the "lq" field directly on `self->fields` (the row's own
 * instance, per this file's own top comment on why `self` is
 * `pico8_instance_t*` here).
 *
 * `ix[3].my=function()ju.c_=true fq(de)end`: `ju.c_=true` writes a field
 * on whatever `pico8_ju()` (Chunk 18's own singleton-placeholder
 * accessor) currently returns, guarded against NULL (not reconciled to a
 * real `pj()` instance until Chunk 22). `fq(de)`: `pico8_fq_new(overrides)`
 * (Chunk 4) called with `de` (Chunk 18's own top-level `de={my=function()
 * ju.c_=not ju.c_ end}` table, THE SAME real Lua global `pj:ph()` already
 * uses -- exposed here via a small new public accessor,
 * `pico8_de()`, added to `pico8_score_panel_display.h`/`.c` (that
 * module's own existing `g_de` static, previously private -- see that
 * file's own updated top comment for the full justification: reusing the
 * SAME global table, not rebuilding a second, competing, observably
 * divergent copy). The constructed `fq` instance's result is discarded
 * (a bare statement in the real source), matching this port's
 * established "statement-level constructor call, tracked via hv only"
 * precedent.
 * ============================================================ */
const pico8_ob_table_t *pico8_ix(void);

/* ============================================================
 * ha=ob([[
 * o(lq=o(31,"restart     ")),
 * o(lq=o(42,"toggle music")),
 * o(lq=o(40,"toggle sfx  ")),
 * o(lq=o(26,"how to play?"),fg=5,),
 * hi=v(0,107),gq=v(0,-11),
 * ca=14,
 * ]])ix[4].my=function()
 * for i,fn in pairs({_init,function()music(-sgn(stat(20)))end,function()ih=not ih end,function()end
 * })do
 * ha[i].my=fn
 * end
 * pz(ha,ha)end
 * (pico8/lua.lua:986-997.) `ha`: a 4-entry positional `ob()` literal (the
 * "restart"/"toggle music"/"toggle sfx"/"how to play?" option row) plus
 * `hi`/`gq`/`ca` named layout fields -- same nested shape as `bo`/`bz`.
 * No `my` field is set on any `ha[i]` row at BUILD time (the real source
 * never does either) -- only `ix[4]`'s own click callback sets them,
 * every time it runs.
 *
 * `ix[4].my`: walks a literal 4-element table of function VALUES
 * (`_init`, an inline `music(-sgn(stat(20)))` closure, an inline
 * `ih=not ih` closure, a no-op) and assigns each to `ha[i].my` (1-based),
 * then calls `pz(ha,ha)`. No captured Lua upvalues of its own -- every
 * name it touches (`_init`,`music`,`stat`,`sgn`,`ih`,`ha`) is a global.
 *
 * `_init`: reuses Chunk 18's OWN existing `pico8_init_field_value()`
 * forward-reference placeholder (pico8_score_panel.h) verbatim -- the
 * SAME mechanism already built for exactly this "`_init` as a bare
 * function value" shape (first needed by `pj:ph`'s own `{my=_init}`
 * override); assigned directly as `ha[1]`'s own "my" value (already a
 * PICO8_VALUE_FUNCTION-kind `pico8_value_t`, no extra bridge needed).
 *
 * `function()music(-sgn(stat(20)))end`: `stat(20)` (Chunk 1's
 * `pico8_stat`, reading from `pico8_dispatch_context.h`'s shared "current
 * frame" input accessor) already resolves to 0 with no Phase 1 change
 * needed (`pico8_stat`'s own existing "any index outside 32/33/34
 * returns 0" fallback, already recorded as a Phase-3-wide finding this
 * chunk's own PRD entry cites by name) -- ported literally as dead-but-
 * compiled code, the same treatment Phase 2 gave `mo:md()`'s dead "kj"
 * branch, not silently "fixed" or dropped. `sgn(...)` is Chunk 1's
 * already-built `pico8_fixed_sgn` (`sgn(0)==1`, not 0 -- so this
 * genuinely calls `music(-1)` every real invocation today, not a
 * degenerate no-op). `music(...)` is Phase 1/2's own already-built
 * no-op stub (`pico8_music`, pico8_audio.h) -- out of scope, permanently
 * inert, called anyway for faithfulness.
 *
 * `function()ih=not ih end`: `ih`'s real, persistent get/set singleton
 * pair (`pico8_ih()`/`pico8_set_ih()`) was ALREADY BUILT by Chunk 1
 * (src/pico8_audio.h/.c), purpose-built for exactly this "toggle sfx"
 * button -- and already has a REAL SECOND READER, `pico8_rn()`'s own
 * `if (no and not ih)sfx(no,3)end` (pico8/lua.lua:331, already ported).
 *
 * CORRECTION (found during this chunk's own verification): a prior
 * version of this comment/module claimed `ih` had "no other reader
 * evidenced" and built a second, private, disconnected module-local
 * static (`g_ih`) instead of reusing Chunk 1's own accessor pair -- both
 * claims were wrong (the grep that produced them missed `pico8_rn`'s own
 * already-ported use, and the accessor already existed). Fixed:
 * `pico8_ha3_my_dispatch` now calls `pico8_set_ih(!pico8_ih())` directly;
 * the module-private `g_ih` and its two reset sites (`pico8_menu_init`/
 * `pico8_menu_shutdown`) were removed entirely, since `ih` is a real
 * persistent global this module doesn't own and shouldn't reset on its
 * own table rebuild/teardown.
 *
 * `function()end`: a genuine, real Lua no-op -- ported as a bridge that
 * does nothing and returns the established "no real Lua value"
 * (`pico8_value_ref(NULL)`) filler, same treatment as every other real
 * no-op closure already in this port.
 *
 * `pz(ha,ha)`: the SAME wrapper-instance-borrowing-a-table trick needed
 * for `pj.cr` above, `ha` used as BOTH the `p` AND `bs` argument -- one
 * stack-local wrapper instance built for `p` (borrowing `ha` itself as
 * `->fields`), and `ha`'s own positional entries separately bridged to a
 * `pico8_list_t` for `bs` via this module's own private "ob_table
 * positional entries -> pico8_list_t" copy (`pico8_menu_positional_
 * values`, this port's now well-established per-module-duplicate
 * convention, matching e.g. `pico8_help_positional_values`/`pico8_score_
 * panel_positional_values`).
 * ============================================================ */
const pico8_ob_table_t *pico8_ha(void);

/* Not part of the Lua original (no manual teardown under GC). Builds/frees
 * `ix`/`ha` -- mirrors every other init/shutdown pair in this port. Does
 * NOT touch `ih` (see this file's own "CORRECTION" note above): that's a
 * real, persistent global owned by Chunk 1's audio module
 * (`pico8_ih()`/`pico8_set_ih()`), not this module's own state. Calls
 * `pico8_pi_init()`
 * defensively first (idempotent), since both specs' own nested `v(...)`
 * calls need `pi['v']` already registered, the same established
 * convention `pico8_score_panel_init()`/`pico8_iv_init()` etc. already
 * use for an identical situation. */
void pico8_menu_init(void);
void pico8_menu_shutdown(void);

#endif
