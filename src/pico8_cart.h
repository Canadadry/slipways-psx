#ifndef PICO8_CART_H
#define PICO8_CART_H

#include "pico8_class.h"
#include "pico8_value.h"

/* Phase 3, Chunk 22 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md), THIS PHASE'S OWN EXIT GATE: `_init()`, `eu(nj)`, `_update60()`,
 * `_draw()` -- the actual root callers of the whole `_init`/`_update60`/
 * `_draw` reachability graph Phase 3's own exit criterion (docs/plan.md)
 * is stated in terms of -- plus `pico8_cart_load()`, a brand-new, C-only
 * function with no Lua-source equivalent (this port's own established
 * naming exception for genuinely new plumbing, e.g. Chunk 11's
 * `pico8_data_init()`).
 *
 * *** THE CART-BOOTSTRAP SEQUENCING PROBLEM, AND WHY THIS FILE EXISTS ***
 * Real Lua's own top-level statements (every `X=Y:jr(...)`/`X=ob(...)`
 * line across the WHOLE cart) execute exactly ONCE, unconditionally, when
 * the cartridge loads -- BEFORE `_init()` is ever called. This port's C
 * translation of that is the collection of per-module `_init()` functions
 * every prior chunk of this phase has built (one per module), but nothing
 * in this port has EVER called every single one of them, in the right
 * order, exactly once, until now -- `docs/plan.md`'s own "Notes de
 * portage" entry on `pico8_probe_init()` (Phase 3, chunk 11, "Angle mort
 * d'intégration séparé") explicitly flags this as still open and assigns
 * closing it to whichever chunk finally wires `_init()`/`eu()` for real --
 * this chunk. `pico8_cart_load()` is that function: it calls EVERY
 * per-module `_init()` this whole phase has built, exactly once, in a
 * dependency-respecting order, so that by the time `pico8__init()` is ever
 * called for real, every class this cart's own top-level statements would
 * have declared by then is already real -- exactly matching real Lua's own
 * "top-level code runs before `_init()`" ordering guarantee.
 *
 * WHERE THIS LIVES: a new, small module (this file + pico8_cart.c),
 * alongside the 4 real entry points below -- this whole chunk's own new
 * module, per this chunk's own brief's explicit "a reasonable, clean
 * choice" option. Not folded into pico8_bootstrap.c (Chunk 6): that
 * module's own job is `jl()`/`dg()`, a DIFFERENT, narrower concern (the
 * per-`_init()`-call singleton/collection reset, not the one-time,
 * whole-cart class bootstrap) -- see pico8_bootstrap.h's own "CHUNK 22
 * FIX" note for the real, load-bearing distinction this chunk draws
 * between "declared once, at cart-load time" (this file's own job) and
 * "rebuilt every `_init()`/`eu()` call" (`jl()`'s own job, unchanged in
 * kind, only in which 4 classes it's allowed to still (re)declare).
 *
 * A REAL, PREVIOUSLY-UNFLAGGED BUG FOUND AND FIXED WHILE BUILDING THIS
 * FUNCTION: see pico8_bootstrap.h's own "CHUNK 22 FIX" comment on
 * `pico8_jl()` for the full trace of a genuine heap-use-after-free this
 * chunk's own whole-game verification program found under ASan (every
 * `ke`-descended class built ONCE by this file, e.g. `jo`/`bg`, held a
 * `.parent` pointer that dangled the moment `pico8_jl()`'s own PRE-Chunk-22
 * unconditional `pico8_ke_init()` call later freed and rebuilt `ke` out
 * from under it) -- fixed there, not here, since the fix lives in
 * `pico8_jl()`'s own body.
 *
 * ORDERING, verified by actually running the whole sequence under ASan/
 * UBSan (see this chunk's own disposable verification program, not
 * committed) rather than guessed: see pico8_cart.c's own top comment on
 * pico8_cart_load() for the full, function-by-function ordering trace and
 * the reasoning behind each position. */

/* Builds every class/data-table this cart's own real top-level statements
 * build exactly once, before `_init()`/`eu()` are ever called for real --
 * see this file's own top comment and pico8_cart.c's own top comment on
 * this function for the full ordering trace. Reads `mem` via
 * pico8_dispatch_mem() (pico8_dispatch_context.h) for `pico8_data_init()`'s
 * own real need for live cart memory (`unstash()`'s own `peek()` reads) --
 * the caller must call pico8_dispatch_set_mem() with a `pico8_mem_t*` that
 * has already had pico8_load_map_json() run against it (matching
 * tests/test_pico8_data_init.c's own established setup) BEFORE calling
 * this function, or the real cart data (`gb`/`dr`/`hc`/`fd`/`nb`/`r`/`iz`)
 * will not decode correctly -- a NULL dispatch mem is a documented,
 * defensive no-op for JUST the pico8_data_init() step (every other class
 * declared here has no such dependency), matching this port's established
 * "compiles safely, not meaningfully exercisable" treatment for a missing
 * prerequisite. Safe to call more than once (each per-module `_init()` it
 * calls is independently idempotent, per that module's own established
 * "safe to call more than once" convention) -- though real Lua's own
 * top-level statements never re-run, so a real caller only ever needs to
 * call this ONCE, before the very first `pico8__init()` call. */
void pico8_cart_load(void);

