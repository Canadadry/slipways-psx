#ifndef PICO8_SCORE_PANEL_H
#define PICO8_SCORE_PANEL_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 18 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): the `pj` class (score/status panel), plus the two data
 * literals only it consumes (`ef`, `bz`), plus three genuinely new
 * forward references this chunk's own real Lua source introduces (`_init`,
 * `ju`; `dk` was ALREADY built by Chunk 12 -- see this file's own note
 * below). `pj:ph()` itself -- the biggest, most involved of the four real
 * methods -- lives in the sibling module `pico8_score_panel_display.c`/
 * `.h` (see that header's own top comment for the split rationale, the
 * same "split off the one oversized method, keep everything else here"
 * precedent `pico8_planet_display.h`'s own top comment already
 * establishes for `jo:fe`).
 *
 * pj=ke:jr([[
 * fa=10,
 * ea=o(
 * o(42,43,1,fn="br"),
 * o("",64,40,6,0.5,fn="qr"),
 * ),
 * gs=o(
 * o("last year!",64,48,9,0.5,fn="qr"),
 * o("2 years remain",64,48,2,0.5,fn="qr"),
 * f,f,
 * o("5 years remain",64,48,2,0.5,fn="qr"),
 * ),
 * cr=o(
 * hi=v(0,11),gq=v(0,11),ca=10,
 * ),
 * i_=o(
 * o(126,127,1,fn="br"),
 * o(0,1,1,fn="br"),
 * o(8,118,34,fn="rf"),
 * o(0,-3,26,fn="rf"),
 * o(81,-3,117,fn="rf"),
 * ),
 * ou=o(
 * o(f,13,121,9,fn="qr"),
 * o(f,37,121,2,fn="qr"),
 * o(f,13,2,9,fn="qr"),
 * o(f,29,2,2,fn="qr"),
 * o(f,59,2,2,fn="qr"),
 * o(f,85,2,13,fn="qr"),
 * o(f,99,2,13,fn="qr"),
 * ),
 * ed=1,
 * ]])
 * (pico8/lua.lua:815-846.)
 *
 * `pj=ke:jr(...)`: EXTENDS `ke` directly (Chunk 3, pico8_ke.h), same
 * inheritance depth/mechanism as nf/js/fq/qb/bg/jo/mh/jc/g_/mz/ie/iv --
 * `pico8_class_declare(pico8_ke_class(), spec)`, nothing new to build for
 * the class-system itself. `pj` never sets `nu=o(...)` (no `cg["pj"]` tag
 * list -- confirmed by reading the spec above line by line, same "real,
 * confirmed divergence, not an oversight" treatment `fv`'s own missing
 * `nu=` already got, pico8_help.h) and never sets `fz` either, so it
 * registers under the shared ""-keyed pico8_class_lookup() slot like most
 * other classes in this port -- `pico8_pj_class()`/`pico8_pj_new()` are
 * this chunk's own plain hand-off points, matching that same precedent.
 *
 * EVERY nested `v(...)` in this spec (`cr`'s own `hi=v(0,11),gq=v(0,11)`)
 * needs Chunk 2's char-keyed `pi['v']` constructor already registered, so
 * `pico8_score_panel_init()` below calls `pico8_pi_init()` defensively
 * first (idempotent), the same established convention `pico8_iv_init()`/
 * `pico8_mouse_reader.c`/`pico8_scroll.c` already use for an identical
 * "this class's own spec has a nested v(...)/o(...) constructor call"
 * situation (pico8_msgbox.h's own top comment spells out exactly this
 * check).
 *
 * FIELD-BY-FIELD, CLASS-LEVEL VS INSTANCE-LEVEL (confirmed by reading
 * `pj:hb`/`pj:kj`/`pj:hk`'s own real bodies below, not assumed): `ea`,
 * `gs`, `cr`, `i_`, `ou` are all read/written as `pj.<field>` in every one
 * of `pj`'s own real methods -- i.e. as the CLASS'S OWN shared spec table
 * (Lua's `pj` names the class table itself; `pj.ea[2][1]=...` mutates that
 * ONE shared table, not a per-instance copy), never as `self.<field>`.
 * This is the exact same "read a class's own default field directly,
 * mutating the shared class-level table in place" idiom `bv()`'s own
 * `ui.hw(rp)` read already established (pico8_probe.h's own top comment)
 * -- here extended to WRITES as well as reads, since `pj` (unlike `ui` in
 * that earlier case) only ever has the ONE real instance (`ju`, Chunk 22's
 * `pj()`) that would ever need per-instance storage anyway, so nothing in
 * the real source ever needed instance-level copies of `ea`/`gs`/`cr`/
 * `i_`/`ou` in the first place. `fa`,`ed` are likewise class-level DATA
 * fields (never reassigned by any of `pj`'s own methods; `ed`'s own
 * per-instance role is `self.ed`, a SEPARATE, genuinely instance-level
 * field `pj:ph()` writes -- the class-level `ed=1` spec default is simply
 * the STARTING value every fresh `pj()` instance's own `self.ed` inherits
 * until the constructed-in-`ph()` message-box instance overwrites it).
 * `self.gz`,`self.c_`,`self.fa` (written by `pj:ph()`) and `self.ed`
 * (written by `pj:ph()`, read by `pj:ph()` itself next tick) are the only
 * genuinely per-INSTANCE fields any of `pj`'s own methods ever touch --
 * confirmed the same way, by reading every real `self.<x>` access below.
 *
 * Ported class-level field accessors (this module's own private
 * `pj_class_field_ref_table()` helper, pico8_score_panel.c) mirror
 * `pico8_ui_class()->defaults`-style direct-class-field reads already
 * established by `bv()` above -- not a new mechanism. */

void pico8_score_panel_init(void);
void pico8_score_panel_shutdown(void);

/* The real `pj` class, built once by pico8_score_panel_init(). Returns
 * NULL before that (or after pico8_score_panel_shutdown()). */
pico8_class_t *pico8_pj_class(void);

/* `pj(overrides)` -- the real Lua constructor call (`eu()`'s own
 * `fm,ju=hy(),pj()`, no overrides, pico8/lua.lua:1182, Chunk 22). Thin
 * wrapper around pico8_class_instantiate(). */
pico8_instance_t *pico8_pj_new(const pico8_ob_table_t *overrides);

/* `ef=ob("...")` (pico8/lua.lua:797, read directly):
 *
 * ef=ob("\"\74\65\78\",\"\70\69\66\",\"\77\65\82\",\"\65\80\82\",\"\77\65"
 *   "\89\",\"\74\85\78\",\"\74\85\76\",\"\65\85\71\",\"\83\69\80\",\"\79"
 *   "\67\84\",\"\78\79\86\",\"\68\69\67\",\"\85\78\68\",\"\68\85\79\","
 *   "\84\69\82\",")
 *
 * DECODED BY HAND, EACH `\NNN` READ AS ITS OWN DECIMAL ASCII CODE (not
 * guessed, not assumed from the PRD's own paraphrase -- worked through
 * character by character): \74=J \65=A \78=N -> "JAN"; \70=F \69=E \66=B
 * -> "FEB"; \77=M \65=A \82=R -> "MAR"; \65=A \80=P \82=R -> "APR"; \77=M
 * \65=A \89=Y -> "MAY"; \74=J \85=U \78=N -> "JUN"; \74=J \85=U \76=L ->
 * "JUL"; \65=A \85=U \71=G -> "AUG"; \83=S \69=E \80=P -> "SEP"; \79=O
 * \67=C \84=T -> "OCT"; \78=N \79=O \86=V -> "NOV"; \68=D \69=E \67=C ->
 * "DEC"; \85=U \78=N \68=D -> "UND"; \68=D \85=U \79=O -> "DUO"; \84=T
 * \69=E \82=R -> "TER". So the STRING this decimal-escaped Lua string
 * literal actually spells out, once Lua's own `\NNN` escape decoding runs
 * (a step that happens BEFORE `ob()` ever sees the string -- `ob()`'s own
 * parser only ever sees the DECODED bytes, exactly like every other
 * quoted-string token it parses), is:
 *
 *   "JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV",
 *   "DEC","UND","DUO","TER",
 *
 * -- an ordinary, unremarkable `ob()` positional string-list literal once
 * decoded (the SAME shape as `es`/`ct`/`b_`/`co`/`fb`, pico8_game_data.h),
 * confirming the PRD's own "15 three-letter month/era abbreviations"
 * description exactly, including the 3 non-month codes actually spelling
 * "UND","DUO","TER" (not guessed at -- PICO-8's own in-game calendar
 * apparently has 15 named periods, not 12; no further semantic meaning
 * for the 3 extras is evidenced anywhere else in this cart's own scope,
 * so none is invented here). Ported as `pico8_ob()` given the DECODED,
 * ordinary-quoted-string spec text directly (this port's C string literal
 * for it has no need to re-embed `\NNN` escapes of its own -- C's own
 * `"..."` literal already spells the plain letters directly), matching
 * `pico8_game_data.c`'s own `pico8_ob(spec, NULL)` + module-level-static +
 * plain-accessor convention exactly. Used exactly once in this cart,
 * `ef[fm.lo]` (`pj:hb`, below) -- built/freed alongside `pj`'s own class
 * registration in this module's shared init/shutdown pair (single
 * consumer, same "fold into this chunk's own module" option `fv.h`'s own
 * `bo` already exercised). */
const pico8_ob_table_t *pico8_ef(void);

/* `bz=ob([[...]])` (pico8/lua.lua:798-814, read directly):
 *
 * bz=ob([[
 * o(26,86,0,fn="br"),
 * o(51,59,1,fn="br"),
 * o(76,77,1,fn="br"),
 * o(24,25,1,fn="br"),
 * o(33,41,1,fn="br"),
 * o("",score=0,ra=v(64,21),c=5),
 * o(55,"planets:   ",
 * score=1,ra=v(64,34)),
 * o(19,"population:",
 * score=2,ra=v(64,43)),
 * o(24,"technology:",
 * score=3,ra=v(64,52)),
 * o(o(27,5),o(f,0),f,o(25,5),"happiness",
 * score=4,ra=v(64,61)),
 * o("total:","",
 * score=5,ra=v(64,74)),
 * o(score=6,ra=v(64,82)),
 * ]])
 *
 * `pj:ph()`'s own end-of-game score-breakdown display list -- built by
 * this chunk (its ONLY real consumer, confirmed by grepping every real
 * `bz` reference in pico8/lua.lua: exactly this declaration plus `pj:ph`'s
 * own single `nv(bz,...)` use, no other call site anywhere in the cart).
 *
 * COUNTED DIRECTLY, NOT ASSUMED: this chunk's own governing brief
 * describes `bz` as "7 positional entries" -- reading the literal above
 * line by line instead shows **12** total positional entries: 5 plain
 * decorative bars (`fn="br"`, no `score=` field at all) FOLLOWED BY 7
 * `score=`-tagged rows (score 0 through 6 inclusive -- the PRD brief's
 * "7" almost certainly describes just this scored portion, entries 6-12,
 * not `bz` as a whole). Flagged and corrected here the same way Chunk 17's
 * own `positional_values`-copy count was independently recounted and
 * corrected mid-chunk (see pico8_help.h's own top comment) -- an
 * evidenced correction, not a guess.
 *
 * Entries 1-5 (the `fn="br"` bars) carry no `.score` field at all --
 * `pj:ph()`'s own mapping closure (pico8_score_panel_display.c) passes
 * these straight through unchanged (Lua's own `if st.score then ... else
 * return st end`, the `else` branch). Entries 6-12 (score=0..6) are each
 * rewritten in place by that same closure every time it runs -- see
 * pico8_score_panel_display.h's own top comment for the full per-field
 * trace. `f` (bare identifier, entry 10's own `o(f,0)`/lone `f`) is ob()'s
 * own established "bare identifier `f` means literal false" token
 * (already used throughout this port, e.g. `fv`'s `fp=f`); nested
 * `o(27,5)`/`o(f,0)`/`o(25,5)` sub-tables inside entry 10 are ordinary
 * REF-to-`pico8_ob_table_t` positional pairs, read (not further decoded
 * by this chunk) by whatever future drawing code entry 10's own `fn="qr"`
 * dispatch eventually consumes them through -- out of this chunk's own
 * scope to interpret further, exactly like every other `fn="..."`-tagged
 * row-item shape already ported elsewhere in this port without this
 * chunk needing to know how `io`/`bl`'s own dispatch renders it.
 *
 * Every nested `v(64,...)` (`ra=v(...)`, all 7 score rows) needs `pi['v']`
 * already registered -- same `pico8_pi_init()` defensive call as `pj`'s
 * own class spec above; one shared call in `pico8_score_panel_init()`
 * covers both. Built/freed alongside `pj`'s own class registration in
 * this module's shared init/shutdown pair (single consumer, matching
 * `ef`'s own identical treatment above). */
const pico8_ob_table_t *pico8_bz(void);

/* *** THE `_init` FORWARD-REFERENCE PLACEHOLDER *** -- `js(ob([[fa=15,
 * ra=v(51,100),lq=o(" again? "),]],{my=_init}))` (`pj:ph()`,
 * pico8_score_panel_display.c) assigns the bare GLOBAL FUNCTION `_init`
 * (Chunk 22's real zero-argument cart entry point, `function _init()cu()
 * jl()...fv()music(0)end`, pico8/lua.lua:1178-1180 -- not yet ported) as a
 * button's own "my" click-callback field. This is THE EXACT SAME SHAPE
 * Chunk 17 already solved for `eu` (`d.my=eu`, pico8_help.h/.c's own "THE
 * eu FORWARD-REFERENCE PLACEHOLDER" section) -- a bare global FUNCTION
 * VALUE with no instance/class behind it, stored as a plain table field,
 * later invoked via `mv(button,"my")`'s already-built generic
 * field-function-call mechanism, exactly like `eu`. This chunk builds the
 * analogous, INDEPENDENTLY-NAMED pair (`_init` and `eu` are two distinct
 * minified/real Lua identifiers, never unified into one generic
 * "placeholder" abstraction -- this port's own established convention is
 * one clearly-named accessor pair per distinct forward-referenced
 * identifier, the same "four separate accessor pairs for four separate
 * singletons" precedent `pico8_mo()`/`pico8_je()`/`pico8_ok()`/`pico8_fm()`
 * already set, extended here to a FUNCTION-VALUE placeholder instead of an
 * instance placeholder, matching `eu`'s own precedent exactly):
 *
 *   static pico8_value_function_t g_init_dispatch = pico8_pj_init_placeholder;
 *   pico8_value_t pico8_init_field_value(void);
 *   void pico8_score_panel_set_init(pico8_value_function_t fn);
 *
 * HOME CHOSEN: this chunk's own new `pico8_score_panel` module -- the
 * ONLY real call site for `_init` as a value (`pj:ph()`) lives here, the
 * same "new chunk's own module hosts a forward-referenced placeholder"
 * precedent `pico8_help.c` already set for `eu` and Chunk 5/10 set for
 * `je`/`fm`. `pico8_init_field_value()` always returns a valid
 * PICO8_VALUE_FUNCTION-kind value (the placeholder before Chunk 22
 * reconciles it, the real `_init` bridge after); `pico8_score_panel_set_
 * init(NULL)` resets to the placeholder (also used by this module's own
 * init/shutdown for test isolation, mirroring `pico8_help_set_eu`'s
 * identical convention).
 *
 * *** CHUNK 22 RECONCILIATION NOTE (mirrors pico8_help.h's own `eu` note
 * verbatim in spirit) *** -- when Chunk 22 ports the real `_init()` entry
 * point into a real `pico8_value_function_t`-shaped bridge (`_init()`
 * itself declares NO Lua parameters at all -- unlike `eu(nj)`, the `self`
 * argument `mv(button,"my")` would pass is simply discarded, the same
 * "excess argument silently dropped" treatment already established
 * throughout this port, e.g. `pico8_dl_dispatch`'s own unused second
 * argument), it must call `pico8_score_panel_set_init(pico8_init_dispatch)`
 * (or equivalent, using whatever name Chunk 22 gives its own bridge) to
 * REPLACE this module's own placeholder -- `pico8_init_field_value()`'s
 * own declaration/signature here never changes, so every EXISTING real
 * reader (any button already built by a PRIOR `pj:ph()` run) keeps
 * whichever value it captured at THAT time (the same "reconciliation only
 * changes what NEW consumers see" caveat `pico8_entity_set_mo()`'s/
 * `pico8_help_set_eu()`'s own reconciliation already documents), while any
 * NEW `pj:ph()` run after Chunk 22's own call resolves to the real
 * dispatch automatically. Chunk 22 must NOT declare a second, competing
 * `pico8_init_field_value()`/`pico8_score_panel_set_init()` pair anywhere
 * else -- this is the one and only home for them. */
pico8_value_t pico8_init_field_value(void);
void pico8_score_panel_set_init(pico8_value_function_t fn);

/* *** THE `ju` FORWARD-REFERENCE PLACEHOLDER *** -- `de={my=function()
 * ju.c_ = not ju.c_ end}` (pico8/lua.lua, the top-level table right after
 * `pj`'s own methods) reaches the SINGLE real `pj` instance ONLY by its
 * GLOBAL NAME `ju`, because `de` is a plain top-level table, not a method
 * defined inside `pj` itself -- it has no `self` upvalue to close over at
 * all (confirmed by reading pico8/lua.lua directly: `de`'s own callback is
 * a bare `function()...end`, zero parameters). This is EXACTLY the
 * established "classless-placeholder-now, real-instance-later get/set
 * accessor pair" idiom `pico8_fm()`/`pico8_mo()`/`pico8_je()`/`pico8_ok()`
 * already set (pico8_probe.h/pico8_entity.h/pico8_ui.h) -- `ju` is simply
 * the fifth such singleton, owned by THIS chunk's own new module (the
 * same "new chunk's own module hosts the singleton it introduces"
 * precedent Chunk 5's `pico8_je()`/Chunk 10's `pico8_fm()` already set for
 * their OWN new singletons). Unlike `fm` (a classless placeholder table
 * before `hy` is real), `ju`'s placeholder is simply NULL until Chunk 22
 * constructs the one real `pj()` instance -- there is no "empty ju table"
 * shape for a classless stand-in to approximate, since nothing reads a
 * `ju.*` field before `ju` is real (`de.my`'s own real call site is itself
 * unreachable before `_init()`/`eu()` build the buttons that could ever
 * click it) -- the same "simply NULL, no placeholder object needed" choice
 * `pico8_dk()` already made for an analogous "genuinely undefined until
 * `eu()` assigns it" global (pico8_economy_score.h's own top comment).
 *
 * *** CHUNK 22 RECONCILIATION NOTE *** -- Chunk 22's own real `eu(nj)`
 * does `fm,ju=hy(),pj()` (pico8/lua.lua:1182, confirmed by reading the
 * source directly): it must call `pico8_pj_set_ju(pico8_pj_new(NULL))` (or
 * equivalent) once that real instance is built, to REPLACE this module's
 * own NULL placeholder -- `pico8_ju()`'s own declaration/signature here
 * never changes. Chunk 22 must NOT declare a second, competing
 * `pico8_ju()`/`pico8_pj_set_ju()` pair anywhere else -- this is the one
 * and only home for them. */
pico8_instance_t *pico8_ju(void);
void pico8_pj_set_ju(pico8_instance_t *instance);

/* `dk` -- ALREADY BUILT by Chunk 12 (`pico8_dk()`/`pico8_economy_set_dk()`,
 * pico8_economy_score.h), needed here only because `pj:ph()`'s own real
 * `dk.px` read (pico8_score_panel_display.c) is a NEW real call site for
 * an ALREADY-EXISTING accessor -- NOT a new forward reference this chunk
 * introduces or needs to build a second, competing pair for. Every read
 * of it in this chunk's own scope is guarded the same defensive way every
 * other not-yet-reconciled forward reference in this port already is
 * (`pico8_dk() != NULL && pico8_ob_table_get_name(pico8_dk(), "px",
 * &v)...`, matching `pico8_tech_gate.c`'s own `pico8_fm() != NULL &&
 * ...`-guarded read sites exactly). No new declaration needed in this
 * header -- callers simply `#include "pico8_economy_score.h"` and call
 * `pico8_dk()` directly. */

/* function pj:hb()local cf=ne(fm.ge)local ou={fm.nz.."$",cf,fm.jk,
 * "+"..fm.po[r.k]+1,fm.happiness.."%",ef[fm.lo],fm.yr
 * }
 * for i,p in pairs(ou)do
 * pj.ou[i][1]=p
 * end
 * eg()end
 * (pico8/lua.lua:829-834.) See pico8_score_panel.c's own comment on this
 * function for the full field-by-field trace, including the `eg()`
 * forward reference (Chunk 21's own real function -- see
 * `pico8_eg()`'s own comment below) and this module's own private
 * concat/field-lookup helpers. `hb` IS one of `bm`'s 5 real dispatch names
 * (`bm=["kj","ev","hd","hk","hb"]`, pico8_entity.c) -- wired as a real
 * PICO8_VALUE_FUNCTION field on `pj`'s own class defaults
 * (`pico8_pj_hb_dispatch`), matching the real evidenced call site
 * (`pico8_economy.c`'s own `pico8_mv(e2,"hb",0,...)`, argc=0) exactly.
 * Needs no `self` at all (every field it touches is class-level `pj.*` or
 * the `fm`/`r`/`ef` singletons) -- `pico8_pj_hb_dispatch` ignores `self`
 * entirely, the same "bridge ignores every argument" shape
 * `pico8_jo_hb_dispatch` already established. */
void pico8_pj_hb(void);

/* function pj:kj()bl(la(pj.i_,pj.ou))if self.gz then
 * self.gz+=0.2
 * local h=min(self.gz,3.2)^2
 * rk(0,h,0)rk(127-h,127)end
 * end
 * (pico8/lua.lua:848-855.) `bl(la(pj.i_,pj.ou))`: concatenates the
 * class-level `i_`/`ou` lists (`pico8_la`, Chunk 1) via this module's own
 * private `pico8_score_panel_positional_values` bridge (the ob_table
 * positional-entries-to-pico8_list_t conversion this port has now
 * duplicated 10+ times -- see pico8_score_panel.c's own comment on this
 * helper for the exact tally and cross-references, "yet another copy" per
 * this port's own accepted convention), then draws the combined list with
 * `nc` omitted (`pico8_bl_default`, the same established entry point
 * `pico8_fv_kj`/Chunk 8's own `bl(la(pj.i_,pj.ou))`-shaped call already
 * uses -- this IS that exact real call site, now finally ported).
 * `if self.gz then...end`: the end-of-game "reveal curtain" animation,
 * gated on `self.gz` being PRESENT (any value, including the real 0
 * `pj:ph()` seeds it with -- Lua truthiness, not "nonzero") rather than
 * absent. `self.gz+=0.2` grows the curtain each frame; `h=min(self.gz,
 * 3.2)^2` (this module's own private `fixed_min`, the established
 * per-module-copy convention, e.g. `pico8_economy_score.c`'s own
 * identical helper) caps it, squared, into a black-bar height; `rk(0,h,0)`
 * (explicit col=0, ordinary `pico8_rk`) draws the top bar, `rk(127-h,127)`
 * (col OMITTED -- the one real evidenced case `pico8_rk_default`,
 * pico8_hud.h, was built by THIS chunk to resolve) draws the bottom one.
 * `kj` IS one of `bm`'s 5 real dispatch names, wired as a real
 * PICO8_VALUE_FUNCTION field (`pico8_pj_kj_dispatch`, argc/arg0 from
 * `dc("kj")`'s own real `qc[nt](qc,qc.ra)` call shape, self.ra unpacked as
 * a REF-to-vec exactly like every other `kj` dispatch bridge -- even
 * though `pj:kj`'s own body never reads its own `p` at all, the same
 * "declared, accepted, genuinely unused" treatment `pico8_fv_kj`'s own `p`
 * already gets). Needs `gfx`+`mem` (bl()'s own drawing, rk()'s own
 * rectfill); a NULL `self` skips the curtain animation only (bl() itself
 * still runs, matching `pico8_fv_kj`'s own "NULL self is a safe no-op for
 * the reveal-curtain part only" precedent). */
void pico8_pj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self);

/* `pj:hk(yr)` (pico8/lua.lua:856-869) -- `hk` IS one of `bm`'s 5 real
 * dispatch names, wired as a real PICO8_VALUE_FUNCTION field
 * (`pico8_pj_hk_dispatch`, pico8_score_panel.c, alongside `pj`'s own
 * class registration), matching the real evidenced call site
 * (`pico8_economy.c`'s own `pico8_mv(e,"hk",1,pico8_value_fixed(new_yr),
 * ...)`, argc=1, arg0=the new year) exactly. `pico8_pj_hk`'s own BODY,
 * however, is declared and defined in `pico8_score_panel_display.h`/`.c`
 * (see that header's own comment on this declaration for the full trace,
 * including the NEW `pico8_ob_table_remove_index()` primitive and the
 * single-static-int `iw.lh` upvalue representation) -- moved there,
 * alongside `pj:ph()`, once this file's own line count crossed the
 * ~430-540-line range this port's own warn-review threshold flags (see
 * docs/portage-progress.md's Vague 18 entry for the exact tally). */

/* `eg()` -- pico8/lua.lua:1173-1177 (`function eg()local ps=#jp(cg.jo,
 * jo.gg)/2 for i,ny in pairs(fb)do local pd=0x3101+4*ny poke(pd,peek(pd)%
 * 128+(ps<i and 128 or 0))end end`), called at the tail of `pj:hb()`'s own
 * real body -- a research-icon palette-shading effect. Originally
 * (Chunk 18) a deliberate, documented NO-OP stub, EXPLICITLY assigned by
 * the PRD's own Implementation Chunks list to Chunk 21 for its real body
 * ("`cr()`, `eg()`, ... Depends on ... Chunk 18 (`pj.cr`/`pj:hb`'s `ef`
 * usage)"). *** CHUNK 21 UPDATE ***: the real body is now ported (see
 * pico8_score_panel.c's own comment on this function's definition for the
 * full field-by-field trace) -- same declared signature, same one real
 * call site (`pico8_pj_hb()`'s own tail call), no other change needed,
 * exactly as this comment originally anticipated. `eg()` needed no get/set
 * indirection either before or after this update: it is never stored as a
 * value anywhere in the real source (line 1173's own top-level `function
 * eg()...end` is the only place it's ever named besides this one bare
 * call), only ever invoked directly by name. */
void pico8_eg(void);

#endif
