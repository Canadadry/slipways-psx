#include "pico8_cart.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_audio.h"
#include "pico8_bg.h"
#include "pico8_bootstrap.h"
#include "pico8_class.h"
#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_economy_score.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_fixed.h"
#include "pico8_galaxy.h"
#include "pico8_game_data.h"
#include "pico8_gfx.h"
#include "pico8_help.h"
#include "pico8_hud.h"
#include "pico8_ke.h"
#include "pico8_layout.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_menu.h"
#include "pico8_mouse_reader.h"
#include "pico8_msgbox.h"
#include "pico8_ob.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_planet_family_trade.h"
#include "pico8_probe.h"
#include "pico8_research.h"
#include "pico8_research_icon.h"
#include "pico8_score_panel.h"
#include "pico8_scroll.h"
#include "pico8_slipway.h"
#include "pico8_trade.h"
#include "pico8_ui.h"
#include "pico8_ui_button.h"
#include "pico8_value.h"
#include "pico8_vec.h"
#include "pico8_wormhole.h"

/* See pico8_cart.h's own top comment for the full "why this file exists"
 * trace. This comment documents pico8_cart_load()'s own ORDERING, verified
 * by actually running the whole sequence under ASan/UBSan (this chunk's
 * own disposable whole-game verification program, not committed) rather
 * than guessed, per this chunk's own explicit instruction.
 *
 * ke/qb/nf/ui (pico8_ke_init/pico8_qb_init/pico8_nf_init/pico8_ui_init):
 * FIRST, in this exact order -- `ke` is the ROOT every ke-descended class
 * this whole port has built extends, directly or transitively (`qb`/`nf`/
 * `ui`/`js`/`fq`/`bg`/`mh`/`iv`/`g_`/`jo`/`jc`/`mz`/`pj`/`ie` and `jo`'s own
 * whole planet-family subtree), so it must exist before ANY of them are
 * declared -- `qb`/`nf`/`ui` themselves are ke-descended (Chunk 4/5), hence
 * this order even among these four. These four used to be built INSIDE
 * pico8_jl() on every call (Chunk 6's own stopgap, in the absence of any
 * real "run once" hook) -- see pico8_bootstrap.h's own "CHUNK 22 FIX" note
 * for the real heap-use-after-free this reassignment of responsibility
 * fixes. pico8_jl() itself (called moments later, by pico8__init()) still
 * defensively builds them if they're somehow still missing (e.g. a test
 * that calls pico8_jl() directly without ever calling this function) --
 * calling pico8_ke_init() etc. here first, unconditionally, is what makes
 * pico8_jl()'s own later defensive check a no-op in the normal case.
 *
 * pico8_layout_init() (lq, jb): a ROOT class pair (`kv:jr()`, no parent),
 * no dependency on ke or anything else -- position doesn't matter, placed
 * early since several later classes' own spec strings construct `jb`
 * instances (`ie`'s own `jb=jb(dr[i])`, this file's own pico8_eu()).
 *
 * pico8_pi_init(): registers ob()'s own char-keyed 'b'/'v'/'s' nested-
 * constructors and bl()'s own string-keyed "br"/"qr"/"rf"/"s"/"ld"
 * dispatch table (Chunk 2/3's own two-registry design). Placed right after
 * pico8_layout_init() so "ld"->lq.ol's own real target (`pico8_lq_class()`)
 * already exists the moment this runs, even though (per pico8_pi.c's own
 * comment) the registration itself doesn't strictly need that yet -- only
 * calling through "ld" later does. Every OTHER class below that has a
 * nested `v(...)`/`b(...)`/`o(...)` constructor in its own spec (`nf`,
 * `qb`, `ui`, `mh`, `iv`, `jo`, `qh`/its planet-family siblings, `ie`, `pj`)
 * also calls pico8_pi_init() defensively itself (idempotent, per each
 * module's own established convention) -- this explicit call is for
 * clarity/completeness, not because a later defensive call would be
 * insufficient on its own.
 *
 * pico8_trade_init() (trade): a ROOT class, no dependency -- position
 * doesn't matter, grouped here with the other independent roots.
 *
 * pico8_game_data_init() (es/ct/b_/co/fb, bn/bq): pure data literals, no
 * class/dependency at all -- position doesn't matter, grouped here too.
 *
 * pico8_js_init()/pico8_fq_init() (js, fq): ke-descended (Chunk 4),
 * independent of each other and of everything below.
 *
 * pico8_bg_init() (bg): ke-descended (Chunk 7). `bg:ldown`'s own `mz`/`g_`
 * forward reference (pico8_class_lookup_named) resolves for real once
 * pico8_probe_init()/pico8_wormhole_init() run later in this same function
 * -- textual/declaration order here doesn't matter for THAT reference
 * (resolved dynamically, by name, at dispatch time, not at declare time),
 * only real, structural `.parent` links (`pico8_class_declare(parent,...)`)
 * do.
 *
 * pico8_mh_init()/pico8_iv_init() (mh, iv): ke-descended (Chunk 8).
 * `iv:pq`'s own `je.ra`/`mo.ra` reads happen at INSTANTIATION time, not
 * class-declare time (no `iv` instance is built anywhere in this whole
 * function), so this position has no ordering hazard despite `je`/`mo`
 * not being real singletons yet at this point in pico8_cart_load() itself
 * (they become real the moment pico8__init()'s own pico8_jl() call runs,
 * strictly after this whole function returns).
 *
 * pico8_probe_init() (g_, `fm` placeholder): ke-descended (Chunk 10) --
 * THE PREVIOUSLY-FLAGGED GAP (docs/plan.md's own "Notes de portage" entry
 * on `pico8_probe_init()`, Phase 3 chunk 11's own "Angle mort
 * d'intégration séparé") THIS CALL CLOSES: before this chunk, this
 * function was never called anywhere outside a handful of tests, so `g_`/
 * `mz` (via `bg:ldown`'s forward reference) and `fm` (via `bv`/`dl`/`d_`/
 * `oj`/`gi`/`cb`'s own forward references) were never exercisable end to
 * end by anything resembling a real play-through. Placed right after
 * `bg` so `bg:ldown`'s own "g_" named-registry lookup starts resolving to
 * something real as early in this sequence as possible (again, position
 * doesn't strictly matter for a by-name dynamic lookup, but this ordering
 * keeps the "when does each forward reference start being real" story
 * simple to read top-to-bottom).
 *
 * pico8_economy_init() (hy): a ROOT class (Chunk 12), no dependency on
 * anything built so far -- position doesn't matter.
 *
 * pico8_data_init(mem) (r/iz/df/hc/fd/gb/rr/dr/nb/ek, Chunk 11): needs a
 * real `mem` with pico8_load_map_json() already run against it (the
 * caller's own responsibility, documented on pico8_cart_load() itself in
 * pico8_cart.h) -- reads it via pico8_dispatch_mem() rather than taking it
 * as a parameter, matching this file's own established "read the shared
 * per-frame dispatch context" convention already used by `cu()`/`_draw()`
 * elsewhere in this same chunk. A NULL dispatch mem skips this step
 * entirely (defensive no-op, documented on pico8_cart_load()'s own
 * declaration) rather than crashing -- every real caller of this function
 * (this chunk's own verification program, and any future Phase 4/5 harness)
 * sets the dispatch mem before calling this function, so this is a
 * "compiles safely, not meaningfully skippable in real use" case, not a
 * genuine alternate code path. Internally calls pico8_stash_init(mem) and
 * pico8_research_init() (building the real `hp` root class and every real
 * `gb` entry, `hp` instances) itself -- NOT called separately here, per
 * this chunk's own "exactly once" instruction (see pico8_data_init.c's own
 * body for the confirmation). Placed after every OTHER class this cart
 * needs so far, since nothing above it reads `gb`/`dr`/etc. at
 * class-declare time (only later, real gameplay dispatch does) -- no hard
 * ordering constraint forces it here specifically, but grouping it near
 * the classes/chunks that DO consume its output (`jo`, `ie`) keeps the
 * sequence's own narrative readable.
 *
 * pico8_jo_init() (jo): ke-descended (Chunk 13) -- the base planet entity.
 * Needs only `ke` (already built); its own real runtime dependencies on
 * `hp`/`hy`/`trade` (via dynamic dispatch, e.g. `jo:cn`'s `trade:kg` call)
 * are resolved at CALL time, not declare time, matching the "mutually-
 * recursive trade cluster is not force-ordered" Implementation Decision.
 *
 * pico8_jc_init() (jc): ke-descended (Chunk 14) -- the slipway entity.
 * Needs only `ke`; its own real runtime dependency on `jo` (`self.qu`/
 * `self.re`) is likewise resolved at call time. Placed right after `jo`
 * since it's the slipway that connects two `jo`-descended planets, purely
 * for narrative grouping.
 *
 * pico8_planet_family_init() (qh, nl, os, hj, pm, ga): MUST run after
 * pico8_jo_init(), since `qh`/`ga` both extend `jo` directly
 * (`pico8_class_declare(pico8_jo_class(),...)`) -- a real, structural
 * ordering requirement, not a stylistic one. Internally declares `qh`
 * before `nl`/`os`/`hj` (which extend `qh`) and `hj` before `pm` (which
 * extends `hj`), all within this one function's own already-correct body
 * (Chunk 15, unchanged by this chunk).
 *
 * pico8_planet_family_trade_init() (nr, fr, mx): MUST run after
 * pico8_planet_family_init(), since `nr` extends `qh` and `mx` extends
 * `hj` (both built by the previous call) -- another real, structural
 * ordering requirement. Internally declares `nr` before `fr` (which
 * extends `nr`).
 *
 * pico8_wormhole_init() (mz): ke-descended (Chunk 16) -- the OTHER half of
 * `bg:ldown`'s own `mz`/`g_` forward reference, now real. No structural
 * dependency on planet-family beyond `ke` itself (`mz`'s own runtime use of
 * `bv`/`dl`/`ov` is resolved at call time).
 *
 * pico8_help_init() (fv, bo): MUST run after pico8_probe_init(), since `fv`
 * extends `g_` directly (`pico8_class_declare(pico8_g__class(),...)`) --
 * a real, structural ordering requirement.
 *
 * pico8_score_panel_init() (pj, ef, bz; internally also calls
 * pico8_score_panel_display_init() itself -- NOT called separately here,
 * per this chunk's own "exactly once" instruction): ke-descended (Chunk
 * 18), no structural dependency on anything above beyond `ke`.
 *
 * pico8_research_icon_init() (ie): ke-descended (Chunk 19), no structural
 * dependency on anything above beyond `ke` (`ie`'s own real runtime use of
 * `oj`/`dl` is resolved at call time; `ie`'s own `self.ke`/`self.jb`
 * fields, set later by pico8_eu() below, are per-INSTANCE overrides, not
 * class-declare-time dependencies).
 *
 * pico8_menu_init() (ix, ha, cb-adjacent UI callback wiring, Chunk 21):
 * placed LAST among the class/data inits -- its own real runtime bodies
 * read `pico8_jo_gg`/`pico8_pj_class()`'s own `cr`/`hb` fields and
 * `pico8_cb`/`pico8_gi`, all of which are real by this point, though (as
 * with every other module here) these are call-time reads, not
 * class-declare-time dependencies, so this position is for narrative
 * grouping (PRD Chunk 21 depends on Chunks 8/11/12/18) rather than a hard
 * requirement.
 *
 * pico8_help_set_eu(pico8_eu_dispatch)/pico8_score_panel_set_init(
 * pico8_init_dispatch): THE TWO PLACEHOLDER-BRIDGE RECONCILIATIONS, done
 * LAST, once every function/class this whole cart needs already compiles
 * and exists (both bridges are plain C function pointers -- they don't
 * depend on `fm`/`ju`/`dk` being real yet, only on `pico8_eu_dispatch`/
 * `pico8_init_dispatch` existing as functions, which they do as soon as
 * this file is compiled) -- see pico8_help.h's own "CHUNK 22
 * RECONCILIATION NOTE" for `eu` and pico8_score_panel.h's own identical
 * note for `_init`, both of which this call satisfies. Done here, inside
 * pico8_cart_load(), rather than scattered at each individual call site
 * that happens to construct a `bo`/`ha`-derived button, per this chunk's
 * own "decide and document one sensible point, don't scatter it ad hoc"
 * instruction -- every `bo` entry's own "my" field (set once, per real
 * `fv:pq()` run) and every `_init`-as-a-value button (set once, per real
 * `pj:ph()` run) reads THROUGH `pico8_eu_field_value()`/
 * `pico8_init_field_value()` at the time IT is built, so as long as this
 * reconciliation runs before the FIRST real `fv()`/`pj:ph()` call
 * (guaranteed here, since `pico8_cart_load()` always runs before
 * `pico8__init()`'s own first `fv()` call), every real button this whole
 * cart ever builds sees the real dispatch, never the placeholder. */
