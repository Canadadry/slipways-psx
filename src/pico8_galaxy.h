#ifndef PICO8_GALAXY_H
#define PICO8_GALAXY_H

#include "pico8_fixed.h"

/* Phase 3, Chunk 20 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `bw(n)` (pico8/lua.lua:744-751) -- galaxy population, spawning
 * `n` starting `jo` (base planet, Chunk 13) instances at rejection-sampled
 * random positions, then building the two CLASS-LEVEL `jo.oc`/`jo.ir`
 * generator-closure fields (`jo:hh()`'s own hazard picker, Chunk 13, has
 * been waiting on these since it was written -- see pico8_planet.h's own
 * top comment on `pico8_jo_set_oc`/`pico8_jo_set_ir` for the accessor
 * contract this chunk finally exercises for real).
 *
 * function bw(n)for id=1,n do
 * local p
 * repeat
 * p=v(rnd(2.75)+0.125,rnd(2.75)+0.125
 * )until ij(p,cg.jo)>0.05
 * jo({id="p"..id,p=p,ra=p*128})end
 * jo.oc,jo.ir=
 * gn(df,2,jf(0.5,dk.en)),jf(0.8,0.5)end
 *
 * *** REJECTION LOOP AGAINST A LIST THAT STARTS EMPTY, BUT DOESN'T STAY
 * EMPTY *** -- `cg.jo` (`pico8_cg("jo")`, pico8_entity.h) holds zero
 * entities before this function's very first iteration ever runs (no `jo`
 * is ever placed anywhere else in the whole cart), so `pico8_ij`'s own
 * documented empty/NULL-list behavior (returns the sentinel
 * `pico8_fixed_from_int(32767)`, see pico8_entity_geom.h's own comment on
 * this declaration) makes the very first candidate position always
 * accepted (32767 > 0.05) -- matching real Lua's own `local mk,nd=32767`
 * initial value with zero loop iterations. But `cg.jo` does NOT stay empty
 * for the REST of this function's own loop: `jo`'s own class spec sets
 * `ke.pq=lt` as its (inherited) "extra constructor" (pico8_ke.h's own top
 * comment on this exact mechanism), so every `jo(...)` construction call
 * below -- via `pico8_jo_new`'s own `pico8_class_instantiate`, which walks
 * the pq chain synchronously before returning -- ALREADY self-registers
 * the freshly built instance into `hv`/`cv`/`cg` (`pico8_lt`,
 * pico8_entity.h), `cg["jo"]` included (`jo`'s own `nu=o("jo","np",
 * "prober")` tag list). So `cg.jo` genuinely grows by one on every
 * successful iteration, and later candidates really are rejection-sampled
 * against every planet placed so far in THIS SAME `bw()` call -- not a
 * no-op loop, confirmed by reading `pico8_ke.c`'s own `pico8_ke_pq_lt`
 * bridge, not assumed from the function name alone.
 *
 * *** `jo({id="p"..id,p=p,ra=p*128})`: CONSTRUCTED FOR SIDE EFFECT,
 * RETURN VALUE DISCARDED, NEVER FREED *** -- real Lua's own bare statement
 * (no `local x=jo(...)`) is ported as `(void)pico8_jo_new(overrides)`,
 * matching the same "construct for the side effect (here: self-
 * registration via `ke.pq=lt`, above), discard the returned pointer,
 * intentionally leak it" precedent already established by `pj:ph`'s own
 * two discarded `js(...)` calls (`src/pico8_score_panel_display.c`,
 * `(void)made1;`/`(void)made2;`) -- every placed entity in this port lives
 * for the rest of the program, matching `pico8_instance_free`'s own
 * documented "never called automatically, the Lua original has no manual
 * teardown under GC either" convention (pico8_class.h). `p` (the accepted
 * candidate vector) and `p*128` (a FRESH, separate heap vector, via
 * `pico8_vec_mul`, pico8_vec.h) are each heap-allocated once accepted and
 * handed to the overrides table as `PICO8_VALUE_REF`s -- `pico8_class_
 * instantiate` shallow-copies each overrides entry into the new instance's
 * own fields (so the temporary overrides table itself is freed right after,
 * same as `pico8_ie_ldown`'s own `id`-string precedent), but the two heap
 * vectors themselves are retained, unfreed, by the new instance's own `p`/
 * `ra` fields -- exactly mirroring real Lua's own "a field holds the same
 * live table object" semantics for `p`, and "a freshly computed value
 * becomes a brand new table object" semantics for `p*128`. `"p"..id`: `id`
 * (the Lua `for` loop counter, itself a plain number, ported here as
 * `pico8_fixed_t` matching every other Lua scalar in this port, e.g. gn's
 * own `og` parameter, pico8_gen.h) stringified via `pico8_value_tostr`
 * then concatenated onto the literal `"p"` via this file's own private
 * two-piece concat helper -- the same per-module-duplicate shape already
 * established by `pj_concat2` (pico8_score_panel.c/pico8_score_panel_
 * display.c) and `ie_concat2` (pico8_research_icon.c, for `ie:ldown`'s own
 * identically-shaped `"s"..ie.id`).
 *
 * *** `gn(df,2,jf(0.5,dk.en))`: THE ONE AND ONLY REAL `gn(` CALL SITE IN
 * THE WHOLE CART *** (confirmed by grepping `pico8/lua.lua` directly: line
 * 732 is `gn`'s own definition, line 751 -- this one -- is its only call)
 * -- see this file's own .c for the `hr`-shape bridge this fact justifies
 * fixing in `pico8_gn` itself (src/pico8_gen.c/.h, a Chunk 1 fix, logged in
 * docs/plan.md's own "Notes de portage"). `df` (Chunk 11's `pico8_df()`,
 * pico8_data_init.h) is bridged from its own positional entries to a
 * `pico8_list_t` via this file's own private `pico8_bw_positional_values`
 * -- the SAME established per-module-duplicate pattern already independently
 * repeated well over a dozen times in this port (e.g. `pico8_jo_
 * positional_values`, pico8_planet_display.c; `pico8_data_init_
 * positional_values`, pico8_data_init.c) -- this is this chunk's own Nth
 * copy, documented as such rather than shared, matching every prior copy's
 * own stated convention. `og=2` becomes `pico8_fixed_from_int(2)`. `jf(0.5,
 * dk.en)` is itself ANOTHER generator closure (`pico8_jf`, pico8_gen.h),
 * NOT a direct `rnd`-style call -- see this file's own .c for the tiny
 * adapter this requires and this header's own note on `dk.en`'s NULL
 * fallback, just below.
 *
 * *** `dk.en`: DEFENSIVE NULL-GUARDED READ, FALLBACK 0 *** -- `dk`
 * (`pico8_dk()`, pico8_economy_score.h) stays NULL until Chunk 22's real
 * `eu(nj)dk=nj` assignment runs, which the real source's own statement
 * order places BEFORE `bw(90)` in the same function (`function eu(nj)
 * dk=nj jl()bg()bw(90)...`) -- so in the fully wired game `dk` is never
 * actually NULL by the time this runs for real, but this chunk's own
 * standalone testability still needs a safe default today. Guarded the
 * same way `pico8_db`'s own `dk.trade` read already is (pico8_economy_
 * score.c): `pico8_dk() != NULL` before reading, falling back to
 * `pico8_fixed_from_int(0)` on a NULL `dk` or a non-fixed/absent `en`
 * field -- the SAME evidenced fallback value `pico8_db` already
 * establishes for a `dk`-derived field, not a differently-guessed default
 * (e.g. one of `bo`'s own real `en` values, -0.99..-0.2, which would be an
 * unevidenced guess at WHICH difficulty is "default").
 *
 * *** `jo.oc,jo.ir=...`: CALLS THE ALREADY-EXISTING SETTERS *** -- see
 * pico8_planet.h's own top comment on `pico8_jo_set_oc`/`pico8_jo_set_ir`
 * (Chunk 13) for their full ownership contract; this file's own .c
 * resolves the one open question that contract left to whoever finally
 * built the real `gn(df,...)` call (this chunk): where the fresh
 * `pico8_jf_state_t*` serving as `gn`'s own inner random source (`r_ctx`)
 * lives and is freed, since `pico8_gn_free` does not touch `r_ctx` itself
 * (confirmed by reading its current body) and `pico8_jo_set_oc` only takes
 * ownership of the `pico8_gn_state_t*` itself. Resolution: a paired,
 * module-private static global in THIS file (`g_bw_oc_r_ctx`), rebuilt and
 * re-freed in lockstep with `jo.oc` every time `pico8_bw()` runs (real
 * gameplay calls it exactly once per game, at `eu(nj)`'s own `bw(90)`, but
 * a repeated call -- e.g. test isolation, or a future "new game" path --
 * must not leak the previous one) -- documented in full in this file's own
 * `pico8_bw()` comment, not extending `pico8_jo_set_oc`'s own contract
 * (a bigger, shared-module change this chunk found no evidence it needs).
 *
 * New module (`pico8_galaxy.c`/`.h`, not `pico8_planet.c`/`.h`, which
 * already owns `jo` itself and stays a class-definition module, not a
 * "spawn planets" one): this chunk's own single function plus one small
 * data-shape fix to already-shipped Chunk 1 code, per this task's own
 * "Module placement" instruction. */
void pico8_bw(pico8_fixed_t n);

/* Not part of the Lua original (no manual teardown under GC): releases
 * this module's own `g_bw_oc_r_ctx` (see this file's own top comment on
 * `pico8_bw()` for why this module -- not `pico8_jo_set_oc`'s own module --
 * owns it). Does NOT touch `jo.oc`/`jo.ir` themselves (`pico8_jo_
 * shutdown()`'s own job, pico8_planet.h) -- this is this module's own
 * narrower half of that teardown, matching every other module's own
 * "only free what THIS module allocated" convention. Safe to call more
 * than once, or before the first `pico8_bw()` call. */
void pico8_galaxy_shutdown(void);

#endif
