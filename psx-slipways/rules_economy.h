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

/* Food processor's traced local-sensitivity override, `hs=0.5`
 * (pico8/lua.lua:1100). Already cited by rules_economy_colony_trade_income's
 * own `sensitivity` parameter design above (Phase 3), but never captured as
 * a citable constant until this chunk. 0.5 is exactly representable in
 * Q16.16 (0.5*65536=32768, no rounding), unlike the difficulty table's `en`
 * literals below. */
#define RULES_ECONOMY_FOOD_PROCESSOR_SENSITIVITY ((fixed_t)(0.5 * FX_ONE))

/* `bo` (pico8/lua.lua:1189-1198), the 4-entry difficulty table, ported as
 * real, accessor-backed data:
 *
 *   bo=ob([[
 *   o(lq=o(49,"forgiving   "),px="forgiving",
 *   trade=7,en=-0.99),
 *   o(lq=o(50,"reasonable  "),px="reasonable",
 *   trade=6,en=-0.6),
 *   o(lq=o(51,"challenging "),px="challenging",
 *   trade=5,en=-0.4),
 *   o(lq=o(52,"tough       "),px="tough",
 *   trade=4,en=-0.2),
 *   ]])
 *
 * `trade` matches the already-ported RULES_ECONOMY_TRADE_FORGIVING/
 * REASONABLE/CHALLENGING/TOUGH #defines exactly (cross-checked by this
 * module's own tests as a regression guard, not merely duplicated). `en` is
 * the planet-type-draw bias (`dk.en`); its only consumer, `jo.oc()`/`gn()`
 * (the planet-type draw), stays unported -- see
 * docs/prd/triage/psx-rules-economy-completion.md. `-0.99`/`-0.6`/`-0.4`/
 * `-0.2` are not exactly representable in Q16.16, folded from their traced
 * double literals via this file's own established literal-folding idiom
 * (g_trade_level_bonus, rules_economy.c).
 *
 * `name` (`px`): source's own raw string literal carries PICO-8's own
 * fixed-width-string padding for its own menu UI (`lq`'s second field, e.g.
 * "forgiving   ", trailing spaces to a fixed column) -- that padding is
 * PICO-8's own display mechanism, not semantically part of the name, so it
 * is trimmed here to the clean string ("forgiving", "reasonable",
 * "challenging", "tough"), matching `px`'s own already-unpadded value in
 * source (the SAME name appears twice in source: once padded inside `lq`
 * for PICO-8's menu column, once as the clean `px` field this struct actually
 * ports).
 *
 * No live "currently selected difficulty" global is introduced by this
 * chunk: `trade_difficulty` stays a plain caller-supplied `int` everywhere
 * else in this module (unchanged from Phase 3); this struct is pure,
 * enumerable data for a future selector to consume. */
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

/* A colony's net income for the year, FLOORED -- the FIRST of this module's
 * two floors (see rules_economy_annual_net_income below for the second),
 * traced to `p.kr = flr(db(p) - da(p))` (pico8/lua.lua:1230-1231, inside
 * hy:lj()'s per-colony loop):
 *
 *   local dd=0
 *   for p in all(cg.jo) do
 *     p.kr = flr(db(p) - da(p))
 *     dd += p.kr
 *   end
 *
 * This per-colony floor is a genuine, source-verified departure from the
 * design doc's own SS10 prose ("Revenu annuel net : floor(SUM(revenu -
 * maintenance) x mult) - frais_generaux", a SINGLE floor over the raw sum)
 * and from this port's own "floor once at the very end" fidelity convention
 * (rules_planets_job_cost's chained multiplications) -- see the PRD's
 * Implementation Decisions, "The double-floor finding." Source floors EACH
 * colony's (trade_income - maintenance) individually, before summing, not
 * once at the very end.
 *
 * `trade_income` (db(p), fixed_t, from rules_economy_colony_trade_income)
 * can be fractional -- ct's -0.333/0.25/0.5 per-level bonuses guarantee it
 * routinely is. `maintenance` (da(p), already an int, from
 * rules_economy_colony_maintenance) is widened to fixed_t before
 * subtracting. The fractional difference is then floored to an int HERE --
 * not truncated toward zero: db(p)-da(p) can be negative (a colony can cost
 * more in maintenance than it earns in trade), and flooring a negative
 * fractional fixed_t must round toward negative infinity (flr(-0.3) = -1),
 * not toward zero (which would wrongly give 0) -- see rules_economy.c's
 * fixed_floor_to_int for the implementation and its correctness argument
 * for both signs.
 */
