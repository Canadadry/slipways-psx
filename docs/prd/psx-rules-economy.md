---
title: "rules_economy.c: reduced annual economic formulas and end conditions -- closing Phase 3's vertical slice"
description: "Port the trade/maintenance/overhead/happiness/score formulas and the bankruptcy/final-year end conditions from hy:lj (pico8/lua.lua:1226-1238) as pure, caller-supplied functions, converging liaison.c's deferred slipway-cost/affordability formulas and rules_tech.c's traced-but-unwireable gi()/fu dependency, while definitively resolving (as still deferred) the bd()/dh() live trade-computation question three prior PRDs left open at this step's door."
status: needs-triage
---

## Problem Statement

`docs/plan.md` Phase 3 step 13 calls for `rules_economy.c` : "formules réduites (commerce, maintenance, bonheur, score) et conditions de fin (banqueroute, limite de temps)." Phase 4 step 16 explicitly names the full version as future work -- "Compléter `rules_economy.c` (formules complètes, niveaux de difficulté, score final)" -- confirming this is the plan's **third** instance of the "reduced now, full later" pattern already established twice: `rules_planets.c` (3-of-10 types/5-of-~24 jobs at step 8, full roster at step 14) and `rules_tech.c` (4-of-19 techs at step 12, full tree at step 14). Step 13 is the last item on Phase 3's own list, so this PRD also closes out the vertical slice entirely (see Further Notes).

This step is also where two already-triaged PRDs converge, each having done real tracing work rather than guessing -- though, as their own resolutions below make explicit, only one of the two is actually resolved (ported) by this step; the other is correctly re-cited here but stays out of scope:

