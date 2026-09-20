#ifndef PICO8_RESEARCH_DISPLAY_H
#define PICO8_RESEARCH_DISPLAY_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_list.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 11 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md), "Research/tech foundation": `hp:lq` (pico8/lua.lua:1409-1413),
 * `hp:jb` (lines 1414-1421), the standalone `jo.ex` (lines 518-538, used by
 * both `hp:lq` here and, later, `jo:hb`, Chunk 13), and `bj(jo)` (lines
 * 1391-1398) -- split out of pico8_research.c/.h (which keeps `hp`'s own
 * core: pq/po/od, plus the `gb` registry these functions read) purely for
 * module size: pico8_research.c crossed the ~430-540-line range already
 * flagged twice by warn-review (~780 lines before this split) -- see
 * docs/portage-progress.md's own Vague 11 entry for the size-check trace.
 * `pico8_hp_lq`/`pico8_bj` read `pico8_gb()`/`pico8_hp_po()`
 * (pico8_research.h) and `pico8_hp_class()` (also pico8_research.h) --
 * this module includes that header rather than duplicating any of it. */

/* function hp:lq()local l=
 * jo.ex(self.i,self.o,max(self:po()[1],1),0,1
 * )add(l,hp.fk)add(l,self.px)
 * return l
 * end
 * (pico8/lua.lua:1409-1413.) Builds this research entry's own icon-row
 * display via jo.ex (below), then appends TWO MORE positional entries
 * (`hp.fk` -- the class's own shared `{false,2}` marker table, appended by
 * REFERENCE, not copied; `self.px`) directly onto the ALREADY-CONSTRUCTED
 * lq instance's own raw fields table -- a genuine, faithfully-preserved
 * Lua quirk: `add()` on any table (including an already-`pq()`'d instance)
 * just appends to its array part, so these two extra entries land
 * AFTER `lq:pq()` already computed `self.fs`/`self.w` from the ORIGINAL
 * items only, and are therefore never drawn by `lq:ol()` (which only reads
 * `self.fs`) -- inert extra data for some other, not-yet-ported reader.
 * NOTE ALSO (documented, not "fixed"): this same instance's own field named
 * "lq" -- if `hp:pq()` set one (self.ii truthy, see pico8_research.c's own
 * pico8_hp_pq) -- would SHADOW the class-level `hp.lq` method slot for
 * plain `self.lq` field access (though not for this function itself, a
 * known direct C call, never routed through that field at all): a genuine
 * source-level naming collision this port preserves rather than resolves,
 * since nothing in this chunk's own scope calls `self:lq()` on an hp
 * instance the colon-call way. `max(self:po()[1],1)` -- `self:po()` is
 * pico8_hp_po (pico8_research.h); `[1]` its own first positional entry
 * (index 1, absent/wrong-kind reads as fixed 0 like any other numeric
 * absence in this port); `max` ported inline as a plain comparison,
 * matching pico8_ma's/pico8_gi's own established `max(...)` precedent (no
 * shared helper exists for this in the whole port yet). Returns NULL if
 * jo.ex itself returns NULL (see pico8_jo_ex's own comment on when that
 * happens) -- a defensive divergence from real Lua, which would crash
 * calling `add()` on nil; not evidenced by any real call site in this
 * chunk's own scope (self.o is always present after hp:pq's own renaming
 * pass). A NULL `self` is a no-op, returning NULL. */
pico8_instance_t *pico8_hp_lq(pico8_instance_t *self);