int rules_economy_colony_net(fixed_t trade_income, int maintenance);

/* Annual net income across the whole empire -- the SECOND of this module's
 * two floors (see rules_economy_colony_net above for the first), traced to
 * `self.ge = flr(dd*ov("skill implants",1,1.15)) - cz()`
 * (pico8/lua.lua:1233, inside hy:lj()):
 *
 *   local dd=0
 *   for p in all(cg.jo) do
 *     p.kr = flr(db(p) - da(p))
 *     dd += p.kr
 *   end
 *   self.ge = flr(dd*ov("skill implants",1,1.15)) - cz()
 *
 * This floor operates over the SUM of already-floored integers (`dd`, i.e.
 * colony_nets here, each one rules_economy_colony_net's own return value)
 * -- not over a raw fractional sum. Ported: sum colony_nets[0..colony_count)
 * as plain ints; multiply by the skill-implants factor (`FX_ONE` normally,
 * `1.15*FX_ONE` if skill_implants_researched -- ov("skill implants",1,1.15)'s
 * own "unresearched -> base value" shape, the same caller-supplied-boolean
 * pattern rules_planets_job_cost already established for
 * job_tech_researched/nanomaterials_researched, rules_planets.c -- no live
 * tech-researched state exists anywhere in this port, per rules_tech.c's own
 * precedent); floor the scaled sum to an int AGAIN; then subtract overhead
 * (already an int, from rules_economy_overhead) AFTER this second floor,
 * exactly matching source's own `- cz()` placement outside the `flr(...)`.
 *
 * Why a naive single-floor-at-the-end implementation would be WRONG:
 * flooring each colony's net individually, then summing the (now-integer)
 * results, is NOT the same arithmetic as summing the raw fractional
 * per-colony nets first and flooring only once at the very end. Concrete
 * example (this module's own discriminating test,
 * host_tests/rules_economy_test.c, uses exactly this): three colonies each
 * with a fractional (trade_income - maintenance) of 0.4 (fixed_t). Source's
 * real double-floor shape: flr(0.4)=0 for EACH colony
 * (rules_economy_colony_net), summed as already-floored ints ->
 * dd=0+0+0=0; with no skill-implants bonus and zero overhead,
 * annual_net_income = flr(0*1) - 0 = 0. A hypothetical SINGLE floor over
 * the raw sum instead (i.e. summing the fractional per-colony nets BEFORE
 * any floor, then flooring once): 0.4+0.4+0.4=1.2 (fixed_t), floored once
 * -> 1 -- a different, WRONG answer (1, not source's real 0) that a
 * single-floor implementation would silently produce. This module's test
 * asserts the real double-floor result (0, via three
 * rules_economy_colony_net calls followed by this function) AND asserts it
 * explicitly differs from that hand-computed single-floor alternative (1),
 * proving the real double-floor shape is implemented, not simplified to
 * one floor (per the PRD's own User Story #9).
 *
 * colony_nets holds the ALREADY-FLOORED per-colony net values (i.e. each
 * one is rules_economy_colony_net's own return value) -- this function does
 * not re-derive them from fractional inputs; it only sums, scales, floors
 * again, and subtracts overhead, exactly as source's own `dd` accumulator
 * (already summing floored p.kr values) does.
 */
int rules_economy_annual_net_income(const int *colony_nets, int colony_count, bool skill_implants_researched, int overhead);