void pico8_cart_load(void) {
    pico8_mem_t *mem;

    pico8_ke_init();
    pico8_qb_init();
    pico8_nf_init();
    pico8_ui_init();

    pico8_layout_init();
    pico8_pi_init();
    pico8_trade_init();
    pico8_game_data_init();

    pico8_js_init();
    pico8_fq_init();
    pico8_bg_init();
    pico8_mh_init();
    pico8_iv_init();
    pico8_probe_init();
    pico8_economy_init();

    mem = pico8_dispatch_mem();
    if (mem != NULL) {
        pico8_data_init(mem);
    }

    pico8_jo_init();
    pico8_jc_init();
    pico8_planet_family_init();
    pico8_planet_family_trade_init();
    pico8_wormhole_init();

    pico8_help_init();
    pico8_score_panel_init();
    pico8_research_icon_init();
    pico8_menu_init();

    pico8_help_set_eu(pico8_eu_dispatch);
    pico8_score_panel_set_init(pico8_init_dispatch);
}

/* Not part of the Lua original -- see pico8_cart.h's own comment on this
 * declaration. Tears down every module pico8_cart_load() builds, in
 * reverse order, so a class is never freed while something declared AFTER
 * it (and therefore potentially holding a `.parent` pointer INTO it, or a
 * runtime reference to one of its instances) still exists. */
