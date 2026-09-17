#ifndef RULES_ECONOMY_H
#define RULES_ECONOMY_H

#include "fixed.h"

#include <stdbool.h>

/* This module is a leaf: fixed.h only, no dependency on world.c/cursor.c/
 * liaison.c/rules_planets.c/rules_discovery.c/rules_tech.c. See
 * docs/prd/triage/psx-rules-economy.md, Implementation Decisions ("Module
 * shape and non-wiring, confirmed") -- matches rules_tech.c's own "simpler
 * than liaison.c's dependency chain" precedent.
 *
 * This is chunk 1/7 of that PRD: only the traced starting/ending constants,
 * the trade-difficulty named values, and the trade level-bonus table (`ct`)
 * plus its accessor. The other traced tables this step's Implementation
 * Chunks list names alongside `ct` -- `es` (maintenance cost, chunk 2),
 * `b_` (colony score, chunk 5), `co` (happiness adjustment, chunk 5) -- are
 * NOT declared here: this chunk introduces no public accessor for them (no
 * consumer, no test), so per the PRD's own chunk-1 latitude ("if es/b_/co
 * are simplest as static const arrays inside rules_economy.c -- not exposed
 * via the header -- that's fine too"), they are deferred whole (table plus
 * accessor together) to the chunk that actually consumes each one, keeping
 * this header free of untested public surface. */

/* hy:pq() traced (pico8/lua.lua:1198-1203) -- the economy's initial state.
 * Captured as plain #defines for a future live-wiring step's initializer,
 * NOT a live struct this step owns or mutates -- see the PRD's Implementation
 * Decisions, "Module shape and non-wiring, confirmed" ("no live credits/
 * science/era/treasury stockpile is introduced"). Mirrors rules_tech.h's own
 * precedent of capturing a traced-but-currently-unwired field (`od`) as
 * informational data. */
#define RULES_ECONOMY_STARTING_CREDITS 100 /* nz, pico8/lua.lua:1199 */
#define RULES_ECONOMY_STARTING_SCIENCE 0   /* jk, pico8/lua.lua:1200 */
#define RULES_ECONOMY_STARTING_ERA 1       /* ew, pico8/lua.lua:1200 */
#define RULES_ECONOMY_STARTING_YEAR 3401   /* yr, pico8/lua.lua:1201 */

/* hy.hl's assignment (pico8/lua.lua:1235-1236) -- the two end-condition
 * constants. Unlike RULES_ECONOMY_STARTING_*, these ARE load-bearing: they
 * are used directly inside rules_economy_check_game_over(), added by a
 * later chunk (6), not merely informational. Captured here in chunk 1
 * alongside the other hy:pq()-adjacent constants per the PRD's own
 * grouping. */
#define RULES_ECONOMY_FINAL_YEAR 3426                  /* self.yr>=3426, pico8/lua.lua:1235 */
#define RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD 3  /* self.nz<3, pico8/lua.lua:1236 */

/* dk.trade, one of {7,6,5,4} per bo (pico8/lua.lua:1189-1198), matching
 * design doc SS14 exactly. Difficulty is a plain caller-supplied int, not a
 * selection mechanic this module owns -- see the PRD's Implementation
 * Decisions, "Module shape and non-wiring, confirmed" ("Difficulty is a
 * plain caller-supplied int"). These four named values are documented here
 * for a future caller's convenience; no "currently selected difficulty"
 * global or enum-driven dispatch is introduced. */
#define RULES_ECONOMY_TRADE_FORGIVING 7
#define RULES_ECONOMY_TRADE_REASONABLE 6
#define RULES_ECONOMY_TRADE_CHALLENGING 5
#define RULES_ECONOMY_TRADE_TOUGH 4

/* Per-partner-level trade bonus, traced to `ct` (pico8/lua.lua:1323-1324):
 * ct = {-0.333, 0, 0.25, 0.5} for levels 1-4. Used by db()'s per-edge trade
 * income term (1 + ct[self.level] + ct[partner.level]) -- the edge/colony
 * trade-income functions themselves are a later chunk (3); this chunk only
 * exposes the table lookup itself, per the PRD's chunk-1 scope.
 *
 * level is clamped to [1,4], mirroring rules_planets_curve_value's own
 * clamping idiom (rules_planets.c) exactly -- unlike that function, there is
 * no "level < 1 returns 0" special case here (ct has no natural zero entry),
 * so out-of-range levels clamp to the nearest in-range boundary (level 1's
 * value below the range, level 4's value above it). */
fixed_t rules_economy_trade_level_bonus(int level);

/* Per-colony annual maintenance cost, traced to `da` (pico8/lua.lua:
 * 1312-1318):
 *
 *   es = ob([[1,0,1,2,]])
 *   function da(pt)
 *     return pt.qh and #cg.qh
 *       or pt:gg() and es[pt.mi]
 *       or 0
 *   end
 *
 * Three branches, both real ones ported (per the PRD's explicit "port both
 * branches" instruction, even though no Laboratory type exists in this port
 * yet -- Phase 4 step 15):
 *   - is_laboratory (`pt.qh`, Laboratory-type entity): returns
 *     laboratory_count (`#cg.qh`, the current total Laboratory count in the
 *     galaxy) -- level has no effect on this branch.
 *   - else if colonized (`pt:gg()`, a regular colonized planet): returns
 *     `es[level]` -- `{1,0,1,2}` for levels 1-4 ("content" is the only free
 *     level). level is clamped to [1,4], mirroring
 *     rules_economy_trade_level_bonus's own clamping idiom exactly.
 *   - else: returns 0.
 */
