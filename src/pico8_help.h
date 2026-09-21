#ifndef PICO8_HELP_H
#define PICO8_HELP_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Phase 3, Chunk 17 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): the `fv` class -- the help-screen overlay ("need help? /
 * slipways.net/help"), pico8/lua.lua:1143-1170, read directly:
 *
 * fv=g_:jr([[
 * fa=10,ra=v(64,28),
 * hi=v(34,56),
 * ca=11,
 * gq=v(0,11),
 * eo=-1,
 * ff=1,
 * fc=5,
 * k_=o(0,1,1,2,4,9,10,9,4,2,1,1,1),
 * qo=1.02,ro=-0.007,
 * qg=0.007,q_=1.02,
 * pn=5,du=15,
 * py=0.0067,pk=1,
 * ki=17,
 * kq=b(-40,78,40,98),
 * ql=11,fg=5,
 * rb=o(
 * o(192,0,11,16,4,fn="s"),
 * o(172,20,107,2,2,fn="s"),
 * o("need help?",40,110,13,fn="qr"),
 * o("slipways.net/help",40,116,5,fn="qr"),
 * ),
 * ]])fv.hd=gk
 * function fv:pq()for i,d in pairs(bo)do
 * d.my=eu
 * end
 * pz(self,bo)end
 * function fv:kj(p)ei(self)bl(self.rb)end
 * (pico8/lua.lua:1143-1170.)
 *
 * `fv=g_:jr([[...]])`: `fv` EXTENDS THE REAL `g_` CLASS (Chunk 10,
 * pico8_probe.h/.c -- `pico8_g__class()`), the first class in this port to
 * extend `g_` rather than `ke`/`kv` directly. The mechanism is identical to
 * every other `pico8_class_declare(parent, spec)` call already proven at
 * inheritance depth by Chunk 15's `qh`->`hj`->`pm`/`mx` chain
 * (pico8_planet_family.c/pico8_planet_family_trade.c) -- nothing new to
 * build here.
 *
 * NO `nu=o(...)` TAG FIELD, A REAL, CONFIRMED DIVERGENCE FROM EVERY OTHER
 * `cg`-registering CLASS THIS PORT HAS PORTED SO FAR: reading `fv`'s own
 * spec string literally (above) shows no `nu=` field at all, unlike `g_`
 * (`nu=o("g_","prober")`), `mz` (`nu=o("mz")`), `mh` (`nu=o("mh")`), etc.
 * `nu` is what `pico8_cg()`'s own tag-registration reads (`pico8_entity.c`'s
 * `pico8_cv`/`cg` construction, Chunk 10) -- since `fv` never sets it, a
 * real `fv` instance is NEVER inserted into any `cg[...]` tag list at all
 * (not even a `cg["fv"]`). This is not an oversight to "fix"; it is exactly
 * what the real, minified source says, and nothing in this chunk's own
 * scope (or any grep of the rest of `pico8/lua.lua` for `cg.fv`/`cg["fv"]`)
 * ever reads such a list, so it is faithfully left absent.
 *
 * NAMED-CLASS REGISTRY: unlike `mz`/`g_` (Chunk 7's forward-reference-
 * driven `pico8_class_register_named`), `fv` never needs a byname lookup
 * anywhere else in the cart -- grepping every real `fv(` call site in
 * `pico8/lua.lua` finds exactly one, `_init()`'s own plain `fv()` call
 * (line 1180, Chunk 22), a direct global-variable call, never a
 * `kv[name]`-style dynamic-by-name construction the way `bg:ldown`'s
 * `mz`/`g_` choice or `jo:hu`'s `kv[hp.cw]` reclassing are. So `fv` is
 * exposed here the same plain way most OTHER classes already are
 * (`pico8_fv_class()`/`pico8_fv_new()`, no `pico8_class_register_named`
 * call) -- `pico8_class_declare()` still registers it, as it does every
 * class, into the shared ""-keyed `pico8_class_lookup()` registry (its
 * spec never sets `fz` either), exactly like every other unnamed `:jr()`
 * class in this port.
 *
 * `fv.hd=gk`: the EXACT SAME "field reused from Chunk 1's standalone `gk`
 * (`pico8_gk`, pico8_entity_geom.h), bridged through a small per-class
 * dispatch wrapper" pattern already established TWICE -- `pico8_jo_hd_
 * dispatch` (`jo.hd=gk`, pico8_planet.c) and `pico8_js_hd_dispatch`
 * (`js.hd=gk`, pico8_ui_button.c). `pico8_fv_hd_dispatch` (pico8_help.c)
 * replicates `pico8_jo_hd_dispatch`'s exact shape verbatim: read `argc`/
 * `arg0`/`arg1` off the dispatch call, forward `arg0`/`arg1` straight
 * through to `pico8_gk(self, arg0, arg1)` (matching `gk(e,mp,rp)`'s own
 * 2-argument real signature exactly -- `js.hd=gk`'s own OWN two-argument
 * threading, not its historically-incomplete single-`arg` predecessor,
 * per pico8_ui_button.c's own comment on that fix).
 *
 * `hd` IS one of `bm`'s 5 real dispatch names (pico8_entity.c's own `bm`
 * literal: "kj","ev","hd","hk","hb") -- reached automatically by a future
 * `dc("hd")` IN ADDITION to plain `mv()` lookup, in principle. CHECKED
 * DIRECTLY, NOT ASSUMED: grepping every real `dc(...)` call site in
 * `pico8/lua.lua` finds exactly one (`_draw()`, line 1188: `dc("kj")
 * dc("ev")`) -- `dc("hd")` is never actually called ANYWHERE in the real
 * source. So `hd` is, in this cart, reached ONLY through `mv()`'s own
 * unrestricted by-name field lookup (the real call site is `ui:gw()`'s own
 * `mv(e,"hd",self.qa,je.ra)`, argc=2, Chunk 5), even though the `bm`-name
 * membership would make a future `dc("hd")` reach it too if one ever
 * existed -- none does. Registering it as a real `PICO8_VALUE_FUNCTION`
 * field named "hd" on `fv`'s own class defaults is still exactly right:
 * that registration is what makes the REAL, evidenced `mv()` call site
 * work; `dc("hd")`'s absence from the source is simply a fact about this
 * cart worth recording, not a reason to skip the registration.
 *
 * `fv:pq()`: a NEW constructor-hook body (not a reuse -- `g_.pq=f_` is a
 * DIFFERENT case, a REUSE of Chunk 9's `pico8_f_`; this is `fv`'s own,
 * genuinely new body, the same category as `mz:pq`, Chunk 16, or `g_:mm`'s
 * "new body" treatment, Chunk 10). Registered via `pico8_class_set_pq
 * (g_fv_class, pico8_fv_pq)`, matching `pico8_class_pq_fn`'s own
 * `void(*)(pico8_instance_t*)` signature exactly -- no adapter needed, the
 * same "already the right shape" treatment `pico8_mz_pq`/`pico8_hp_pq`
 * already establish. Two things happen in it, in order:
 *
 *   1. `for i,d in pairs(bo)do d.my=eu end` -- walks every positional
 *      entry of `bo` (this chunk's OWN responsibility to build, see below)
 *      and sets each entry's own "my" field to the value of the GLOBAL
 *      FUNCTION `eu` (Chunk 22's real entry point, `eu(nj)`, not yet
 *      built). See "THE `eu` FORWARD-REFERENCE PLACEHOLDER" section below
 *      for the full trace of how this genuinely new kind of forward
 *      reference is represented.
 *
 *   2. `pz(self,bo)` -- Chunk 8's `pico8_pz(const pico8_instance_t *p,
 *      const pico8_list_t *bs)` (pico8_msgbox.h), which populates the `ui`
 *      singleton's own click-target list (`ok.le`) by wrapping each `bs`
 *      element into a real `js` button instance (`js(b)`, per `pico8_pz`'s
 *      own documented body) positioned starting at `p.hi or p.ra` (`fv`'s
 *      own `hi=v(34,56)` field, since it IS set here, takes priority over
 *      `ra=v(64,28)` -- both real fields on `fv`'s own spec, `pz`'s own
 *      `p.hi or p.ra` reads whichever is genuinely set). `pz` takes a
 *      `const pico8_list_t*`, but `bo` is a `pico8_ob_table_t*` (an
 *      ob()-parsed literal with 4 positional entries) -- this exact
 *      "convert an ob_table's positional entries to a pico8_list_t for a
 *      function expecting a list" bridge is an ALREADY ESTABLISHED,
 *      DOCUMENTED, DUPLICATED pattern in this port: `pico8_tech_gate_
 *      positional_values` (pico8_tech_gate.c), `pico8_layout_positional_
 *      values` (pico8_layout.c), `pico8_msgbox_positional_values`
 *      (pico8_msgbox.c), `pico8_research_positional_values`/its own
 *      `pico8_research_display.c` sibling copy, `pico8_data_init_
 *      positional_values` (pico8_data_init.c), the two `positional_values`
 *      copies in `pico8_planet_trade.c`/`pico8_planet_family_trade.c`, and
 *      `pico8_jo_positional_values` (pico8_planet_display.c) -- NINE
 *      existing private per-module copies once re-counted directly by grep
 *      (a first pass under-counted this at six/"7th", corrected here after
 *      an independent recount -- more than the three this chunk's own
 *      brief named either way). `pico8_help_positional_values`
 *      (pico8_help.c) is this module's own 10th copy, following the exact
 *      same shape -- documented duplication, not a new pattern. It is used
 *      for BOTH
 *      `bo` here AND `self.rb` in `fv:kj` below (see that method's own
 *      note for why ONE helper trivially serves both without a second
 *      copy needed inside this same module).
 *
 * `fv:kj(p)`: `ei(self)` (Chunk 9's `pico8_ei`, pico8_particle.h, drawing
 * dispatch already reused elsewhere, e.g. `g_.kj=ei`/`hj.pq=f_`'s sibling
 * `mm=g_.mm` reuse) called here as a PLAIN DIRECT FUNCTION CALL (not a
 * field-reuse assignment this time -- `fv`'s own NEW `kj` body calls
 * `pico8_ei` as one step, then does its own extra work), followed by
 * `bl(self.rb)` (Chunk 2's `pico8_bl`/`pico8_bl_default`, pico8_pi.h).
 * `bl(rb,nc)`'s real signature takes an extra `nc` (camera-shake) argument,
 * but `bl(self.rb)` supplies only ONE Lua argument -- checked directly
 * against this exact subsystem's own already-established two-entry-point
 * convention for an omitted trailing argument (pico8_pi.h's own comment on
 * `pico8_bl`/`pico8_bl_default`, and `pico8_msgbox.c`'s own real `bl(self,
 * nc)` vs. `mh:ev`'s always-supplied `nc` for contrast): `pico8_bl_default
 * (gfx,mem,rb)` is exactly the established "nc omitted" entry point (the
 * SAME one Chunk 8's own `bl(la(pj.i_,pj.ou))` call already uses), so
 * `fv:kj` calls THAT, not `pico8_bl` with a fabricated `nc` value.
 *
 * `self.rb` is `fv`'s own class-spec `rb=o(...)` field (4 positional
 * sub-records, an existing REF-holding `pico8_ob_table_t*` once parsed) --
 * the SAME "convert ob_table positional entries to a pico8_list_t" bridge
 * as `bo` above. This module reuses the exact SAME `pico8_help_positional_
 * values` helper for both `bo` (in `pq`) and `self.rb` (in `kj`) rather
 * than writing a second copy: the helper's own signature (`const pico8_ob_
 * table_t* -> pico8_list_t`) is already fully generic over WHICH table it
 * walks, so a second, textually-identical copy inside this same module
 * would be duplication with zero benefit, unlike the existing 6 copies
 * above, which are genuinely separate per-MODULE duplicates (this port's
 * own accepted "duplicated across modules, not within one" convention --
 * see e.g. `pico8_research.c`'s own comment on why its sibling `pico8_
 * research_display.c` keeps a SEPARATE copy rather than sharing this one).
 *
 * `kj` IS one of `bm`'s 5 real dispatch names (`_draw()`'s own `dc("kj")`,
 * confirmed above as a REAL, exercised call site, unlike `dc("hd")`) --
 * registered as a real `PICO8_VALUE_FUNCTION` field on `fv`'s own class
 * defaults, with a `pico8_fv_kj_dispatch` bridge matching the standard
 * `(gfx,mem,self,p)`-reading-context shape already used by e.g. `pico8_jo_
 * kj_dispatch`/`pico8_mz_kj_dispatch`. The real `kj(p)` parameter `p` is
 * declared but NEVER referenced anywhere in `fv:kj`'s own body (`ei(self)
 * bl(self.rb)end` reads neither `p.x` nor `p.y`) -- confirmed by reading
 * the literal source above, not assumed; `pico8_fv_kj` still accepts `p`
 * (matching the standard signature shape every other real `kj` dispatch
 * target uses) but leaves it genuinely unused, exactly like `pico8_js_
 * ldown_dispatch`'s own documented unused-argument precedent.
 *
 * *** THE `eu` FORWARD-REFERENCE PLACEHOLDER -- THIS CHUNK'S OWN GENUINELY
 * NEW DESIGN DECISION, mirroring pico8_wormhole.h's own "SELF.ET
 * REPRESENTATION" section and pico8_probe.h's own `fm` trace in spirit ***
 *
 * `d.my=eu` is a NEW KIND of forward reference this port hasn't hit
 * before. Every prior forward reference (`bg:ldown`->`mz`/`g_`, Chunk 7;
 * `bv`/`dl`/`d_`->`fm`, Chunk 10; `jo:hu`->`kv[hp.cw]`, Chunk 13) forward-
 * references a CLASS (resolved via `pico8_class_lookup_named()`) or a
 * SINGLETON INSTANCE (resolved via a get/set instance-placeholder accessor
 * pair, `pico8_fm()`/`pico8_probe_set_fm()`). This one forward-references
 * a bare GLOBAL FUNCTION VALUE with no instance/class behind it at all --
 * `eu` is just a top-level Lua function (`function eu(nj)dk=nj jl()bg()
 * bw(90)...end`, pico8/lua.lua:1181-1186, Chunk 22), about to become a
 * callback stored as a plain table field on each `bo` entry, later invoked
 * by `mv(entry,"my")` once some future button-dispatch mechanism reaches
 * it (confirmed against the real `mv(e,rm,...)` semantics, pico8_entity.h:
 * `mv(self,"my")` calls the "my" field function WITH `self` ITSELF as its
 * first real argument -- exactly how `js:ldown`'s own real `mv(self,"my")`
 * call site, pico8/lua.lua:277, feeds `eu`'s own single declared parameter
 * `nj`; the exact SAME shape the PRD's own Chunk 21 description calls out
 * for `ix[2].my=cr`, a sibling button-callback data table this chunk's own
 * grep of `pico8/lua.lua` confirms is a structurally IDENTICAL but
 * entirely SEPARATE data table -- `bo` itself is never read by anything in
 * Chunk 21's own real source; grepping every real `bo` reference in
 * `pico8/lua.lua` finds only `fv:pq`'s own two uses and `bo`'s own literal
 * declaration, three total, confirming `bo` really is used ONLY by
 * `fv:pq()` in this cart, not shared with `ix`/`ha`).
 *
 * RESOLUTION (evidenced, adapted from this port's own established
 * idiom, not invented): the established idiom for "a not-yet-real
 * singleton, safely readable/dispatchable today, reconciled later by
 * name" is the get/set accessor-pair convention (`pico8_mo()`/`pico8_je()`/
 * `pico8_ok()`/`pico8_fm()`). This chunk adapts it here for a bare
 * FUNCTION POINTER placeholder instead of an instance pointer:
 *
 *   static pico8_value_function_t g_eu_dispatch = pico8_fv_eu_placeholder;
 *   pico8_value_t pico8_eu_field_value(void);
 *   void pico8_help_set_eu(pico8_value_function_t fn);
 *
 * `pico8_fv_eu_placeholder` (pico8_help.c, static) is a documented no-op
 * matching `pico8_value_function_t`'s own exact `(void*,int,pico8_value_t,
 * pico8_value_t)->pico8_value_t` shape -- reads nothing, returns
 * `pico8_value_ref(NULL)` (this port's own established "nothing to act on
 * yet" filler, matching every other not-yet-exercisable forward reference
 * in this PRD, e.g. Phase 2's `mo:md()`/`"kj"` dead branch). `pico8_eu_
 * field_value()` wraps WHICHEVER function `g_eu_dispatch` currently holds
 * (the placeholder, or the real one once reconciled) in a fresh
 * `PICO8_VALUE_FUNCTION`-kind `pico8_value_t`, ready to be stored via
 * `pico8_ob_table_set_name(d, "my", pico8_eu_field_value())` for every
 * positional entry `d` of `bo` -- storing the PLACEHOLDER'S OWN VALUE (a
 * function pointer wrapped as a `pico8_value_t`), never `bo` itself, per
 * this chunk's own instruction. `pico8_help_set_eu()` is the setter Chunk
 * 22 will call later (see the RECONCILIATION NOTE immediately below); it
 * is also called (with `NULL`, resetting to the placeholder) by `pico8_
 * help_shutdown()`/`pico8_help_init()`'s own re-init, mirroring `pico8_
 * probe_init()`'s own "rebuild `fm`'s placeholder fresh on every re-init,
 * even across a prior `pico8_probe_set_fm()` call" convention exactly (see
 * `pico8_probe.c`'s `pico8_probe_init()`), so one test's real-dispatch
 * registration can never leak into another's.
 *
 * WHY NOT THE INSTANCE-PLACEHOLDER SHAPE (`pico8_fm()`'s own classless-
 * instance pattern) INSTEAD: `eu` is never called through `:`-method-call
 * sugar on a persistent global TABLE the way `fm:lu(...)`/`fm:nx(...)` are
 * -- it is a bare function VALUE assigned onto `bo` entries' own "my"
 * field, later invoked via `mv(entry,"my")`'s existing, already-built
 * generic field-function-call mechanism (`pico8_mv`, Chunk 5) exactly like
 * any other real `PICO8_VALUE_FUNCTION`-kind field. Wrapping it in a
 * classless `pico8_instance_t` the way `pico8_fm()`/`bv`'s own `{}`
 * throwaway do would manufacture a fake "self" table `eu` never has in the
 * real source and give `mv()` nothing useful to call through (`mv()`
 * dispatches BY NAME on an INSTANCE's OWN fields, not by treating an
 * instance as if it, itself, were directly callable) -- a genuine
 * representation mismatch, not merely a stylistic choice, so the narrower
 * "get/set a bare function pointer, matching `pico8_value_function_t`'s
 * own already-existing shape exactly" adaptation is the evidenced correct
 * one here, not the instance-placeholder idiom used everywhere else.
 *
 * *** CHUNK 22 RECONCILIATION NOTE, mirroring pico8_probe.h's own `fm`
 * reconciliation note verbatim in spirit *** -- when Chunk 22 ports the
 * real `eu(nj)` entry point (pico8/lua.lua:1181-1186) into a real
 * `pico8_value_function_t`-shaped bridge (reading `nj`=`self` off its own
 * `void *self` parameter, exactly as `js:ldown`'s own real `mv(self,"my")`
 * call site requires), it must call `pico8_help_set_eu(pico8_eu_dispatch)`
 * (or equivalent, using whatever name Chunk 22 gives its own bridge
 * function) to REPLACE this module's own placeholder -- `pico8_eu_field_
 * value()`'s own declaration/signature here never changes, so every
 * existing real reader (every `bo` entry's own "my" field, set once at
 * `fv:pq()` time) automatically starts resolving to the real dispatch the
 * NEXT time a NEW `fv` instance is constructed after Chunk 22's own
 * `pico8_help_set_eu()` call runs (existing already-constructed `bo`
 * entries from a PRIOR `fv:pq()` run keep whichever `pico8_value_t` they
 * were given at THAT time, exactly like Lua's own "eu" upvalue capture
 * would freeze it too, since `bo`'s own entries are mutated once, not
 * re-read live) -- the same "reconciliation only changes what NEW
 * consumers see" caveat `pico8_entity_set_mo()`'s own reconciliation of
 * `pico8_mo()` already establishes. Chunk 22 must NOT declare a second,
 * competing `pico8_eu_field_value()`/`pico8_help_set_eu()` pair anywhere
 * else -- this is the one and only home for them.
 *
 * `bo`'s OWN CONSTRUCTION -- pico8/lua.lua:1189-1198:
 *
 * bo=ob([[
 * o(lq=o(49,"forgiving   "),px="forgiving",
 * trade=7,en=-0.99),
 * o(lq=o(50,"reasonable  "),px="reasonable",
 * trade=6,en=-0.6),
 * o(lq=o(51,"challenging "),px="challenging",
 * trade=5,en=-0.4),
 * o(lq=o(52,"tough       "),px="tough",
 * trade=4,en=-0.2),
 * ]])
 *
 * Built via `pico8_ob(...)` (Phase 2's parser, pico8_ob.h) parsing this
 * exact literal string, the same pattern `pico8_game_data.c`'s own `es`/
 * `ct`/`b_`/`co`/`fb` accessors already establish (`pico8_ob(spec, NULL)`,
 * cached in a module-level static, exposed as `const pico8_ob_table_t*`).
 * Each of the 4 positional entries is itself a nested `o(...)` record with
 * `lq` (a scroll-position/label pair, `o(icon_sprite,"label text")`), `px`
 * (the difficulty's own short name string -- read by, per this PRD's own
 * Chunk 21 paragraph, a future `cb()`/tech-tree-screen consumer, NOT by
 * anything in this chunk's own scope), `trade` and `en` (economy-tuning
 * numbers Chunk 12's `hy`/economy formulas read once a real `dk` selection
 * exists, Chunk 22). `bo` is placed in THIS module (not a new `pico8_
 * difficulty.c`/alongside `pico8_game_data.c`) because it is used ONLY by
 * `fv:pq()` in this chunk's own scope (confirmed by the grep noted above)
 * -- `pico8_help_init()`/`pico8_help_shutdown()` build/free it alongside
 * `fv`'s own class registration in one shared init/shutdown pair, per this
 * chunk's own "fold into this chunk's own module's existing init/shutdown"
 * option, since a separate `bo`-only init/shutdown pair would exist purely
 * to be called at the exact same time as this module's own anyway. */

void pico8_help_init(void);
void pico8_help_shutdown(void);

/* The real `fv` class, built once by pico8_help_init(). Registers itself
 * into the shared ""-keyed pico8_class_lookup() registry only (its spec
 * never sets `fz`, matching every other unnamed `:jr()` class) -- see this
 * file's own top comment for why no `pico8_class_register_named()` call is
 * made for "fv" (no real call site ever looks it up by name). Returns NULL
 * before pico8_help_init() (or after pico8_help_shutdown()). */
pico8_class_t *pico8_fv_class(void);

/* `fv(overrides)` -- the real Lua constructor call (`_init()`'s own
 * `fv()`, no overrides, pico8/lua.lua:1180, Chunk 22). Thin wrapper around
 * pico8_class_instantiate(). */
pico8_instance_t *pico8_fv_new(const pico8_ob_table_t *overrides);

/* `bo=ob([[...]])` (pico8/lua.lua:1189-1198) -- see this file's own top
 * comment for the full trace. Built once by pico8_help_init(), freed by
 * pico8_help_shutdown(). Returns NULL before pico8_help_init() (or after
 * pico8_help_shutdown()). */
const pico8_ob_table_t *pico8_bo(void);

/* `eu` forward-reference placeholder accessor pair -- see this file's own
 * top "THE eu FORWARD-REFERENCE PLACEHOLDER" section for the full trace
 * and the Chunk 22 reconciliation note. `pico8_eu_field_value()` always
 * returns a valid PICO8_VALUE_FUNCTION-kind value (the placeholder before
 * Chunk 22 reconciles it, the real dispatch after); `pico8_help_set_eu
 * (NULL)` resets to the placeholder (used by pico8_help_shutdown()/
 * pico8_help_init()'s own re-init for test isolation). */
pico8_value_t pico8_eu_field_value(void);
void pico8_help_set_eu(pico8_value_function_t fn);

/* function fv:pq()for i,d in pairs(bo)do
 * d.my=eu
 * end
 * pz(self,bo)end
 * (pico8/lua.lua:1166-1169.) See this file's own top comment for the full
 * trace. Wired via `pico8_class_set_pq(g_fv_class, pico8_fv_pq)` -- matches
 * `pico8_class_pq_fn`'s own signature exactly, no adapter needed. A NULL
 * `self` is a safe no-op (still walks/mutates `bo`'s own entries, which is
 * harmless and idempotent, but skips the `pz()` call, which needs a real
 * instance). */
void pico8_fv_pq(pico8_instance_t *self);

/* function fv:kj(p)ei(self)bl(self.rb)end
 * (pico8/lua.lua:1170.) See this file's own top comment for the full
 * trace, including why `p` is accepted but never read, and why `bl`'s
 * omitted `nc` argument routes through `pico8_bl_default`. A NULL `self`
 * is a safe no-op (via `pico8_ei`'s own NULL guard; `self.rb` reads as
 * absent, so `pico8_bl_default` is called with an empty list). A NULL
 * `gfx`/`mem` is likewise a safe no-op for the drawing performed by both
 * `pico8_ei` and `pico8_bl_default`. */
void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p);

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape to
 * pico8_fv_kj's own signature, reading gfx/mem from the shared per-frame
 * dispatch context (pico8_dispatch_context.h) -- the same split as
 * `pico8_jo_kj`/`pico8_jo_kj_dispatch` (pico8_planet.c) and `pico8_mz_kj`/
 * `pico8_mz_kj_dispatch` (pico8_wormhole.c). `kj` IS one of `bm`'s 5 real
 * dispatch names, and `_draw()`'s own real `dc("kj")` call site (pico8/
 * lua.lua:1188) confirms `p`=`self.ra` (unpacked as a REF-to-vec, zero-
 * vector fallback if absent/malformed, matching every other `kj` dispatch
 * bridge's identical treatment) -- even though `pico8_fv_kj`'s own body
 * never reads it. */
pico8_value_t pico8_fv_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

#endif