void pico8_cart_shutdown(void) {
    pico8_menu_shutdown();
    pico8_research_icon_shutdown();
    pico8_score_panel_shutdown();
    pico8_help_shutdown();

    pico8_wormhole_shutdown();
    pico8_planet_family_trade_shutdown();
    pico8_planet_family_shutdown();
    pico8_jc_shutdown();
    pico8_jo_shutdown();

    pico8_data_shutdown();

    pico8_economy_shutdown();
    pico8_probe_shutdown();
    pico8_iv_shutdown();
    pico8_mh_shutdown();
    pico8_bg_shutdown();
    pico8_fq_shutdown();
    pico8_js_shutdown();

    pico8_game_data_shutdown();
    pico8_trade_shutdown();
    pico8_layout_shutdown();

    pico8_ui_shutdown();
    pico8_nf_shutdown();
    pico8_qb_shutdown();
    pico8_ke_shutdown();
}

/* function _init()cu()jl()ie.gy,kc=
 * ie.eh
 * fv()music(0)end
 * (pico8/lua.lua:1178-1180.)
 *
 * `cu()`: Chunk 1's pico8_cu(pico8_mem_t*) (pico8_hud.h) -- builds the
 * palette-remap table at 0x5000. Reads `mem` via pico8_dispatch_mem(),
 * matching every other already-ported function's own established pattern
 * for reading the shared per-frame dispatch context.
 *
 * `jl()`: Chunk 6's bare pico8_jl(void) (pico8_bootstrap.h) -- rebuilds
 * hv/cv/cg and the real mo/je/ok singletons (and, defensively, ke/qb/nf/ui
 * if pico8_cart_load() somehow hasn't already built them -- see
 * pico8_bootstrap.h's own "CHUNK 22 FIX" note).
 *
 * `ie.gy,kc=ie.eh`: a real Lua multiple-assignment with FEWER values (one,
 * `ie.eh`) than names (two, `ie.gy` and `kc`) -- Lua pads the extra name
 * with nil, the SAME "pad with nil" rule already used elsewhere in this
 * port (e.g. `mh:pq`'s own `self.p_,kc=true`). `ie.gy=ie.eh` is a plain
 * CLASS-LEVEL field copy on `ie`'s own class (Chunk 19,
 * pico8_research_icon.h/.c) -- both `gy` and `eh` are read/written via
 * direct class-level field access on `pico8_ie_class()->defaults`, the
 * exact same "read/write a class's own default field directly" idiom
 * already established for `pj.ea`/`ui.hw`/`mz.jj`/`ie.gy` itself (Chunk
 * 19's own `ie:hb`). The copied VALUE is the raw `pico8_value_t` read off
 * "eh" (a PICO8_VALUE_REF to `eh`'s own heap `pico8_vec_t`), stored as-is
 * under "gy" -- an ALIAS at the moment of assignment, exactly mirroring
 * real Lua's own "assigning a table value copies the reference" semantics
 * (the same "snapshot vs alias" distinction pico8_research_icon.h's own
 * top comment already works through for `self.ra=ie.gy`) -- NOT a fresh
 * heap copy of the vector's numeric content, which would be observably
 * equivalent here (nothing ever mutates `eh`'s own heap vector in place)
 * but representationally wrong. `kc=nil`: Chunk 8's own
 * pico8_msgbox_set_kc(NULL) (pico8_msgbox.h) -- called explicitly (not
 * assumed already NULL from module init), since real Lua's own `_init()`
 * deliberately clears any previously-open message box on a fresh game
 * start/restart, matching `_init`'s own real reuse as the restart button's
 * own `my` callback (Chunk 21's `ha[1].my=_init`).
 *
 * `fv()`: Chunk 17's pico8_fv_new(NULL) (pico8_help.h), result discarded
 * (construct for side effect, matching this port's established
 * "construct for side effect, discard the result" precedent, e.g. Chunk
 * 20's placed `jo` instances).
 *
 * `music(0)`: Phase 1/2's own no-op stub, pico8_music(int) (pico8_audio.h).
 *
 * Requires pico8_cart_load() to have already run (see this function's own
 * declaration in pico8_cart.h) -- `ie`'s class and `fv`'s class/`bo` table
 * must already be real. */