/* Empire happiness for the year, traced to `bf` (pico8/lua.lua:1365-1373):
 *
 *   co = ob([[-5,0,1,1,]])
 *   function bf()
 *     local h = 100 + 2*(fm.po[r.h]+fm.po[r.w])
 *     for pt in all(cg.jo) do
 *       if pt:gg() then h += co[pt.mi] end
 *     end
 *     return h
 *   end
 *
 * `r.h` is `25` (Annexe A) -> `iz[25]="joy"`; `r.w` is `12` -> `iz[12]=
 * "luxury"` (the same code identified in rules_economy_colony_trade_income's
 * doc comment above -- both bf() and db() key off r.w="luxury", one more
 * piece of the same already-decoded table this module's own PRD reuses
 * rather than re-guessing, per the PRD's Implementation Decisions,
 * "Happiness"). `joy_production`/`luxury_production` stand in for
 * `fm.po[r.h]`/`fm.po[r.w]` -- the aggregated per-resource production totals
 * `bx()` would have produced, taken here as plain caller-supplied parameters
 * for the same reason rules_economy_colony_trade_income's own
 * luxury_production parameter is (see the PRD's Implementation Decisions,
 * item (B) on bx()/ck() -- no live per-planet production state exists in
 * this module).
 *
 * `co` (`{-5,0,1,1}` for levels 1-4) is the per-colonized-colony adjustment,
 * summed over colony_levels[0..colony_count) -- every entry stands in for a
 * colony satisfying `pt:gg()` (regular, colonized), matching this module's
 * established "caller already filtered to the relevant colonies" convention
 * (rules_economy_colony_trade_income's partner_levels). level is clamped to
 * [1,4], mirroring rules_economy_trade_level_bonus's own clamping idiom
 * exactly.
 *
 * Returns fixed_t, NOT int: nothing in source floors `h` (bf() returns a
 * plain Lua number, added to directly and later multiplied by 0.01 inside
 * ky(), never passed through flr() anywhere in its own body or at any of its
 * call sites). Deliberately UNCAPPED: no clamp to 100 is applied here,
 * matching both source's own un-floored, uncapped arithmetic above (there is
 * no min()/max() anywhere in bf()) and the design doc's own explicit
 * "n'est pas plafonne" ("is not capped") note on this exact value -- so a
 * heavy joy/luxury economy can push happiness well above 100, and enough
 * unhappy (level-1) colonies can push it below 100, even negative.
 */
fixed_t rules_economy_happiness(fixed_t joy_production, fixed_t luxury_production, const int *colony_levels, int colony_count);

/* ky()'s own returned tuple shape, traced to `ky()` (pico8/lua.lua:
 * 1339-1363) -- see rules_economy_compute_score below for the full formula.
 * Field order matches ky()'s own `{di, gl, el, fm.happiness.."%", total,
 * min(flr(total/2000),5)}` return, minus the happiness-percent-as-string
 * field (fm.happiness is rules_economy_happiness's own return value, already
 * available to the caller as the fixed_t it passed in -- not duplicated
 * here).
 */
typedef struct {
    int colonies_score;
    int population_score;
    int tech_score;
    int total;
    int stars;
} rules_economy_score_t;

