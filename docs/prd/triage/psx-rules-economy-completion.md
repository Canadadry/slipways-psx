---
title: "rules_economy.c: closing the formula/difficulty/score gaps left open by the Phase 3 reduced slice (Phase 4 step 16)"
description: "Complete rules_economy.c per docs/plan.md Phase 4 step 16 ('formules complètes, niveaux de difficulté, score final'): port dh()'s colony-level-target formula (resolving the Trading Hub dynamic-level gap), gi()/ck()'s tech cost-discount formula (resolving a twice-deferred gap), and bo's 4-entry difficulty table as real data -- while confirming, from source, that ky()'s score formula is already 100% complete from Phase 3 and needs zero new code, that no live 'currently selected difficulty' state is warranted (no observable consumer exists anywhere in this port), and that a predecessor PRD's claim about Trading Hub's `qw` field being a trade-sensitivity override is a citation mistake (it is a sprite palette-swap argument, confirmed by direct trace)."
status: needs-triage
---

## Problem Statement

`docs/plan.md` Phase 4 step 16 calls for `rules_economy.c`: "Compléter `rules_economy.c` (formules complètes, niveaux de difficulté, score final) tels que documentés dans `slipways-mechanics.md`." `docs/prd/psx-rules-economy.md` (the graduated Phase 3 PRD, step 13) already ported a real, host-tested pure module covering trade income, maintenance, overhead, the double-floor net-income aggregation, happiness, score, game-over conditions, and the slipway-cost/affordability formulas `liaison.c` had deferred to it -- but its own Out of Scope and Further Notes sections name several concrete gaps left for "whichever step... most likely a continuation of Phase 4":

- **`bd()`/`dh()`'s live colony-level derivation** (`pico8/lua.lua:1265-1310`) -- deferred outright, because no live per-planet colonization/delivery state exists anywhere in this port.
- **`gi()`'s live cost formula** (`pico8/lua.lua:1471-1474`, `fm.fu`-dependent) -- deferred a *second* time, having already been deferred once by `docs/prd/psx-rules-tech.md` at step 12.
- **A live difficulty-selection mechanic** -- Phase 3's own PRD explicitly named "Phase 4 step 16 ('niveaux de difficulté')" as owning this, while itself only taking `trade_difficulty` as a plain caller-supplied `int`.
- **The `jo:fe`/`jo:hu` colonization-commit gap** -- flagged independently by `docs/prd/psx-rules-planets.md`, `docs/prd/psx-rules-discovery.md`, `docs/prd/psx-effects.md`, and `docs/prd/psx-rules-economy.md` itself, and confirmed by all four as "unowned by any current plan step, Phase 3 or Phase 4."

`docs/prd/psx-structures-and-transforms.md` (Phase 4 step 15, graduated immediately before this step) adds three more named cross-references in its own Further Notes: Food processor's traced `hs=0.5` local-sensitivity override (a real value never captured as a citable constant), a claimed "Trading Hub's own dynamic `qw` sensitivity override," and two dynamic-behavior gaps it explicitly assigns to step 16: **Trading Hub's dynamic level calculation** (§9: level = distinct resources received, not the regular colony formula) and **Synthesizer's `kf()` partner-need-detection logic** (`pico8/lua.lua:1049-1056`).

This PRD resolves each of these from source -- not from the predecessor PRDs' own speculation about what step 16 would find -- and states explicitly, per this task's own instruction, whether "compléter" can stay a pure, host-testable data/formula extension (the precedent every prior structures/tech/planets/economy PRD has followed) or whether difficulty selection or the final score genuinely need a first piece of live game state this port doesn't have yet.

**The key scoping question, resolved from source below**: it can stay pure. Every one of the concretely-portable gaps below (`dh()`'s colony-level formula, `gi()`/`ck()`'s cost-discount formula, `bo`'s difficulty table) is portable as a caller-supplied-parameter pure function, using the exact same "aggregate/count as a plain parameter instead of a live read" idiom Phase 3's own `db()`/`da()`/`bf()`/`ky()` already established. Difficulty selection has **no live consumer anywhere in this port** to justify a "currently selected difficulty" global -- mirroring `docs/prd/psx-rules-tech.md`'s own precedent of introducing a live boolean *only* for state with a real, observable consumer (Select's menu toggle), and declining to for state with none (per-tech-researched status). The final score (`ky()`) turns out to need **zero new code at all** -- Phase 3's `rules_economy_compute_score` already implements the complete formula, verified below against both `pico8/lua.lua`'s own trace and `docs/design/slipways-mechanics.md` §13. The one genuinely unresolved question this step's own research reopens -- `jo:fe`/`jo:hu`'s colonization-commit gap -- is **not** resolved by introducing live state here either, for the same concrete reason Phase 3's PRD already gave (it is a structurally larger, differently-scoped colonization-state-machine undertaking, not a formula `docs/plan.md`'s own step-16 wording names); see Further Notes for why this now deserves a stronger flag than "deferred again."

## Solution