- `docs/prd/psx-liaison-validity.md` explicitly deferred the slipway cost formula (`dj`/`bk`, `pico8/lua.lua:1259-1264`) and the affordability gate (`fm:lu`/`d_`, `pico8/lua.lua:569-576`,`1217-1224`) to "Phase 3 step 13 (`rules_economy.c`)" -- both are traced but not ported anywhere yet. **This step ports both** (see Implementation Decisions, "Slipway cost" and "Affordability"), resolving that deferral for real.
- `docs/prd/psx-rules-tech.md` traced `hy:kt` (`pico8/lua.lua:1446-1454`) in full, including its call to `self:lj()` -- the exact annual recompute this step's Problem Statement below confirms is the central function step 13 must port -- and separately traced `gi()`'s cost formula (its `fm.fu` discount-factor dependency, `pico8/lua.lua:1471-1474`), deferring both to "whichever step first has a real science economy... most likely a continuation of Phase 4 step 14, once `rules_economy.c` exists" -- a conditional, not a firm assignment to this step. **This step does not port either**: it confirms the `fm.fu` link concretely (see Implementation Decisions, item (B) below, and Out of Scope) but leaves `hy:kt`'s research-commit action and `gi()`'s cost formula out of scope here too, exactly as `rules_tech.c`'s own conditional language anticipated -- a citation correction, not a scope change.
- A third, independently-repeated finding also converges here: `docs/prd/psx-effects.md` flagged `bd()`'s trade computation itself as "most plausibly `rules_economy.c` (Phase 3 step 13) territory... but this PRD does not bind step 13 to that placement"; `docs/prd/psx-rules-planets.md`'s Further Notes made the closely related "most plausibly step 13, not committed here" observation about the `dm`/default_need field rather than `bd()`/`dh()` directly; and `docs/prd/psx-rules-discovery.md` flagged the related-but-distinct colonization-commit gap (`jo:fe`/`jo:hu`) as unowned by any step, without naming step 13 for `bd()`/`dh()` itself. This PRD makes the call, from source (see Implementation Decisions): **`bd()`/`dh()` remain out of scope here too**, and for the same root cause all three prior PRDs converge on -- no live per-planet colonization state exists anywhere in this port (the `jo:fe`/`jo:hu` "commit a colonization job" gap `docs/prd/psx-rules-discovery.md`'s Further Notes flagged has *still* not been assigned an owner by any step in `docs/plan.md`, Phase 3 or Phase 4).

**The key scoping question, resolved from source below**: this port has no live colonization state anywhere (confirmed independently by `rules_planets.c`, `liaison.c`, `rules_discovery.c`, `effects.c`, and `rules_tech.c`'s own PRDs). Every formula `hy:lj()` (the annual economic recompute, `pico8/lua.lua:1226-1238`) computes reads live per-planet state (`cg.jo`, `pt.by`, `pt.mi`, `fm.po`) this port doesn't have. So, exactly like `rules_planets.c`'s own step-8 precedent, "formules réduites" here means **pure functions taking caller-supplied parameters** (colony levels, partner-level lists, production totals, colony/route counts) rather than reading any live global state -- the same "unwired pure module" shape used four times already in this port (`rules_planets.c`, `rules_discovery.c`, `effects.c`, and the pure-core half of `liaison.c`/`rules_tech.c`). Unlike `liaison.c`/`rules_tech.c`, `docs/plan.md`'s own step-13 wording carries **no live-wiring language** ("branché sur..."/"accessible en permanence") -- it reads exactly like step 8's/step 10's/step 11's formula-listing phrasing -- so this step follows the **unwired** precedent, confirmed explicitly below, not the live-wired one.

## Solution

Add `psx-slipways/rules_economy.h`/`rules_economy.c`: a pure, host-testable C module (no PSX headers, no dynamic allocation, Q16.16 fixed-point, the same conventions `rules_planets.c`/`rules_discovery.c`/`rules_tech.c` already established) covering exactly the categories `docs/plan.md`'s step-13 one-liner names -- commerce, maintenance, happiness, score, bankruptcy, time limit -- plus the two formulas `liaison.c` explicitly deferred here (slipway cost, affordability). Every function is traced to `hy:lj()` (`pico8/lua.lua:1226-1238`, the annual recompute) or one of the module-level helper functions it calls (`db`, `da`, `cz`, `bf`, `ky`, all `pico8/lua.lua:1245-1373`), or to `dj`/`bk`/`fm:lu` for the converged slipway-economy pieces.

Two genuine, source-verified findings drive this module's design beyond a mechanical line-for-line port:

1. **`hy:lj()` floors twice, not once** -- each colony's net (`p.kr=flr(db(p)-da(p))`) is floored *individually*, before being summed; the sum is then multiplied by the skill-implants factor and floored *again* (`self.ge=flr(dd*ov(...))-cz()`). This is a real, source-confirmed departure from the design doc's own single-floor prose (§10: "Revenu annuel net : `floor(Σ(...))`," no mention of a per-colony floor) and from this port's own "floor once at the very end" convention `rules_planets_job_cost` established for a *different* formula. Both floors are ported exactly as source has them (see Implementation Decisions), with a dedicated test proving the two-step floor is implemented, not simplified to one.
2. **`fixed_sqrt` (recently added to `fixed.h` for `effects.c`) turns out to be exactly what `dj()`'s slipway-cost formula needs**, confirming the task's own "doesn't obviously need sqrt, but check" prompt was worth checking: `dj`'s `(#(to.p-pb.p)/bq)^0.75` is a fractional power with no `fixed_pow` anywhere in this codebase, but `x^0.75 = x^(1/2) \cdot x^(1/4) = \sqrt{x}\cdot\sqrt{\sqrt{x}}$, computable as two chained `fixed_sqrt` calls plus one `fixed_mul` -- no new fixed-point primitive needed. `bk()`'s own `sqrt(#cg.jc)` (an integer route-count square root) needs the same primitive again.

A third finding, independent of any of the above and worth flagging with equal weight: **the design doc's own parenthetical guess about `db()`'s external-trade bonus resource is wrong, and this PRD corrects it from the project's own already-decoded data.** §10's prose reads "`kw.qe==énergie`… en fait la ressource « joker » globale" (i.e., the author first guessed "energy," then second-guessed themselves to "the global joker resource"). Neither guess survives a direct check against the design doc's *own* Annexe A: `r.w` (the code the source condition `pt.kw.qe==r.w` actually compares against, `pico8/lua.lua:1333`) is `12` per Annexe A's `r` dump, and `iz[12]="luxury"` per Annexe A's `iz` dump (`docs/design/slipways-mechanics.md:304-306`) -- neither "energy" (`r.e=54`) nor the joker (`r["?"]=36`). The ×1.5 bonus in `db()` fires when a colony's own produced resource is **luxury**, full stop; this PRD ports that condition correctly, cites the exact Annexe A rows that settle it, and recommends (Further Notes) the design doc's own prose be corrected.

The module is deliberately **not wired into `game_init()`/`game_update()`/`game_draw()`**, following `rules_planets.c`'s/`rules_discovery.c`'s/`effects.c`'s unwired precedent exactly, for the reason stated in the Problem Statement (no live-wiring language in the plan's own step-13 wording, and no live colonization/credit/trade state anywhere in this port for it to read). Like those three, it compiles into the real PSn00bSDK cross-build via the existing flat `file(GLOB _sources *.c)` glob with zero caller.

## User Stories

1. As a developer porting Slipways to PSX, I want a `rules_economy.c` module that exists and compiles today, so that whichever future step wires a live economy has real, source-traced trade/maintenance/happiness/score formulas to build against instead of inventing placeholders.
2. As a developer porting Slipways to PSX, I want `hy:lj()` (`pico8/lua.lua:1226-1238`) traced in full -- the stabilization loop it runs first (`bd()`/`bb()`), the per-resource totals it computes (`bx()`/`ck()`), and the five formulas it calls (`db`/`da`/`cz`/`bf`/`ky`) -- so this step's scope decisions (which pieces are ported, which are deferred) are demonstrably grounded in the complete function, not a partial reading of it.
3. As a developer porting Slipways to PSX, I want `hy:pq()` (`pico8/lua.lua:1198-1203`, the economy's initial state) traced and its starting values captured as documented, traced constants, so a future live-wiring step has them on record without re-decoding source, even though no live economy state exists yet for them to initialize.
4. As a developer porting Slipways to PSX, I want the trade-income formula (`db`, `pico8/lua.lua:1325-1337`) ported exactly, including the per-partner level-bonus table (`ct`, `{-0.333,0,0.25,0.5}`) and the corrected external-trade bonus condition (luxury, not energy or the joker), so a future caller gets the real formula, not an approximation of the design doc's own imprecise prose.
5. As a developer porting Slipways to PSX, I want the design doc's `kw.qe==r.w` misidentification (guessed "energy," then "joker," actually "luxury") documented with an exact citation to the design doc's own Annexe A rows, so a future reader sees this as a settled correction, not an open question.
6. As a developer porting Slipways to PSX, I want the maintenance formula (`da`, `pico8/lua.lua:1312-1318`) ported with both its branches -- the Laboratory self-count cost and the regular-colony per-level cost table (`es`, `{1,0,1,2}`) -- even though no Laboratory type exists in this port yet (Phase 4 step 15), so the formula is complete and tested, not half-ported around a currently-unreachable branch.
7. As a developer porting Slipways to PSX, I want the empire-overhead formula (`cz`, `pico8/lua.lua:1319-1321`, `floor(0.18 × count²)`) ported as a plain function of a colonized-planet count, so later steps can compute it the instant they have a real count, without re-deriving the formula.
8. As a developer porting Slipways to PSX, I want the annual net-income aggregation ported with its **real** double-floor shape (per-colony `flr(db-da)`, then a second `flr` after the skill-implants multiplier, then overhead subtracted), not the design doc's single-floor simplification, so a future caller's numbers match the shipped game exactly, including cases where the two floors produce a different total than a single end-floor would.
9. As a developer porting Slipways to PSX, I want a test that specifically exercises a case where per-colony flooring changes the final result versus flooring only once at the end, so the double-floor finding is proven by an executable assertion, not just narrated in this PRD.
10. As a developer porting Slipways to PSX, I want the happiness formula (`bf`, `pico8/lua.lua:1365-1373`, `100 + 2×(joy+luxury) + Σ per-level adjustment`) ported uncapped, exactly as source computes it (no artificial clamp to 100%), so a future caller sees the same wide swings the source game produces.
11. As a developer porting Slipways to PSX, I want the score formula (`ky`, `pico8/lua.lua:1339-1363`) ported with its full breakdown (colonies, population×40, tech, total, stars), matching source's own returned tuple shape, so a future scoreboard/end-screen can display the same categories the original game does.
12. As a developer porting Slipways to PSX, I want the end-condition check (`hy.hl`'s assignment, `pico8/lua.lua:1235-1236`) ported as a pure function of year/net-income/treasury, with the final-year check taking precedence over bankruptcy exactly as source's own `and`/`or` short-circuit does, so a simultaneous "year 3426 AND bankrupt" case resolves identically to source.
13. As a developer porting Slipways to PSX, I want the slipway-cost formula (`dj`/`bk`, `pico8/lua.lua:1259-1264`) ported here as `docs/prd/psx-liaison-validity.md` explicitly asked, using two chained `fixed_sqrt` calls for the `x^0.75` term, so `liaison.c`'s own deferred cost math finally has a real, tested implementation to call.
14. As a developer porting Slipways to PSX, I want the affordability gate (`fm:lu`, `pico8/lua.lua:1217-1224`, via `d_()`, `pico8/lua.lua:569-576`) ported as a pure predicate (`treasury >= cost`) plus a pure debit computation, so `liaison.c`'s own deferred `fm:lu`/`d_` reference finally has a real function to call once a live treasury exists.
15. As a developer porting Slipways to PSX, I want `hy:kt`'s dependency chain checked directly against this step's own new `ck()` trace, so the concrete, source-backed link between this step and `rules_tech.c`'s deferred `gi()` is made explicit: `gi()` needs `fm.fu`, and `fm.fu` is exactly what this step's `hy:lj()`-derived `ck()` computes -- even though this step doesn't call `gi()` or `ck()` itself.
16. As a developer porting Slipways to PSX, I want it stated explicitly whether `hy:kt` (the tech-research commit action) is ported by this step, with the reasoning recorded, so a reader isn't left wondering whether "converges rules_tech.c's deferral" means this step actually researches techs (it does not).
17. As a developer porting Slipways to PSX, I want the `bd()`/`dh()` question -- flagged as "most plausibly step 13" by three independent prior PRDs -- resolved definitively here, with the concrete missing-state reason spelled out (no live colonization-commit state anywhere in the plan, Phase 3 or 4), so this recurring open question is finally closed rather than passed along a fourth time.
18. As a developer porting Slipways to PSX, I want it stated plainly that this step introduces no live credits/science/era/treasury stockpile, matching the "no unobservable placeholder state" reasoning `rules_tech.c`'s own PRD already applied to per-tech-researched status, so a future reader isn't left guessing whether a `g_economy_state` global exists somewhere to feed real values into (it does not).
19. As a developer porting Slipways to PSX, I want the economy's traced starting/ending values (100 starting credits, year 3401→3426, era 1, 0 starting science) captured as plain, documented `#define`s even without a live struct to initialize, mirroring `rules_tech.c`'s precedent of capturing `od` (a traced-but-currently-unwired field) as informational data, so a future live-wiring step doesn't have to re-decode `hy:pq()`.
20. As a developer porting Slipways to PSX, I want the trade-difficulty constant (`dk.trade`, one of `{7,6,5,4}` per `bo`, `pico8/lua.lua:1189-1198`) taken as a plain caller-supplied `int` parameter rather than resolved from any live "current difficulty" selector, so the commerce formula is complete and testable across all four values without inventing a difficulty-selection mechanic that Phase 4 step 16 ("niveaux de difficulté") explicitly owns.
21. As a developer porting Slipways to PSX, I want the per-colony "local sensitivity" field (`hs`, default `1` for every regular colony per the `jo` class, `pico8/lua.lua:387`) taken as a plain caller-supplied `fixed_t` parameter rather than hardcoded to `1`, so the trade-income formula stays complete for the day a Phase-4 structure with a different `hs` (e.g. the Food-processor-like entity at `hs=0.5`, `pico8/lua.lua:1096`) needs it, without a signature change.
22. As a developer porting Slipways to PSX, I want every numeric constant and formula in this module traceable to an exact `pico8/lua.lua` line range, matching the rigor of every prior PSX-side PRD, so a future reader can audit fidelity without re-deriving it from scratch.
23. As a developer porting Slipways to PSX, I want this module fully host-testable (`gcc`, no PSn00bSDK toolchain) via `make test`, matching every other pure-logic module in this codebase.
24. As a developer porting Slipways to PSX, I want this module to compile cleanly as part of the real PSn00bSDK cross-build (`make psx-slipways`) even though nothing calls it yet, matching `rules_planets.c`'s/`rules_discovery.c`'s own precedent.
25. As a developer picking up Phase 4 step 16, I want this step's exact deferrals (full difficulty selection, `hy:kt`'s live research commit, `bd()`/`dh()`'s live colony simulation, `gi()`/`ck()`'s live application) documented with their concrete missing-prerequisite state named, so step 16 knows exactly what still needs building, not just "more of the same formulas."
26. As a developer reading this PRD later, I want it noted explicitly that this closes Phase 3 (`docs/plan.md`'s vertical slice) entirely, so the significance of this step's completion is visible without cross-referencing the plan document separately.

## Implementation Decisions

**`hy:lj()` traced in full (`pico8/lua.lua:1226-1238`) -- the annual economic recompute, and exactly which of its pieces this step ports.**

```
function hy:lj()
  repeat bd() until not bb()          -- (A) colony-level/trade stabilization loop
  self.po,self.fu = bx(),ck()         -- (B) per-resource production totals / discount factors
  local dd=0
  for p in all(cg.jo) do
    p.kr = flr(db(p) - da(p))         -- (C) per-colony net, FLOORED HERE (first floor)
    dd += p.kr
  end
  self.ge = flr(dd*ov("skill implants",1,1.15)) - cz()   -- (D) net income, FLOORED AGAIN
  self.happiness = bf()               -- (E) happiness
  self.hl = self.yr>=3426 and "final score"
            or self.ge<=0 and self.nz<3 and "bankrupt"   -- (F) end condition
  for e in all(cv.hb) do e:hb() end   -- (G) notify score-panel-open listeners, UI-only
end
```

- **(A) `bd()`/`bb()`** (`pico8/lua.lua:1265-1275`/`1276-1290`, plus `dh()`, `1291-1310`): a fixed-point-iteration loop that recomputes every colony's trade-partner cache (`pt.cc`), delivery-derived level (`pt.mi`, via `dh()`), and production (`pt.kw.mu`) until nothing changes (`bb()` returns falsy). **Not ported by this step** -- traced in full below, "The `bd()`/`dh()` question, resolved" -- it needs a live per-planet colonization/needs/delivery model this port has nowhere in its codebase, not even a "planet is colonized" boolean.
- **(B) `bx()`/`ck()`** (`pico8/lua.lua:1245-1251`/`1252-1256`): `bx()` sums `pt.kw.mu` (a colony's own production quantity) into a per-resource-code total (`self.po`), over every planet in `cg.jo`; `ck()` builds the per-resource discount table `fm.fu` that `gi()` (tech cost) reads, starting every resource at `1` and subtracting `0.15` per Laboratory (`cg.qh`) whose last-produced resource matches. **Not ported by this step**: both need the same live per-planet production/structure state (A) needs. This is the concrete link to `rules_tech.c`'s deferred `gi()`: `fm.fu` (what `ck()` produces) is exactly what `gi()` reads, so a future step porting `ck()` here would hand `gi()` its missing input directly -- but that step isn't this one, because `ck()` itself needs live structure/production state this port doesn't have. This step instead takes the aggregated **outputs** `bx()`/`ck()` would have produced (a colony's own production total for a resource, a joy/luxury production total) as plain caller-supplied parameters to the formulas below.
- **(C)/(D) the per-colony net and annual net income**: **ported by this step**, including both floors, exactly as source has them (see "The double-floor finding" below).
- **(E) `bf()`**: **ported by this step** (see "Happiness").
- **(F) the end condition**: **ported by this step** (see "End conditions").
- **(G) `cv.hb` listeners**: a UI event-notification list (score-panel refresh), no formula content; not applicable to a pure module with no UI.

**`hy:pq()` traced (`pico8/lua.lua:1198-1203`) -- the economy's initial state, captured as documented constants, not live state.**

```
function hy:pq() qj(self, ob([[
nz=100, ge=0,
jk=0, ew=1,
invented=o(),
lo=1, yr=3401,
]])) end
```

`nz` (treasury) starts at `100`; `ge` (net income) at `0`; `jk` (science) at `0`; `ew` (era) at `1`; `invented` (researched-tech set) empty; `lo` (elapsed months this year) at `1`; `yr` (current year) at `3401`. Ported as plain `#define`s (`RULES_ECONOMY_STARTING_CREDITS`, `RULES_ECONOMY_STARTING_SCIENCE`, `RULES_ECONOMY_STARTING_ERA`, `RULES_ECONOMY_STARTING_YEAR`) for a future live-wiring step's initializer -- **not** a live struct this step owns or mutates, per the "no unobservable placeholder state" resolution below. `RULES_ECONOMY_FINAL_YEAR` (`3426`) and `RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD` (`3`, from `self.nz<3`) are the two starting-state-adjacent constants that ARE load-bearing -- used directly inside `rules_economy_check_game_over` below, not merely informational.

**Trade income (`db`, `pico8/lua.lua:1325-1337`) -- ported exactly, including the corrected external-trade-bonus resource.**

```
function db(pt)
  local total=0
  for e in all(pt.by) do
    local oz=e:to()
    if oz.hf then
      total += dk.trade * pt.hs * (1+ct[pt.mi]+ct[oz.mi])
    end
  end
  if pt.kw.qe==r.w then
    total += pt.kw.mu * dk.trade * 1.5
  end
  return total
end
```

- Per active outgoing trade edge to a partner flagged `oz.hf` (a "regular colony," per the `jo` class default `hf=1` at `pico8/lua.lua:387` -- structures deviate, e.g. the Food-processor-like entity at `pico8/lua.lua:1096` sets `hs=0.5` but this port has no structures yet, Phase 4 step 15): `+ trade_difficulty × self.hs × (1 + level_bonus[self.level] + level_bonus[partner.level])`. `ct` (`pico8/lua.lua:1323-1324`) is `{-0.333, 0, 0.25, 0.5}` for levels 1-4.
- **External-trade bonus, corrected**: `pt.kw.qe==r.w` -- `r.w` is `12` (Annexe A's `r` dump, `docs/design/slipways-mechanics.md:304`), and `iz[12]="luxury"` (Annexe A's `iz` dump, `docs/design/slipways-mechanics.md:305`). The design doc's own §10 prose guesses "énergie… en fait la ressource « joker » globale" -- both wrong: `r.e=54` (energy) and `r["?"]=36` (joker) are distinct codes from `r.w=12`. **The real condition is "this colony's own produced resource is luxury"**, and the bonus is `+ production_quantity × trade_difficulty × 1.5`, unconditional on any trade edge existing.
- **Ported as**: `fixed_t rules_economy_trade_edge_income(int trade_difficulty, fixed_t sensitivity, int self_level, int partner_level)` (one edge's contribution) and `fixed_t rules_economy_colony_trade_income(int trade_difficulty, fixed_t sensitivity, int self_level, const int *partner_levels, int partner_count, bool produces_luxury, fixed_t luxury_production)` (the full per-colony sum, summing edges then adding the luxury bonus iff `produces_luxury`). `level_bonus` (`ct`) is exposed as `fixed_t rules_economy_trade_level_bonus(int level)`, levels clamped to `[1,4]` matching `rules_planets_curve_value`'s established clamping idiom. `partner_levels`/`partner_count` stand in for `pt.by`/`e:to()` -- the caller supplies the levels of whatever colonies are currently connected and already known to be "regular" (`hf`), since no live route/colonization list exists here to filter that condition itself (mirrors `effects.c`'s identical `active`-parameter resolution for the structurally analogous "no live trade state exists" problem).

**Maintenance (`da`, `pico8/lua.lua:1312-1318`) -- both branches ported.**

```
es = ob([[1,0,1,2,]])
function da(pt)
  return pt.qh and #cg.qh
    or pt:gg() and es[pt.mi]
    or 0
end
```

`pt.qh` (truthy only for Laboratory-type entities, none exist in this port yet -- Phase 4 step 15) costs the **current total Laboratory count** (`#cg.qh`); a regular colonized planet (`pt:gg()`, i.e. `pt.hf and pt.me=="ly"`) costs `es[level]` (`{1,0,1,2}` for levels 1-4 -- "content" is the only free level, matching the design doc exactly); anything else costs `0`. **Ported as** `int rules_economy_colony_maintenance(bool is_laboratory, int laboratory_count, bool colonized, int level)` -- both branches implemented and tested even though `is_laboratory` has no real caller passing `true` yet (mirrors `rules_planets_job_cost`'s precedent of porting a formula's full branch logic ahead of a live caller for one of its branches).

**Empire overhead (`cz`, `pico8/lua.lua:1319-1321`).**

```
function cz()
  local mu = #jp(cg.jo, jo.gg)
  return flr(0.18*mu^2)
end
```

`floor(0.18 × colonized_count²)`, where `colonized_count` is the number of planets satisfying `jo:gg()` (regular, colonized). **Ported as** `int rules_economy_overhead(int colonized_count)`.

**The double-floor finding -- both floors ported exactly, a genuine departure from this module's own single-floor precedent elsewhere in this port.**

- `p.kr = flr(db(p)-da(p))` (`pico8/lua.lua:1230-1231`) floors **each colony's** net **before** summing -- `db`/`da` can both be fractional (`ct`'s `0.25`/`0.5`/`-0.333` bonuses), so this per-colony floor is a real truncation step, not a no-op. **Ported as** `int rules_economy_colony_net(fixed_t trade_income, int maintenance)`.
- `self.ge = flr(dd * ov("skill implants",1,1.15)) - cz()` (`pico8/lua.lua:1233`) floors the **sum of already-floored integers** times the skill-implants multiplier (`1` or `1.15`), then subtracts the (already-integer) overhead. **Ported as** `int rules_economy_annual_net_income(const int *colony_nets, int colony_count, bool skill_implants_researched, int overhead)`.
- This differs from `docs/design/slipways-mechanics.md` §10's own prose ("Revenu annuel net : `floor(Σ(revenu - maintenance) × mult) - frais_généraux`," a *single* floor over the raw sum) and from this port's own "floor once at the very end" fidelity convention `rules_planets_job_cost` established for a *different* formula (chained fixed-point multiplications, one floor). Both are correct in their own context -- `rules_planets_job_cost`'s multiplications never have an intermediate source-level floor between them, while `hy:lj()`'s do. A dedicated test constructs colony net values where per-colony flooring changes the final `annual_net_income` result relative to a hypothetical single end-floor, proving this port reproduces source's actual two-floor arithmetic rather than the design doc's simplified prose.

**Happiness (`bf`, `pico8/lua.lua:1365-1373`) -- ported uncapped.**

```
co = ob([[-5,0,1,1,]])
function bf()
  local h = 100 + 2*(fm.po[r.h]+fm.po[r.w])
  for pt in all(cg.jo) do
    if pt:gg() then h += co[pt.mi] end
  end
  return h
end
```

`r.h` is `25` (Annexe A) → `iz[25]="joy"`; `r.w` is `12` → `iz[12]="luxury"` (the same code identified above for `db()`'s bonus -- both `bf()` and `db()` key off `r.w`="luxury", one more piece of the same already-decoded table this PRD reuses rather than re-guessing). `co` (`{-5,0,1,1}` for levels 1-4) is the per-colony adjustment. **Ported as** `fixed_t rules_economy_happiness(fixed_t joy_production, fixed_t luxury_production, const int *colony_levels, int colony_count)` -- `fixed_t`, not `int`, since nothing in source floors `h`, and no cap at `100` (matching the design doc's own explicit "n'est pas plafonné" note, now confirmed directly against the un-floored, uncapped source arithmetic).

**Score (`ky`, `pico8/lua.lua:1339-1363`) -- ported with the full breakdown.**

```
b_ = ob([[0,100,200,400,]])
function ky()
  local di=0
  for pt in all(cg.jo) do
    if pt:gg() then di+=b_[pt.mi] end
  end
  local gl = fm.po[r.p]*40
  local el = flr((#fm.invented*1.2)^2)*10
  local total = flr((di+gl+el)*0.01*fm.happiness)
  return {di, gl, el, fm.happiness.."%", total, min(flr(total/2000),5)}
end
```

`r.p` is `19` → `iz[19]="people"` (matches `RESOURCE_PEOPLE` in `rules_planets.h`, produced by `JOB_HIVEWORLD`). **Ported as**:

```c
typedef struct {
    int colonies_score;
    int population_score;
    int tech_score;
    int total;
    int stars;
} rules_economy_score_t;

rules_economy_score_t rules_economy_compute_score(const int *colony_levels, int colony_count,
                                                    int population, int tech_invented_count,
                                                    fixed_t happiness_percent);
```

`colonies_score = Σ b_[level]` (`{0,100,200,400}` for levels 1-4); `population_score = population*40`; `tech_score = floor((tech_invented_count*1.2)²)*10`; `total = floor((colonies_score+population_score+tech_score)*0.01*happiness_percent)`; `stars = min(floor(total/2000), 5)`. `happiness_percent` is exactly `rules_economy_happiness`'s return value passed straight in (source's own `fm.happiness` field, set once per year by `bf()` inside `hy:lj()`), keeping the two functions composable without a shared live state.

**End conditions (`hy.hl`'s assignment, `pico8/lua.lua:1235-1236`) -- ported with source's own short-circuit precedence.**

```
self.hl = self.yr>=3426 and "final score"
          or self.ge<=0 and self.nz<3 and "bankrupt"
```

Lua's `and`/`or` chain short-circuits left to right: if `yr>=3426`, `hl` is `"final score"` **immediately**, and the bankruptcy clause is never evaluated -- final year wins outright over a simultaneous bankruptcy, not merely "checked first." Otherwise, `hl` is `"bankrupt"` only if **both** `ge<=0` (net income) **and** `nz<3` (treasury) hold; otherwise `hl` is `false` (the game continues). **Ported as**:

```c
typedef enum {
    RULES_ECONOMY_GAME_CONTINUES = 0,
    RULES_ECONOMY_GAME_OVER_FINAL_YEAR,
    RULES_ECONOMY_GAME_OVER_BANKRUPT,
} rules_economy_game_over_t;

rules_economy_game_over_t rules_economy_check_game_over(int year, int net_income, int treasury);
```

using `RULES_ECONOMY_FINAL_YEAR` (`3426`) and `RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD` (`3`) from `hy:pq()`'s trace above. A test exercises `year>=3426` **and** a simultaneously-bankrupt `net_income`/`treasury` pair, asserting `RULES_ECONOMY_GAME_OVER_FINAL_YEAR` wins, matching source's short-circuit exactly.

**Slipway cost (`dj`/`bk`, `pico8/lua.lua:1259-1264`) -- converging `liaison.c`'s deferral, needs `fixed_sqrt` twice.**

```
bn, bq = 6, 0.0977
function dj(pb,to) local ec = (#(to.p-pb.p)/bq)^0.75
  return flr(bn*bk()*max(0.5,ec)) end
function bk()
  return max(1, sqrt(#(cg.jc or {}))*0.27-0.1) end
```

- `#(to.p-pb.p)` is squared distance (`km:__len`, the same "`#v` is squared, not linear" fact `world.h`/`liaison.h` already rely on). `bq=0.0977` is a plain divisor -- since `fixed.h` has no `fixed_div`, this is ported as a multiplication by a compile-time reciprocal constant (`RULES_ECONOMY_SLIPWAY_COST_DIVISOR_RECIP = (fixed_t)((1.0/0.0977) * (double)FX_ONE)`), the same "fold a double-literal constant at compile time" idiom `liaison.h`'s `LIAISON_RANGE_SQ`/`LIAISON_BLOCK_TOLERANCE_SQ` already use, not a general-purpose division primitive this codebase doesn't otherwise need.
- `x^0.75` (`x` = squared-distance-over-`bq`) has no `fixed_pow` anywhere in this codebase. **This is exactly what `fixed_sqrt` (added to `fixed.h` for `effects.c`'s segment-length calculation) turns out to solve**: `x^0.75 = x^{1/2} \cdot x^{1/4} = \sqrt{x} \cdot \sqrt{\sqrt{x}}$ -- `fixed_t s1 = fixed_sqrt(x); fixed_t s2 = fixed_sqrt(s1); fixed_t ec = fixed_mul(s1, s2);` computes it exactly, no new primitive required. `max(0.5, ec)` clamps the result.
- `bk()`'s `sqrt(#cg.jc)` is a **second**, independent use of `fixed_sqrt` in this same formula -- `#cg.jc` is a plain Lua table-length (the count of existing slipways), not a vector `#`, converted to `fixed_t` (`count << 16`) before `fixed_sqrt`. `max(1, sqrt(count)*0.27-0.1)`.
- `dj(pb,to) = floor(bn * bk() * max(0.5, ec))`, `bn=6`, floored once at the very end (matching this port's usual convention here, unlike `hy:lj()`'s own double floor above -- these are two independently-traced source functions, not the same formula, and each is ported exactly as its own source floors it).
- **Ported as**: `fixed_t rules_economy_slipway_cost_factor(int existing_route_count)` (`bk`) and `int rules_economy_slipway_cost(fixed_t distance_sq, int existing_route_count)` (`dj`). `distance_sq` is the same squared-distance quantity `liaison_check`/`liaison.h`'s constants already use, so a future live-wiring step can pass `liaison.c`'s own computed squared distance directly; `existing_route_count` mirrors `liaison.c`'s `route_count` parameter shape (currently always `0` from its permanently-empty route list, same as every other caller-supplied count in this port today).

**Affordability (`fm:lu`/`d_`, `pico8/lua.lua:1217-1224`/`569-576`) -- converging `liaison.c`'s deferral, split into a pure gate and a pure debit.**

```
function hy:lu(jt) if self.nz<jt then iv({"-no money-",8}) return end
  self.nz-=jt
  if jt~=0 then iv({ne(-jt,10,"$")}) end
  return true
end
```

The gate (`self.nz<jt` → refuse) and the mutation (`self.nz-=jt`) are two different concerns; the `iv(...)` calls are UI toast/sound feedback, out of scope (no UI-text/SPU system exists). **Ported as** `bool rules_economy_can_afford(int treasury, int cost)` (`treasury >= cost`, source's negated condition) and `int rules_economy_debit(int treasury, int cost)` (`treasury - cost`, valid only when `rules_economy_can_afford` already returned `true` -- the caller's responsibility, matching `d_()`'s own `if not self.et and fm:lu(...)` short-circuit ordering).

**The `bd()`/`dh()` question, resolved: still deferred, and here is exactly why now that this step exists to answer it.**

- `dh(pt)` (`pico8/lua.lua:1291-1310`) derives a colony's level from **live delivery counts** (`pt.cc`, populated by `bd()`, `pico8/lua.lua:1265-1275`, itself driven by `pt:ib()`'s trade-partner walk). `bb()` (`pico8/lua.lua:1276-1290`) applies that level to production. None of `pt.cc`/`pt.by`/`pt.jy`/`pt.mi`/`pt.me` exist anywhere in this port's codebase -- not in `world.c`'s `galaxy_t` (positions only), not in `rules_planets.c` (pure data/formula tables, no per-planet instances), not in `liaison.c` (a permanently-empty route list of plain index pairs, no resource/level data), not in `rules_discovery.c` (a pure cost/range/roll module with no planet-state ownership), not in `effects.c` (an `active` boolean supplied by the *caller*, explicitly because no such live state exists -- `docs/prd/psx-effects.md`'s own words).
- **Resolution**: `bd()`/`dh()` remain out of scope for this step, same as the three prior PRDs that flagged this question anticipated might be the outcome ("most plausibly... but not committed"). The reason is concrete, not a restatement of "no live state exists": porting `dh()`/`bb()` faithfully would require first inventing an entire live per-planet colonization/needs/trade-partner data model (a "which planets are colonized, with what job, needing what, connected to whom" structure) that spans far beyond "formules réduites (commerce, maintenance, bonheur, score)" -- it would mean **this** step silently absorbing the still-unassigned `jo:fe`/`jo:hu` colonization-commit mechanic `docs/prd/psx-rules-discovery.md`'s Further Notes already flagged as owned by no step in the current plan, Phase 3 or Phase 4. That is a strictly larger, differently-scoped undertaking than what step 13's own plan wording names, and inventing it here -- unasked, unnamed by the plan -- would be exactly the kind of scope creep this project's "traçabilité"/"pas de contenu retiré silencieusement" guardrails exist to catch in the other direction (adding unplanned content), not just to prevent dropping planned content.
- **This closes the loop three PRDs opened**: `rules_planets.c`, `rules_discovery.c`, and `effects.c` each independently deferred this same question to "most plausibly step 13." This PRD's answer, now that step 13 actually exists to give one, is: **not here either**, because the missing prerequisite (`jo:fe`/`jo:hu`'s live colonization-commit state) is structurally a different, still-unowned piece of work, not a formula this step's own category list names. The gap remains flagged, not filled -- see Further Notes.

**Module shape and non-wiring, confirmed.**

- Every function above takes caller-supplied levels/counts/booleans/production totals as plain parameters -- no `struct` this module defines is mutable process-wide state, matching `rules_planets.c`'s/`rules_discovery.c`'s established "generic, caller-supplies-the-state" shape exactly.
- **No live credits/science/era/treasury stockpile is introduced.** The task's own framing asks this to be checked, not assumed: `rules_tech.c`'s PRD already established the precedent for exactly this situation (a real live boolean was introduced for the menu's open/closed state, because it was *directly observable and had a real live consumer*; a live per-tech-researched state was **not** introduced, because nothing could ever set it to anything but a caller-supplied value with no live consumer to observe it). This step has **no live consumer at all** (unwired, unlike `rules_tech.c`'s Select-driven menu) -- introducing a `g_economy_state` global that nothing reads or writes meaningfully would be exactly the "unobservable, purposeless scaffolding" `rules_tech.c`'s own reasoning already rejected for a structurally identical case. The traced starting/ending values (`RULES_ECONOMY_STARTING_*`, `RULES_ECONOMY_FINAL_YEAR`) are captured as plain `#define`s for a future step to consume, exactly as `rules_tech.c` captured `od` (traced but unwired) -- not as a live struct this module owns.
- `rules_economy.c` has no dependency on `world.c`/`cursor.c`/`liaison.c`/`rules_planets.c`/`rules_discovery.c`/`rules_tech.c` -- it is a leaf module (`fixed.h` only), matching `rules_tech.c`'s own "simpler than `liaison.c`'s dependency chain" precedent. `liaison.c`/`rules_tech.c` are expected to call **into** `rules_economy.c` once a future step wires live state (`liaison.c`'s drag-commit for `rules_economy_slipway_cost`/`rules_economy_can_afford`; a future tech-research action for the general net-income/happiness/score path once `bd()`/`dh()` exist) -- no such call is added by this step.
- **File layout, `psx-slipways/` (flat, `file(GLOB _sources *.c)`, matching every prior unwired module's precedent exactly)**: `rules_economy.h`/`rules_economy.c` (the module), `host_tests/rules_economy_test.c` (host tests).
- **Difficulty is a plain caller-supplied `int`, not a selection mechanic.** `dk.trade` (one of `{7,6,5,4}` per `bo`, `pico8/lua.lua:1189-1198`, matching design doc §14 exactly) is taken as `int trade_difficulty` on `rules_economy_trade_edge_income`/`rules_economy_colony_trade_income`. The four named values are documented as `#define`s (`RULES_ECONOMY_TRADE_FORGIVING`=7, `_REASONABLE`=6, `_CHALLENGING`=5, `_TOUGH`=4) for a future caller's convenience, but no "currently selected difficulty" global or enum-driven dispatch is introduced -- `docs/plan.md` Phase 4 step 16 explicitly names "niveaux de difficulté" as its own scope (the selection mechanic and its effects on *other* systems, e.g. `rules_discovery.c`'s still-deferred `jo.oc()`/`gn` type-draw bias `en`), and this step's job is only to make the commerce formula itself complete and correct for whichever value a future caller supplies.

## Implementation Chunks

Tracer-bullet order, pure formulas only, no wiring -- mirroring `rules_planets.c`'s/`rules_discovery.c`'s unwired chunk shape:

1. **Traced static tables and simple constants.** `rules_economy.h` gains `ct`/`es`/`b_`/`co` as static `const` tables (trade level bonus, maintenance cost, colony score, happiness adjustment, levels 1-4), plus `RULES_ECONOMY_FINAL_YEAR`, `RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD`, `RULES_ECONOMY_STARTING_*`, `RULES_ECONOMY_TRADE_*`, and `rules_economy_trade_level_bonus(int level)`. Host-tested: each table's values match the traced source exactly for levels 1-4; out-of-range levels clamp to `[1,4]`.
2. **Maintenance and overhead.** `rules_economy_colony_maintenance` (`da`, both branches), `rules_economy_overhead` (`cz`). Tests: Laboratory branch returns `laboratory_count` regardless of `level`; regular-colonized branch returns `es[level]`; neither-condition returns `0`; overhead matches hand-computed `floor(0.18*n²)` for several `n`.
3. **Trade income.** `rules_economy_trade_edge_income`, `rules_economy_colony_trade_income` (`db`, including the corrected luxury bonus). Tests: single-edge income for every `(self_level, partner_level)` combination in `[1,4]×[1,4]`; multiple edges sum correctly; the luxury bonus applies only when `produces_luxury` is true and scales with `luxury_production`; all four `trade_difficulty` values exercised.
4. **Net income aggregation -- the double-floor test.** `rules_economy_colony_net`, `rules_economy_annual_net_income`. Tests: per-colony flooring matches `flr(db-da)` for fractional inputs; a constructed multi-colony case where summing pre-floored integers gives a different total than flooring the raw fractional sum once at the end (proving the two-floor shape is real, not simplified); the skill-implants multiplier (`1`/`1.15`) applies before the second floor; overhead subtracts after.
5. **Happiness and score.** `rules_economy_happiness` (`bf`, uncapped, `fixed_t`), `rules_economy_compute_score` (`ky`, full breakdown struct). Tests: happiness base `100` plus joy/luxury/per-level terms, including values both above and below `100`; each score component (`colonies_score`/`population_score`/`tech_score`) matches hand-computed values; `total`/`stars` match source's own formula including the `min(...,5)` cap.
6. **End conditions.** `rules_economy_check_game_over`. Tests: `year<3426` and `net_income>0` (or `treasury>=3`) → `RULES_ECONOMY_GAME_CONTINUES`; `net_income<=0 && treasury<3 && year<3426` → `RULES_ECONOMY_GAME_OVER_BANKRUPT`; `year>=3426` alone → `RULES_ECONOMY_GAME_OVER_FINAL_YEAR`; `year>=3426` **and** a simultaneously-bankrupt pair → `RULES_ECONOMY_GAME_OVER_FINAL_YEAR` (final year wins, matching source's short-circuit).
7. **Slipway cost and affordability -- converging `liaison.c`'s deferral.** `rules_economy_slipway_cost_factor` (`bk`, using `fixed_sqrt` on an integer route count), `rules_economy_slipway_cost` (`dj`, using the two-chained-`fixed_sqrt` `x^0.75` trick), `rules_economy_can_afford`, `rules_economy_debit`. Tests: `bk()` at `existing_route_count=0` clamps to the `max(1,...)` floor; `bk()` increases monotonically with route count; `dj()` matches hand-computed values (via a double-precision reference computation) within a documented fixed-point tolerance for a spread of distances and route counts, mirroring `fixed_sqrt`'s own existing `EPSILON`-tolerance test convention (`host_tests/rng_and_fixed_test.c`); `can_afford`/`debit` cover the boundary (`treasury==cost` affords exactly) and refusal cases.

## Testing Decisions

- **What makes a good test here**: assert on this module's observable output (constant/table values, formula return values for given inputs, the `rules_economy_score_t`/`rules_economy_game_over_t` fields) -- never on internal representation, matching every prior PRD's own standard.
- **Fully host-testable** (all chunks, `gcc`, no PSn00bSDK toolchain): every function in this module takes plain `int`/`fixed_t`/`bool`/array parameters and has zero dependency on any other `psx-slipways/*.c` file (only `fixed.h`, header-only) -- simpler than even `rules_tech.c`'s dependency chain. New `host_tests/rules_economy_test.c`, new `RULES_ECONOMY_TEST_HOST_SRCS` (`psx-slipways/rules_economy.c psx-slipways/host_tests/rules_economy_test.c`) and `test-rules-economy` target, compiled and run with the host's native `gcc` (`-std=c11 -Wall -Wextra -I psx-slipways`), output under the existing shared `psx-slipways/build-host/` directory -- matching `test-rules-tech`'s minimal-dependency shape exactly.
- `test`'s prerequisite list gains `test-rules-economy` (alongside the existing `test-planetgen test-galaxy-render test-cursor test-rules-planets test-liaison test-rules-discovery test-effects test-rules-tech`), and `test-rules-economy` is added to `.PHONY`, keeping `CLAUDE.md`'s "use `make test` to verify changes" promise intact.
- **Fixed-point tolerance for `dj`/`bk`**: unlike every other formula in this module (integer or exact-fraction arithmetic), `dj`'s `x^0.75` term involves two `fixed_sqrt` calls, each already carrying `fixed_sqrt`'s own documented irrational-rounding tolerance (`host_tests/rng_and_fixed_test.c`'s `EPSILON=0.001` convention for `test_fixed_sqrt_irrational_within_tolerance`). This step's own `dj`/`bk` tests compare against a `double`-computed reference value within an implementation-chosen tolerance band (wider than `0.001` alone, since two chained `fixed_sqrt` calls compound rounding) -- the exact band is an empirical, implementation-time choice, matching this project's established precedent (`docs/plan.md` Phase 1 step 2's BFS-connectivity-rate tolerance, `rules_discovery.c`'s empty-roll Monte-Carlo tolerance) for "bounded empirically, not analytically" cases, not invented arbitrarily here.
- **Not host-testable**: nothing -- this module has no PSX-side rendering or input surface at all (unwired, like `rules_planets.c`/`rules_discovery.c`/`effects.c`). No `psx-slipways-*-test-pcsx` target is added; `psx-slipways-boot-test-pcsx` (unmodified) still confirms the real PSn00bSDK cross-build compiles this module in via the flat glob without a live caller.
- **Prior art reused**: `host_tests/t.h`'s PASS/FAIL harness; `rules_planets_test.c`'s/`rules_discovery_test.c`'s pattern of testing traced constants and formulas against hand-computed source-derived values; `rng_and_fixed_test.c`'s `EPSILON`-tolerance convention for `fixed_sqrt`-derived results, extended here to `dj`/`bk`.

## Out of Scope

- **`bd()`/`dh()`'s live colony-level derivation and trade-partner computation** (`pico8/lua.lua:1265-1310`). Needs a live per-planet colonization/needs/delivery model this port has nowhere -- the still-unassigned `jo:fe`/`jo:hu` colonization-commit gap `docs/prd/psx-rules-discovery.md`'s Further Notes already flagged. Resolved definitively here as still deferred (see Implementation Decisions); not assigned to any specific future step by this PRD either, since the missing prerequisite is a colonization-commit mechanic, not an economy formula.
- **`bx()`/`ck()`'s live per-planet resource aggregation** (`pico8/lua.lua:1245-1256`). Needs the same live per-planet production/structure state `bd()`/`dh()` need. This step's formulas take the aggregated totals `bx()`/`ck()` would have produced as plain caller-supplied parameters instead.
- **`hy:kt`, the tech-research commit action** (`pico8/lua.lua:1446-1454`, debiting `fm.jk`, marking `fm.invented`, advancing `fm.ew`, calling `self:lj()`). Already fully traced by `docs/prd/psx-rules-tech.md`; not part of `docs/plan.md`'s step-13 wording (which names commerce/maintenance/bonheur/score/end-conditions, not "recherche"). This step supplies the real `hy:lj()` recompute `hy:kt` would eventually call, but does not itself perform a research commit.
- **`gi()`'s live cost formula** (`pico8/lua.lua:1471-1474`, `fm.fu`-dependent). `ck()` (this step's own deferred piece, above) is what would produce `fm.fu`; without it, `gi()` stays exactly as unwireable as `docs/prd/psx-rules-tech.md` already found it.
- **A live difficulty-selection mechanic and its effects on other systems** (the `en` planet-type-draw bias, `rules_discovery.c`'s still-deferred `jo.oc()`/`gn` type draw). `docs/plan.md` Phase 4 step 16 ("niveaux de difficulté") owns this explicitly; this step only makes the commerce formula itself correct for a caller-supplied `trade_difficulty` value.
- **A live credits/science/era/treasury stockpile of any kind.** No `rules_economy.c`-owned mutable global is introduced; see Implementation Decisions, "Module shape and non-wiring, confirmed."
- **`hy:nx`/`hy:hk`'s month-accumulation and annual-tick-triggering mechanism** (`pico8/lua.lua:1204-1216`) -- the live clock/scheduler that decides *when* to call `hy:lj()`, as opposed to `hy:lj()`'s own formulas (ported here). No live "elapsed months" counter exists anywhere in this port; `docs/prd/psx-rules-discovery.md` already ported the generic months-cost constant (`RULES_DISCOVERY_PROBE_COST_MONTHS`) without needing a live clock either.
- **The full 10-type/full-job planet roster, the 7 non-colonizable structures, and the 4 transform/destroy actions.** Unaffected by this step; still `docs/plan.md` Phase 4 steps 14/15, per `rules_planets.c`'s own PRD.
- **Any wiring into `game_init()`/`game_update()`/`game_draw()`.** No caller exists yet for any of this module's functions; matches `rules_planets.c`'s/`rules_discovery.c`'s/`effects.c`'s own unwired precedent.
- **Sound/UI feedback** (`iv(...)`'s "-no money-" toast, `hy:lu`'s `$`-amount popup, `rn(...)` sound cues). No SPU/audio or UI-text system exists yet (Phase 5).
- **The colonization-commit mechanic itself** (`jo:fe`/`jo:hu`). Still flagged, still unowned by any step in the current plan (Phase 3 or Phase 4) -- see Further Notes.

## Further Notes

- **This closes Phase 3 entirely.** `docs/plan.md`'s "Vertical slice" phase (steps 8-13) is now fully covered by a PRD at every step: `rules_planets.c` (8), `liaison.c` (9), `rules_discovery.c` (10), `effects.c` (11), `rules_tech.c` (12), and this PRD (13). Phase 4 ("Clone complet," steps 14-16) is the next phase the plan names, and every deferral this PRD makes (full difficulty, `hy:kt`'s live research, `bd()`/`dh()`'s live colony simulation) points there, per `docs/plan.md`'s own step 14/15/16 wording.
- **The `bd()`/`dh()` question is closed, but the underlying gap it depends on is not.** Three prior PRDs (`rules_planets.c`, `rules_discovery.c`, `effects.c`) each independently deferred this question to "most plausibly step 13." This PRD's answer is that it doesn't belong here either -- the real blocker is `jo:fe`/`jo:hu` (choosing and committing a colonization job), which `docs/prd/psx-rules-discovery.md`'s Further Notes already flagged as owned by **no step** in `docs/plan.md`'s current Phase 3 or Phase 4 list. With Phase 3 now fully covered and Phase 4's three named steps (14: types/tech, 15: structures/transforms, 16: full economy/difficulty/final score) all pointing at extending *existing* reduced slices rather than introducing colonization-commit itself, this gap is worth a project-level flag for whoever scopes Phase 4's steps concretely: **something** in Phase 4 needs to actually wire `jo:fe`/`jo:hu`-equivalent state before `bd()`/`dh()`, live trade-flow (`effects.c`'s `active` flag), or a live economy (this step's formulas, called with real values instead of test fixtures) can ever run for real. This PRD does not resolve it, consistent with the three PRDs before it -- it only narrows down definitively that it is *not* this step's job.
- **The `r.w`="luxury" correction applies to both `db()` and `bf()`.** This PRD's Annexe-A-sourced correction (`r.w=12="luxury"`, not "energy" or the joker) resolves the design doc's §10 external-trade-bonus guess, and the same code is independently confirmed by `bf()`'s own, unambiguous use of `r.w` for its "luxury production" term -- two source call sites agreeing on the same resource code is stronger evidence than either alone. Recommended, as a small separate follow-up (not part of this PRD, consistent with every prior PSX-side PRD's own practice of flagging design-doc corrections without making them here): `docs/design/slipways-mechanics.md` §10 could drop its "énergie… en fait la ressource « joker »" parenthetical and state plainly that the external-trade bonus fires on luxury production, citing Annexe A's own already-decoded `r`/`iz` rows.
- **Where Phase 4 step 16 should plug in**: `rules_economy_trade_edge_income`'s `trade_difficulty` parameter and the four named `RULES_ECONOMY_TRADE_*` constants are ready for a real difficulty-selection mechanic to supply; `rules_economy_colony_maintenance`'s `is_laboratory`/`laboratory_count` parameters are ready for Phase 4 step 15's Laboratory structure; `rules_economy_slipway_cost`/`rules_economy_can_afford`/`rules_economy_debit` are ready for `liaison.c`'s drag-commit once colonization gating exists there. None of this step's function signatures are expected to change when any of these land -- only real call sites need to be added.
- **Why `fixed_sqrt` was worth checking explicitly**: `fixed.h`'s own comment on `fixed_sqrt` frames it as added for `effects.c`'s segment-length calculation, with no mention of any other planned consumer. This PRD's research confirms it has a second, independent real use here (`dj`'s `x^0.75` via two chained calls, `bk`'s own direct `sqrt` on a route count) -- worth recording so a future reader doesn't assume `fixed_sqrt` is `effects.c`-only infrastructure that happens to be reusable, when it is in fact exactly the primitive two independent formulas in this codebase both need.
- No "Incertain" items were surfaced by this PRD's research that couldn't be resolved directly from source: every constant and formula cited above traces to an exact `pico8/lua.lua` line range or to the design doc's own already-decoded Annexe A tables, and the two scoping questions the task posed (does this step introduce a live credits/science stockpile; is `bd()`/`dh()` finally in scope here) are both resolved with concrete, source-backed "no" answers above, not left open.
