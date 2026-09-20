#ifndef PICO8_DATA_INIT_H
#define PICO8_DATA_INIT_H

#include "pico8_mem.h"
#include "pico8_ob.h"

/* Phase 3, Chunk 11 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md), "Data-table initialization sequencing" (Implementation
 * Decisions): the non-function, top-level statements pico8/lua.lua runs
 * once at cart-load time to build `df`/`hc`/`fd`/`gb`/`rr`/`dr`/`nb`/`ek`
 * (pico8/lua.lua:1377-1445, read directly) -- `r`/`iz` themselves were
 * already Chunk 1's own job (pico8_stash.c/.h, `pico8_stash_init()`),
 * confirmed by re-reading that module's own header before writing this one
 * (see this file's own top comment on pico8_data_init() below for exactly
 * how this module reuses, rather than duplicates, that existing function).
 * This module owns `df`/`hc`/`fd`/`dr`/`nb`/`ek` and the single explicit
 * "game data init" C function (`pico8_data_init()`) the PRD calls for --
 * this phase's own equivalent of a "module load" step, not folded into a
 * future `_init()`/`jl()`, neither of which the original source ties this
 * code to (it runs unconditionally at chunk-load time). `gb`/`rr` (the
 * `for ri in all(fd) do...add(gb,hp(ri))end` loop) are instead owned by
 * pico8_research.c/.h -- they need the `hp` CLASS to exist first (this
 * chunk's own note: "build hp before writing this init function's body"),
 * and `gb` is written to both by that loop AND from inside `hp:pq()` itself
 * (`ho(gb,hp.mr,hp.gp)(self)`, pico8_research.c) -- keeping `gb` and the
 * class whose instances populate it in the same module avoids a second
 * module needing write access to another module's singleton, mirroring
 * how `hv`/`cv`/`cg` themselves live alongside the entity mechanism that
 * writes them (pico8_entity.c), not alongside every module that happens to
 * construct an entity. pico8_data_init() (below) calls
 * pico8_research_build_gb(fd) as one step of its own sequence, exactly like
 * pico8_bootstrap.c's own pico8_jl() orchestrates sub-inits from other
 * modules rather than inlining their bodies.
 *
 * New module (not pico8_stash.c, Chunk 1's own module for r/iz/unstash --
 * a distinct, already-shipped concern this module calls into rather than
 * extends; not pico8_game_data.c either, Chunk 1's own SEPARATE module for
 * the small literal tables es/ct/b_/co/bn/bq/fb -- a deliberately similar
 * but distinct name, chosen because that module is already a fixed, small,
 * unrelated set of Chunk-1 literals, and reusing it here would conflate two
 * different chunks' own data under one file). */
void pico8_data_init(pico8_mem_t *mem);

/* Not part of the Lua original (no manual teardown under GC) -- releases
 * df/hc/fd/dr/nb/ek's own backing storage AND pico8_research_shutdown()'s
 * own gb/hp-class teardown (this module defensively called
 * pico8_research_init() as part of building gb, so it tears that down too,
 * in reverse order -- matching pico8_bootstrap_shutdown()'s own established
 * "tear down everything this orchestrator built" convention). Does NOT
 * touch r/iz (pico8_stash_shutdown() is Chunk 1's own module's job, not
 * called from here, since this module never owns r/iz -- only reuses
 * pico8_stash_init()). Safe to call more than once, or before the first
 * pico8_data_init(). */
void pico8_data_shutdown(void);

/* df -- see pico8_data_init()'s own comment for the exact build sequence.
 * Owned by this module; callers must not free it. */
const pico8_ob_table_t *pico8_df(void);

/* hc=ob(unstash(8383)) -- real content undecoded by this chunk's own static
 * reading (it's built from live cart memory, not a literal in
 * pico8/lua.lua), consumed by `hp:po()` (pico8_research.c) as `hc[self.ja]`
 * /`hc[qp.mc]`. Owned by this module; callers must not free it. */
const pico8_ob_table_t *pico8_hc(void);

/* fd=ob(unstash(8494)) -- the still-undecoded research-project data this
 * PRD's own Out of Scope section flags (`jo:hu`'s `kv[hp.cw]` dynamic
 * re-classing names a class only knowable once this table is actually
 * inspected at runtime -- Chunk 13's own problem, not this chunk's).
 * Consumed once, by pico8_research_build_gb(pico8_fd()) (this module's own
 * pico8_data_init(), below), to build `gb`. Owned by this module; callers
 * must not free it. */
const pico8_ob_table_t *pico8_fd(void);

/* dr=ob(unstash(9807)) -- decoded but not consumed by any function in this
 * chunk's own scope (no real call site reads the global `dr` anywhere in
 * pico8/lua.lua within this PRD's Chunk 11-22 range read so far); built and
 * exposed here for whichever later chunk's own real call site needs it,
 * the same "expose for a future evidenced consumer" reasoning already
 * established for r/iz (pico8_stash.h). Owned by this module; callers must
 * not free it. */
const pico8_ob_table_t *pico8_dr(void);

/* nb=ob(unstash(10457)), after its own foreach(nb,ho(...)) renaming pass --
 * the tech list pico8_tech_gate.c's own cb()/gi() read and mutate (`t.e_`).
 * Owned by this module; callers must not free it. */
const pico8_ob_table_t *pico8_nb(void);

/* ek=ob([[2,4,7,12,]]) -- the tier thresholds `hy:kt()` (Chunk 12) compares
 * `#self.invented` against. Owned by this module; callers must not free
 * it. */
const pico8_ob_table_t *pico8_ek(void);

#endif
