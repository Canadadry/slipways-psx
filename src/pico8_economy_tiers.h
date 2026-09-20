#ifndef PICO8_ECONOMY_TIERS_H
#define PICO8_ECONOMY_TIERS_H

#include "pico8_class.h"
#include "pico8_fixed.h"

/* Phase 3, Chunk 12 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `bd`,`bb`,`dh` (pico8/lua.lua:1265-1310, read directly) --
 * the planet development-TIER pipeline, split out of what was originally
 * one ~894-line pico8_economy_formulas.c once this chunk's own module-size
 * checkpoint flagged it well past the ~430-540 range already twice
 * flagged by warn-review (see pico8_economy_formulas.h's own top comment
 * for the full 3-way split rationale, and docs/portage-progress.md's own
 * Vague 12 entry). These three call each other directly (`bd` populates
 * every planet's `.cc`/`.by` trade bookkeeping, `bb` calls `dh` to decide
 * whether a planet's tier changed, `hy:lj()`'s own `repeat bd() until not
 * bb()` loop drives both) -- a real, evidenced call-graph seam, not an
 * arbitrary line cut, and none of the three calls into
 * pico8_economy_formulas.c/pico8_economy_score.c's own functions.
 *
 * Every one of these reads `cg.jo` (Chunk 13's not-yet-built planet tag)
 * and concrete `jo`/`hp`/`trade` fields (`.by`,`.cc`,`.jy`,`.mi`,`.hn`,
 * `.rl`,`.lp`,`.hg`,...) that don't exist until Chunks 13-14 land --
 * confirmed mechanically inert (cg is queried by TAG STRING, not by C
 * type, see pico8_economy_formulas.h's own identical note), not a
 * blocker. A colon-call to a not-yet-built METHOD (`pt:ib()`) is routed
 * through pico8_mv()'s dynamic dispatch (the same treatment Chunk 9's
 * `ez:self:mm()` and Chunk 10's `fm:lu()`/`fm:nx()` already established);
 * a colon-call to an ALREADY-BUILT method (`e:to()` -> pico8_trade_to,
 * Chunk 3; `hn:po()` -> pico8_hp_po, Chunk 11) is a direct call instead. */

/* function bd()for pt in all(cg.jo)do
 * pt.cc={}
 * end
 * for pt in all(cg.jo)do
 * pt.by=pt:ib()for e in all(pt.by)do
 * for i,w in pairs(e.rl)do
 * w.hg[e.lp[i+1].id]=true
 * end
 * add(e:to().cc,e)end
 * end
 * end
 * (pico8/lua.lua:1265-1275.) Rebuilds every planet's own incoming-trade
 * bookkeeping: first resets every `cg.jo` planet's `.cc` field to a fresh
 * empty list (a NEW heap pico8_list_t each call, freeing whatever a
 * previous pico8_bd() call built for that same planet first, matching this
 * port's established "own previous build freed before rebuilding"
 * convention); then, for every planet, `pt.by=pt:ib()` (`jo:ib()`, Chunk
 * 13 -- forward-ref mv()-dispatched, see this file's own top comment) and,
 * for every trade route `e` reaching it, marks each slipway `w` along the
 * route's own `.hg` set (a REF-to-ob_table used as a boolean membership
 * set) with the id of the NEXT planet in the route's own `.lp` chain
 * (`e.lp[i+1]`, `w`'s own `.hg` field absent -- `jc` not yet built, Chunk
 * 14 -- is a documented no-op write rather than the real Lua's own
 * "index a nil value" crash), then appends `e` onto the route's own
 * DESTINATION planet's `.cc` (`e:to()`, Chunk 3's ALREADY-BUILT
 * `pico8_trade_to`, a direct call). `cg.jo` is empty today (Chunk 13 not
 * yet built), so both loops run zero times in practice -- ported
 * faithfully for structure/compilation, not exercised end-to-end until
 * Chunk 13/14 land, per this PRD's own explicit permission for this
 * exact chunk. Called (twice, via the `repeat...until` loop) from
 * `hy:lj()`. */
void pico8_bd(void);

/* function bb()local jg
 * for pt in all(cg.jo)do
 * local jw=dh(pt)if not pt.mi or jw>pt.mi then
 * if pt.mi==1 and pt.hn.dm then
 * add(pt.jy,{qe=pt.hn.dm})end
 * pt.mi,jg=
 * jw,true
 * end
 * if pt.mi~=0 then
 * local po=pt.hn:po()pt.kw.mu=
 * po[min(pt.mi,#po)]+
 * (pt.jh or 0)end
 * end
 * return jg
 * end
 * (pico8/lua.lua:1276-1290.) Recomputes every planet's own development
 * tier (`.mi`, via pico8_dh below) and, when the tier just increased,
 * refreshes its production output (`.kw.mu`) from its research project's
 * own `:po()` table (`pt.hn`, an `hp` instance -- Chunk 11's ALREADY-BUILT
 * `pico8_hp_po`, a direct call, not mv()-routed). `jg` (whether ANY
 * planet's tier changed this pass -- real Lua leaves it `nil` if none did)
 * is ported as a plain `int` (0/1): the exact VALUE identity of a bare
 * `true`/`nil` is never itself observed by any real call site, only its
 * truthiness (`hy:lj()`'s own `until not bb()`), matching this port's
 * established "simplify a boolean-only-observed-by-truthiness result to
 * plain int" convention (e.g. pico8_oj). Called repeatedly by `hy:lj()`'s
 * own `repeat bd() until not bb()` loop. `cg.jo` empty today -> zero
 * iterations in practice, same inert-but-faithful status as pico8_bd. */
int pico8_bb(void);

/* function dh(pt)
 * if (pt.me~="ly")return 0
 * local jd=1
 * for i in all(pt.jy)do
 * i.gc=
 * #jp(pt.cc,function(im)
 * return im.qe==i.qe
 * end)
 * jd*=i.gc
 * end
 * if (jd==0)return 1
 * if pt.nl then
 * return min(#jp(pt.jy,function(i)
 * return i.gc>0
 * end),3)end
 * if not pt.hf then
 * local li=pt.jy[#pt.jy]
 * return li and min(li.gc+1,4)or 2
 * end
 * return min(4,1+mid(#pt.by,#pt.cc,1))end
 * (pico8/lua.lua:1291-1310.) A planet's development tier: 0 unless
 * `pt.me=="ly"` (its own icon/state name, `ke:ks`'s own `.me` field,
 * Chunk 3). For each of `pt`'s own wants (`.jy`, a Lua list of `{qe=...}`
 * tables), counts how many of `pt.cc`'s own incoming trades (Chunk 13's
 * `.cc`, see pico8_bd above) carry that SAME resource kind (`im.qe==i.qe`
 * -- compared by CONTENT, not pointer identity, since this port's own
 * ob() never interns strings, the same gap already resolved for `ho()`
 * via pico8_gu_content, Chunk 1 -- this file's own private
 * `economy_value_equal_content` sibling), writing the count back onto
 * `i.gc` (a real, observed side effect, not discarded) and multiplying
 * them all together into `jd` (any want with zero matches makes the whole
 * planet's tier collapse to 1). `pt.nl`/`pt.hf` are boolean-ish planet
 * subtype markers (Chunk 15); `min`/`mid` use this file's own private
 * `fixed_min`/pico8_fixed_mid. A NULL `pt` reads every field as its
 * documented default (matching this port's pervasive convention),
 * returning 0 (the `pt.me~="ly"` branch's own result for an absent
 * `.me`). */
pico8_fixed_t pico8_dh(const pico8_instance_t *pt);

#endif