/* Empire score for the year, traced to `ky` (pico8/lua.lua:1339-1363):
 *
 *   b_ = ob([[0,100,200,400,]])
 *   function ky()
 *     local di=0
 *     for pt in all(cg.jo) do
 *       if pt:gg() then di+=b_[pt.mi] end
 *     end
 *     local gl = fm.po[r.p]*40
 *     local el = flr((#fm.invented*1.2)^2)*10
 *     local total = flr((di+gl+el)*0.01*fm.happiness)
 *     return {di, gl, el, fm.happiness.."%", total, min(flr(total/2000),5)}
 *   end
 *
 * - `colonies_score` (`di`): sum of `b_[level]` (`{0,100,200,400}` for
 *   levels 1-4) over colony_levels[0..colony_count) -- every entry stands in
 *   for a colonized colony (`pt:gg()`), matching rules_economy_happiness's
 *   own "caller already filtered" convention. level clamped to [1,4].
 * - `population_score` (`gl`): `population*40` (`fm.po[r.p]*40` -- `r.p` is
 *   `19` -> `iz[19]="people"`, matching RESOURCE_PEOPLE in rules_planets.h,
 *   produced by JOB_HIVEWORLD). Plain int arithmetic, no fixed_t needed --
 *   both operands and the multiplier are exact integers.
 * - `tech_score` (`el`): `floor((tech_invented_count*1.2)^2)*10`. `1.2` is
 *   not exactly representable in Q16.16 (folded from the traced double
 *   literal via the same idiom RULES_ECONOMY_LUXURY_TRADE_MULTIPLIER/
 *   RULES_ECONOMY_SKILL_IMPLANTS_MULTIPLIER already use, rules_economy.c),
 *   so this needs real fixed-point multiplication, unlike population_score.
 *   Computed as `fixed_t x = fixed_mul(tech_invented_count*FX_ONE,
 *   <1.2 in Q16.16>)`, then `fixed_mul(x,x)`, floored to an int via this
 *   module's own fixed_floor_to_int (rules_economy.c, added in chunk 4,
 *   reused here rather than duplicated), then multiplied by 10.
 * - `total`: `floor((colonies_score+population_score+tech_score)*0.01*
 *   happiness_percent)`. `happiness_percent` is exactly
 *   rules_economy_happiness's own return value passed straight in (source's
 *   own `fm.happiness` field, set once per year by bf() inside hy:lj()),
 *   keeping the two functions composable without any shared live state --
 *   see rules_economy_happiness's own doc comment above. Computed with
 *   int64_t intermediates throughout (rules_economy.c), NOT by first
 *   widening the raw integer sum to a fixed_t and calling fixed_mul, for
 *   two independent reasons: overflow (population_score is unbounded by
 *   this module, so a large enough population pushes sum*FX_ONE past
 *   fixed_t's int32 range, even beyond what this module's own tests
 *   exercise) and precision (even for sums well inside range, a
 *   Q16.16-truncated "0.01" literal measurably corrupts the result --
 *   verified to produce 15991 instead of the exact 16000 for this
 *   module's own stars-clamping test inputs, an error large enough to
 *   fail that test's exact-match assertion on its own, independent of
 *   any overflow). The int64_t composition avoids both while still
 *   producing the exact same mathematical floor(...) source computes.
 *   Uses this module's own floor-toward-negative-infinity convention
 *   (fixed_floor_to_int's own
 *   correctness argument, rules_economy.c) rather than C's truncate-toward-
 *   zero integer division, since `happiness_percent` is itself uncapped and
 *   can go negative (rules_economy_happiness's own doc comment above), which
 *   would make `total` negative too -- a case plain truncating division
 *   would floor incorrectly.
 * - `stars`: `min(floor(total/2000), 5)`. `total` is already a plain,
 *   already-floored int here, so this is an int/int floor-division, not a
 *   fixed_t operation -- but still NOT C's plain truncating `/`, for the
 *   same reason `total` itself needed care above: `total` can be negative
 *   (an unhappy-enough empire), and truncating division rounds a negative
 *   quotient toward zero, not toward negative infinity (e.g. `-500/2000`
 *   truncates to `0` in C, but `floor(-500/2000)` is `-1`) -- so this is
 *   computed with the same floor-toward-negative-infinity correction
 *   fixed_floor_to_int already establishes, applied here to a plain int
 *   division instead of a fixed_t one. No lower clamp is applied (source has
 *   none): `stars` can go negative for a deeply unhappy empire, only the
 *   upper `min(...,5)` bound from source is ported.
 */
rules_economy_score_t rules_economy_compute_score(const int *colony_levels, int colony_count, int population,
                                                    int tech_invented_count, fixed_t happiness_percent);