void pico8__init(void) {
    pico8_mem_t *mem = pico8_dispatch_mem();
    pico8_value_t eh_val;

    pico8_cu(mem);
    pico8_jl();

    if (pico8_ie_class() != NULL && pico8_ob_table_get_name(pico8_ie_class()->defaults, "eh", &eh_val)) {
        pico8_ob_table_set_name(pico8_ie_class()->defaults, "gy", eh_val);
    }
    pico8_msgbox_set_kc(NULL);

    (void)pico8_fv_new(NULL);
    pico8_music(0);
}

/* `_init` stored as a bare function value (`pj:ph()`'s own
 * `js(ob([[...]],{my=_init}))`, pico8_score_panel_display.c) -- bridges
 * pico8_value_function_t's `(self,argc,arg0,arg1)` shape to pico8__init's
 * own zero-argument real signature. `_init()` declares NO Lua parameters
 * at all, so `self` (the button `mv(button,"my")` would pass) is simply
 * discarded, the same "excess argument silently dropped" treatment already
 * established throughout this port (e.g. `pico8_dl_dispatch`'s own unused
 * second argument). This IS the bridge pico8_score_panel.h's own "CHUNK 22
 * RECONCILIATION NOTE" for `_init` calls for -- installed via
 * pico8_score_panel_set_init() inside pico8_cart_load() above. */