Extend `psx-slipways/rules_economy.h`/`rules_economy.c` in place (no new file, matching every extension-of-an-existing-module precedent in this codebase, e.g. `docs/prd/psx-structures-and-transforms.md`'s own choice for `rules_planets.h`/`.c`): add the colony-level-target formula (`dh()`), the colony-level "never decreases" ratchet (`bb()`'s own update rule), the tech cost-discount formula pair (`ck()`/`gi()`), and the 4-entry difficulty table (`bo`) as a proper accessor-backed struct, plus one traced constant (Food processor's `hs=0.5`). `rules_economy.c` stays a leaf module -- zero new dependency on `rules_planets.c`/`rules_tech.c`/any other `psx-slipways/*.c` file, exactly matching Phase 3's own "simpler than `liaison.c`'s dependency chain" design.

Three genuine, source-verified findings drive this step's scope beyond a mechanical continuation of Phase 3:

1. **`ky()`'s score formula needs no new code.** Re-tracing `ky()` (`pico8/lua.lua:1339-1363`) against `rules_economy_compute_score`'s existing implementation (`rules_economy.c:279-381`) and against `docs/design/slipways-mechanics.md` §13's own bullet list confirms all five fields (`colonies_score`/`population_score`/`tech_score`/`total`/`stars`) are already ported byte-for-byte, including the two subtleties Phase 3 already got right (the `int64_t`-composed `0.01`-multiplication to avoid Q16.16 truncation error, and the floor-toward-negative-infinity correction for a negative `happiness_percent`). "Score final" in step 16's own wording is fully satisfied by work already done; this PRD's only job here is to state that explicitly and cite the confirmation, not to write a formula that already exists.
2. **`dh()` (the colony-level-target formula) is portable as a pure function today, and resolves the Trading Hub dynamic-level gap directly.** Read in full (`pico8/lua.lua:1291-1310`), `dh()`'s branches all operate on values a caller can already supply as plain counts -- per-need delivery counts, a partner-route count, a total-deliveries count -- exactly the same "caller supplies the aggregate `bd()`/`bx()` would have produced" idiom Phase 3's `rules_economy_colony_trade_income`/`rules_economy_happiness` already use for `partner_levels[]`/`colony_levels[]`. Only the *live* stabilization loop that produces those counts (`bd()`/`bb()`'s `repeat...until` fixpoint iteration over real per-planet trade routes) needs state this port doesn't have; `dh()`'s own arithmetic does not. This directly resolves `docs/prd/psx-structures-and-transforms.md`'s "Trading Hub's dynamic level calculation" cross-reference, without needing any live route state to do it.
3. **`docs/prd/psx-structures-and-transforms.md`'s own claim about Trading Hub's `qw` field is a citation mistake, corrected here from a direct trace.** That PRD's Further Notes describe `qw` (`nl`'s own `qw=15` field, `pico8/lua.lua:1061`, dynamically overridden by `function nl:ly(t)... self.qw=8+t%42/6`, `pico8/lua.lua:1063-1066`) as "a second, distinct per-structure trade-sensitivity mechanism beyond Food processor's static `hs=0.5`." Grepping every read site of `.qw` in `pico8/lua.lua` finds exactly one: `jo:kj(p)` (the generic colony/structure sprite-draw method), `dv(self.qw)` (`pico8/lua.lua:546`) -- `dv(no)` (`pico8/lua.lua:142`, `memcpy(0x5f00,0x5000+shl(flr(no),4),16)`) is PICO-8's own draw-palette-swap primitive. `db()`'s trade-income formula (`pico8/lua.lua:1330`, `total+=dk.trade*pt.hs*(...)`) reads only `pt.hs`, never `pt.qw`, at its one and only accumulation site. `qw` is a **cosmetic sprite-palette index**, entirely unrelated to trade economics -- this PRD excludes it from `rules_economy.c`'s scope entirely and recommends the predecessor PRD's Further Notes be read with this correction in mind (Phase 5 sprite-rendering territory, not economy).

The module is **not** wired into `game_init()`/`game_update()`/`game_draw()`, following the same unwired precedent as every prior `rules_economy.c`/`rules_planets.c`/`rules_discovery.c`/`effects.c`/`rules_tech.c` extension -- confirmed below, not assumed, since this is the explicit key scoping question this task asks to resolve.

## User Stories

1. As a developer porting Slipways to PSX, I want it stated explicitly, with a citation trail, that `ky()`'s score formula needs zero new code in this step, so a future reader isn't left wondering whether "score final" in step 16's own wording implies a gap that was silently skipped.
2. As a developer porting Slipways to PSX, I want `dh()` (`pico8/lua.lua:1291-1310`) traced in full and ported as a pure colony-level-target formula -- the "any need unmet → 1" gate, the Trading Hub branch (distinct resources received, capped 3), the single-need-structure branch (Laboratory/Ascension Gate/Synthesizer/Protostar, last need's delivery count + 1, capped 4), and the regular-colony branch (`1 + median(partner routes, total deliveries, 1)`, capped 4) -- so a future live-wiring step has the real, complete level-target math ready to call.
3. As a developer porting Slipways to PSX, I want the Trading Hub branch's own internal simplification (once the "any need unmet" gate has passed, every entry in that same needs array already satisfies `gc>0`, so its own "count of satisfied needs" computation is mathematically always equal to the full needs count) ported literally as source computes it (an independent count, not shortcut to a reused variable), with a discriminating test proving the two are equal here, so structural fidelity to source is preserved even where it's provably redundant -- exactly this module's already-established practice for the double-floor and other "don't simplify away a real source step" findings.
4. As a developer porting Slipways to PSX, I want `bb()`'s "colony level never decreases" ratchet (`pico8/lua.lua:1278-1283`, `if not pt.mi or jw>pt.mi then...`) ported as a small, dedicated pure function, so a future live-wiring step doesn't have to re-derive this one-line rule from `bb()`'s own trace.
5. As a developer porting Slipways to PSX, I want `ck()` (`pico8/lua.lua:1252-1256`) and `gi()` (`pico8/lua.lua:1471-1474`) both ported as pure functions -- the per-resource tech-cost discount factor and its application to a tech's base cost -- so the twice-deferred `gi()`/`fm.fu` gap (first flagged by `docs/prd/psx-rules-tech.md`, deferred a second time by `docs/prd/psx-rules-economy.md`) is finally resolved for real, not deferred a third time.
6. As a developer porting Slipways to PSX, I want it stated explicitly which part of `gi()`/`ck()`'s dependency chain stays out of scope even after this step -- which Laboratories currently produce which resource (needs live per-planet production state, the same `bx()`/`ck()` aggregation gap Phase 3 already named) and each tech's own `t.nk` resource-mapping field (which belongs to `rules_tech.h`'s already-graduated `tech_t` struct, not this leaf module) -- so a future reader doesn't assume this step wires a live science economy.
7. As a developer porting Slipways to PSX, I want `bo` (`pico8/lua.lua:1189-1198`, the 4-entry difficulty table) ported as real, accessor-backed data -- name, trade multiplier, and planet-type-draw bias (`en`) for each of `forgiving`/`reasonable`/`challenging`/`tough` -- rather than staying four loose, disconnected `#define`s, so "niveaux de difficulté" is a genuine, citable, selectable concept a future caller can enumerate, not an implicit convention spread across four macro names.
8. As a developer porting Slipways to PSX, I want it confirmed, from evidence and not by analogy alone, that no live "currently selected difficulty" global is warranted by this step -- no title/difficulty-selection screen exists anywhere in this port's plan (Phase 0-2), mirroring `docs/prd/psx-rules-tech.md`'s own "no live state without a real, observable consumer" precedent for per-tech-researched status -- so a future reader isn't left guessing whether a `g_current_difficulty` global exists somewhere for this step to have wired.
9. As a developer porting Slipways to PSX, I want the `bo` table's `en` field (planet-type-draw bias) captured as real data even though its only consumer (`jo.oc()`/`gn()`) stays unported (`docs/prd/psx-rules-discovery.md`'s own confirmed-out-of-scope finding, unchanged by this step), so a future `rules_discovery.c` extension doesn't have to re-decode `bo` a second time just to find data this step's own research already surfaced.
10. As a developer porting Slipways to PSX, I want `docs/prd/psx-structures-and-transforms.md`'s "Trading Hub's dynamic `qw` sensitivity field" claim checked directly against source and corrected if wrong, rather than carried forward uncritically into this step's own scope -- confirmed wrong (`qw` is a sprite-palette-swap argument, `pico8/lua.lua:546`), with the correction recorded here.
11. As a developer porting Slipways to PSX, I want Food processor's traced `hs=0.5` local-sensitivity value (`pico8/lua.lua:1100`, already cited by Phase 3's own `rules_economy_colony_trade_income` parameter design) captured as a documented `#define` for a future caller, so the one concrete value `docs/prd/psx-structures-and-transforms.md` flagged as "ready for step 16 to wire against" is actually recorded, not merely referenced a second time.
12. As a developer porting Slipways to PSX, I want Synthesizer's `kf()` partner-need-detection logic (`pico8/lua.lua:1049-1056`) re-confirmed, from a direct trace of its own call chain, as needing live partner-connection state this port doesn't have (the same `bd()` stabilization-loop gap), so `docs/prd/psx-structures-and-transforms.md`'s own deferral of this item is verified rather than merely repeated.
13. As a developer porting Slipways to PSX, I want `bb()`'s two live-state-mutation triggers this step's own re-read of `bb()` newly surfaces -- the "auto-add default need at level 1" rule (`pico8/lua.lua:1279-1280`) and `jh`'s one-time "connected to an energy source" production bonus (`jo:cn`, `pico8/lua.lua:479-489`, read back at `pico8/lua.lua:1287`) -- traced and explicitly kept out of scope (both mutate a persistent per-planet needs list this port doesn't have), rather than silently dropped or left as unexplained `Incertain` fields.
14. As a developer porting Slipways to PSX, I want the `jo:fe`/`jo:hu` colonization-commit gap re-confirmed still unowned by any step in `docs/plan.md`'s current Phase 3, 4, 5, or 6 list, with this fact flagged more prominently than the four prior PRDs' own "deferred again" phrasing -- since this is the last `rules_economy.c`-touching step the plan names, and nothing in Phase 5 (graphics/sound) or Phase 6 (hardening) is scoped to introduce new game-logic state -- so whoever plans beyond step 16 sees this as a standing, unresolved project-level gap, not a routine deferral.
15. As a developer porting Slipways to PSX, I want every numeric constant and formula in this extension traced to an exact `pico8/lua.lua` line range, matching the rigor of every prior PSX-side PRD, so a future reader can audit fidelity without re-deriving it from scratch.
16. As a developer porting Slipways to PSX, I want this extension fully host-testable (`gcc`, no PSn00bSDK toolchain) via `make test`, extending the existing `host_tests/rules_economy_test.c`/`test-rules-economy` target rather than adding a new one, matching every prior pure-formula module's convention.
17. As a developer porting Slipways to PSX, I want this extension to compile cleanly as part of the real PSn00bSDK cross-build (`make psx-slipways`) even though nothing calls it yet, matching `rules_economy.c`'s own Phase 3 precedent.