int rules_economy_colony_maintenance(bool is_laboratory, int laboratory_count, bool colonized, int level);

/* Empire-wide annual overhead, traced to `cz` (pico8/lua.lua:1319-1321):
 *
 *   function cz()
 *     local mu = #jp(cg.jo, jo.gg)
 *     return flr(0.18*mu^2)
 *   end
 *
 * `floor(0.18 * colonized_count^2)`, where colonized_count (`mu`) is the
 * number of planets satisfying `jo:gg()` (regular, colonized) -- supplied
 * directly by the caller here, since no live per-planet list exists in this
 * module. Computed with plain integer arithmetic, not fixed_t -- see
 * rules_economy.c's implementation for the empirical comparison against a
 * Q16.16 route showing the integer route is strictly more correct here,
 * not merely simpler. */
int rules_economy_overhead(int colonized_count);

/* One trade edge's contribution to db()'s total, traced to db()'s inner
 * edge-loop body (pico8/lua.lua:1325-1337, specifically the accumulation
 * line at 1333):
 *
 *   function db(pt)
 *     local total=0
 *     for e in all(pt.by) do
 *       local oz=e:to()
 *       if oz.hf then
 *         total += dk.trade * pt.hs * (1+ct[pt.mi]+ct[oz.mi])   -- line 1333
 *       end
 *     end
 *     ...
 *
 * `dk.trade` (trade_difficulty), `pt.hs` (sensitivity, a per-colony "local
 * sensitivity" field, default 1 for a regular colony per the `jo` class,
 * pico8/lua.lua:387), `pt.mi` (self_level) and `oz.mi` (partner_level) are
 * all plain caller-supplied parameters here -- no live route/colonization
 * list exists in this module to derive them from (see the PRD's Implementation
 * Decisions, "Trade income": "the caller supplies the levels of whatever
 * colonies are currently connected and already known to be 'regular' (hf)").
 * `ct` is rules_economy_trade_level_bonus, already ported in chunk 1.
 *
 * Not floored: this is a single edge's fractional contribution, summed with
 * others (and floored only once, per-colony) by rules_economy_colony_net --
 * a later chunk (4), not this one. Returns fixed_t, matching db()'s own
 * fractional intermediate total.
 */
fixed_t rules_economy_trade_edge_income(int trade_difficulty, fixed_t sensitivity, int self_level, int partner_level);

/* A colony's full trade income for the year: every outgoing edge's
 * contribution (rules_economy_trade_edge_income, summed over
 * partner_levels[0..partner_count)) plus the corrected external-trade
 * ("luxury") bonus, traced to db() in full (pico8/lua.lua:1325-1337):
 *
 *   function db(pt)
 *     local total=0
 *     for e in all(pt.by) do
 *       local oz=e:to()
 *       if oz.hf then
 *         total += dk.trade * pt.hs * (1+ct[pt.mi]+ct[oz.mi])
 *       end
 *     end
 *     if pt.kw.qe==r.w then
 *       total += pt.kw.mu * dk.trade * 1.5
 *     end
 *     return total
 *   end
 *
 * **The luxury-bonus correction**: `pt.kw.qe==r.w` -- the design doc's own
 * SS10 prose guesses "energie... en fait la ressource << joker >> globale",
 * but neither guess survives a direct check against the design doc's own
 * Annexe A: `r.w` is `12` and `iz[12]="luxury"` (docs/design/
 * slipways-mechanics.md:304-306) -- neither "energy" (r.e=54) nor the joker
 * (r["?"]=36). The real condition is "this colony's OWN produced resource is
 * luxury", not any external/global resource -- ported here as the caller-
 * supplied `produces_luxury` boolean (standing in for `pt.kw.qe==r.w`) and
 * `luxury_production` (standing in for `pt.kw.mu`, the colony's own
 * production quantity, an aggregate rules_economy.c has no live source for --
 * see the PRD's Implementation Decisions, item (B) on bx()/ck()).
 * `produces_luxury and luxury_production` -- the bonus is
 * `luxury_production * trade_difficulty * 1.5`, added iff produces_luxury is
 * true, unconditional on any trade edge existing (it is a separate `if`
 * block in source, not part of the edge loop above).
 *
 * partner_levels/partner_count stand in for `pt.by`/`e:to()`'s edge walk,
 * already filtered to partners known to be "regular" (`oz.hf`) by the
 * caller -- mirrors effects.c's identical `active`-parameter resolution for
 * the structurally analogous "no live trade state exists" problem (see the
 * PRD's Implementation Decisions, "Trade income").
 *
 * Not floored: matches rules_economy_trade_edge_income's own "not floored
 * here" convention -- the per-colony floor is rules_economy_colony_net's
 * job (a later chunk).
 */
fixed_t rules_economy_colony_trade_income(int trade_difficulty, fixed_t sensitivity, int self_level,
                                           const int *partner_levels, int partner_count, bool produces_luxury,
                                           fixed_t luxury_production);

#endif