pico8_value_t pico8_init_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8__init();
    return pico8_value_ref(NULL);
}

/* Not a port of any named Lua function: `nv(ix,js)`'s own real second
 * argument, `js`, IS the bare constructor `js(overrides)` itself (Chunk 4's
 * pico8_js_new) -- this is the pico8_nv_fn-shaped adapter that lets
 * pico8_nv() (Chunk 1's pico8_nv, pico8_collect.h) call it with the right
 * shape, the same "ob_table REF entry -> pico8_js_new -> REF to the new
 * instance" mapping pico8_pz_step (pico8_msgbox.c) already establishes for
 * an almost-identical real call site. */
static int pico8_eu_ix_map(pico8_value_t e, void *ctx, pico8_value_t *out_value) {
    const pico8_ob_table_t *row;
    pico8_instance_t *made;

    (void)ctx;
    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    row = (const pico8_ob_table_t *)e.as.ref;
    made = pico8_js_new(row);
    *out_value = pico8_value_ref(made);
    return 1;
}

/* function eu(nj)dk=nj
 * jl()bg()bw(90)fm,ju=hy(),pj()nv(ix,js)local gx={qh,os,nr,fr,nl,mx,hj
 * }
 * for i,b in pairs(gx)do
 * ie({ke=b,jb=jb(dr[i])})end
 * fm:lj()end
 * (pico8/lua.lua:1181-1186.) The real "start game" routine, assigned as a
 * bare function VALUE onto every `bo` entry's own "my" field (Chunk 17's
 * `fv:pq()`, `for i,d in pairs(bo)do d.my=eu end`), later invoked via
 * `mv(self,"my")` (confirmed real call sites: `js:ldown`'s own
 * `mv(self,"my")`, pico8/lua.lua:277 -- ZERO extra Lua arguments) -- so
 * `nj` IS BOUND TO `self`, the clicked button's own real instance. This is
 * exactly what pico8_eu_dispatch() (below) does: reads `nj`=`self` off its
 * own `void *self` parameter and forwards it here, per pico8_help.h's own
 * "CHUNK 22 RECONCILIATION NOTE" for `eu`.
 *
 * `dk=nj` -- THE `dk`/`self` REPRESENTATION SUBTLETY, RESOLVED AND VERIFIED
 * (not guessed): `pico8_economy_set_dk()` (pico8_economy_score.h) expects
 * a plain `pico8_ob_table_t*`, but `nj`/`self` here is a real
 * `pico8_instance_t*` -- confirmed by reading `pico8_pz_step`'s actual
 * body (pico8_msgbox.c), which is what `fv:pq()`'s own `pz(self,bo)` call
 * uses to build each `bo` entry into a clickable widget: it calls
 * `pico8_js_new(b)`, i.e. every `bo` entry genuinely becomes a real
 * `js`-class `pico8_instance_t`, not a raw table. Since
 * `pico8_class_instantiate()` (pico8_class.c) copies EVERY field of its
 * `overrides` argument (here, the original `bo` entry `b`, including
 * `px`/`trade`/`en`/`my`) directly into the new instance's own `->fields`
 * table (`pico8_instance_t.fields` is ITSELF a plain `pico8_ob_table_t*`,
 * confirmed by reading the struct definition, pico8_class.h), the correct,
 * faithful resolution is: pass `nj->fields` -- the constructed js
 * instance's OWN backing fields table (which genuinely holds
 * `px`/`trade`/`en`, copied verbatim from the original `bo` entry at
 * construction time) -- rather than inventing a second, parallel raw-
 * ob_table path. This does NOT need a classless-wrapper-instance trick the
 * way `mz.jj` (Chunk 16) or `self.ke` (Chunk 19) did: those cases needed a
 * wrapper because they had a bare CLASS or a bare TABLE that needed
 * dressing up as something `pico8_instance_get_field`'s chain-walk could
 * consume; here we already have a REAL instance in hand (`nj`/`self`
 * itself), and `pico8_economy_set_dk()`'s own signature only ever wants
 * the `pico8_ob_table_t*` a `pico8_dk()`-style reader consults via
 * `pico8_ob_table_get_name` -- `nj->fields` already IS exactly that table,
 * with no extra indirection needed. A NULL `nj` maps to a NULL `dk`
 * (defensive, matching this port's established convention).
 *
 * `jl()`: called AGAIN here -- real Lua really does call `jl()` a SECOND
 * time (once via `_init()`, once via `eu()`), which is faithful, not a
 * bug: pico8_jl()'s own body unconditionally rebuilds hv/cv/cg (and
 * reconstructs mo/je/ok) from scratch every call, exactly matching real
 * Lua's own unconditional `hv,cv,cg={},{},{}` inside `jl()`'s literal body
 * -- ported literally, not special-cased or skipped on this second call.
 *
 * `bg()`: Chunk 7's pico8_bg_new(NULL) (pico8_bg.h), result discarded
 * (construct for side effect).
 *
 * `bw(90)`: Chunk 20's pico8_bw(pico8_fixed_t) (pico8_galaxy.h), called
 * with pico8_fixed_from_int(90).
 *
 * `fm,ju=hy(),pj()`: THE FM/JU RECONCILIATION, per pico8_economy.h's own
 * "*** FM RECONCILIATION DECISION ***" and pico8_score_panel.h's own two
 * "*** CHUNK 22 RECONCILIATION NOTE ***" comments (for `_init`-as-a-value
 * and for `ju`), both already telling this function exactly what to do:
 * `pico8_probe_set_fm(pico8_hy_new(NULL))` replaces the classless `fm`
 * placeholder with a real `hy` instance; `pico8_pj_set_ju(pico8_pj_new(
 * NULL))` replaces the NULL `ju` placeholder with a real `pj` instance.
 * ORDER MATTERS: both run BEFORE `fm:lj()` at the very end of this same
 * function, since that call needs `fm` to already be the real instance.
 *
 * `nv(ix,js)`: Chunk 1's pico8_nv (pico8_collect.h) mapping every one of
 * `ix`'s 4 positional entries (Chunk 21's pico8_ix(), pico8_menu.h --
 * bridged to a `pico8_list_t` via this port's own well-established
 * "ob_table positional entries -> list" bridge) through pico8_js_new
 * (Chunk 4, via the private pico8_eu_ix_map adapter above) -- turning each
 * `ix[i]` override-table into a real `js` button instance. The MAPPED
 * RESULT (a `pico8_list_t` of the 4 new `js` instances) is itself
 * DISCARDED in real Lua (`nv(ix,js)` is a bare statement) -- ported the
 * same way: the temporary `pico8_list_t` bridge/mapped-result lists are
 * freed after use (caller-owns-and-frees, matching every other
 * `pico8_nv`/`pico8_jp` caller in this port), while the 4 CONSTRUCTED
 * INSTANCES themselves are intentionally leaked forever (same
 * "placed/constructed entity lives forever" precedent as Chunk 20's placed
 * planets).
 *
 * `local gx={qh,os,nr,fr,nl,mx,hj}`: a plain local array of 7 real,
 * already-built `pico8_class_t*` values (Chunk 13/15's own class
 * accessors).
 *
 * `for i,b in pairs(gx)do ie({ke=b,jb=jb(dr[i])})end`: 7 iterations
 * (1-based, matching `gx`'s own 7 elements in order), each constructing a
 * real `ie` instance (Chunk 19's pico8_ie_new, pico8_research_icon.h) with
 * a 2-field overrides table: `ke=b` (the CLASS pointer itself, stored as a
 * `pico8_value_ref` -- matches `ie`'s own already-established
 * `self.ke`-as-class treatment from Chunk 19; no wrapper needed HERE since
 * this is just storing the raw class pointer as a field value -- the
 * wrapper trick is only needed when READING fields off it later, which
 * Chunk 19's own `ie` methods already handle) and `jb=jb(dr[i])` (Chunk
 * 3's own bare constructor call, pico8_jb_new, called with `dr[i]` --
 * Chunk 11's own pico8_dr(), its own i-th positional entry, a 1-based
 * pico8_ob_table_get_index lookup, passed AS `jb`'s own overrides -- each
 * `dr[i]` entry is itself a REF to a `pico8_ob_table_t`, confirmed against
 * `pico8_data_init.c`'s own construction of `dr` and
 * tests/test_pico8_data_init.c's own real-asset assertions). Each
 * constructed `ie` instance's result is discarded (construct for side
 * effect); the temporary 2-field `overrides` table is freed right after
 * `pico8_ie_new()` returns (`pico8_class_instantiate` shallow-copies its
 * entries first, so the temporary is safe to release immediately --
 * exactly the same "build a fresh overrides table, instantiate, free the
 * temporary" precedent already established by `pico8_bw`'s own galaxy
 * placement, pico8_galaxy.c).
 *
 * `fm:lj()`: Chunk 12's pico8_hy_lj(pico8_instance_t*) (pico8_economy.h),
 * a plain direct call on the now-real `fm` (pico8_fm()).
 *
 * Requires pico8_cart_load() to have already run, same reasoning as
 * pico8__init() above. */