/* End condition, traced to `hy.hl`'s assignment (pico8/lua.lua:1235-1236):
 *
 *   self.hl = self.yr>=3426 and "final score"
 *             or self.ge<=0 and self.nz<3 and "bankrupt"
 *
 * Lua's `and`/`or` chain short-circuits left to right, and that short-circuit
 * is the whole point of this function, not an incidental detail: if
 * `self.yr>=3426` is true, the expression evaluates to "final score"
 * IMMEDIATELY -- the entire `self.ge<=0 and self.nz<3 and "bankrupt"` clause
 * is structurally never reached, not merely "checked after" or "lower
 * priority." Lua's `and` returns its right operand only when the left is
 * truthy, and `or` returns its left operand (without evaluating its right)
 * whenever that left operand is already truthy -- `self.yr>=3426 and "final
 * score"` is a truthy string the instant `self.yr>=3426` holds, so the `or`
 * short-circuits and its own right-hand side (the bankruptcy clause) is
 * simply never evaluated. A simultaneous "year>=3426 AND bankrupt" case
 * therefore resolves to "final score" every time, matching source exactly --
 * not some combined or undefined result.
 *
 * Only when `self.yr>=3426` is false does evaluation ever reach the
 * bankruptcy clause, which is itself a further `and`: both `self.ge<=0`
 * (net income) AND `self.nz<3` (treasury) must hold for `hl` to become
 * "bankrupt"; if either alone is false, the whole clause is falsy and `hl`
 * stays false (the game continues).
 *
 * Ported as a plain C `if`-return sequence -- a direct structural mirror of
 * the short-circuit, since C's own top-to-bottom `if`/`return` evaluation
 * naturally matches Lua's left-to-right `and`/`or` short-circuit here:
 *
 *   if (year >= RULES_ECONOMY_FINAL_YEAR) return RULES_ECONOMY_GAME_OVER_FINAL_YEAR;
 *   if (net_income <= 0 && treasury < RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD)
 *       return RULES_ECONOMY_GAME_OVER_BANKRUPT;
 *   return RULES_ECONOMY_GAME_CONTINUES;
 *
 * The first `if` returning unconditionally, before the second `if` is even
 * reached, is exactly what reproduces "the bankruptcy clause is never even
 * evaluated" -- not just "evaluated second." Comparison operators matched
 * exactly to source: `>=` for year (`self.yr>=3426`), `<=` for net_income
 * (`self.ge<=0`, so a net_income of exactly 0 counts as bankrupt-eligible),
 * and strict `<` for treasury (`self.nz<3`, so treasury exactly equal to
 * RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD does NOT count -- only
 * strictly below it does).
 */
typedef enum {
    RULES_ECONOMY_GAME_CONTINUES = 0,
    RULES_ECONOMY_GAME_OVER_FINAL_YEAR,
    RULES_ECONOMY_GAME_OVER_BANKRUPT,
} rules_economy_game_over_t;

rules_economy_game_over_t rules_economy_check_game_over(int year, int net_income, int treasury);