## Implementation Decisions

**`ky()` re-confirmed complete -- no new code.** `rules_economy_compute_score` (`rules_economy.c:279-381`) already implements `colonies_score`/`population_score`/`tech_score`/`total`/`stars` exactly as `ky()` computes them (`pico8/lua.lua:1339-1363`), and `docs/design/slipways-mechanics.md` §13's own five-bullet breakdown matches field-for-field, with no discrepancy found. The two implementation subtleties Phase 3 already handled correctly -- computing `total` via `int64_t` numerator/denominator instead of a truncated `(fixed_t)(0.01*FX_ONE)` literal (avoiding a measured `15991` vs. the exact `16000` error), and flooring toward negative infinity for a negative `happiness_percent`/`total` -- are unchanged by this step. **Nothing is added or modified here.**

**`dh()` (`pico8/lua.lua:1291-1310`) -- ported as a pure colony-level-target formula.**

```
function dh(pt)
  if (pt.me~="ly")return 0
  local jd=1
  for i in all(pt.jy)do
    i.gc=#jp(pt.cc,function(im) return im.qe==i.qe end)
    jd*=i.gc
  end
  if (jd==0)return 1
  if pt.nl then
    return min(#jp(pt.jy,function(i) return i.gc>0 end),3)
  end
  if not pt.hf then
    local li=pt.jy[#pt.jy]
    return li and min(li.gc+1,4)or 2
  end
  return min(4,1+mid(#pt.by,#pt.cc,1))
end
```

