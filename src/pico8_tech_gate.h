#ifndef PICO8_TECH_GATE_H
#define PICO8_TECH_GATE_H

#include "pico8_fixed.h"
#include "pico8_list.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 11 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md), "Research/tech foundation": the four economy/tech-gate
 * predicates `oj`, `ov`, `gi`, `cb` (pico8/lua.lua:1455-1474, read directly
 * -- not guessed at). All four forward-reference the not-yet-built `fm`
 * global (Chunk 12's `hy` economy singleton) via ordinary Lua FIELD access
 * on it (`fm.invented`, `fm.fu`, `fm.jk`, `fm.ew`) -- never a `fm:method()`
 * call, so this module needs pico8_instance_get_field(pico8_fm(),...) only,
 * NOT pico8_mv() -- unlike Chunk 10's own `fm:lu()`/`fm:nx()` (real Lua
 * method-call sugar). `pico8_fm()` itself (pico8_probe.h, Chunk 10) is
 * reused exactly as-is -- this module does NOT declare a second, competing
 * `fm` accessor, per this chunk's own explicit instruction.
 *
 * New module (not pico8_research.c, which owns the `hp` CLASS itself and
 * `jo.ex`/`bj` -- these four are pure standalone predicates with no class
 * of their own, a distinct concern; not pico8_probe.c/pico8_class.c/
 * pico8_entity.c/pico8_ui.c/pico8_msgbox.c, all at/over the flagged
 * ~430-540-line range -- see docs/portage-progress.md). */

/* function oj(px)
 * return not px or gu(fm.invented,px)
 * end
 * (pico8/lua.lua:1464-1466.) "Is `px` (a tech/research-project code string,
 * or Lua nil/false meaning 'no prerequisite') already unlocked?" `not px`
 * uses real Lua truthiness (nil/false falsy, everything else -- including
 * fixed 0 -- truthy): ported via this module's own private truthy helper,
 * which -- like pico8_probe.c's own pico8_probe_truthy -- ALSO treats a
 * PICO8_VALUE_REF(NULL) as falsy, matching this port's own established
 * "absent Lua value" filler convention. `gu(fm.invented,px)`: `fm.invented`
 * is read via pico8_instance_get_field(pico8_fm(),"invented",...) -- absent
 * today (fm's own placeholder has no fields at all, per pico8_probe.h's own
 * top comment), so this reads as a NULL list, and pico8_gu_content(NULL,px)
 * already documents "no iteration" for that case, matching real Lua's own
 * all(nil) contract. USES pico8_gu_content, NOT pico8_gu: every real px
 * this chunk's own scope can produce (`self.il`, "nanomaterials", `t.px`,
 * `b.rr`) is a string independently parsed by THIS port's own ob() (which,
 * unlike real Lua, never interns string literals -- see pico8_collect.h's
 * own dedicated note on this exact root cause, first flagged by ho()), so
 * `fm.invented`'s own entries (added elsewhere, by `hy:kt`, Chunk 12, via
 * `add(self.invented,t.px)`) and a caller's own freshly-parsed `px` are
 * NEVER the same pointer even when equal in content -- pico8_gu's
 * pointer-identity rule would silently never match here, exactly the same
 * gap ho() already hit and resolved the same way. Not yet exercised by a
 * real end-to-end path (fm is still a placeholder), but this is the
 * correct, evidenced choice for when Chunk 12 makes fm real. */
int pico8_oj(pico8_value_t px);

/* function ov(px,lk,mc)
 * return oj(px)
 * and mc or lk
 * end
 * (pico8/lua.lua:1467-1470.) Lua's `A and B or C` idiom: if oj(px) is
 * truthy, the result is `mc` UNLESS `mc` itself is falsy (Lua's own `and`
 * returns its second operand's value even when falsy, but then the outer
 * `or C` kicks in) -- so the exact three-way outcome (oj(px) falsy -> lk;
 * oj(px) truthy and mc truthy -> mc; oj(px) truthy and mc falsy -> lk) is
 * reproduced via this module's own truthy helper on `mc`, not simplified to
 * a plain C ternary on oj(px) alone. `px`/`lk`/`mc` stay as pico8_value_t
 * (never narrowed to a concrete kind) since no real call site for this
 * function exists in this chunk's own scope to confirm what kind `lk`/`mc`
 * actually hold (a later chunk's own real call site is the evidence to
 * narrow this, not a guess made here). */