void pico8_eu(pico8_instance_t *nj) {
    pico8_class_t *gx[7];
    int i;

    pico8_economy_set_dk(nj != NULL ? nj->fields : NULL);

    pico8_jl();
    (void)pico8_bg_new(NULL);
    pico8_bw(pico8_fixed_from_int(90));

    pico8_probe_set_fm(pico8_hy_new(NULL));
    pico8_pj_set_ju(pico8_pj_new(NULL));

    if (pico8_ix() != NULL) {
        pico8_list_t ix_items;
        pico8_list_t mapped;
        int j;

        pico8_list_init(&ix_items);
        for (j = 0; j < pico8_ix()->count; j++) {
            if (pico8_ix()->entries[j].key.kind == PICO8_OB_KEY_INDEX) {
                pico8_list_add(&ix_items, pico8_ix()->entries[j].value);
            }
        }
        mapped = pico8_nv(&ix_items, pico8_eu_ix_map, NULL);
        pico8_list_free(&mapped);
        pico8_list_free(&ix_items);
    }

    gx[0] = pico8_qh_class();
    gx[1] = pico8_os_class();
    gx[2] = pico8_nr_class();
    gx[3] = pico8_fr_class();
    gx[4] = pico8_nl_class();
    gx[5] = pico8_mx_class();
    gx[6] = pico8_hj_class();

    for (i = 0; i < 7; i++) {
        pico8_value_t dr_val;
        const pico8_ob_table_t *dr_entry = NULL;
        pico8_ob_table_t *overrides;
        pico8_instance_t *jb_instance;

        if (pico8_dr() != NULL && pico8_ob_table_get_index(pico8_dr(), i + 1, &dr_val) &&
            dr_val.kind == PICO8_VALUE_REF && dr_val.as.ref != NULL) {
            dr_entry = (const pico8_ob_table_t *)dr_val.as.ref;
        }
        jb_instance = pico8_jb_new(dr_entry);

        overrides = pico8_ob_table_new();
        pico8_ob_table_set_name(overrides, "ke", pico8_value_ref(gx[i]));
        pico8_ob_table_set_name(overrides, "jb", pico8_value_ref(jb_instance));
        (void)pico8_ie_new(overrides);
        pico8_ob_table_free(overrides);
    }

    pico8_hy_lj(pico8_fm());
}