- `pt.me~="ly"` (not colonized) → `0`. `not colonized` is a plain caller-supplied `bool colonized` here (the same "no live 'is colonized' flag exists" gap `docs/prd/psx-rules-economy.md`'s original Problem Statement already named for the whole module).
- `jd` is the *product* of every need's delivered-count (`i.gc`, computed by counting `pt.cc` entries matching that need's resource -- `pt.cc` is `bd()`'s own live delivery cache, out of scope; the *count itself* is the caller-supplied input here). `jd==0` iff *any* need has zero deliveries (a product of non-negative integers is zero iff at least one factor is). **Ported as**: a plain caller-supplied `const int *need_delivery_counts, int need_count` array -- mirrors `rules_economy_colony_trade_income`'s existing `partner_levels[]` pattern exactly. `need_count==0` (Protostar, no needs at all) leaves `jd` at its initial `1` (the `for` loop never runs) -- **not** the "any need unmet" case; ported as: the "any unmet" check only fires when `need_count>0` *and* some entry is `0`.
- **Trading Hub branch** (`pt.nl`, `structure_t`'s Trading Hub, `docs/prd/psx-structures-and-transforms.md`): once the "any unmet" gate has passed, `#jp(pt.jy,function(i) return i.gc>0 end)` -- counting entries with `gc>0` -- is *provably* always equal to `need_count` (every entry in the same array already passed the same `gc>0` test the "any unmet" gate itself just verified). **Ported literally as an independent count**, not shortcut to `need_count` directly, matching this module's established "preserve a real source step even where it is provably redundant" practice (the double-floor finding, Phase 3's own precedent) -- with a dedicated test asserting the counted value *equals* `need_count` in this call path, proving the redundancy is understood, not accidental. Capped at `3` (`min(...,3)`).
- **Single-need-structure branch** (`not pt.hf`, e.g. Laboratory/Ascension Gate/Synthesizer/Protostar -- structures with `hf` unset per `docs/prd/psx-structures-and-transforms.md`'s own trace): `li=pt.jy[#pt.jy]` (the *last* need entry) -- `li and min(li.gc+1,4) or 2`. If the needs list is non-empty, `min(last_need_delivery_count+1,4)`; if empty (Protostar, `need_count==0`), a **fixed level 2** -- this is the one branch where `jd==0`'s "any unmet" gate can never fire for an empty-needs entity (there's nothing to be unmet), so an empty-needs structure always lands here at a flat `2`, not `1`. **Ported as**: `need_count==0 ? 2 : min(need_delivery_counts[need_count-1]+1,4)`.
- **Regular-colony branch** (`pt.hf` true, the fallback once neither of the two structure branches matched): `min(4,1+mid(#pt.by,#pt.cc,1))` -- `mid()` is PICO-8's built-in 3-argument median. `#pt.by` (outgoing trade-partner-edge count) and `#pt.cc` (total deliveries received) are both plain caller-supplied `int`s here (`partner_route_count`, `total_deliveries_received`), matching `rules_economy_colony_trade_income`'s own `partner_count` parameter shape. All-integer arithmetic -- no `fixed_t` needed anywhere in `dh()`.
- **Ported as**: a `rules_economy_colony_kind_t` enum (`RULES_ECONOMY_COLONY_KIND_REGULAR`, `_TRADING_HUB`, `_SINGLE_NEED_STRUCTURE`) plus one dispatcher function mirroring `dh()`'s own single-function-with-branches shape (matching `rules_economy_check_game_over`'s own precedent for a branching formula, rather than splitting into three separately-exported branch functions with no shared entry point):

```c
int rules_economy_colony_level(bool colonized, rules_economy_colony_kind_t kind,
                                const int *need_delivery_counts, int need_count,
                                int partner_route_count, int total_deliveries_received);
```

**`bb()`'s "level never decreases" ratchet (`pico8/lua.lua:1278,1281-1283`) -- ported as a small, dedicated pure function.**

```
local jw=dh(pt)if not pt.mi or jw>pt.mi then
  ...
  pt.mi,jg=jw,true
end
```

`jw` (this year's target level, `dh()`'s own return value) only overwrites the colony's persisted level `pt.mi` if `pt.mi` is unset (`not pt.mi`, i.e. never assigned) or the new target is strictly greater. Since `rules_economy_colony_level`'s own "not colonized" branch already returns `0`, and `dh()` never returns `0` for anything colonized (its four branches all return `1` or higher), `0` is a safe, source-faithful stand-in for "unset" -- `target_level > current_level` alone (with `current_level` initialized to `0`) reproduces `not pt.mi or jw>pt.mi` exactly, with no separate "is this the first assignment" boolean needed. **Ported as**:

```c
int rules_economy_colony_level_ratchet(int current_level, int target_level);
```

returning `target_level` if greater than `current_level`, else `current_level` unchanged.

**`ck()`/`gi()` (`pico8/lua.lua:1252-1256`, `1471-1474`) -- the twice-deferred tech cost-discount, resolved as two pure functions.**

```
function ck()local d=cm(1)for l in all(cg.qh)do
  d[l.jy[#l.jy].qe]-=0.15
end
return d
end
...
function gi(t)
  return flr(t.od*max(0.55,fm.fu[t.nk]))
end
```

- `ck()` starts every resource's discount factor at `1`, then subtracts `0.15` per currently-existing Laboratory (`cg.qh`) whose *last* need's resource matches that resource code -- i.e. per Laboratory currently consuming that specific resource (Laboratory's own flexible-need resolution, `docs/prd/psx-structures-and-transforms.md`'s `ba` set, decides *which* resource a given Laboratory ends up consuming; that resolution is itself out of scope, see below). **Ported as** a pure function of a caller-supplied count -- `laboratories_producing_count`, mirroring `rules_economy_colony_maintenance`'s own existing `laboratory_count` parameter shape exactly (though semantically distinct: that one is a *total* Laboratory count for the maintenance formula's `pt.qh` branch; this one is a *per-resource* count, "how many Laboratories currently consume resource X"):

```c
fixed_t rules_economy_tech_discount_factor(int laboratories_producing_count);
/* 1 - 0.15 * laboratories_producing_count, unclamped -- ck() itself never
 * floors or clamps this value; gi() applies the only clamp, at the point
 * of use (see below). */
```

- `gi(t)` applies the discount to a tech's base cost, floored, with a `0.55` floor on the discount factor itself (never cheaper than 55% of base cost, however many Laboratories are stacked on that resource): `floor(base_cost * max(0.55, discount_factor))`. **Ported as**:

```c
int rules_economy_tech_cost(int base_cost, fixed_t discount_factor);
```

`base_cost` composes directly with `rules_tech_get(id)->base_cost` (`rules_tech.h`'s already-graduated `tech_t.base_cost` field) once a caller has both values -- no signature change needed on `rules_tech.h`'s side.

**What stays out of scope in `gi()`/`ck()`'s own dependency chain, and why.** Two pieces remain genuinely unported: (1) **which Laboratories currently produce which resource** -- `l.jy[#l.jy].qe`, each Laboratory's own currently-resolved flexible need -- needs the same live per-planet production/joker-resolution state `bx()`/`ck()`'s full aggregation already needed in Phase 3 (unchanged gap, restated here for this step's own dependency chain, not newly discovered); (2) **each tech's own `t.nk` field** (which of the 6 Laboratory-consumable resources a given tech's cost discount is tied to) -- this is `tech_t`'s own data, not `rules_economy.c`'s; adding it belongs to `rules_tech.h` (an already-graduated module) if a future step chooses to extend it, not to this leaf module, which stays dependency-free of `rules_tech.h` exactly as Phase 3 established.

**`bo` (`pico8/lua.lua:1189-1198`) -- the 4-entry difficulty table, ported as real, accessor-backed data.**

```
bo=ob([[
o(lq=o(49,"forgiving   "),px="forgiving",
trade=7,en=-0.99),
o(lq=o(50,"reasonable  "),px="reasonable",
trade=6,en=-0.6),
o(lq=o(51,"challenging "),px="challenging",
trade=5,en=-0.4),
o(lq=o(52,"tough       "),px="tough",
trade=4,en=-0.2),
]])
```

Matches `docs/design/slipways-mechanics.md` §14's own table exactly (`trade`/`en` for all 4 named levels), no discrepancy. `trade` values are already ported as `RULES_ECONOMY_TRADE_FORGIVING`/`_REASONABLE`/`_CHALLENGING`/`_TOUGH` (Phase 3) -- this step doesn't change those, it adds the *missing two fields* (`px`, the display name, and `en`, the planet-type-draw bias) alongside them in one real, enumerable struct, and cross-checks the existing `trade` `#define`s against the new struct's own `trade` field as a regression guard. **Ported as**:

```c
typedef enum {
    RULES_ECONOMY_DIFFICULTY_FORGIVING,
    RULES_ECONOMY_DIFFICULTY_REASONABLE,
    RULES_ECONOMY_DIFFICULTY_CHALLENGING,
    RULES_ECONOMY_DIFFICULTY_TOUGH,
    RULES_ECONOMY_DIFFICULTY_COUNT,
} rules_economy_difficulty_id_t;

typedef struct {
    const char *name;
    int trade;
    fixed_t en;
} rules_economy_difficulty_t;

const rules_economy_difficulty_t *rules_economy_difficulty(rules_economy_difficulty_id_t id);
```

`en`'s -0.99/-0.6/-0.4/-0.2 literals are not exactly representable in Q16.16, folded via this file's own established double-literal-folding idiom (`g_trade_level_bonus`, Phase 3). **No live "currently selected difficulty" global is introduced** -- see "Live-wiring verdict" below.

**`en`'s only consumer stays unported, unchanged by this step.** `en` feeds exactly one call site in source, `jo.oc,jo.ir=gn(df,2,jf(0.5,dk.en)),jf(0.8,0.5)` (`pico8/lua.lua:750-751`) -- the actual planet-*type* draw (`jo.oc()`), as opposed to the empty-planet roll (`jo.ir()`, already ported, `docs/prd/psx-rules-discovery.md`, explicitly confirmed *independent* of `dk.en`). `gn()`/`jo.oc()` remain unported (`docs/prd/psx-rules-discovery.md`'s own confirmed-out-of-scope finding, restated, not reopened) -- capturing `en`'s real traced values here gives a future `rules_discovery.c` extension the exact data it would need, without this step attempting that extension itself.

**Live-wiring verdict, checked explicitly, not assumed.** `docs/plan.md`'s own step-16 wording names no live mechanism to reuse (unlike step 15's "placement" clause) -- it reads exactly like step 13's own "formules réduites"-style phrasing, just for the *complete* formula set. Source's own live difficulty-selection singleton, `dk` (`pico8/lua.lua:1181`, `function eu(nj) dk=nj`), has **no call site anywhere in this decoded `pico8/lua.lua`** -- `eu()` is defined but never invoked in the source this project has decoded, consistent with a title-screen difficulty-picker flow outside this project's current decode/scope. `docs/plan.md`'s own Phase 0-2 plan (galaxy setup, controller mapping) names no title/difficulty-selection screen anywhere. Mirroring `docs/prd/psx-rules-tech.md`'s own explicit precedent -- a live boolean was introduced *only* for state with a real, observable consumer (Select's menu-toggle, rendered every frame); a live per-tech-researched set was **not** introduced, because nothing in this port could ever read or write it meaningfully -- a live "currently selected difficulty" global here would be exactly that same rejected shape: unobservable, purposeless scaffolding with no consumer anywhere in this port. `trade_difficulty` stays a plain caller-supplied `int` (unchanged from Phase 3); `rules_economy_difficulty()`'s new struct is pure, enumerable data for a future selector to consume, not a live selection state this step owns.

