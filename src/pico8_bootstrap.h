#ifndef PICO8_BOOTSTRAP_H
#define PICO8_BOOTSTRAP_H

/* Phase 3, Chunk 6 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): "`jl()` completion + `dg()`" -- the singleton-construction
 * half of `jl()` Phase 2's own `pico8_entity_init()` deliberately left out
 * (see that function's own top comment: "jl()'s own `je=nf(),ok=ui()` are
 * real Phase 3 game singletons, out of this chunk's scope entirely"), plus
 * `dg()`, the per-tick `hv` dispatch loop. This is a NEW, small module
 * (not pico8_entity.c, already at/over the flagged ~430-540-line
 * threshold and explicitly told not to grow further; not pico8_ui.c,
 * pico8_scroll.c or pico8_mouse_reader.c either, so that `mo`/`je`/`ok`'s
 * REAL construction lives in exactly one place, alongside the completed
 * `jl()`, per this PRD's own "Singleton globals ownership" decision) --
 * per this chunk's own "Structurally has no remaining-function dependency
 * of its own beyond Chunk 4/5's classes existing" scope note.
 *
 * function jl()hv,cv,cg={},{},{}
 * mo,je,ok=
 * qb(),nf(),ui()end
 * (pico8/lua.lua:153-155.)
 *
 * `hv,cv,cg={},{},{}` -- Phase 2's own narrow slice, unchanged: still
 * pico8_entity_init() (pico8_entity.h), which also (re)builds `ci` and its
 * own bare classless `mo` placeholder as a safe pre-jl()-completion
 * default (see that function's own comment for why a real class isn't
 * built there). `mo,je,ok=qb(),nf(),ui()` is this chunk's own real work:
 * construct one real instance of each of Chunk 4/5's classes (`qb`, `nf`,
 * `ui`) and make them THE `mo`/`je`/`ok` every other module's `pico8_mo()`/
 * `pico8_je()`/`pico8_ok()` accessor now returns -- see pico8_jl()'s own
 * comment below for the exact reconciliation of Phase 2's/Chunk 5's own
 * placeholder globals.
 *
 * bm=ob([[
 * "kj","ev",
 * "hd",
 * "hk","hb",
 * ]])function dg()for _,qc in pairs(hv)do
 * local fn=qc[qc.me]
 * if fn then
 * fn(qc,qc.t)end
 * if qc.p_ then
 * he(qc)end
 * qc.t+=1
 * end
 * end
 * (pico8/lua.lua:173-181; `bm` itself is Chunk 10/Phase 2's own, already
 * ported as pico8_entity.c's g_bm_names -- reproduced above only because
 * it's the source line directly preceding dg()'s own definition in the
 * real file, not part of this chunk's own work.)
 *
 * For every entity `qc` currently in `hv`, in order: look up the field
 * named by `qc`'s OWN CURRENT `me` value (ke's own default "ph", or
 * whatever a later `self:ks(me)` call set it to -- not yet ported, no
 * real call site in scope), and, if present, call it with `(qc,qc.t)` --
 * ported via pico8_mv(), the SAME generic "resolve a field then call it
 * with self plus one argument" mechanism dc() (pico8_entity.c) already
 * uses for its own `qc[nt](qc,qc.ra)` line, per this PRD's own "dc/mv"
 * decision that every such shape route through it. Real Lua's own
 * `if fn then fn(qc,qc.t) end` is a plain truthy-then-CALL with no
 * `type(fn)=="function"` guard (unlike mv()'s own body) -- meaning a
 * `qc.me` that names a field holding a non-function, truthy value would
 * genuinely crash real Lua ("attempt to call a X value"); pico8_mv()'s own
 * defensive "found a non-function field -> return it, don't call it"
 * behavior is reused here rather than reproducing that crash, exactly the
 * same category of documented, never-exercised divergence dc() already
 * carries for its own identical call shape (no real class in this port's
 * own scope ever sets a non-function value under a name `.me` can select).
 *
 * `if qc.p_ then he(qc) end`: real Lua truthiness (nil/false falsy,
 * everything else -- including 0 -- truthy), matching every other
 * truthiness check already established in this port (e.g. pico8_ui.c's
 * own pico8_ui_truthy) -- a private per-module copy here too, same
 * reasoning those files already give.
 *
 * `qc.t+=1`: a field-value REBIND (`qc.t=qc.t+1`), landing on `qc`'s own
 * instance table regardless of where `t`'s CURRENT value was found on the
 * class chain -- the same "field write always lands on the instance"
 * convention already established throughout this port (e.g. pico8_scroll.c's
 * own `self.p+=self.v`). `qc.t`'s absence (never evidenced -- ke's own
 * default always sets `t=0`) is a defensive no-op divergence, same
 * category as every other absent-field case in this port.
 *
 * DELIBERATE DIVERGENCE FLAGGED, NOT "FIXED": dg() iterates `hv` with
 * `pairs(hv)`, NOT `all(hv)` -- unlike PICO-8's own `all()` (pico8_list.h's
 * own pico8_list_all()/pico8_list_iter_next(), which specially recovers
 * when the JUST-RETURNED element is the one deleted, re-reading the same
 * slot instead of advancing past it), plain Lua `pairs()`/`next()` over an
 * array has NO such recovery: `he(qc)` (triggered by `qc.p_`) shifts every
 * later element down by one to close the gap (pico8_list_del), and a
 * naive index-based continuation (this port's own pico8_dg(), below) then
 * advances past whatever slid into the just-vacated slot -- meaning
 * exactly ONE entity (the one that would have occupied the deleted
 * entity's OLD position) is silently skipped THIS tick whenever an entity
 * removes itself from `hv` mid-loop. This is a genuine, real quirk of the
 * original cart's own `pairs(hv)` choice (not a bug this port introduces
 * or "improves away"): this port's pico8_dg() therefore deliberately does
 * NOT use pico8_list_all()'s own smarter deletion-tolerant iterator here
 * (that would silently change real, observable behavior relative to the
 * literal Lua source) -- it re-reads `pico8_hv()`'s own live count/items
 * on every loop step (so a shrinking list is still read correctly) but
 * never compensates for a deletion at or before the current index, exactly
 * mirroring real Lua `next()`'s own raw index-continuation behavior on a
 * shifted array. Not specially tested (no real call site in this port's
 * own scope yet ever exercises TWO OR MORE entities self-destructing via
 * `.p_` on the very same pico8_dg() call -- Phase 3's later chunks, not
 * this one, are where `.p_` first gets set on a real class) -- flagged
 * here per this PRD's own "forward references/uncertain behavior called
 * out explicitly, not guessed past" convention. */