/* Slipway-cost factor, traced to `bk` (pico8/lua.lua:1259-1264):
 *
 *   function bk()
 *     return max(1, sqrt(#(cg.jc or {}))*0.27-0.1)
 *   end
 *
 * `#(cg.jc or {})` is a PLAIN Lua table-length -- the count of existing
 * slipways -- NOT a vector `#` (squared distance). This is a genuinely
 * different use of `#` than every prior squared-distance use of it in this
 * codebase (world.h/liaison.h's `km:__len`-derived constants, and
 * rules_economy_slipway_cost's own `distance_sq` parameter below): here it is
 * a route COUNT, converted to fixed_t (`count << 16`, i.e. `count * FX_ONE`)
 * before being handed to fixed_sqrt, not a squared-distance quantity that
 * fixed_sqrt then un-squares. `existing_route_count` is `#(cg.jc or {})`, the
 * plain route count, taken as a caller-supplied int mirroring liaison.c's own
 * `route_count` parameter shape (currently always 0 from its permanently-
 * empty route list, same as every other caller-supplied count in this port
 * today -- see the PRD's Implementation Decisions, "Slipway cost").
 *
 * This is a SECOND, independent use of fixed_sqrt in this same module (the
 * first being rules_economy_slipway_cost's two chained calls for its own
 * `x^0.75` term below) -- fixed_sqrt was added to fixed.h for effects.c's
 * segment-length calculation, and this module's own PRD explicitly confirms
 * it turns out to be exactly the primitive both of THIS module's formulas
 * need too, not effects.c-only infrastructure that merely happens to be
 * reusable.
 *
 * `max(1, sqrt(count)*0.27-0.1)`: at count=0, sqrt(0)=0, so
 * 0*0.27-0.1=-0.1, which is below the `max(1,...)` floor -- the result
 * clamps to exactly 1*FX_ONE in that case (see the dedicated test).
 * 0.27/0.1 are folded from their traced double literals via this module's
 * own established literal-folding idiom (g_trade_level_bonus, above).
 */
fixed_t rules_economy_slipway_cost_factor(int existing_route_count);

/* Slipway cost, traced to `dj` (pico8/lua.lua:1259-1264):
 *
 *   bn, bq = 6, 0.0977
 *   function dj(pb,to) local ec = (#(to.p-pb.p)/bq)^0.75
 *     return flr(bn*bk()*max(0.5,ec)) end
 *
 * `#(to.p-pb.p)` is SQUARED distance (`km:__len`, the same "`#v` is squared,
 * not linear" fact world.h/liaison.h already rely on) -- `distance_sq` here
 * is exactly that quantity, the SAME squared-world-distance quantity
 * `liaison_check`/liaison.h's own range/tolerance constants already use, so
 * a future live-wiring step can hand this function liaison.c's own already-
 * computed squared distance directly. This is the concrete convergence point
 * `docs/prd/psx-liaison-validity.md` deferred here: that PRD traced `dj`/`bk`
 * and explicitly asked for them to be ported in this step (Phase 3 step 13,
 * rules_economy.c) once they existed to call -- this function is that port.
 *
 * `bq=0.0977` is a plain divisor. Since fixed.h has no fixed_div, this is
 * ported as a multiplication by a compile-time-folded RECIPROCAL constant
 * (RULES_ECONOMY_SLIPWAY_COST_DIVISOR_RECIP, rules_economy.c), the same
 * "fold a double-literal constant at compile time" idiom liaison.h's
 * LIAISON_RANGE_SQ/LIAISON_BLOCK_TOLERANCE_SQ already use -- NOT a general-
 * purpose fixed-point division primitive this codebase doesn't otherwise
 * need.
 *
 * `x^0.75` (x = distance_sq * (1/bq)) has no fixed_pow anywhere in this
 * codebase. Computed as x^0.75 = x^(1/2) * x^(1/4) = sqrt(x) * sqrt(sqrt(x)):
 * two chained fixed_sqrt calls plus one fixed_mul
 * (`s1 = fixed_sqrt(x); s2 = fixed_sqrt(s1); ec = fixed_mul(s1, s2);`) --
 * this is this module's FIRST use of fixed_sqrt (the second being
 * rules_economy_slipway_cost_factor's direct `sqrt(count)` above).
 * `max(0.5, ec)` clamps the result before the final multiplication.
 *
 * `existing_route_count` is passed straight through to
 * rules_economy_slipway_cost_factor (`bk()`) -- see that function's own doc
 * comment for why it is a plain route count, not a squared-distance `#`.
 *
 * Floored once, at the very end (`bn=6`, `floor(bn * bk() * max(0.5,ec))`) --
 * matching this port's usual "floor once at the very end" convention here
 * (unlike rules_economy_annual_net_income's own genuine double floor above):
 * dj/bk are two independently-traced source functions from a wholly
 * different part of source (pico8/lua.lua:1259-1264, not hy:lj()'s
 * 1226-1238), each ported exactly as its own source floors it.
 */