**Module shape and non-wiring, otherwise unchanged.** No new file; `rules_economy.h`/`.c` gain the functions/types above, following the exact same flat, `file(GLOB _sources *.c)`-compiled, pure-data-plus-pure-function convention Phase 3 already established for this module. `host_tests/rules_economy_test.c` and the existing `test-rules-economy` Makefile target are extended, not duplicated -- no new target, no new `*_TEST_HOST_SRCS` variable.

## Data model changes needed

- `rules_economy.h` gains: `rules_economy_colony_kind_t` (3 values); `rules_economy_colony_level(...)`; `rules_economy_colony_level_ratchet(int, int)`; `rules_economy_tech_discount_factor(int)`; `rules_economy_tech_cost(int, fixed_t)`; `rules_economy_difficulty_id_t` (4 values + `_COUNT`); `rules_economy_difficulty_t` struct; `rules_economy_difficulty(rules_economy_difficulty_id_t)`; `#define RULES_ECONOMY_FOOD_PROCESSOR_SENSITIVITY` (traced `0.5`, `pico8/lua.lua:1100`).
- No existing function's signature changes (`rules_economy_trade_level_bonus`, `rules_economy_colony_maintenance`, `rules_economy_overhead`, `rules_economy_trade_edge_income`, `rules_economy_colony_trade_income`, `rules_economy_colony_net`, `rules_economy_annual_net_income`, `rules_economy_happiness`, `rules_economy_compute_score`, `rules_economy_check_game_over`, `rules_economy_slipway_cost_factor`, `rules_economy_slipway_cost`, `rules_economy_can_afford`, `rules_economy_debit` all unchanged).
- No change to `rules_planets.h`/`rules_tech.h` -- this module stays a leaf with zero new dependency, exactly as Phase 3 established.

