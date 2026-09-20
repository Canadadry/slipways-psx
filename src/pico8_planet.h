#ifndef PICO8_PLANET_H
#define PICO8_PLANET_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gen.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 13 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md), FIRST sub-lot: the `jo` class itself -- confirmed by reading
 * the real source to be the base PLANET entity (every planet on the galaxy
 * map, colonized or not, is a `jo` instance or a subclass of one --
 * `qh`/`nl`/`os`/`nr`/`fr`/`hj`/`pm`/`mx`/`ga`, all Chunk 15, extend it via
 * `jo:jr(...)`). Tagged `nu=o("jo","np","prober")` -- "jo" (every planet),
 * "np" (read by `bv()`'s own `cg.np` nearest-neighbor search, Chunk 10 --
 * this sub-lot is what finally makes that search find real planets), and
 * "prober" (shared with `g_`, Chunk 10 -- no real reader confirmed in scope
 * yet, kept rather than guessed at, same treatment `pico8_probe.h` already
 * gave `g_`'s own identical tag).
 *
 * jo=ke:jr([[
 * nu=o("jo","np","prober"),
 * me="unknown",
 * hf=1,hs=1,
 * fa=6,
 * kq=b(-7,-7,7,7),
 * ma=0,
 * ka=o(2,3,9,12),
 * dz=o(
 * o("unhappy",c=8),
 * o("content",c=3),
 * o("prosperous",c=9),
 * o("rich",c=12),
 * ),
 * cq=o(12,36,24,25),
 * kw=o(mu=0),
 * gq=v(0,11),ca=9,
 * em=o(f,3,0,-1),
 * ]])jo.hd=gk
 * (pico8/lua.lua:384-402.) A `ke` subclass (`ke:jr(...)`), confirmed by
 * reading the source, not assumed. `kq=b(-7,-7,7,7)` and `gq=v(0,11)` each
 * need the 'b'/'v' char-keyed `pi` constructors (pico8_pi.h) already
 * registered before this spec string is parsed -- see pico8_jo_init()'s own
 * comment below for the same defensive `pico8_pi_init()` call
 * `pico8_scroll.c`/`pico8_mouse_reader.c` already established for an
 * identical ordering hazard. `jo.hd=gk` -- a plain field assignment (like
 * `ke.pq=lt`/`js.hd=gk`, Chunk 4), NOT a `function jo:hd()...end`
 * definition: wires jo's own "hd" (hover/hit-test) dispatch to the SAME
 * already-built `pico8_gk` (pico8_entity_geom.h) `js.hd=gk` already reuses,
 * through a small bridge (`pico8_jo_hd_dispatch`, this file's own .c) --
 * "hd" IS one of `bm`'s 5 real dispatch names, so this is exactly the
 * "real bm-name field, mv()-routed as well as `dc()`-routed" treatment
 * `pico8_bg.h`/`pico8_ui_button.h` already established, just reusing an
 * existing function body rather than a new one. Every other spec field
 * (`me`,`hf`,`hs`,`fa`,`ma`,`ka`,`dz`,`cq`,`kw`,`ca`,`em`) is plain data,
 * read directly off `self`/`jo_class->defaults` by whichever method needs
 * it (this sub-lot's own methods read `kw`; `ka`/`dz`/`cq` are read by
 * `jo:gh`/`jo:kj`, deferred -- see this file's own "LEFT FOR FOLLOW-UP"
 * note below).
 *
 * *** REGISTRATION INTO THE NAMED-CLASS FORWARD-REFERENCE REGISTRY ***
 * (pico8_class.h's `pico8_class_register_named`/`pico8_class_lookup_named`,
 * Chunk 7): `pico8_jo_init()` calls
 * `pico8_class_register_named("jo", g_jo_class)` alongside its own
 * `pico8_class_declare()` call. This closes, FOR REAL, `jo.ex`'s own
 * `jo.em` read (Chunk 11, `pico8_research_display.c`'s own
 * `pico8_jo_ex` -- see that file's own top comment: "WHOEVER BUILDS THE
 * REAL `jo` CLASS (Chunk 13) MUST call
 * pico8_class_register_named(\"jo\", jo_class)"). It does NOT close
 * `bg:ldown`'s own `mz`/`g_` forward reference (Chunk 7) -- that one looks
 * up "mz"/"g_" specifically, never "jo" (confirmed by re-reading
 * `pico8_bg.c`'s own `pico8_bg_ldown`), so this registration is unrelated
 * to it despite sharing the same underlying registry mechanism -- the task
 * brief's own framing ("closes Chunk 7's bg:ldown forward reference")
 * refers to the REGISTRY MECHANISM Chunk 7 built being put to its second
 * real use here, not to `bg:ldown`'s own specific "mz"/"g_" lookup
 * resolving differently because of it.
 *
 * *** PRD METHOD-COUNT DISCREPANCY, FOUND AND DOCUMENTED, NOT GUESSED
 * PAST *** (same treatment already given the `ob()`/`fi()` prose errors,
 * `docs/plan.md`'s own "Notes de portage"): the PRD's Chunk 13 text lists
 * 16 methods (`pq`,`hh`,`ldown`,`fe`,`gh`,`gg`,`hu`,`lz`,`kf`,`kd`,`ll`,
 * `gv`,`cn`,`ib`,`hk`,`kj`). Reading `pico8/lua.lua` directly finds a 17th
 * real method, `function jo:hb()self.lq=jo.ex(self.jy,self.kw.qe,
 * self.kw.mu,#self.by,0)end` (line 539-540) -- a genuine, load-bearing
 * `bm`-dispatch-name method (`hb` is one of `bm`'s 5 real names, dispatched
 * by `hy:lj()`'s own `for e in all(cv.hb)do e:hb()end` loop, Chunk 12,
 * already built and waiting for exactly this). This is a PRD undercount,
 * not a bug in the source or in this port -- `jo:hb` is ported in THIS
 * sub-lot (see below) precisely because reading it found it to be one of
 * the simplest, most self-contained of the 17.
 *
 * *** THIS SUB-LOT'S OWN SPLIT (read every one of the (now) 17 methods
 * before choosing, per this task's own instruction not to guess from name
 * length) ***: PORTED HERE: `pq`, `lz`, `gg`, `kd`, `ll`, `hb` (6 of 17).
 * Chosen because each is either a one-line field-setter with NO same-class
 * or forward-referenced-closure dependency (`gg`,`kd`,`ll`), or depends
 * only on already-built standalone functions/classes (`lz` on Chunk 1's
 * `nv`/`mw`; `hb` on Chunk 11's already-built `pico8_jo_ex`) or on another
 * method THIS SAME sub-lot also ports (`pq` on `lz`) -- none of the six
 * needs a method from the LEFT FOR FOLLOW-UP list below to exist.
 *
 * LEFT FOR FOLLOW-UP SUB-LOTS (11 of 17), with the real reason each was
 * excluded (not a guess from name length):
 *   - `hh` -- reads `jo.oc()`/`jo.ir()`, two STATEFUL GENERATOR CLOSURES
 *     (Chunk 1's `pico8_gn_state_t*`/`pico8_jf_state_t*`) only assigned by
 *     `bw()` (Chunk 20) -- representing "a class-level field that is a
 *     live generator, not a plain ob()-table value" needs new plumbing
 *     this sub-lot chose not to invent speculatively; also calls
 *     `self:kd()` (fine, ported here) but the generator-closure gap alone
 *     defers it. `df.ll` (Chunk 11's already-built `pico8_df()`) is NOT a
 *     blocker on its own.
 *   - `ldown` -- calls `self:fe()` (deferred, see below) and `jc(...)`
 *     (Chunk 14, not yet built).
 *   - `fe` -- calls `pz`,`nv`,`bj`,`mv(b,"lq")`,`fm:lu`,`self:hu` in one
 *     body; genuinely the most call-heavy of the 17, deferred whole.
 *   - `gh` -- reads `jo.dz[self.mi]`/`jo.cq` (class-level nested-table
 *     lookups, mechanically simple) but builds a multi-branch, several-
 *     nested-table `bs` list (needs `jp`, `la`, `gu`, string
 *     concatenation, and `pz()`) -- read in full and found to be
 *     genuinely more involved than its short name suggests, not the
 *     "simpler" method this task's own suggested example guessed it was.
 *   - `hu` -- the flagged `kv[hp.cw]` dynamic-reclassing forward
 *     reference; also calls `self:ks`,`self:lz`,`self:kd`,`fm:nx`,`rn`.
 *     Mechanically ready (this sub-lot's own `pico8_class_lookup_named`
 *     is already generic) but bundled with the other planet-lifecycle
 *     methods for a coherent follow-up lot instead of ported alone.
 *   - `kf`,`cn`,`ib` -- the mutually-recursive trade-propagation cluster
 *     (this PRD's own explicit "not force-ordered" decision); `kf` alone
 *     also needs `self:kd()` (available) and `sw:eb(self)` (Chunk 14, mv()
 *     forward-ref) but is left with its two cluster siblings rather than
 *     split off alone, since `jo:ib`'s own local closure-shaped `lw.kf`
 *     table field (distinct from the `jo:kf` METHOD of the same name --
 *     see `pico8/lua.lua:500-514`) is easiest to get right by porting the
 *     whole `cn`/`ib` pair together, and `kf` (the method) is the cluster's
 *     third leg.
 *   - `gv` -- its one real branch calls `self:gh()`, deferred above; left
 *     with it rather than compiled-but-unlinkable.
 *   - `hk` -- a real `bm`-dispatch-name method (needs its own dispatch
 *     bridge, like `hb` here) but semantically paired with `hy:hk()`'s own
 *     `cv.hk` year-tick dispatch (Chunk 12) and best reviewed alongside
 *     the other `cv.hb`/`cv.hk` planet-lifecycle callbacks in one lot with
 *     `hh`/`hu` rather than split off alone this session.
 *   - `kj` -- the PRD's own explicitly-flagged "first real, non-dead
 *     `dc('kj')` exercise" method; calls `self:gg()` (ported here, so this
 *     is ready once picked up) and reads `jo.ka` (class-level, simple) but
 *     also `self.fo`/`self.nk`/`dv`/`palt`/`fillp`/`sqrt` in one body --
 *     kept for the sub-lot that does the module's own entity/dispatch
 *     smoke-test-coverage assessment justice, rather than folded in here
 *     where its own "first real dc('kj')" significance would be
 *     underserved by a rushed one-line assessment.
 *
 * New module (`pico8_planet.c`/`.h`, not `pico8_ke.c`/`pico8_entity.c`/
 * `pico8_research.c`/`pico8_economy.c`, all either the base class module or
 * already at/near the flagged ~430-540-line range): `jo` is this phase's
 * own single largest concentration of methods on one class (17, once the
 * PRD's own undercount is corrected), so a dedicated module, planned for a
 * multi-sub-lot split from the very first line, is this sub-lot's own
 * module-size checkpoint call -- see docs/portage-progress.md's own
 * "Vague 13" entry for the line-count trace. Follow-up sub-lots may either
 * extend this same file (if there's room under the threshold) or split
 * into sibling modules (e.g. `pico8_planet_trade.c` for `cn`/`ib`/`kf`,
 * `pico8_planet_display.c` for `gh`/`kj`/`hb`... though `hb` already lives
 * here) once their own real size is known -- not pre-committed here.
 *
 * *** SUB-LOT 2 (this session) *** -- per the task brief's own explicit
 * split (not sub-lot 1's own suggested follow-up grouping): PORTS `gh`,
 * `gv`, `hu`, `hh`, `hk` (5 of the remaining 11). `ldown`/`fe` (call into
 * `jc`, Chunk 14, still not built) and the trade cluster `kf`/`cn`/`ib`
 * plus `kj` are left for further sub-lots -- see each new declaration's
 * own comment below for the exact trace.
 *
 * `gh`/`gv`: read in full, confirmed genuinely more involved than their
 * short names suggest (`gh` builds a multi-row info-panel list for `pz()`,
 * Chunk 8, via `jp`/`la`/`gu`/`ne`, Chunk 1; `gv` just calls `self:gh()` on
 * one branch, hence ported together). Neither `gh` nor `gv` is one of
 * `bm`'s 5 real dispatch names, but both are still real `mv()`-routed
 * fields (`ui:jz`'s own generic `mv(h,rm,...)` dispatch, pico8_ui.h,
 * reaches "gv" by name at a real call site, `local nw=self:jz("gv",
 * held)`, pico8/lua.lua:245) -- wired the same "field function, not a
 * `cv` auto-registration" way `gg`/`hb` already are (sub-lot 1).
 *
 * `hu`: the flagged `kv[hp.cw]` dynamic-reclassing forward reference.
 * Ported against the ALREADY-GENERIC `pico8_class_lookup_named`
 * (pico8_class.h, Chunk 7) -- no concrete class name is hardcoded, per the
 * PRD's own Out-of-Scope note that which class(es) `hp.cw` can actually
 * hold is only knowable once Chunk 11's `unstash()`-decoded research data
 * is inspected (confirmed, separately, by `pico8_class.h`'s own comment:
 * only `pm`/`ga`, Chunk 15, ever set a real `fz` name at all) -- today
 * `pico8_class_lookup_named(cw)` always returns NULL (no class with that
 * name registered yet), so this compiles and is mechanically ready, not
 * yet meaningfully exercisable, the same treatment already given every
 * other forward reference in this port.
 *
 * `hh`: reads `jo.oc()`/`jo.ir()`, two CLASS-LEVEL fields holding live
 * generator-closure state (`pico8_gn_state_t*`/`pico8_jf_state_t*`, Chunk
 * 1's pico8_gen.h) that only `bw()` (Chunk 20, not yet built) ever
 * assigns (`jo.oc,jo.ir=gn(df,2,jf(0.5,dk.en)),jf(0.8,0.5)`, pico8/
 * lua.lua:750-751). This sub-lot builds the real plumbing for these two
 * fields (module-private static globals, plus `pico8_jo_set_oc`/
 * `pico8_jo_set_ir` accessors for Chunk 20 to call, mirroring
 * `pico8_entity_set_mo`'s own established "singleton globals ownership"
 * accessor convention) rather than inventing a placeholder value -- both
 * default to NULL until Chunk 20 runs, and `jo:hh()` reads them
 * defensively (a NULL generator contributes nothing, matching this port's
 * "class-level field simply reads as absent" convention), a literal
 * forward-referencing call, not a deferral.
 *
 * `hk`: a real `bm` dispatch name (`bm=ob([["kj","ev","hd","hk","hb",]])`,
 * pico8/lua.lua:169-172) -- wired with the same "class-default
 * PICO8_VALUE_FUNCTION field, real bridge, real `cv` auto-registration at
 * `lt()` time" treatment `hd`/`hb` already established (sub-lot 1),
 * closing `hy:hk()`'s own `for e in all(cv.hk)do e:hk(self.yr)end`
 * dispatch loop (pico8_economy.h, Chunk 12, already built and already
 * calling `pico8_mv(e,"hk",1,...)` against exactly this bucket) for real
 * the moment any real `jo` instance is `lt()`'d. `jo:hk()` itself ignores
 * the dispatched `self.yr` argument entirely (its own body declares no
 * parameter for it, matching real Lua's own "extra call arguments are
 * simply discarded" semantics) -- the bridge accepts and ignores
 * `argc`/`arg0`/`arg1`, the same pattern `pico8_jo_hb_dispatch` already
 * uses. */

/* *** SUB-LOT 3 (this session) *** -- two parts, per the task brief's own
 * explicit ordering: FIRST, the module split sub-lot 2's own write-up
 * flagged as required (`docs/portage-progress.md`'s "Vague 13" entry:
 * 767/722 lines, well past the ~430-540 threshold) -- `hb`/`gh`/`gv`
 * moved verbatim to the new `pico8_planet_display.c`/`.h` (see this
 * file's own "SUB-LOT 3 MODULE SPLIT NOTICE" comment further below, right
 * before `jo:hu`'s declaration, for the exact list and rationale); no
 * `pico8_planet_trade.c` created yet, since `kf`/`cn`/`ib` (the only
 * methods that would go there) aren't ported yet -- nothing to move.
 * SECOND, `ldown`/`fe` are ported, now that `hu` (sub-lot 2) exists to
 * satisfy `fe`'s own `self:hu()` call -- `jc` (Chunk 14) remains the one
 * real blocker, still not built for `ldown`'s own `jc({...})` call, so it
 * compiles against the SAME already-generic `pico8_class_lookup_named`
 * mechanism `hu` already established, exactly as the PRD's own "forward
 * references are ported, not deferred" convention prescribes (see
 * `pico8_jo_ldown`'s own comment below for the full trace). `ldown`
 * itself is declared below, after `hk`; `fe` is declared in
 * `pico8_planet_display.h` instead -- porting both into THIS file first
 * (as sub-lot 2's own plan originally proposed) measured 663 lines, still
 * over the ~430-540 range even after the split above, almost entirely
 * because of `fe`'s own bulk (the "my" closure's representation) -- see
 * `pico8_planet_display.h`'s own "STEP 2 REBALANCE" comment for the full
 * size trace and why `fe` specifically (not `ldown`) was the one moved.
 * This brings `jo` to 13 of its 17 real methods ported; left for later
 * sub-lots: the mutually-recursive trade cluster `kf`/`cn`/`ib` and `kj`
 * (the PRD's own flagged "first real `dc('kj')` exercise"). */

void pico8_jo_init(void);

/* Not part of the Lua original (no manual teardown under GC): frees the
 * `jo` class itself. Does NOT free any `jo` instance (still owned by
 * whoever `pico8_class_instantiate`'d it, matching `pico8_instance_free`'s
 * own established convention). Safe to call more than once, or before the
 * first `pico8_jo_init()`. */
void pico8_jo_shutdown(void);

/* The real `jo` class, built once by `pico8_jo_init()`. Retrievable via
 * `pico8_class_lookup_named("jo")` once `pico8_jo_init()` has run (this is
 * the registration this file's own top comment traces) -- NOT via the
 * shared `""`-keyed `pico8_class_lookup()` registry the way `ke`'s own
 * root-class precedent works, since `jo` genuinely is `ke`-descended
 * (`jo.iq` is `ke`, not `NULL`) and therefore never collides into that
 * shared slot the way a ROOT class does. Returns NULL before
 * `pico8_jo_init()` (or after `pico8_jo_shutdown()`). */
pico8_class_t *pico8_jo_class(void);

/* `jo(overrides)` -- the real Lua constructor call (`bw()`'s own
 * `jo({id="p"..id,p=p,ra=p*128})`, Chunk 20, the ONLY real construction
 * call site found by reading the whole cart -- `jo:hu`'s own `kv[hp.cw]
 * ({...})` reclassing call, Chunk 13's own follow-up sub-lot, constructs a
 * DIFFERENT (sub)class, never plain `jo` itself). Thin wrapper around
 * `pico8_class_instantiate`, matching every other class's own established
 * `pico8_X_new` convention. */
pico8_instance_t *pico8_jo_new(const pico8_ob_table_t *overrides);

/* function jo:lz(hp)self.hn,self.ep=
 * hp
 * self.jy=nv(hp.i,mw)self.kw={qe=hp.o,mu=0}
 * end
 * (pico8/lua.lua:461-464.) Applies a research project `hp` (an `hp`
 * instance, Chunk 11 -- the Lua PARAMETER name `hp` shadows the CLASS name
 * `hp` within this function's own scope only, so the C parameter is
 * renamed `hp_instance` here, the same renaming precedent `pico8_bj`'s own
 * `jo_instance` parameter already established, pico8_research_display.h)
 * to `self`, turning `self` into (or updating) a colonized/assigned
 * planet's own building/production state.
 *
 * `self.hn,self.ep=hp`: a Lua multi-assignment with ONE right-hand value
 * and TWO left-hand targets -- Lua pads the extra target with nil, so this
 * is `self.hn=hp_instance; self.ep=nil`, NOT `self.ep=hp_instance` too
 * (the exact same subtlety `pico8_msgbox.c`'s own `mh:ev`'s `self.p_,kc=
 * true` already documents and gets right). `self.ep` is written as an
 * EXPLICIT `pico8_value_ref(NULL)` (this port's own "genuinely nil, not
 * merely absent" convention, e.g. `pico8_d_`'s own return value) rather
 * than left untouched, since a PRIOR call (e.g. `jo:hh()`'s own
 * `self.ep=self.nk.d`, deferred to a follow-up sub-lot) could have set a
 * real value here that a later `:lz()` call must genuinely clear, matching
 * real Lua's own nil-assignment semantics exactly.
 *
 * `self.jy=nv(hp.i,mw)`: `hp_instance.i` (a REF to a heap `pico8_list_t`
 * of `{qe=...}` wrapper records, built by `hp:pq()`'s own `fi()` call,
 * Chunk 11 -- see `pico8_research.c`'s own `pico8_hp_pq_i_step`) is mapped
 * through `mw` (Chunk 1's `pico8_mw`, a shallow copy) into a FRESH list of
 * FRESH shallow-copied records, becoming `self`'s own "wants" list --
 * ported via `pico8_nv` (`pico8_collect.h`) with a small per-element
 * adapter bridging `mw`'s own `(const pico8_ob_table_t*)->pico8_ob_table_t*`
 * shape to `pico8_nv_fn`'s `(pico8_value_t,ctx,*out_value)->int` shape (the
 * same "kp(e) or nil" convention `pico8_nv`'s own header comment
 * documents: a non-REF/NULL element contributes nothing. Note: real Lua's
 * `mw(nil)` does NOT crash (`qj(o,mr)`'s `pairs(mr or {})` guards it,
 * returning an empty table) -- this defensive skip isn't sidestepping a
 * real crash, it's just this port's usual "wrong-kind field reads as
 * absent" convention, applied here too. An absent/wrong-kind
 * `hp_instance.i` (or a NULL `hp_instance`) yields an empty `self.jy`.
 *
 * `self.kw={qe=hp.o,mu=0}`: a fresh table with `hp_instance`'s own "o"
 * field (the resource kind this planet now produces, whatever value/kind
 * it holds -- copied as-is, not reinterpreted) and a fresh `mu=0` (output
 * amount, recomputed later by `jo:hb()`/economy code). Replaces whatever
 * `self.kw` previously held (the class-level default `kw=o(mu=0)`, or an
 * earlier `:lz()` call's own result) -- a genuine field REBIND, matching
 * real Lua's own `self.kw={...}` semantics (the old table becomes
 * unreachable, not mutated in place).
 *
 * A NULL `self` is a safe no-op (nothing to write onto). A NULL
 * `hp_instance` still writes `self.hn=REF(NULL)`/`self.ep=REF(NULL)`/
 * `self.jy=` an empty list/`self.kw={mu=0}` (every field read off it
 * defaults defensively, matching this port's pervasive convention) rather
 * than crashing the way real Lua's own `hp.i`/`hp.o` would on a nil `hp`. */
void pico8_jo_lz(pico8_instance_t *self, pico8_instance_t *hp_instance);

/* function jo:gg()
 * return self.hf and self.me=="ly"end
 * (pico8/lua.lua:449-450.) "Is this a colonized, currently-thriving
 * planet?" -- Lua's `A and B`: if `self.hf` (truthy by the class's own
 * default, `hf=1`) is falsy, the WHOLE expression's value is `self.hf`
 * itself (whatever falsy value that is, not necessarily `false`); only
 * when `self.hf` is truthy does the result become the genuine boolean
 * `self.me=="ly"` (compared by STRING CONTENT, `strcmp` -- this port's own
 * `ob()` never interns strings, the same gap already resolved for `ho()`/
 * `oj()` via `pico8_gu_content`, so a direct-value `==` here would be
 * wrong the same way `pico8_value_equal`'s own pointer-identity rule would
 * be for two independently-`ks()`-set `.me` strings). Returned AS a
 * `pico8_value_t` (not a plain C `int`) specifically so the falsy-but-not-
 * `false` case (an absent/falsy `self.hf`) is preserved faithfully for
 * whichever future caller inspects it that way -- every real call site
 * found in scope (`jo:kj`'s own `self:gg()`, deferred; `da`/`cz`/`ky`/`bf`,
 * Chunk 12, already built, via `pico8_mv(e,"gg",...)`) only ever tests
 * truthiness, but this port doesn't collapse a Lua-genuine value to a
 * plain bool speculatively where the exact value is easy to preserve (see
 * `pico8_bb`'s own contrasting "simplify a boolean-only-observed-by-
 * truthiness result" precedent, `pico8_economy_tiers.h` -- that one
 * simplifies because the ORIGINAL value there is only ever `true`/`nil`,
 * never a third kind; `jo:gg()`'s own `self.hf` COULD genuinely be some
 * other value some future caller inspects).
 *
 * Wired BOTH as a real `PICO8_VALUE_FUNCTION` field ("gg", `pico8_
 * jo_gg_dispatch`, this sub-lot's own bridge) on `jo`'s own class defaults
 * -- closing, for real, Chunk 12's own `da`/`cz`/`ky`/`bf` forward
 * references to `pt:gg()` the moment any real `jo` instance exists -- AND
 * directly callable as this plain C function for any future same-class
 * caller (`jo:kj`'s own `self:gg()`, deferred) that already knows it's
 * calling a `jo`(-or-subclass) instance, the exact same "both a dispatch-
 * table field AND a plain callable C function" treatment `g_:mm`
 * (`pico8_probe.h`) already established. A NULL `self` reads `self.hf` as
 * absent -- returns `pico8_value_ref(NULL)` (Lua nil, the correct result
 * of `nil and ...`, not a guessed `false`). */
pico8_value_t pico8_jo_gg(const pico8_instance_t *self);

/* function jo:kd()self.ma=1.25
 * end
 * (pico8/lua.lua:468-469.) Resets the "particle halo" decay field
 * (`.ma`, `pico8_ma`'s own field, `pico8_entity_geom.h`, Chunk 1) to its
 * peak value -- called whenever a planet's own state changes in a way that
 * should re-trigger the halo effect (`jo:hh`,`jo:hu`,`jo:kf`, all deferred
 * to follow-up sub-lots -- confirmed by grepping every real `:kd()` call
 * site in scope: all three are same-class `jo` calls, never `mv()`/`dc()`-
 * routed, so this is a plain, known, directly-callable C function, the
 * same "known method, no dispatch table" treatment `pico8_ke_ks`/
 * `pico8_hp_po` already established -- no bridge needed). A NULL `self` is
 * a safe no-op. */
void pico8_jo_kd(pico8_instance_t *self);

/* function jo:ll(t)self.p_=t>60
 * end
 * (pico8/lua.lua:470-471.) Sets the planet's own "pending removal" flag
 * (`.p_`, read by a future `dg()`-style per-tick sweep, Chunk 6, already
 * built -- see `pico8_bootstrap.h`'s own `pico8_dg` for the general
 * `if qc.p_ then he(qc) end` shape this flag feeds) once a timer `t`
 * exceeds 60 ticks. NO REAL CALL SITE FOR `jo:ll` WAS FOUND ANYWHERE IN
 * `pico8/lua.lua` by this sub-lot's own exhaustive grep (unlike every
 * other method here) -- ported faithfully regardless, per this whole PRD's
 * "forward references are ported, not deferred" convention (the same
 * treatment already given `jo:hk`'s own dead `stat(19)`-style branches
 * elsewhere in this port): whichever later chunk's own real call site
 * turns up (most likely `_update60()`, Chunk 22, or a not-yet-read
 * `mv(e,"ll",...)`) will simply find this already built and correct. Not
 * a `bm` name, so no dispatch bridge is wired for it -- if a future chunk's
 * own real call site turns out to be `mv()`-routed rather than a direct
 * `self:ll(...)` call, that chunk is responsible for wiring the field
 * then, using this same function as the bridge's target (matching every
 * other bridge-around-an-existing-function precedent in this port). A NULL
 * `self` is a safe no-op. */
void pico8_jo_ll(pico8_instance_t *self, pico8_fixed_t t);

/* *** SUB-LOT 3 (this session) MODULE SPLIT NOTICE *** -- `hb`, `gh`,
 * `gv` (sub-lot 2's own ports, plus their private helpers
 * `pico8_jo_gh_ctx_t`/`pico8_jo_gh_jp_step`/`pico8_jo_gh_row`/
 * `pico8_jo_positional_values`/`field_truthy`/`field_fixed`/`field_list`/
 * `field_ref_table`) moved VERBATIM (every trace comment intact, no
 * logic changed) to the new `pico8_planet_display.c`/`.h` sibling module
 * in this sub-lot's own required proactive split (see this file's own
 * "New module" paragraph above and `docs/portage-progress.md`'s "Vague
 * 13" entry for why: 767/722 lines, well past the ~430-540 threshold).
 * See `pico8_planet_display.h` for their declarations and full trace
 * comments. `pico8_jo_hb_dispatch`/`pico8_jo_gv_dispatch` (the two
 * `mv()`-dispatch bridges installed by `pico8_jo_init()` below) stay in
 * `pico8_planet.c`, now calling across the file boundary via
 * `pico8_planet_display.h`. */

/* function jo:hu(hp)local fp=2
 * if hp.cw then
 * self.p_=true
 * self=kv[hp.cw]({p=self.p,ra=self.ra,id=self.id
 * })elseif hp.go then
 * self.nk=df[hp.go]
 * else
 * self:ks("ly")self:lz(hp)self.fx=true
 * fp=ov("autoassemblers",3,2)end
 * fm:nx(fp)rn(hp.rn or 55)self:kd()end
 * (pico8/lua.lua:451-460.) Applies a completed research project `hp`
 * (parameter renamed `hp_instance` here -- it shadows the CLASS name `hp`
 * within this function's own Lua scope only, the exact same renaming
 * precedent `jo:lz`'s own `hp_instance` parameter already established
 * above) to `self`: either RECLASSES this planet into a different
 * concrete class (`hp.cw` truthy), assigns a pending "next colonization
 * kind" (`hp.go` truthy), or colonizes it outright (neither).
 *
 * `local self=kv[hp.cw]({...})` is a Lua LOCAL REASSIGNMENT of the
 * parameter `self`, not a mutation of the original table -- every
 * statement AFTER this reassignment (`fm:nx(fp)rn(...)self:kd()`) that
 * reads/calls through `self` operates on the NEW instance in this branch,
 * while `self.p_=true` (evaluated BEFORE the reassignment) still marks
 * the ORIGINAL planet for removal (`dg()`'s own per-tick `if qc.p_ then
 * he(qc) end` sweep, Chunk 6, already built). Ported as a local C
 * variable `working_self` initialized to `self` and reassigned in this
 * branch, with `self->fields`'s own `p_` write happening on the ORIGINAL
 * pointer first -- `self:kd()` at the very end calls
 * `pico8_jo_kd(working_self)`, matching this exact reassignment
 * semantics precisely, not a guessed simplification.
 *
 * `kv[hp.cw](...)`: the flagged dynamic-reclassing forward reference,
 * ported against the already-generic `pico8_class_lookup_named` (see this
 * file's own top "SUB-LOT 2" comment for the full trace on why no
 * concrete class name is hardcoded here) -- `{p=self.p,ra=self.ra,
 * id=self.id}` (read off the ORIGINAL `self`, each field omitted from the
 * overrides table when absent, matching every other "nil key is simply
 * never set" table-constructor precedent already established in this
 * port, e.g. `pz()`'s own `b.fa` handling) becomes the new instance's own
 * overrides. `pico8_class_lookup_named(cw)` returns NULL today (no class
 * registers under any real `hp.cw` string yet), so `pico8_class_
 * instantiate` is never actually reached by this sub-lot's own real
 * exercise -- `working_self` stays NULL in that case, and every
 * subsequent `pico8_jo_kd(NULL)` etc. call is this port's own established
 * "NULL self is a safe no-op" convention, not a crash.
 *
 * `df[hp.go]`: `df` (Chunk 11's already-built `pico8_df()`) indexed
 * polymorphically by `hp.go`'s own runtime kind -- a PICO8_VALUE_FIXED
 * indexes by `pico8_ob_table_get_index`, a PICO8_VALUE_STRING by
 * `pico8_ob_table_get_name`, mirroring `pico8_ho_apply`'s own identical
 * `r[ri[i]]` polymorphic-key precedent (pico8_collect.c).
 *
 * `ov("autoassemblers",3,2)`: Chunk 11's already-built `pico8_ov`
 * (pico8_tech_gate.h), narrowed to a `pico8_fixed_t` for the local `fp`
 * (defaulting back to the Lua source's own literal initial value, 2, in
 * the unevidenced case its result isn't PICO8_VALUE_FIXED). `fm:nx(fp)`:
 * Chunk 12's already-built `pico8_hy_nx` against `pico8_fm()` (Chunk 10's
 * own placeholder singleton accessor, pico8_probe.h). `rn(hp.rn or 55)`:
 * Chunk 1's inert `pico8_rn` stub, `hp_instance`'s own "rn" field (this
 * file's own `pico8_jo_truthy`-checked) or the literal fallback 55.
 *
 * A NULL `self` is a safe no-op (nothing to reclass/colonize). A NULL
 * `hp_instance` reads every one of its own fields as absent (this port's
 * pervasive convention), taking the final `else` branch (`hp.cw`/`hp.go`
 * both falsy) -- colonizing `self` with an entirely empty research
 * project, faithfully reproducing what real Lua would do reading fields
 * off a nil `hp` EXCEPT that real Lua would crash on `hp.cw`; no real
 * call site in scope ever passes a genuinely nil `hp`, so this is purely
 * defensive, matching this port's pervasive "absent argument reads as
 * absent" convention rather than a guessed behavior change. */
void pico8_jo_hu(pico8_instance_t *self, pico8_instance_t *hp_instance);

/* function jo:hh()self.nk=
 * jo.ir()<0.45
 * and df.ll
 * or jo.oc()self.ep=self.nk.d
 * self:ks(self.nk.me or "known")self:kd()end
 * (pico8/lua.lua:407-411.) Picks this planet's own next "hazard" (`self.nk`)
 * roughly 45% of the time from the fixed `df.ll` entry, otherwise from the
 * `jo.oc()` weighted-anti-repeat generator -- both class-level fields
 * assigned only by `bw()` (Chunk 20, not yet built; see this file's own
 * top "SUB-LOT 2" comment for the full trace on why this sub-lot builds
 * the real accessor plumbing for them regardless). `jo.ir()<0.45 and
 * df.ll or jo.oc()` is Lua's `A and B or C` idiom at FULL strength here
 * (not the simpler "B is always truthy" shortcut some other ports in this
 * codebase get to take): if `df.ll` itself were ever absent/falsy, the
 * expression falls through to `jo.oc()` even when the roll succeeded --
 * ported as an explicit `use_ll` flag requiring BOTH the roll AND a
 * genuinely truthy `df.ll` lookup, not a plain ternary on the roll alone.
 * `self.ep=self.nk.d` and `self:ks(self.nk.me or "known")` read fields off
 * whatever `self.nk` resolved to (a `pico8_ob_table_t*` REF either way --
 * `df.ll` is one of `df`'s own named entries, and `jo.oc()`'s own ticket
 * pool is built FROM `df`'s positional entries, so both branches produce
 * the same underlying record shape); a NULL/absent `self.nk` (today's
 * always-true case, since neither generator is ever real yet) reads both
 * as absent, taking `self:ks("known")` and leaving `self.ep` as
 * `pico8_value_ref(NULL)`. `self:kd()` (this sub-lot's own `pico8_jo_kd`,
 * sub-lot 1) always runs last, unconditionally. A NULL `self` is a safe
 * no-op. */
void pico8_jo_hh(pico8_instance_t *self);

/* function jo:hk()if self.kr and self.kr~=0 and gd(self)then
 * iv({ra=self.ra,ne(self.kr,10,"$")})end
 * end
 * (pico8/lua.lua:515-517.) Pops up a floating income/expense toast
 * (`iv`, Chunk 8's already-built `pico8_iv_new`, pico8_msgbox.h) once per
 * dispatched year-tick, when this planet actually has a nonzero `self.kr`
 * (its own income/expense field, set only by `hy:kt()`, Chunk 12) AND
 * `gd(self)` (Chunk 1's `pico8_gd`, "is this entity currently visible on
 * screen") is true. `self.kr and self.kr~=0`: real Lua truthiness (a
 * fixed 0 IS truthy) combined with an explicit numeric inequality --
 * ported as "field present (any kind) AND, when it's a real fixed value,
 * not equal to 0" via this file's own `pico8_jo_truthy` plus a direct
 * `!=0` check, not collapsed to just the numeric check alone (an absent
 * `self.kr` must also skip the toast, matching `self.kr and ...`'s own
 * short-circuit).
 *
 * `iv({ra=self.ra,ne(self.kr,10,"$")})`: `ra=self.ra` omitted from the
 * overrides table entirely when `self.ra` is absent (the established
 * "nil key is never set" table-constructor convention, matching `jo:hu`'s
 * own `{p=...,ra=...,id=...}` treatment above) -- `iv:pq()`'s own
 * already-built default (`self.ra=self.ra or je.ra+mo.ra-v(0,5)`,
 * pico8_msgbox.h) then supplies a position when it's absent, exactly as
 * intended. `ne(self.kr,10,"$")`'s own two return values (text, color)
 * expand into `iv`'s own overrides table as POSITIONAL entries 1 and 2
 * (the last expression in the table constructor, real Lua's own
 * multi-return expansion rule -- see `pico8_hy_lu`'s own identical
 * `ne(-jt,10,"$")` call, pico8_economy.c, for the established precedent
 * this mirrors exactly).
 *
 * `hk` IS one of `bm`'s 5 real dispatch names (see this file's own top
 * "SUB-LOT 2" comment) -- wired as a real `PICO8_VALUE_FUNCTION` field
 * via `pico8_jo_hk_dispatch` (ignoring the dispatched `self.yr` argument
 * entirely, matching `jo:hk()`'s own parameterless body and
 * `pico8_jo_hb_dispatch`'s own identical "bridge ignores every argument"
 * precedent), closing `hy:hk()`'s own `cv.hk` dispatch loop (pico8_
 * economy.h, Chunk 12, already built) for real. Also directly callable
 * as this plain C function. A NULL `self` is a safe no-op. Read-only on
 * `self` -- no field is written. */
void pico8_jo_hk(const pico8_instance_t *self);

/* function jo:ldown()if self.me=="known" then
 * self:fe()
 * return true
 * elseif self.me=="ly" then
 * return jc({qu=self})end
 * end
 * (pico8/lua.lua:412-417.) Left-click on a planet: if it's still an
 * un-colonized but already-`known` (surveyed) planet, opens the research
 * panel (`self:fe()`, `pico8_planet_display.h`'s own `pico8_jo_fe` --
 * see this file's own "STEP 2 REBALANCE" note, `pico8_planet_display.h`,
 * for why `fe` itself lives there rather than here) and returns the
 * genuine Lua boolean `true` (`ui:bh()`'s own `type(pw.kn)=="table"` check,
 * pico8_ui.h, treats a non-table truthy return as "handled the click,
 * nothing to hold as a drag" -- distinct from `bg:ldown`'s own "return a
 * fresh table to hold as a drag" case); if it's already colonized
 * (`self.me=="ly"`, compared by STRING CONTENT like `jo:gg`/`jo:gv`),
 * constructs and returns a fresh `jc` (slipway) instance with `qu=self`
 * (the slipway's own "already connected from" planet field, per `jo:gv`'s
 * own comment on that same field) -- the caller (`ui:bh()`) holds onto
 * this returned table as a slipway-drag-in-progress. Any other `self.me`
 * (the class-level default, "unknown") falls off the end, returning Lua
 * nil (`pico8_value_ref(NULL)`).
 *
 * `jc({qu=self})`: a DIRECT constructor-style call using the class's own
 * name as a global -- `jc` (Chunk 14, the slipway class, not yet built)
 * doesn't exist yet. Ported against the SAME already-generic
 * `pico8_class_lookup_named` mechanism `jo:hu`'s own `kv[hp.cw]` forward
 * reference already established (pico8_class.h, Chunk 7) -- looked up by
 * the literal name `"jc"` (this port's own established convention of
 * using the real Lua global's own name as the lookup key, matching
 * `bg:ldown`'s own `"mz"`/`"g_"` choice, pico8_bg.h) rather than a direct
 * `pico8_jc_new`-style call this port has no header for yet. Returns
 * `pico8_value_ref(NULL)` today (no class has registered under `"jc"`
 * yet) -- compiles and is mechanically ready, not yet meaningfully
 * exercisable, the SAME "forward references are ported, not deferred"
 * treatment every other such reference in this port already gets; once
 * Chunk 14 calls `pico8_class_register_named("jc", jc_class)` alongside
 * its own `pico8_class_declare()`, this starts resolving and
 * instantiating for real, with no further change needed here.
 *
 * Not one of `bm`'s 5 real dispatch names, but still a real `mv()`-routed
 * field -- `ui:jz`'s own dynamically-named `self:jz(bt.."down",self.qa,
 * je.ra)` (pico8_ui_button.h, Chunk 5, already built) reaches "ldown" by
 * name with 2 extra arguments the real, parameterless `jo:ldown()` simply
 * discards (this file's own `pico8_jo_ldown_dispatch`, pico8_planet.c) --
 * the same "field function, not a `cv` auto-registration" treatment
 * `gg`/`hb`/`gv`/`hk` already established. Also directly callable as this
 * plain C function. A NULL `self` (or a `self.me` that isn't a string) is
 * a safe no-op, returning Lua nil. */
pico8_value_t pico8_jo_ldown(pico8_instance_t *self);

/* `jo:fe()` -- MODULE-SIZE REBALANCE (this same sub-lot's own split, see
 * pico8_planet_display.h's own top comment for the trace): declared and
 * defined in `pico8_planet_display.c`/`.h` instead of here, alongside
 * `gh`/`gv`/`hb`. `jo:ldown`'s own `self:fe()` call above reaches it via
 * `pico8_planet_display.h` (already included by this module's own `.c`
 * file). See `pico8_planet_display.h` for `fe`'s full Lua-source trace
 * and the closure-representation technique its own "my" field needed. */

/* jo.oc,jo.ir=gn(df,2,jf(0.5,dk.en)),jf(0.8,0.5) (pico8/lua.lua:750-751,
 * inside `bw()`, Chunk 20 -- not yet built). Two CLASS-LEVEL fields (NOT
 * per-instance) holding live generator-closure state, read by `jo:hh()`
 * above. Exposed as setter accessors (this port's established "singleton
 * globals ownership" convention, mirroring `pico8_entity_set_mo`'s own
 * "frees the previous value, a NULL is a valid way back to 'none'"
 * precedent, pico8_entity.h) rather than a raw extern -- `pico8_jo_init()`
 * does NOT construct these itself (their real Lua construction genuinely
 * lives inside `bw()`, Chunk 20's own scope, not this class's), but
 * `pico8_jo_shutdown()` DOES release whichever generator is currently set
 * (this module's own owned storage, freed the same "not part of the Lua
 * original, no GC in C" way every other class/instance teardown in this
 * port already is), so a repeated `pico8_jo_init()` (test isolation)
 * never leaks a previous test case's generators into the next. Both
 * default to NULL (the state `jo:hh()` sees before Chunk 20 ever runs). */
void pico8_jo_set_oc(pico8_gn_state_t *oc);
void pico8_jo_set_ir(pico8_jf_state_t *ir);

#endif
