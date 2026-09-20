#ifndef PICO8_ECONOMY_SCORE_H
#define PICO8_ECONOMY_SCORE_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"

/* Phase 3, Chunk 12 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `da`,`cz`,`db`,`ky`,`bf` (pico8/lua.lua:1312-1376, read
 * directly) -- the score/happiness formulas, plus the `dk` forward-
 * reference accessor pair, split out of what was originally one ~894-line
 * pico8_economy_formulas.c once this chunk's own module-size checkpoint
 * flagged it well past the ~430-540 range already twice flagged by
 * warn-review (see pico8_economy_formulas.h's own top comment for the
 * full 3-way split rationale, and docs/portage-progress.md's own
 * Vague 12 entry). None of these five calls into
 * pico8_economy_formulas.c/pico8_economy_tiers.c's own functions.
 *
 * `da`/`cz`/`ky`/`bf` read `cg.jo`/`cg.qh` (Chunk 13/15's not-yet-built
 * planet tags) and dispatch `pt:gg()` forward-references through
 * pico8_mv() (the same treatment Chunk 9's `ez:self:mm()` already
 * established) -- confirmed mechanically inert (cg is queried by TAG
 * STRING, not by C type, see pico8_economy_formulas.h's own identical
 * note), not a blocker. `db`'s own `e:to()` is Chunk 3's ALREADY-BUILT
 * `pico8_trade_to`, a direct call.
 *
 * `dk` FORWARD REFERENCE (`db`'s own `dk.trade`): this chunk's own db() is
 * the FIRST real reference anywhere in this port to the global `dk` (the
 * selected difficulty-option table, `eu(nj)dk=nj`, pico8/lua.lua:1181 --
 * per the PRD's own "Singleton globals ownership" Implementation
 * Decision, `dk` is built by `eu()`, Chunk 22, alongside `fm`/`ju`). Unlike
 * `fm` (a classless PLACEHOLDER pico8_instance_t, pico8_probe.h -- `fm` is
 * always a real Lua table from `hy:pq()` on), real Lua's own `dk` is
 * genuinely undefined (nil) until `eu()` assigns it: no Lua code ever
 * builds an empty placeholder table for it ahead of time. So this port's
 * own placeholder is simply NULL (no pico8_ob_table_t at all) rather than
 * an empty-but-real one, read defensively by pico8_db() below exactly like
 * every other possibly-absent field/global in this port. `pico8_dk()`/
 * `pico8_economy_set_dk()` follow the same get/set accessor-pair
 * convention as pico8_fm()/pico8_probe_set_fm() (never a raw extern);
 * Chunk 22's own `eu()` must call `pico8_economy_set_dk(...)` once the
 * player's chosen `bo` entry is known -- not done here, matching this
 * chunk's own explicit instruction not to build singletons this chunk
 * doesn't own. */

/* es=ob([[1,0,1,2,]]) (pico8/lua.lua:1311, already ported as pico8_es(),
 * pico8_game_data.h -- reused here.)
 * function da(pt)
 * return pt.qh
 * and #cg.qh
 * or pt:gg()
 * and es[pt.mi]
 * or 0
 * end
 * (pico8/lua.lua:1312-1318.) Groups as `(pt.qh and #cg.qh) or (pt:gg() and
 * es[pt.mi]) or 0` (Lua's `and` binds tighter than `or`): if `pt.qh`
 * (a subtype marker, Chunk 15) is truthy, the result is `#cg.qh` itself
 * (a count is never falsy, so this branch always wins once `pt.qh` is
 * set); otherwise, if `pt:gg()` (forward-ref, Chunk 13 -- mv()-dispatched,
 * see this file's own top comment) is truthy AND `es[pt.mi]` resolves,
 * the result is that `es` entry; otherwise 0. `pt:gg()`'s own dispatch is
 * NON-const (pico8_mv() takes a non-const `pico8_instance_t*`), so unlike
 * this file's other `const`-parameter formulas, `pt` here is a plain
 * (mutable) pointer. */
pico8_fixed_t pico8_da(pico8_instance_t *pt);

/* function cz()
 * local mu=#jp(cg.jo,jo.gg)
 * return flr(0.18*mu^2)
 * end
 * (pico8/lua.lua:1319-1322.) `jo.gg` here is the raw Lua FUNCTION value
 * (colon-sugar `function jo:gg()...end` desugars to `jo.gg=function(self)
 * ...end`), passed to `jp()` as its own predicate -- `jp` then calls
 * `ow(e)`, i.e. `jo.gg(e)`, which is exactly `e:gg()`. So this port calls
 * `pico8_mv(e,"gg",0,...)` per element (the same forward-ref dispatch
 * `pico8_da` above uses), which reproduces the exact same runtime
 * behavior as passing the raw method reference, not an approximation of
 * it. `mu^2` is a plain square (pico8_fixed_mul(mu,mu) on the fixed
 * representation of the plain integer count), not `pico8_dj`'s own
 * fractional-power case. */