## Implementation Chunks

Tracer-bullet order, pure formulas only, no wiring -- mirroring `docs/prd/psx-rules-economy.md`'s/`docs/prd/psx-structures-and-transforms.md`'s own chunk shape, scaled to this step's actual (smaller, gap-closing) scope:

1. **Traced constants: the difficulty table and Food processor's sensitivity value.** `rules_economy_difficulty_id_t`/`rules_economy_difficulty_t`/`rules_economy_difficulty()` (the 4-entry `bo` table, all 3 fields per entry); `RULES_ECONOMY_FOOD_PROCESSOR_SENSITIVITY`. No formula logic yet. Tests: each of the 4 difficulty entries' `name`/`trade`/`en` matches the traced table exactly; each entry's own `trade` field equals the corresponding pre-existing `RULES_ECONOMY_TRADE_*` `#define` (a regression guard tying the new struct to Phase 3's own already-tested constants); `RULES_ECONOMY_DIFFICULTY_COUNT==4`; `RULES_ECONOMY_FOOD_PROCESSOR_SENSITIVITY` equals `0.5*FX_ONE` exactly.
2. **The colony-level-target formula (`dh()`), all four branches.** `rules_economy_colony_kind_t`, `rules_economy_colony_level`. Tests: not-colonized → `0` regardless of kind/counts; any-need-unmet (`need_count>0`, some entry `0`) → `1` for all three kinds; Trading Hub with all needs satisfied → `min(need_count,3)` for `need_count` in `{0,1,2,3,4}` (covering the cap); Trading Hub's "counted satisfied == need_count" equality asserted as its own discriminating test; single-need structure with `need_count==0` → fixed `2`; single-need structure with a non-empty needs array → `min(last_delivery_count+1,4)` across a spread of delivery counts including the `4`-cap boundary; regular colony → `min(4,1+median(partner_route_count,total_deliveries_received,1))` for several `(partner_route_count,total_deliveries_received)` pairs including ones where the median is each of the three arguments in turn (exercising `mid()`'s own three-way tie-breaking, not just the common case).
3. **The colony-level ratchet.** `rules_economy_colony_level_ratchet`. Tests: `target>current` → `target`; `target<=current` → `current` unchanged (including `target==current`); `current==0` (first assignment, standing in for `not pt.mi`) with any `target>=1` → `target`.
4. **Tech cost-discount, both functions.** `rules_economy_tech_discount_factor`, `rules_economy_tech_cost`. Tests: `laboratories_producing_count` of `0,1,2,4,7` → `1.0,0.85,0.70,0.40,-0.05` (unclamped, confirming `ck()` itself never floors); `rules_economy_tech_cost` at a discount factor above `0.55` (no clamp fires), at exactly `0.55` (boundary), and below `0.55` (clamp fires, e.g. the `count=7` case above) -- each against a hand-computed `floor(base_cost*max(0.55,factor))` value for a spread of `base_cost`s matching `rules_tech.h`'s own traced `base_cost` range (4-60); an end-to-end test chaining both functions for a representative `(base_cost, laboratories_producing_count)` pair against a single hand-computed reference value.