pico8_value_t pico8_ov(pico8_value_t px, pico8_value_t lk, pico8_value_t mc);

/* function gi(t)
 * return flr(t.od*max(0.55,
 * fm.fu[t.nk]))
 * end
 * (pico8/lua.lua:1471-1474.) `t` is a real entry of `nb` (Chunk 11's own
 * tech-list data, pico8_data_init.h) -- a plain ob()-parsed record, NOT a
 * class instance (`nb=ob(unstash(10457))` never goes through kv:jr), so
 * `t` is a `pico8_ob_table_t*` here, read via pico8_ob_table_get_name, not
 * pico8_instance_get_field. `t.od`/`t.nk` are its own named fields
 * (renamed by nb's own foreach(nb,ho(...)) pass, pico8_data_init.c).
 * `fm.fu[t.nk]` -- `fm`'s own "fu" field (a per-planet-kind cost-multiplier
 * table, Chunk 12), indexed by `t.nk`'s OWN value (polymorphic: FIXED index
 * or STRING name, same "the real Lua key's type isn't known until runtime"
 * split already established by pico8_ft_index/pico8_ft_name and
 * pico8_ho_apply's own r[ri[i]] lookup) -- absent today (fm is still a
 * classless placeholder with no "fu" field at all) reads defensively as
 * fixed 0, the same "absent numeric field reads as 0" divergence already
 * established throughout this port (e.g. pico8_entity_geom.c). `max(0.55,
 * x)` ported inline as a plain C comparison on the raw pico8_fixed_t
 * representation, matching pico8_ma's own identical `max(0,...)` precedent
 * (pico8_entity_geom.c) -- not a new shared helper, since no `max` helper
 * exists anywhere in this port yet and this is the second, not third,
 * independent occurrence. `t.od` absent/wrong-kind reads as fixed 0 (same
 * divergence). A NULL `t` reads every field as its documented default,
 * matching this port's pervasive defensive convention. */
pico8_fixed_t pico8_gi(const pico8_ob_table_t *t);

/* function cb(bi)
 * return jp(nb,function(t)
 * t.e_=gi(t)<=fm.jk
 * return t.lv==fm.ew
 * and not oj(t.px)
 * and (t.e_
 * or bi)
 * end)
 * end
 * (pico8/lua.lua:1455-1463.) Filters `nb` (Chunk 11's own tech list), but
 * with a REAL SIDE EFFECT on every entry visited, not just the ones kept:
 * `t.e_=gi(t)<=fm.jk` (a mutable "affordable?" flag, "fm.jk" -- the
 * player's own current research-points balance, Chunk 12 -- absent today,
 * reads as fixed 0, same divergence as gi()'s own fm.fu) is written onto
 * EVERY `t` the filter's own predicate visits, unconditionally, before the
 * kept/not-kept decision is even computed -- ported faithfully (the write
 * happens inside the predicate callback, exactly like the real Lua closure,
 * not hoisted out or skipped for excluded entries). Kept iff `t.lv==fm.ew`
 * (`fm.ew` -- the player's own current tech era/tier, absent today reads as
 * fixed 0, likely matching very few/no real `t.lv` values until Chunk 12)
 * AND `t` isn't already unlocked (`not oj(t.px)`) AND (it's newly
 * affordable (`t.e_`) OR the caller passed `bi` truthy -- `bi` is this
 * function's own "bypass affordability" flag, kept as a pico8_value_t to
 * preserve exact Lua truthiness since no real call site in this chunk's own
 * scope confirms its concrete kind). `nb` is walked via this module's own
 * private positional-values helper (matching pico8_layout_positional_values'
 * own established per-module-duplicate convention, pico8_layout.c) since
 * pico8_jp (pico8_collect.h) takes a pico8_list_t, not a pico8_ob_table_t.
 * Returns a new, heap-backed list the caller owns and must release with
 * pico8_list_free() (matching pico8_jp's own convention). */
pico8_list_t pico8_cb(pico8_value_t bi);

#endif
