#ifndef PICO8_RESEARCH_H
#define PICO8_RESEARCH_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 11 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md), "Research/tech foundation": THE CORE of the `hp` class --
 * `pq`,`po`,`od` (pico8/lua.lua:1399-1408,1422-1433) -- plus its own `gb`
 * registry (the `for ri in all(fd) do...add(gb,hp(ri))end` loop). `hp:lq`,
 * `hp:jb`, the standalone `jo.ex`, and `bj(jo)` live in the sibling module
 * pico8_research_display.c/.h instead (see that file's own top comment for
 * why: this module alone crossed the ~430-540-line range at ~780 lines
 * before the split -- see docs/portage-progress.md's own Vague 11 entry).
 * `oj`/`ov`/`gi`/`cb` (the other items this chunk's own PRD text lists
 * alongside these) are NOT in this module either -- see pico8_tech_gate.c/
 * .h, a deliberately separate module for those four pure predicates (no
 * class of their own, a distinct concern).
 *
 * `gb`/`rr` (also this chunk's own job, per Implementation Decisions'
 * "Data-table initialization sequencing") are OWNED here, not in
 * pico8_data_init.c: `gb` is written to both by the `for ri in all(fd) do
 * ... add(gb,hp(ri)) end` loop (pico8_research_build_gb below) AND from
 * inside `hp:pq()` itself (`ho(gb,hp.mr,hp.gp)(self)`, `gb[self.px]=self`)
 * -- keeping the registry alongside the class whose own construction
 * populates it avoids a second module needing write access to this one's
 * singleton, the same reasoning hv/cv/cg live alongside lt()/he()
 * (pico8_entity.c) rather than alongside every module that constructs an
 * entity. pico8_data_init.c calls pico8_research_build_gb(pico8_fd()) as
 * one step of its own orchestrated sequence (mirroring pico8_bootstrap.c's
 * own pico8_jl(), which calls into other modules' sub-inits rather than
 * inlining their bodies) -- this module is built (pico8_research_init())
 * BEFORE that happens, per this chunk's own explicit instruction to build
 * `hp` before writing the game-data-init function's body.
 *
 * New module (not pico8_probe.c/pico8_class.c/pico8_entity.c/pico8_ui.c/
 * pico8_msgbox.c, all at/over the flagged ~430-540-line range -- see
 * docs/portage-progress.md). `hp` is a ROOT class of the kv:jr system
 * (`hp=kv:jr([[...]])`, confirmed by reading the real source, NOT a `ke`
 * subclass) -- like `lq`/`jb`/`trade` (Chunk 3), it never self-registers
 * into hv/cv/cg (only `ke.pq=lt`-descended classes do that), so this chunk
 * exercises nothing structurally new in the entity/dispatch mechanism
 * itself -- see docs/portage-progress.md's own Vague 11 entry for the
 * explicit per-lot assessment. None of `hp`'s own methods (`lq`,`jb`,`po`,
 * `od`) are `bm` dispatch names, and `jb`/`po`/`od` are never called via
 * mv()/dc() anywhere in the real source (every real call site for those
 * three uses plain `self:method()` syntax) -- so all four are still
 * ported as direct, known C functions (pico8_ke_ks's own "known method,
 * direct call" precedent).
 *
 * *** CORRECTION (Chunk 13, sub-lot 3) *** -- this paragraph originally
 * claimed NONE of `hp`'s methods are ever called via mv(), full stop.
 * That was wrong for `lq` specifically: `jo:fe()` (Chunk 13, pico8/
 * lua.lua:418-423) has a real `mv(b,"lq")` call site where `b` is an `hp`
 * instance -- found while porting `fe`, the same kind of "prose error in
 * already-shipped code, found and fixed" this port has already logged a
 * few times (see docs/plan.md's own "Notes de portage"). Fixed by also
 * wiring "lq" as a real PICO8_VALUE_FUNCTION field on `hp`'s class
 * defaults (`pico8_research.c`'s own `pico8_hp_lq_dispatch`, alongside
 * `pico8_research_init()`'s own `pico8_class_declare()` call) -- `jb`/
 * `po`/`od` remain plain direct calls, unaffected; `pico8_hp_lq` itself
 * (pico8_research_display.h) is UNCHANGED, still a plain known C
 * function directly callable exactly as it already was. This is a pure
 * additive fix: it doesn't change what `pico8_hp_lq` computes, only
 * makes it reachable through `pico8_mv()` too, the way the real source
 * genuinely needs. */
void pico8_research_init(void);

/* Not part of the Lua original (no manual teardown under GC): frees every
 * hp instance still referenced by `gb` (walked via `gb`'s own POSITIONAL
 * entries only -- `gb`'s NAMED entries, `gb[self.px]`, are always aliases
 * of the SAME instance already reached positionally, per `rc[ri.px]=ri`
 * inside hp:pq(); freeing via both would double-free), then `gb` itself and
 * the `hp` class. Safe to call more than once, or before the first
 * pico8_research_init(). */
void pico8_research_shutdown(void);

/* The real `hp` root class, built once by pico8_research_init(). Not
 * retrievable via pico8_class_lookup() (registers under the shared ""
 * name-registry slot, like ke/lq/jb/trade -- hp's own spec never sets an
 * `fz` field). Returns NULL before pico8_research_init() (or after
 * pico8_research_shutdown()). */
pico8_class_t *pico8_hp_class(void);

/* `hp(overrides)` -- the real Lua constructor call (`add(gb,hp(ri))`,
 * pico8/lua.lua:1440). Thin wrapper around pico8_class_instantiate,
 * matching pico8_lq_new/pico8_jb_new's own established convention
 * (pico8_layout.c). */
pico8_instance_t *pico8_hp_new(const pico8_ob_table_t *overrides);

/* gb,rr={}
 * for ri in all(fd)do
 * if type(ri)=="string" then
 * rr=ri
 * else
 * ri.rr=rr
 * add(gb,hp(ri))end
 * end
 * (pico8/lua.lua:1434-1441.) `fd` (the still-undecoded research-project
 * data, pico8_data_init.h) mixes two kinds of positional entries: a bare
 * STRING acts as a running "category" header (`rr`, reset here, updated
 * every time one is seen), and every other entry (a nested REF'd
 * ob()-parsed record) is tagged with the CURRENT `rr` (`ri.rr=rr` --
 * `rr`'s own string is never copied: it's a value independently parsed by
 * this port's own ob() from `fd`'s own backing memory, which outlives this
 * whole function, so borrowing the pointer directly is safe and avoids
 * introducing a third independent strdup-like duplicate) then constructed
 * into a real `hp` instance and appended to `gb` (`hp:pq()`, wired as this
 * class's own pq, does the REST of the real per-record work -- the
 * `ho(gb,hp.mr,hp.gp)(self)` renaming-and-registration pass -- the moment
 * pico8_hp_new/pico8_class_instantiate runs). Resets `gb`/`rr` to fresh
 * empty state first (mirrors the real `gb,rr={}` reset, and this port's
 * own "safe to call more than once" convention), freeing whatever a
 * previous call built (see pico8_research_shutdown's own comment on why
 * only `gb`'s positional entries need freeing). `fd`'s own positional
 * values are walked via this module's own private positional-values
 * helper (matching pico8_layout_positional_values' own established
 * per-module-duplicate convention) since `fd` is a pico8_ob_table_t, not a
 * pico8_list_t. A NULL `fd` is treated as an empty table (`all(nil)`'s own
 * documented "no iteration" contract) -- `gb` ends up empty, not a crash. */
void pico8_research_build_gb(const pico8_ob_table_t *fd);

/* `gb` itself, owned by this module (built/freed by
 * pico8_research_build_gb()/pico8_research_shutdown()). Consumed by
 * pico8_bj() (pico8_research_display.h) and, later, `jo:fe()` (Chunk 13,
 * `bj(self)`). Callers must not free it. */
const pico8_ob_table_t *pico8_gb(void);

/* function hp:po()local qp=hc[self.ja]
 * return self.il and oj(self.il)and hc[qp.mc]
 * or qp
 * end
 * (pico8/lua.lua:1422-1425.) `hc[self.ja]`/`hc[qp.mc]` -- polymorphic
 * FIXED-index-or-STRING-name lookups into `hc` (pico8_data_init.h),
 * matching pico8_ho_apply's own established r[ri[i]] convention. Returns
 * `hc[qp.mc]` (a DIFFERENT, "upgraded" `hc` entry) only when `self.il` is
 * truthy AND `oj(self.il)` (pico8_tech_gate.h) says that prerequisite is
 * already unlocked; otherwise returns `qp` itself. Both are BORROWED
 * pointers into `hc`'s own storage (never copied, never owned by the
 * caller) -- matching pico8_r()/pico8_iz()'s own const-borrowed
 * convention. Returns NULL if `self`/`self.ja`/`hc[self.ja]` don't
 * resolve (defensive; `hc` is real cart data by the time any real hp
 * instance exists, so this is not expected to be exercised as absent in
 * practice, only as a safety net matching this port's pervasive
 * defensive-read convention). */
const pico8_ob_table_t *pico8_hp_po(const pico8_instance_t *self);

/* function hp:od()local od=self.fj
 * if self.il and oj(self.il)then
 * od*=1.5
 * end
 * if not self.ii and oj("nanomaterials")then
 * od*=0.85
 * end
 * return flr(od)end
 * (pico8/lua.lua:1426-1433.) `self.fj` (base cost) absent/wrong-kind reads
 * as fixed 0 (this port's established numeric-absence divergence). Both
 * `oj(...)` calls route through pico8_tech_gate.h's own pico8_oj, the
 * exact same `fm`-forward-referencing predicate `hp:po()` above already
 * uses -- no separate mechanism. A NULL `self` reads every field as its
 * documented default (fixed 0 for `fj`, falsy for `il`/`ii`), so this
 * returns `flr(0)` = 0 rather than crashing. */
pico8_fixed_t pico8_hp_od(const pico8_instance_t *self);

#endif