## Testing Decisions

- **What makes a good test here**: assert on this module's observable output (struct field values, formula return values for given inputs, enum-typed results) -- never on internal representation, matching every prior PRD's own standard in this codebase, including Phase 3's own `rules_economy_test.c`.
- **Fully host-testable, all 4 chunks** (`gcc`, no PSn00bSDK toolchain): every new function takes plain `int`/`fixed_t`/`bool`/array parameters and has zero dependency on any other `psx-slipways/*.c` file beyond `fixed.h`, unchanged from Phase 3's own leaf-module shape. This step **extends** the existing `host_tests/rules_economy_test.c` and `test-rules-economy` Makefile target -- no new target, no new `*_TEST_HOST_SRCS` var, since no new module is created. `test`'s existing prerequisite list (`Makefile:12`) already includes `test-rules-economy`; no change needed there.
- **Regression discipline**: chunk 1's difficulty-table test cross-checks the new struct's `trade` fields against Phase 3's own already-tested `RULES_ECONOMY_TRADE_*` `#define`s, matching this module's and every prior PRD's "re-assert previously-landed values" practice.
- **Not host-testable**: nothing new -- this extension has no PSX-side rendering or input surface, same as every other function already in this module (unwired, like the rest of `rules_economy.c`). No `psx-slipways-*-test-pcsx` target is added; `psx-slipways-boot-test-pcsx` (unmodified) still confirms the real PSn00bSDK cross-build compiles this module in via the flat glob without a live caller.
- **Prior art reused**: `host_tests/t.h`'s PASS/FAIL harness; `rules_economy_test.c`'s own existing "traced constant/formula vs. hand-computed source-derived value" pattern, extended to the new functions; `rules_economy_check_game_over`'s own precedent for a single dispatcher function covering multiple source branches, reused for `rules_economy_colony_level`.

## Out of Scope