/* Not part of the Lua original (no manual teardown under GC) -- tears down
 * every real per-module class/data-table pico8_cart_load() builds, in
 * reverse order, safe to call more than once or before the first
 * pico8_cart_load(). Test-isolation support, mirroring pico8_bootstrap_
 * shutdown()'s own identical role for pico8_jl(). */
void pico8_cart_shutdown(void);

/* function _init()cu()jl()ie.gy,kc=
 * ie.eh
 * fv()music(0)end
 * (pico8/lua.lua:1178-1180.) See pico8_cart.c's own comment on this
 * function's definition for the full line-by-line trace (`ie.gy,kc=ie.eh`'s
 * own real Lua multiple-assignment-with-fewer-values-than-names padding
 * rule included). Requires pico8_cart_load() to have already run (`ie`'s
 * own class, `fv`'s own class/`bo` table, all need to be real already) --
 * a defensive no-op is NOT attempted for a missing prerequisite here, since
 * every real call site of `_init()` in this cart (the initial boot, and
 * the restart button's own `my` callback, Chunk 21's `ha[1].my=_init`)
 * only ever happens after pico8_cart_load() has already run once. */
void pico8__init(void);

/* `_init` stored as a bare function VALUE (`js(ob([[...]],{my=_init}))`,
 * `pj:ph()`, pico8_score_panel_display.c) -- bridges pico8_value_
 * function_t's `(self,argc,arg0,arg1)` shape to pico8__init's own
 * zero-argument real signature. `self` (the button `mv(button,"my")`
 * would pass) is simply discarded -- `_init()` declares NO Lua parameters
 * at all, the same "excess argument silently dropped" treatment already
 * established throughout this port (e.g. pico8_dl_dispatch's own unused
 * second argument) -- per pico8_score_panel.h's own "CHUNK 22
 * RECONCILIATION NOTE" for `_init`, which this function resolves: see
 * pico8_cart.c's own pico8_cart_load() for the pico8_score_panel_set_init()
 * call that installs this bridge as the real dispatch target. */
pico8_value_t pico8_init_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* function eu(nj)dk=nj
 * jl()bg()bw(90)fm,ju=hy(),pj()nv(ix,js)local gx={qh,os,nr,fr,nl,mx,hj
 * }
 * for i,b in pairs(gx)do
 * ie({ke=b,jb=jb(dr[i])})end
 * fm:lj()end
 * (pico8/lua.lua:1181-1186.) See pico8_cart.c's own comment on this
 * function's definition for the full line-by-line trace, including the
 * `dk`/`self` representation resolution (`nj` IS `self`, the clicked `bo`-
 * derived `js` button instance -- `pico8_economy_set_dk` receives that
 * instance's own `->fields` table, not a second, parallel raw-ob_table
 * path -- see that comment for the full reasoning) and the fm/ju
 * reconciliation ordering. `nj` is a real `pico8_instance_t*` (the clicked
 * button), matching `js:ldown`'s own real `mv(self,"my")` call site
 * (pico8/lua.lua:277, zero extra arguments -- `nj`=`self`). Requires
 * pico8_cart_load() to have already run, same reasoning as pico8__init()
 * above. */
void pico8_eu(pico8_instance_t *nj);

/* `eu` stored as a bare function VALUE (`fv:pq()`'s own `d.my=eu`, for
 * every positional entry of `bo`) -- bridges pico8_value_function_t's
 * `(self,argc,arg0,arg1)` shape to pico8_eu's own single-real-Lua-parameter
 * signature, reading `nj`=`self` off its own `void *self` parameter (`argc`/
 * `arg0`/`arg1` are all unused -- `eu(nj)`'s only real parameter, `nj`, IS
 * the dispatch `self`, never a `mv()` extra argument). Per pico8_help.h's
 * own "CHUNK 22 RECONCILIATION NOTE" for `eu`, which this function
 * resolves: see pico8_cart.c's own pico8_cart_load() for the
 * pico8_help_set_eu() call that installs this bridge as the real dispatch
 * target. */
pico8_value_t pico8_eu_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* function _update60()dg()ok:lj()end
 * (pico8/lua.lua:1187.) See pico8_cart.c's own comment on this function's
 * definition for the full trace. Requires pico8_cart_load() (for `ok` to
 * be meaningfully populated) and at least one prior pico8__init()/
 * pico8_eu() call (for `ok`/`dg()`'s own `hv` to hold anything worth
 * dispatching) -- a NULL `pico8_ok()` (pico8_jl() never having run) is a
 * defensive no-op for the `ok:lj()` half only, matching pico8_ui_lj's own
 * documented NULL-`self` tolerance; `dg()` itself already tolerates an
 * empty/not-yet-built `hv` (pico8_bootstrap.h's own pico8_dg() contract). */
void pico8__update60(void);

/* function _draw()cls()dv()dc("kj")dc("ev")end
 * (pico8/lua.lua:1188.) See pico8_cart.c's own comment on this function's
 * definition for the full trace. Reads gfx/mem via pico8_dispatch_gfx()/
 * pico8_dispatch_mem() (pico8_dispatch_context.h) -- a NULL either is a
 * defensive no-op for the drawing calls that need it (pico8_cls/
 * pico8_dv_default), matching every other real drawing entry point in this
 * port; `dc()` itself takes neither and needs no such guard. */
void pico8__draw(void);

#endif
