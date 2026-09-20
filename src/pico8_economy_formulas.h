#ifndef PICO8_ECONOMY_FORMULAS_H
#define PICO8_ECONOMY_FORMULAS_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_list.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Phase 3, Chunk 12 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md), "hy class + economy formulas": `cm`,`bx`,`ck`,`dj`,`bk` --
 * the production-table and trade-route-cost formulas (pico8/lua.lua:1239-
 * 1264, read directly), the first 5 of this chunk's 13 standalone
 * pure(-ish) economy formulas. The remaining 8 live in two sibling
 * modules, split from what was originally one ~894-line file once this
 * chunk's own module-size checkpoint flagged it well past the ~430-540
 * range already twice flagged by warn-review (docs/portage-progress.md's
 * own Vague 12 entry has the full split rationale):
 *   - pico8_economy_tiers.c/.h: `bd`,`bb`,`dh` -- the planet development-
 *     tier pipeline (bd populates the trade bookkeeping bb/dh consume,
 *     bb calls dh directly).
 *   - pico8_economy_score.c/.h: `da`,`cz`,`db`,`ky`,`bf` -- the score/
 *     happiness formulas, plus the `dk` forward-reference accessor pair
 *     (db's own first real reference to it).
 * None of these three groups' own functions call INTO another group
 * (bx/ck/dj/bk here don't call bd/bb/dh/da/cz/db/ky/bf or vice versa) --
 * the split follows real call-graph seams, not an arbitrary line count.
 * Kept separate from pico8_economy.c (the `hy` CLASS itself, whose own
 * `hy:lj()`/`hy:hk()` call into all three of these formula modules): none
 * of the 13 formulas has a class of their own (never `self:` method-call
 * syntax, always plain `fn(...)` calls), the same "distinct concern, no
 * class" split pico8_tech_gate.c already established for `oj`/`ov`/`gi`/
 * `cb` versus pico8_research.c's own `hp` class (Chunk 11).
 *
 * `bx`/`ck` read `cg.jo`/`cg.qh` (Chunk 13/15's not-yet-built planet entity
 * tags) and concrete fields of a `jo`/`qh` instance (`.kw`,`.jy`,`.mi`,...)
 * that don't exist until then. Confirmed mechanically inert, not a
 * blocker: cg is queried by TAG STRING (pico8_cg("jo")/pico8_cg("qh")),
 * not by C type, so pico8_cg("jo") already compiles and correctly returns
 * NULL/empty today (nothing has ever been lt()'d with that tag) -- the
 * exact same "compiles, not yet meaningfully exercised" status every
 * other forward reference in this PRD already carries (bg:ldown's mz/g_,
 * bv/dl/d_'s fm -- see pico8_bg.h/pico8_probe.h). */

/* function cm(jm)local p={}
 * for _,qe in pairs(r)do
 * p[qe]=jm
 * end
 * return p
 * end
 * (pico8/lua.lua:1239-1244.) Builds a fresh table keyed by every VALUE
 * (not key) found in `r` (Chunk 1's decoded alphabet-index table,
 * pico8_stash.h), each initialized to `jm`. `r`'s own values are
 * polymorphic (FIXED or STRING, runtime-decided) -- keyed via the same
 * FIXED-index/STRING-name split already established by pico8_ho_apply's
 * `r[ri[i]]` (pico8_collect.c) and pico8_gi's `fm.fu[t.nk]`
 * (pico8_tech_gate.c). `r` is currently empty against the real
 * pico8/map.json asset (the flagged, separately-tracked unstash()/map.json
 * gap, docs/plan.md's "Notes de portage") so this loop runs zero times in
 * practice today; the mechanism itself is exercised and correct via
 * synthetic memory in this chunk's own disposable verification program,
 * matching Chunk 11's own precedent for the same gap. Returns a fresh,
 * heap-allocated table the caller owns and must release with
 * pico8_ob_table_free(). */
pico8_ob_table_t *pico8_cm(pico8_fixed_t jm);

/* function bx()local p=cm(0)for pt in all(cg.jo)do
 * if pt.jy then
 * p[pt.kw.qe]+=pt.kw.mu
 * end
 * end
 * return p
 * end
 * (pico8/lua.lua:1245-1251.) Per-resource production totals: for every
 * `cg.jo` entity (a planet, Chunk 13) with a truthy `.jy` field (its own
 * "wants" list), adds its `.kw.mu` (output amount) into the running total
 * keyed by `.kw.qe` (resource kind) -- `.kw` is read as a REF to a nested
 * pico8_ob_table_t (matching `fm.fu`'s own established REF-to-table field
 * convention, pico8_tech_gate.c). A missing key in `p` (never evidenced,
 * since cm(0) pre-seeds every key found in `r` -- itself empty today, see
 * pico8_cm's own comment) defaults to fixed 0 before accumulating, this
 * port's pervasive numeric-absence divergence. `self.po` in `hy:lj()`
 * (pico8_economy.h) holds this function's own result. Returns a fresh
 * table the caller owns, released with pico8_ob_table_free(). */
pico8_ob_table_t *pico8_bx(void);

/* function ck()local d=cm(1)for l in all(cg.qh)do
 * d[l.jy[#l.jy].qe]-=0.15
 * end
 * return d
 * end
 * (pico8/lua.lua:1252-1256.) Per-resource demand-crowding penalty: for
 * every `cg.qh` entity (a colonized planet subtype, Chunk 15) with a
 * non-empty `.jy` list, subtracts 0.15 from the running total keyed by its
 * LAST want's `.qe`. `.jy` is a Lua LIST (REF to a heap pico8_list_t, each
 * element itself a REF to a nested `{qe=...}` table -- matching `pt.jy`'s
 * own established shape, see pico8_dh's comment below) -- an absent/empty
 * `.jy` is a documented no-op for that `l` (real Lua would error indexing
 * `nil.qe`; this port skips instead, the same defensive divergence used
 * throughout). Returns a fresh table the caller owns, released with
 * pico8_ob_table_free(). `self.fu` in `hy:lj()` holds this result. */
pico8_ob_table_t *pico8_ck(void);

/* bn,bq=6,0.0977 (pico8/lua.lua:1257-1258, already ported as
 * pico8_bn()/pico8_bq(), pico8_game_data.h -- reused here, not
 * re-declared.)
 * function dj(pb,to)local ec=
 * (#(to.p-pb.p)/bq)^0.75
 * return flr(bn*bk()*max(0.5,ec))end
 * (pico8/lua.lua:1259-1261.) Trade-route base cost between two planets
 * `pb`/`to` (their `.p` map-position vectors, this port's established
 * "vector field = REF to a heap pico8_vec_t" convention, defaulting to the
 * zero vector when absent). `#(to.p-pb.p)` is the SQUARED distance
 * (pico8_vec_len_sq, PICO-8's `#` on a `km` vector -- NOT the Euclidean
 * distance). `^0.75` is a genuinely fractional power with no existing
 * fixed-point primitive in this port (every prior `^` use in scope,
 * `cz()`'s `mu^2`, is a plain square, portable via one pico8_fixed_mul);
 * ported via the same "convert to double, use the host's math library,
 * convert back" least-bad option already used for `pico8_bq()`'s own
 * literal (pico8_game_data.h), NOT claimed bit-exact against real PICO-8's
 * own `^` implementation -- a new, documented instance of the same
 * already-flagged numeric-fidelity risk category as `rnd`/`sin`/`cos`
 * (docs/plan.md's "Notes de portage"), not a new mechanism otherwise.
 * `max(0.5,ec)` ported as a plain raw-representation comparison, matching
 * pico8_gi's own identical `max(0.55,...)` precedent (no shared min/max
 * helper existed before this chunk; see pico8_economy_formulas.c's own
 * private `fixed_min`/`fixed_max`, added once THIS chunk's own repeated
 * need -- 4+ occurrences across dh/ky/bk/dj -- crossed the "duplicate
 * twice, share the third time" threshold that precedent already used). */
pico8_fixed_t pico8_dj(const pico8_instance_t *pb, const pico8_instance_t *to);

/* function bk()
 * return max(1,sqrt(#(cg.jc or {}))*0.27-0.1
 * )end
 * (pico8/lua.lua:1262-1264.) A global trade-cost scale factor driven by
 * how many slipways (`cg.jc`, Chunk 14) currently exist; `cg.jc or {}`
 * (nil-safe -- `pico8_cg("jc")` already returns NULL until any `jc` is
 * ever lt()'d, ported as "NULL list counts as 0", the same treatment
 * `pico8_cg()`'s own header comment already documents). */
pico8_fixed_t pico8_bk(void);

#endif