int rules_economy_slipway_cost(fixed_t distance_sq, int existing_route_count);

/* Affordability gate, traced to `fm:lu`'s gate (pico8/lua.lua:1217-1224, via
 * `d_()`, pico8/lua.lua:569-576):
 *
 *   function hy:lu(jt) if self.nz<jt then iv({"-no money-",8}) return end
 *     self.nz-=jt
 *     ...
 *
 * `self.nz<jt` refuses (returns without debiting); this function ports the
 * NEGATED condition directly -- `treasury >= cost` AFFORDS -- matching
 * source's own "refuse if treasury is strictly less than cost" gate exactly
 * (treasury==cost affords: the boundary is inclusive on the affording side).
 * The `iv(...)` toast call is UI feedback, out of scope (no UI-text system
 * exists yet, Phase 5). Converges liaison.c's own deferred `fm:lu`/`d_`
 * reference: a future live-wiring step in liaison.c's drag-commit path can
 * call this directly once a live treasury exists.
 */
bool rules_economy_can_afford(int treasury, int cost);

/* Colony-level-target formula, traced to `dh()` (pico8/lua.lua:1291-1310):
 *
 *   function dh(pt)
 *     if (pt.me~="ly")return 0
 *     local jd=1
 *     for i in all(pt.jy)do
 *       i.gc=#jp(pt.cc,function(im) return im.qe==i.qe end)
 *       jd*=i.gc
 *     end
 *     if (jd==0)return 1
 *     if pt.nl then
 *       return min(#jp(pt.jy,function(i) return i.gc>0 end),3)
 *     end
 *     if not pt.hf then
 *       local li=pt.jy[#pt.jy]
 *       return li and min(li.gc+1,4)or 2
 *     end
 *     return min(4,1+mid(#pt.by,#pt.cc,1))
 *   end
 *
 * This is the module's most complex new formula, four branches dispatched
 * in source order (this port's `if`-return sequence mirrors that order
 * exactly, since later branches assume earlier gates already failed --
 * matching rules_economy_check_game_over's own precedent for a single
 * dispatcher covering multiple source branches):
 *
 * 1. `pt.me~="ly"` (not colonized) -> `0`, regardless of every other field.
 *    Ported as the negated `!colonized` guard, fired first and
 *    unconditionally, exactly like rules_economy_check_game_over's own
 *    first `if`.
 * 2. `jd` is the PRODUCT of every need's delivered-count (`i.gc`), starting
 *    at `1` -- `need_count==0` (Protostar, no needs at all) leaves the loop
 *    unentered and `jd` at its initial `1`, which is NOT the "any unmet"
 *    case. Ported as: multiply `need_delivery_counts[0..need_count)` into a
 *    running product starting at 1.
 * 3. "Any unmet" gate: fires ONLY when `need_count>0` AND the product is
 *    `0` (a product of non-negative integers is zero iff at least one
 *    factor is) -> `1`. Deliberately guarded by `need_count>0` so an empty
 *    needs array (product stays at its initial 1) never triggers this,
 *    even though `jd==0` would otherwise read as "false" either way here --
 *    the guard is what keeps this branch from ever firing for
 *    `need_count==0`, matching source's own `for` loop never running at
 *    all in that case (there's nothing to have been unmet).
 * 4. Trading Hub branch (`pt.nl`, RULES_ECONOMY_COLONY_KIND_TRADING_HUB):
 *    `#jp(pt.jy,function(i) return i.gc>0 end)` -- an INDEPENDENT count of
 *    entries with a positive delivery count, ported literally as its own
 *    real loop rather than shortcut to `need_count` even though, once gate
 *    3 above has passed, every remaining entry is provably `>0` and so this
 *    count is provably always equal to `need_count` -- matching this
 *    module's established "preserve a real source step even where it is
 *    provably redundant" practice (the double-floor finding, Phase 3).
 *    Capped at `3` (`min(...,3)`). At `need_count==0` this returns
 *    `min(0,3)=0` -- deliberately DIFFERENT from the single-need-structure
 *    branch's own `need_count==0` case below (which returns a flat `2`),
 *    since Trading Hub's `pt.nl` branch has no "empty needs" special case
 *    of its own in source.
 * 5. Single-need-structure branch (`not pt.hf`,
 *    RULES_ECONOMY_COLONY_KIND_SINGLE_NEED_STRUCTURE, e.g. Laboratory/
 *    Ascension Gate/Synthesizer/Protostar): `li=pt.jy[#pt.jy]` (the LAST
 *    need entry) -- `li and min(li.gc+1,4) or 2`. Non-empty needs ->
 *    `min(last_delivery_count+1,4)`; empty needs (Protostar) -> a fixed
 *    `2` (this is the one branch where gate 3's "any unmet" check can never
 *    fire for an empty-needs entity, since there is nothing to be unmet).
 * 6. Regular-colony branch (the fallback, `pt.hf` true,
 *    RULES_ECONOMY_COLONY_KIND_REGULAR): `min(4,1+mid(#pt.by,#pt.cc,1))` --
 *    `mid()` is PICO-8's built-in 3-argument median, ported via a small
 *    file-local `median3` helper (rules_economy.c). `partner_route_count`
 *    stands in for `#pt.by` (outgoing trade-partner-edge count),
 *    `total_deliveries_received` for `#pt.cc` (total deliveries received),
 *    both plain caller-supplied ints, mirroring
 *    rules_economy_colony_trade_income's own `partner_count` parameter
 *    shape. All-integer arithmetic -- no fixed_t needed anywhere in this
 *    formula.
 *
 * `need_delivery_counts`/`need_count` stand in for `pt.jy`/each entry's own
 * `i.gc` (already-computed delivered count) -- the caller supplies the
 * already-counted per-need delivery totals `bd()`'s own live stabilization
 * loop would have produced (out of scope here, see the PRD's Out of Scope:
 * "bd()/bb()'s live stabilization loop and delivery-count computation
 * itself"), the same "caller supplies the aggregate a live loop would have
 * produced" idiom this module already established for
 * rules_economy_colony_trade_income's partner_levels[]/
 * rules_economy_happiness's colony_levels[].
 */