- **The `jo:fe`/`jo:hu` colonization-commit mechanic itself.** Re-confirmed, not newly investigated: no step in `docs/plan.md`'s current Phase 3, 4, 5, or 6 list owns it. See Further Notes for why this deserves a stronger flag now than the four prior PRDs' own "deferred again" language.
- **`bd()`/`bb()`'s live stabilization loop and delivery-count computation itself** (`pico8/lua.lua:1265-1290`). Only the *pure math* consuming its output (`dh()`'s level-target formula, ported here) is in scope; the live `repeat...until` fixpoint iteration over real per-planet trade routes needs the same missing colonization/route state `jo:fe`/`jo:hu` would provide.
- **The "auto-add default need at level 1" rule** (`pico8/lua.lua:1279-1280`) and **`jh`'s one-time "connected to an energy source" production bonus** (`jo:cn`, `pico8/lua.lua:479-489`, read back at `1287`). Both traced (not `Incertain`) but both mutate a persistent per-planet needs list this port doesn't have -- live-state mutations, not pure formulas; see Implementation Decisions and User Story 13.
- **Which Laboratories currently produce which resource** (the live input `rules_economy_tech_discount_factor`'s `laboratories_producing_count` parameter stands in for). Needs the same live per-planet production/joker-resolution state `bx()`/`ck()`'s full aggregation already needed in Phase 3 -- unchanged, restated for this step's own dependency chain.
- **Each tech's own `t.nk` resource-mapping field.** Belongs to `rules_tech.h`'s already-graduated `tech_t` struct if a future step extends it; not this leaf module's data to own.
- **`en`'s only consumer, `jo.oc()`/`gn()` (the planet-type draw).** Re-confirmed out of scope, unchanged from `docs/prd/psx-rules-discovery.md`'s own finding -- `en`'s traced values are captured here as data only.
- **A live "currently selected difficulty" global or menu-driven selector.** No observable consumer exists anywhere in this port (no title/difficulty-selection screen in `docs/plan.md`'s Phase 0-2, and source's own `eu()` setter has no call site in this project's decode) -- see Implementation Decisions, "Live-wiring verdict."
- **Synthesizer's `kf()` partner-need-detection logic** (`pico8/lua.lua:1049-1056`) and the shared joker-need-resolution mechanism it overrides (`jo:kf`, `pico8/lua.lua:465-467`, and the wider `ba`/`ht` resolution machinery `docs/prd/psx-structures-and-transforms.md` already captured as static, unapplied data). Needs live partner-connection state from the same `bd()` stabilization loop -- re-confirmed, not newly investigated.
- **Trading Hub's `qw` field.** Confirmed, by direct trace, to be a sprite-palette-swap argument (`jo:kj`, `pico8/lua.lua:546`), not a trade-economics field at all -- excluded from this module's scope entirely, correcting `docs/prd/psx-structures-and-transforms.md`'s Further Notes.
- **Any wiring into `game_init()`/`game_update()`/`game_draw()`.** No caller exists yet for any of this module's functions, old or new; matches every prior unwired precedent in this codebase.
- **Sound/UI feedback, and any live credits/science/era/treasury/needs-list stockpile.** Unchanged from Phase 3's own established position -- no live economy state of any kind is introduced by this module.

## Further Notes

- **This is the last `rules_economy.c`-touching step `docs/plan.md` currently names, and `jo:fe`/`jo:hu` still has no owner.** Four prior PRDs (`rules_planets.c`, `rules_discovery.c`, `effects.c`, `rules_economy.c`'s own Phase 3 PRD) each flagged this gap and deferred it, reasonably, to "a later step." This is that later step, and the honest finding is: it's still not this step's job (the missing prerequisite is a colonization-choice/planet-commit state machine, structurally larger than anything `docs/plan.md`'s own step-16 wording names), **and nothing after step 16 is scoped to introduce it either** -- Phase 5 (steps 17-20) is sprites/audio/juice extraction, Phase 6 (steps 21-22) is hardware-compatibility hardening with an explicit "no rule/mechanic changes" guardrail. If this gap is never assigned a step, the entire live economy/trade/colonization simulation this port has spent five PRDs' worth of pure formulas preparing for can never actually run end-to-end, no matter how complete `rules_economy.c`/`rules_planets.c`/`rules_discovery.c`/`effects.c`'s own formulas become. This is worth a project-level flag for whoever next revises `docs/plan.md`: either a new step needs to be added (most naturally somewhere in Phase 4, alongside or after step 16) to own `jo:fe`/`jo:hu` and the live per-planet colonization/route/needs model it implies, or the plan should explicitly and knowingly accept that this port's "vertical slice" and "clone complet" phases produce a complete *formula library* without a live game loop ever exercising it for real.
- **Where a future colonization-commit step should plug in.** This step's own new functions are ready for it with zero expected signature change: `rules_economy_colony_level`'s `need_delivery_counts[]`/`partner_route_count`/`total_deliveries_received` parameters are exactly the aggregates a live `bd()`/`bb()`-equivalent stabilization loop would produce per planet per year; `rules_economy_colony_level_ratchet` is exactly the one-line update rule such a loop would call once per planet per year; `rules_economy_tech_discount_factor`'s `laboratories_producing_count` parameter is exactly what a live per-planet Laboratory scan (the same one `bx()`/`ck()`'s full aggregation already needs) would supply.
- **The `qw` correction is worth carrying forward.** `docs/prd/psx-structures-and-transforms.md`'s Further Notes should be read with this step's correction in mind: `qw` is Trading Hub's own dynamic sprite-palette argument (a Phase 5 sprite-rendering concern), not a second trade-sensitivity mechanism. Food processor's `hs=0.5` remains the one real, traced, structure-specific sensitivity override in this game -- now captured as `RULES_ECONOMY_FOOD_PROCESSOR_SENSITIVITY`.
- **`ky()` needed zero new code -- worth recording as a genuine finding, not an oversight.** This task's own instructions asked whether `rules_economy_compute_score`'s existing breakdown struct is genuinely complete or missing something per `slipways-mechanics.md`'s own scoring section; the answer, checked directly, is that it is already complete. A reader auditing this PRD's diff against Phase 3's should not expect any change to `rules_economy_compute_score` or its call sites.
- No "Incertain" items were left unresolved by this step's own research: every constant and formula cited above traces to an exact `pico8/lua.lua` line range, and the two fields this step's own re-reading of `bb()` newly surfaced (`dm`-triggered default-need auto-add, `jh`'s energy-connection bonus) are both fully traced and explicitly out of scope for a concrete, stated reason (live-state mutation, not an unread or unclear field) -- not silently dropped.