/* `eu` stored as a bare function value (`fv:pq()`'s own `d.my=eu`, for
 * every positional entry of `bo`) -- bridges pico8_value_function_t's
 * `(self,argc,arg0,arg1)` shape to pico8_eu's own single-real-Lua-
 * parameter signature, reading `nj`=`self` off its own `void *self`
 * parameter (`argc`/`arg0`/`arg1` are unused: `eu(nj)`'s only real
 * parameter, `nj`, IS the dispatch `self`, never a `mv()` extra argument).
 * This IS the bridge pico8_help.h's own "CHUNK 22 RECONCILIATION NOTE" for
 * `eu` calls for -- installed via pico8_help_set_eu() inside
 * pico8_cart_load() above. */
pico8_value_t pico8_eu_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_eu((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* function _update60()dg()ok:lj()end
 * (pico8/lua.lua:1187.) `dg()`: Chunk 6's bare pico8_dg(void)
 * (pico8_bootstrap.h) -- the per-tick hv dispatch loop. `ok:lj()`: Chunk
 * 5/6's pico8_ui_lj(pico8_mem_t*, pico8_instance_t*) (pico8_ui.h), called
 * on pico8_ok() (pico8_ui.h) with `mem` from pico8_dispatch_mem(). */
void pico8__update60(void) {
    pico8_dg();
    pico8_ui_lj(pico8_dispatch_mem(), pico8_ok());
}

/* function _draw()cls()dv()dc("kj")dc("ev")end
 * (pico8/lua.lua:1188.) `cls()`: Phase 1's screen-clear, pico8_cls(mem)
 * (pico8_gfx.h). `dv()`: Chunk 1's pico8_dv_default(pico8_mem_t*)
 * (pico8_hud.h) -- the col/`no`-omitted entry point, matching real Lua's
 * own bare `dv()` call, which omits the argument `pico8_dv`'s full form
 * takes. `dc("kj")`,`dc("ev")`: Chunk 2's pico8_dc(const char*)
 * (pico8_entity.h), two literal calls -- `pico8_dc` takes neither `gfx`
 * nor `mem` itself (its own real signature, confirmed by reading
 * pico8_entity.h directly), so no dispatch-context read is needed for
 * these two calls specifically. */
void pico8__draw(void) {
    pico8_mem_t *mem = pico8_dispatch_mem();

    pico8_cls(mem);
    pico8_dv_default(mem);
    pico8_dc("kj");
    pico8_dc("ev");
}
