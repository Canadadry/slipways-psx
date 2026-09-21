#ifndef PICO8_ENTITY_H
#define PICO8_ENTITY_H

#include "pico8_class.h"
#include "pico8_list.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* pico8/lua.lua's entity registration/dispatch mechanism -- hv/cv/cg
 * (line 153: `function jl()hv,cv,cg={},{},{}`), lt/he (lines 156-168),
 * bm (lines 169-173), dc/mv (lines 3-9, 188-199), ci (line 187), and the
 * placeholder mo global -- ported on top of Chunk 9's class system
 * (pico8_class.h) and Chunk 8's ob()-table type (pico8_ob.h). Kept in its
 * own module, separate from pico8_class.[ch], per this chunk's own
 * "Module layout" instruction and the PRD's structural note against
 * repeating pico8_gfx.c's "grew disproportionately large" finding
 * (docs/plan.md's Notes de portage). See pico8_entity.c's own top comment
 * for the exact Lua source each piece below mirrors, and per-function
 * comments there for the line-by-line trace. No per-function unit test
 * for this chunk (verified by source/port comparison plus Chunk 11's
 * smoke test instead, per the PRD's Testing Decisions). */

/* Builds/rebuilds every module-level global this chunk owns (hv, cv, cg,
 * bm, ci, mo) to a fresh, empty state -- mirrors jl()'s own
 * `hv,cv,cg={},{},{}` (pico8/lua.lua:152) plus its `mo=qb()` assignment,
 * narrowed to just the entity/dispatch pieces this chunk is actually
 * responsible for (jl()'s own `je=nf(),ok=ui()` are real Phase 3 game
 * singletons, out of this chunk's scope entirely). Safe to call more than
 * once: frees whatever this module previously built first (via
 * pico8_entity_shutdown()) before rebuilding from scratch -- intended so
 * Chunk 11's smoke test (not built by this chunk) can call it once per
 * test case for isolation, mirroring pico8_class_registry_clear()'s and
 * pico8_ob_clear_constructors()'s own established test-isolation
 * convention. Must be called before any of hv/cv/cg/lt/he/dc/mv/ci/mo
 * below are used. Phase 3 Chunk 6 (docs/prd/triage/phase3-etape1-function-
 * by-function-c-port.md) completes the rest of jl() -- the real `mo=qb()/
 * je=nf()/ok=ui()` construction -- in the new pico8_bootstrap.c/pico8_jl(),
 * which calls this function first, then pico8_entity_set_mo() below to
 * replace the classless placeholder mo built here with a real instance. */
void pico8_entity_init(void);

/* Releases this module's own bookkeeping storage (hv/cv/cg's list
 * backing arrays, bm's and ci's ob()-parsed tables, mo's instance) -- NOT
 * the entities themselves (still owned by whoever
 * pico8_class_instantiate'd them; freeing an entity is the caller's job,
 * same as pico8_instance_free's own comment). Not part of the Lua
 * original (no manual teardown under GC), same reasoning as
 * pico8_class_free/pico8_instance_free. Called automatically by
 * pico8_entity_init() before it rebuilds; also safe to call directly for
 * final teardown. A second call (or a call before the first
 * pico8_entity_init()) is a safe no-op. */
void pico8_entity_shutdown(void);

/* hv (pico8/lua.lua:152's `hv,...={},...}`): every entity lt()'d and not
 * yet he()'d, in registration order, as a list of PICO8_VALUE_REF values
 * (each .as.ref a pico8_instance_t*). */
const pico8_list_t *pico8_hv(void);

/* cv[name] (pico8/lua.lua:152/167-170's `cv={}` / `bm=ob([[...]])`): the
 * entities currently registered under bm field name `name` ("kj", "ev",
 * "hd", "hk", or "hb"), or NULL if `name` isn't one of bm's 5 names. */
const pico8_list_t *pico8_cv(const char *name);

/* cg[tag] (pico8/lua.lua:152's `cg={}`, grown on demand by lt() from an
 * entity's `.nu` tag list): the entities currently registered under
 * `tag`, or NULL if no entity has ever been lt()'d carrying that tag.
 * Unlike pico8_cv()'s fixed-size backing storage, cg's own storage grows
 * (realloc) as brand-new tags are first seen -- don't hold onto a
 * returned pointer across a later lt() call that might register a tag
 * cg has never seen before; call pico8_cg() again afterward instead. */
const pico8_list_t *pico8_cg(const char *tag);

/* lt(e) / he(e) (pico8/lua.lua:155-163) -- see pico8_entity.c. */
pico8_instance_t *pico8_lt(pico8_instance_t *e);
void pico8_he(pico8_instance_t *e);

/* mv(e,rm,...) (pico8/lua.lua:5-9) -- see pico8_entity.c's own top comment
 * for the full trace, and pico8_value.h's own comment on
 * pico8_value_function_t for Phase 3 Chunk 5's arity-extension rationale
 * (0, 1, or 2 real extra arguments, confirmed against every real mv() call
 * site in pico8/lua.lua). `argc` says how many of `arg0`/`arg1` are real;
 * a caller passes pico8_value_ref(NULL) for any slot argc doesn't cover,
 * matching this port's established "no real Lua argument" filler
 * convention. Returns 1 and writes *out_value when `e` is non-NULL and has
 * a field named `method_name` anywhere on its class-chain fallback
 * (pico8_instance_get_field) -- calling it with `e`, `argc`, `arg0` and
 * `arg1` if the field is PICO8_VALUE_FUNCTION-kind, and using its result,
 * or using the field's own value as-is otherwise. Returns 0 (absent,
 * *out_value left untouched) if `e` is NULL or has no such field at all. */
int pico8_mv(pico8_instance_t *e, const char *method_name, int argc, pico8_value_t arg0, pico8_value_t arg1,
             pico8_value_t *out_value);

/* dc(nt) (pico8/lua.lua:184-195) -- see pico8_entity.c. */
void pico8_dc(const char *nt);

/* ci (pico8/lua.lua:183's `ci=ob([[0.5,1,1,1,1,1,1,1,1,0,0,0,0,0,0,]])`):
 * the per-layer blend-factor table dc()'s `nt=="kj"` branch indexes into.
 * 1-indexed exactly like the original (dc() reads ci[o] for o=1..15),
 * matching pico8_ob_table_get_index's own convention. Returned table is
 * owned by this module (built/freed by pico8_entity_init()/_shutdown());
 * callers must not free it. */
const pico8_ob_table_t *pico8_ci(void);

/* mo -- Phase 2's OWN placeholder for the real camera/palette singleton
 * (NOT a port of any real Lua source -- see pico8_entity.c's own comment
 * on this global), until Phase 3 Chunk 6 (see pico8_entity_set_mo() below)
 * replaces it with a real `qb` instance. Before Chunk 6 runs, this is a
 * single classless instance with no fields set, existing solely so dc()'s
 * literal `mo:md(ci[o])` line has a real object to dispatch through and
 * compiles/links (also read defensively by pico8_entity_geom.c's gd() and
 * pico8_ui.c's ui:lj(), both of which already tolerate an absent/zero
 * `.ra` field the same way). Owned by this module (built/freed by
 * pico8_entity_init()/_shutdown(), or replaced by pico8_entity_set_mo());
 * callers must not free it. */
pico8_instance_t *pico8_mo(void);

/* Chunk 6 (jl() completion, docs/prd/triage/phase3-etape1-function-by-
 * function-c-port.md): reconciles pico8_mo()'s own placeholder. Frees
 * whatever pico8_mo() previously returned (the classless placeholder
 * pico8_entity_init() builds above, or an earlier real instance from a
 * prior call) and makes `instance` the new one -- pico8_mo()'s own
 * signature/declaration are unchanged, so every existing real caller
 * (pico8_entity_geom.c's gd(), pico8_ui.c's ui:lj()) keeps working
 * unmodified. The real `mo=qb()` construction itself happens in the new
 * jl()-completion module (pico8_bootstrap.c), which calls this setter
 * right after building the real instance -- this module deliberately
 * never includes pico8_scroll.h (the qb class) itself, keeping this
 * already-oversized module from growing further. A NULL `instance` is a
 * valid way to go back to "no mo" (pico8_instance_free's own NULL-is-a-
 * no-op convention); passing back the current pico8_mo() value is a safe
 * no-op, not a use-after-free. */
void pico8_entity_set_mo(pico8_instance_t *instance);

#endif