#include "pico8_class.h"

/* jl() (pico8/lua.lua:153-155) -- see this file's own top comment for the
 * full trace. Builds/rebuilds hv/cv/cg/ci and this port's OWN mo/je/ok
 * reconciliation (pico8_entity_set_mo()/pico8_ui_set_je()/pico8_ui_set_ok(),
 * see those functions' own comments) from scratch every time it's called --
 * matching this port's own established "safe to call more than once"
 * convention (pico8_entity_init()'s own contract), needed because the real
 * cart's own `_init()` genuinely calls `jl()` TWICE (pico8/lua.lua:1178 and
 * 1182 -- confirmed by reading the source, not assumed), resetting entity
 * state in between. Calls, in this exact order (each of the first four
 * already independently idempotent, calling its own shutdown first):
 * pico8_entity_init() (hv/cv/cg/ci/placeholder-mo), pico8_ke_init() (`ke`
 * -- NOT called defensively by pico8_qb_init()/pico8_nf_init()/
 * pico8_ui_init() themselves, unlike their own defensive pico8_pi_init()
 * calls -- see pico8_pi.c's own "future bootstrap sequence" comment on
 * pico8_ke_init(): this IS that bootstrap sequence), pico8_qb_init(),
 * pico8_nf_init(), pico8_ui_init() -- THEN constructs the three real
 * instances (pico8_qb_new(NULL), pico8_nf_new(NULL), pico8_ui_new(NULL),
 * matching `qb()`/`nf()`/`ui()`'s own no-argument real call shape) and
 * hands each to its reconciled accessor's own setter. Every real
 * ke-descended instance self-registers into hv/cv/cg via its class's own
 * inherited-or-own `pq` the moment pico8_class_instantiate runs (ke.pq=lt,
 * pico8_ke.c) -- so mo/je/ok are NOT specially exempted from this: they
 * land in `hv` (and, for `ok`, in `cv["ev"]` too -- `ui`'s own `ev` default
 * field IS one of bm's 5 dispatch names) exactly like any other real
 * entity, matching the real Lua's own unremarkable behavior here. */
void pico8_jl(void);

/* Not part of the Lua original (no manual teardown under GC) -- tears
 * down everything pico8_jl() itself builds, in the reverse order, safe to
 * call more than once or before the first pico8_jl(). Frees the real
 * mo/je/ok instances via their own owning module's shutdown (pico8_ui_
 * shutdown() frees je/ok; pico8_entity_shutdown() frees whatever pico8_mo()
 * currently returns, real or placeholder). */
void pico8_bootstrap_shutdown(void);

/* dg() (pico8/lua.lua:173-181) -- see this file's own top comment for the
 * full trace, including the deliberate, documented `pairs()`-vs-`all()`
 * skip-one-on-self-deletion divergence. Requires pico8_jl() (or at least
 * pico8_entity_init()) to have already run -- iterates whatever pico8_hv()
 * currently holds. A no-op if hv is empty. */
void pico8_dg(void);

#endif