pico8_fixed_t pico8_cz(void);

/* ct=ob([[-0.333,0,0.25,0.5,]]) (pico8/lua.lua:1323-1324, already ported
 * as pico8_ct(), pico8_game_data.h -- reused here.)
 * function db(pt)
 * local total=0
 * for e in all(pt.by)do
 * local oz=e:to()
 * if oz.hf then
 * total+=dk.trade*pt.hs*(1+ct[pt.mi]+ct[oz.mi])
 * end
 * end
 * if pt.kw.qe==r.w then
 * total+=pt.kw.mu*dk.trade*1.5
 * end
 * return total
 * end
 * (pico8/lua.lua:1325-1337.) Per-planet trade income: for every trade
 * route `e` reaching `pt` (`.by`, Chunk 13's `jo:ib()` result, see
 * pico8_economy_tiers.h's own pico8_bd), if its destination `oz` (`e:to()`,
 * Chunk 3's ALREADY-BUILT `pico8_trade_to`, direct call) is a "hub"
 * (`.hf` truthy), adds a distance-tier bonus scaled by `dk.trade` (this
 * file's own top comment on the `dk` forward reference) and `pt.hs`. Then,
 * if `pt`'s own exported resource (`pt.kw.qe`) is the WORLD-resource
 * sentinel (`r.w`, compared by CONTENT via `economy_value_equal_content`,
 * same gap as `pico8_dh` (pico8_economy_tiers.h)), adds a flat 1.5x bonus
 * on `pt.kw.mu`. `pt` is `const` (no mv()-dispatched colon-call inside
 * this function -- `e:to()` is a known direct call). */
pico8_fixed_t pico8_db(const pico8_instance_t *pt);

/* b_=ob([[0,100,200,400,]]) (pico8/lua.lua:1338, already ported as
 * pico8_b_(), pico8_game_data.h -- reused here.)
 * function ky()
 * local di=0
 * for pt in all(cg.jo)do
 * if pt:gg()then
 * di+=b_[pt.mi]
 * end
 * end
 * local gl=
 * fm.po[r.p]*40
 * local el=
 * flr((#fm.invented*1.2)^2)*10
 * local total=flr((
 * di+
 * gl+
 * el
 * )*0.01*fm.happiness)
 * return {
 * di,
 * gl,
 * el,
 * fm.happiness.."%",
 * total,
 * min(flr(total/2000),5)
 * }
 * end
 * (pico8/lua.lua:1339-1363.) The end-of-game score breakdown: `di`
 * (development score, summed via this file's own private
 * `sum_by_mi_visit` helper -- the SAME "if pt:gg() then total+=table[
 * pt.mi] end" shape `pico8_bf` below also needs, shared rather than
 * duplicated a third time), `gl` (research-population bonus, `fm.po`'s
 * own REF-to-ob_table shape set by `hy:lj()`'s `bx()`), `el` (tech-tree
 * bonus), `total` (the weighted, happiness-scaled sum), and a final
 * capped `total/2000` tier. `fm.happiness.."%"` is a genuine string
 * concatenation (`pico8_value_tostr` on the FIXED happiness value, then
 * appended with a literal "%" via this file's own small one-shot
 * `concat_percent` helper -- not `pico8_hp_jb_join`'s own growable-buffer
 * shape, since only two known pieces are ever concatenated here, a
 * smaller, distinct need). Returns the 6-entry positional table as a
 * fresh, heap-allocated `pico8_ob_table_t*` the caller owns (matching
 * `g_:mm()`'s own established "Lua table literal return value = REF to a
 * heap pico8_ob_table_t" precedent, pico8_probe.h) -- released with
 * pico8_ob_table_free(). */
pico8_ob_table_t *pico8_ky(void);

/* co=ob([[-5,0,1,1,]]) (pico8/lua.lua:1364, already ported as pico8_co(),
 * pico8_game_data.h -- reused here.)
 * function bf()
 * local h=100+2*(
 * fm.po[r.h]+
 * fm.po[r.w]
 * )
 * for pt in all(cg.jo)do
 * if pt:gg()then
 * h+=co[pt.mi]
 * end
 * end
 * return h
 * end
 * (pico8/lua.lua:1365-1376.) The happiness score: a base of
 * 100+2*(housing+world-resource output), plus a per-planet-tier modifier
 * summed via the same shared `sum_by_mi_visit` helper `pico8_ky` above
 * uses. `hy:lj()`'s own `self.happiness=bf()` stores this result. */
pico8_fixed_t pico8_bf(void);

/* `dk` -- see this file's own top comment for the full forward-reference
 * trace. NULL until Chunk 22's `eu()` calls pico8_economy_set_dk(). */
const pico8_ob_table_t *pico8_dk(void);
void pico8_economy_set_dk(const pico8_ob_table_t *dk);

#endif