typedef enum {
    RULES_ECONOMY_COLONY_KIND_REGULAR,
    RULES_ECONOMY_COLONY_KIND_TRADING_HUB,
    RULES_ECONOMY_COLONY_KIND_SINGLE_NEED_STRUCTURE,
} rules_economy_colony_kind_t;

int rules_economy_colony_level(bool colonized, rules_economy_colony_kind_t kind,
                                const int *need_delivery_counts, int need_count,
                                int partner_route_count, int total_deliveries_received);

/* Affordability mutation, traced to `fm:lu`'s debit (pico8/lua.lua:1217-1224):
 *
 *   self.nz-=jt
 *
 * Plain `treasury - cost`. Meaningful ONLY when rules_economy_can_afford has
 * already returned true for the SAME (treasury, cost) pair -- this is the
 * caller's responsibility, not something this function checks or enforces,
 * matching `d_()`'s own call-site short-circuit ordering exactly
 * (`if not self.et and fm:lu(...)`, pico8/lua.lua:569-576 -- `fm:lu` itself
 * is only ever invoked after `d_()`'s own gating, and `fm:lu`'s internal
 * `self.nz<jt` check is what actually refuses/mutates; this function ports
 * only the mutation half, `rules_economy_can_afford` the gate half, exactly
 * as the PRD's own "split into a pure gate and a pure debit" instruction
 * asks). Calling this for an unaffordable pair is not undefined behavior --
 * it is plain integer subtraction, so it cannot crash or overflow for any
 * value this module's own int range supports -- but the resulting negative
 * treasury value is not meaningful per source's own gating, and no caller in
 * this port should ever construct that case.
 */
int rules_economy_debit(int treasury, int cost);

#endif