/* function hp:jb()
 * if (self.ii)return
 * local it
 * for i in all(self.i)do
 * it=(it and it.."," or "")..iz[tostr(i.qe)]
 * end
 * return {{it,13,iz[tostr(self.o)]}}
 * end
 * (pico8/lua.lua:1414-1421.) Returns NULL (Lua's own implicit nil) when
 * `self.ii` is truthy -- no real call site in this chunk's own scope
 * exercises the rest of this function's own body, so it is ported
 * faithfully but not end-to-end exercisable yet (same "compiles, not yet
 * meaningfully exercised" treatment as every other forward-reference-
 * adjacent function in this PRD). Builds a comma-joined string of
 * `iz[tostr(i.qe)]` over every entry of `self.i` (Lua string concatenation
 * ported via a growable buffer, matching pico8_unstash's own established
 * "incremental growable string" pattern -- documented as this pattern's
 * second, not third, independent occurrence, not a new one to fold into a
 * shared helper for a single not-yet-exercised function), then returns a
 * fresh one-item table wrapping a 3-tuple `{it,13,iz[tostr(self.o)]}`.
 * `it` is stored as a PICO8_VALUE_STRING pointing to this freshly-built
 * buffer -- never freed by anything (matching pico8_ob_table_free's own
 * documented "never frees a STRING value's own memory" contract, the same
 * "detect_leaks=0"-consistent convention this whole port already accepts
 * for ob()-parsed strings). A NULL `self` is a no-op, returning NULL. */
pico8_ob_table_t *pico8_hp_jb(const pico8_instance_t *self);

/* function jo.ex(is,o,jq,mj,d)
 * if (not o)return
 * local l,lc,ml=
 * {},#is>=3 and 1 or 0,o==r.w and {25,23}or {o}
 * local pe=jq*#ml>=3
 * for i in all(is)do
 * add(l,{i.qe,3-lc,i.gc~=0 and 0 or 7
 * })end
 * if #is>0 then
 * add(l,{false,d+lc})add(l,(d==1 or pe)and {35,2+d}or {32,5})end
 * if jq>5 then
 * add(l,tostr(jq))add(l,{false,-4})l.c=13
 * jq=1
 * end
 * for qq in all(ml)do
 * for i=1,max(jq,1)do
 * add(l,{qq,6,(i<=jq and i>mj)and 0 or 7,pe and -4 or -3
 * })end
 * add(l,jo.em)end
 * return lq(l)end
 * (pico8/lua.lua:518-538.) A resource-icon-ROW builder, standalone despite
 * its `jo.` namespacing -- `function jo.ex(...)` is plain Lua sugar for
 * "attach a static function field to whatever table `jo` currently is",
 * NOT a method (real call sites, `jo.ex(...)`, never use colon syntax) and
 * NOT dispatched through mv()/dc(); ported as an ordinary standalone C
 * function that does not need the `jo` CLASS (Chunk 13, not yet built) to
 * exist AT ALL for its own execution -- only ONE single Lua GLOBAL FIELD
 * read from within its body, `jo.em` (a spacer/icon marker appended once
 * per `ml` entry), genuinely forward-references that not-yet-built class.
 * That single read goes through pico8_class_lookup_named("jo") (the SAME
 * generic forward-reference registry Chunk 7 built for `mz`/`g_`, reused
 * here rather than inventing a second one) -- NULL today, so `jo.em` reads
 * as this port's own "absent Lua value" filler (pico8_value_ref(NULL)),
 * the same "compiles, not yet meaningfully exercised" treatment as every
 * other forward reference in this PRD. WHOEVER BUILDS THE REAL `jo` CLASS
 * (Chunk 13) MUST call pico8_class_register_named("jo", jo_class) alongside
 * its own pico8_class_declare() call for this to resolve for real --
 * mirroring pico8_bg.h's own reconciliation note for `mz`/`g_`.
 *
 * `if (not o) return end`: returns NULL (Lua nil) when `o` is falsy --
 * dead-but-compiled in this chunk's own scope (every real caller passes a
 * genuinely truthy `o`), same treatment as Phase 2's own `mo:md()`/"kj"
 * dead branch.
 *
 * `o==r.w`: `r` (pico8_stash.h, Chunk 1) read via its own "w" field;
 * compared with pico8_value_equal (NOT pico8_gu_content) -- this is a
 * direct equality between two values independently read from the SAME
 * underlying `r` table (whichever kind they hold, FIXED or REF), so
 * pointer/value identity is exactly correct here, unlike ho()'s/oj()'s own
 * cross-source string-content gap (`o`'s own value, if it came from an
 * `r[...]` lookup via hp:pq's own renaming pass, IS the literal same stored
 * value `r.w` also reads).
 *
 * `is`/`o`/`jq`/`mj`/`d` map directly to this function's own C parameters;
 * `#is` reads as 0 for a NULL `is` (matching `all(nil)`'s own "no
 * iteration" contract, applied here to Lua's `#` operator too, since no
 * real call site ever passes a genuinely nil `is`). `for i=1,max(jq,1)`'s
 * own loop bound is read via a direct fixed-point truncation (every real
 * quantity this chunk's own scope can produce is a whole number) rather
 * than a general non-integer-aware Lua for-loop port -- documented, not a
 * new mechanism. `jo.em`'s own concrete VALUE, once Chunk 13 makes `jo`
 * real, is read straight off `pico8_class_lookup_named("jo")->defaults`,
 * matching `bg:pq`'s own established "read a class's own default field
 * directly" precedent (pico8_bg.h) rather than an instance's.
 *
 * The final `l` (a fresh pico8_ob_table_t this function itself builds,
 * freed here immediately after pico8_lq_new(l) shallow-copies its entries
 * -- matching pico8_jb_pq_step's own identical "free the outer wrapper
 * right after passing it to a class constructor" precedent, pico8_layout.c)
 * is passed to pico8_lq_new (pico8_layout.h) exactly like the real `lq(l)`
 * constructor call. A NULL return only happens via the `not o` branch
 * above. */
pico8_instance_t *pico8_jo_ex(const pico8_list_t *is, pico8_value_t o, pico8_fixed_t jq, pico8_fixed_t mj,
                               pico8_fixed_t d);

/* function bj(jo)
 * return jp(gb,function(b)if oj(b.rr)then
 * local mg
 * fi(b.lr,function(k)mg=mg or k==jo.nk.px
 * end)
 * return mg
 * end
 * end)end
 * (pico8/lua.lua:1391-1398.) The Lua parameter is literally named `jo` --
 * a LOCAL variable shadowing the (not-yet-built, Chunk 13) `jo` CLASS
 * within this function's own scope only; it names an arbitrary caller-
 * supplied entity (a planet instance, at the one real call site in scope,
 * `jo:fe()`, Chunk 13's own `bj(self)`), never the class itself, so this
 * function needs NO forward-reference machinery of its own (renamed here
 * to `jo_instance` in C to avoid any reader confusing it with the actual
 * `jo` class this port elsewhere reaches via pico8_class_lookup_named).
 * Filters `gb` (pico8_research.h's own registry), keeping every research
 * entry `b` whose own category (`b.rr`, set by pico8_research_build_gb) is
 * already unlocked (`oj(b.rr)`, pico8_tech_gate.h) AND whose own
 * requirement-code string (`b.lr` -- a compact string of single-character
 * codes, one per required planet-kind, mirroring `self.i`'s own encoding in
 * hp:pq) contains a character equal to `jo.nk.px` (`jo_instance`'s own
 * "kind" field, itself a REF to one of `df`'s own renamed single-character-
 * `px`-keyed records, pico8_data_init.h) -- compared as PLAIN C CHARACTERS
 * here, not through pico8_gu/pico8_gu_content's own list-membership
 * machinery at all: `fi()`'s own per-character callback (pico8_ob.h) hands
 * back a raw `char`, and `jo.nk.px` is always exactly one character by
 * construction (df's own ho() renaming pass), so this sidesteps the
 * whole ho()-style cross-source string-identity gap entirely -- a direct
 * `char==char` comparison is exactly correct here, not an approximation.
 * `gb`'s own positional entries are walked via this module's own private
 * positional-values helper (a separate copy from pico8_research.c's own,
 * per this file's own top comment on the module-size split). Returns a
 * new, heap-backed list the caller owns and must release with
 * pico8_list_free() (matching pico8_jp's own convention). A NULL
 * `jo_instance` (or one missing `.nk`/`.nk.px`) matches every real string
 * in `b.lr` as "not found" (defensive; never evidenced as a real call
 * shape). */
pico8_list_t pico8_bj(const pico8_instance_t *jo_instance);

#endif
