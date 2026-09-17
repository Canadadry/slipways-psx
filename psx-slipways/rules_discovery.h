#ifndef RULES_DISCOVERY_H
#define RULES_DISCOVERY_H

#include <stdbool.h>

#include "fixed.h"
#include "planetgen.h"
#include "rng.h"

/* Probe cost -- corrected from docs/plan.md's own stated value, traced
 * directly to source. See docs/prd/triage/psx-rules-discovery.md,
 * Implementation Decisions / "Probe cost" for the full trail.
 *
 * mz's data table (pico8/lua.lua:752-761): "od=3,fp=1," (line 758) -- od is
 * the generic credits-cost field (fm:lu(self.od) debits it, the same field
 * rules_planets.c's job costs use); fp is the generic months-cost field.
 * The generic action-commit function d_(self) (pico8/lua.lua:569-576),
 * called from mz:ey() (pico8/lua.lua:774-775), runs
 * "fm:nx(self.fp or 0)" -- fm:nx is hy:nx(hx) (pico8/lua.lua:1204-1209,
 * "self.lo+=hx ..."), and hx is unambiguously a MONTHS argument (the
 * design doc's own SS2 confirms this: "hy:nx(hx) avance l'horloge de hx
 * mois"). So the probe costs od=3 credits and fp=1 month. */
#define RULES_DISCOVERY_PROBE_COST_CREDITS 3

/* This is 1, NOT 3. Both docs/design/slipways-mechanics.md SS4 ("et 3 mois
 * de temps") and docs/plan.md's own Phase 3 step 10 one-liner ("coût (3
 * crédits, 3 mois)") state 3 months -- but the source data they were
 * transcribed from contradicts them: mz's fp=1 (pico8/lua.lua:758) is the
 * literal months-argument passed, via d_()'s "fm:nx(self.fp or 0)" (line
 * 571) and hy:nx(hx)'s months semantics (lines 1204-1209), to the game
 * clock. Per this project's traceability guardrail, source wins: this
 * constant is 1, and both docs' "3 mois"/"3 months" prose should be read as
 * a design-doc/plan transcription error, not a second, equally-valid
 * figure. See docs/prd/triage/psx-rules-discovery.md, Implementation
 * Decisions / "Probe cost" and Further Notes for the full citation trail
 * and the recommended (not-in-scope) doc correction. */
#define RULES_DISCOVERY_PROBE_COST_MONTHS 1

/* Probe launch-range gate -- ports mz:hq() (pico8/lua.lua:767-771):
 * "nq,self.ij=ij(self.p,self.ox) if nq>ov("space folding",0.19,0.27) then
 * self.et=mz.jj end". self.ox is the caller's "founded" (colonized) planet
 * list (mz:pq(), line 764-766); ij(p,j_) (line 351-358) returns the minimum
 * SQUARED distance from p to the nearest point in j_.
 *
 * This reuses world.h's WORLD_NEIGHBOR_RANGE_SQ directly rather than
 * defining a second, independently-drifting constant -- that macro already
 * cites this exact mz:hq() trace (world.h:22-29) as the base 0.19 probe
 * range, before any "space folding" tech extension (left unmodeled, no tech
 * system exists yet -- same scope WORLD_NEIGHBOR_RANGE_SQ and liaison.h's
 * LIAISON_RANGE_SQ already give their own tech extensions). See
 * docs/prd/triage/psx-rules-discovery.md, Implementation Decisions /
 * "Probe launch-range gate".
 *
 * Computes the minimum squared distance from target to every position in
 * founded_positions[0, founded_count), and returns whether that minimum is
 * <= WORLD_NEIGHBOR_RANGE_SQ (source's own comparison is "nq > threshold"
 * -> too far, so in-range is "<=" -- matching liaison.c's identical
 * boundary convention for its own range check). founded_count == 0 always
 * returns false (no founded planet exists to be "in range" of -- mirrors
 * ij()'s own effectively-infinite minimum when its input list is empty).
 * A caller-supplied position list, not a read of any live galaxy/
 * colonization singleton -- no live "founded planets" state exists
 * anywhere in this port yet. */
bool rules_discovery_probe_in_range(planet_pos_t target, const planet_pos_t *founded_positions, int founded_count);

/* Empty-planet roll -- ports jo.ir, the actual stateful generator jo:hh()
 * (pico8/lua.lua:407-411) tests against 0.45, NOT a plain Bernoulli(0.45)
 * draw. See docs/prd/triage/psx-rules-discovery.md, Implementation
 * Decisions / "Empty-planet roll" for the full trail.
 *
 * jo:hh() (pico8/lua.lua:407-411): "self.nk = jo.ir()<0.45 and df.ll or
 * jo.oc() ...". jo.ir is assigned ONCE per game session, at galaxy-
 * generation time (pico8/lua.lua:750-751: "jo.oc,jo.ir =
 * gn(df,2,jf(0.5,dk.en)), jf(0.8,0.5)") -- a single closure instance reused
 * for every discovery for the rest of that session, not re-created per
 * probe.
 *
 * jf(n_,b) (pico8/lua.lua:333-338) is a closure over a mutable b (bias): a
 * mean-reverting random walk, not rnd(1). Each call:
 *   v = rnd(1-abs(b)) + max(b,0)
 *   b += (0.5-v)*n_
 *   return v
 * For jo.ir = jf(0.8,0.5): n_=0.8 (baked into this instantiation, not a
 * general parameter -- jo.ir takes no n_/b arguments of its own), initial
 * b=0.5 -- both literal, matching design doc SS3's "independante de la
 * difficulte" (jf(0.8,0.5) takes no dk.en argument, unlike jo.oc's
 * jf(0.5,dk.en)). */
typedef struct {
    fixed_t bias;
} rules_discovery_empty_roll_t;

/* Sets state->bias to FX_ONE/2 (0.5), the literal initial b in
 * jo.ir=jf(0.8,0.5) (pico8/lua.lua:750-751). */
void rules_discovery_empty_roll_init(rules_discovery_empty_roll_t *state);

/* Ports one call of jf(0.8,0.5)'s closure body (pico8/lua.lua:333-338) plus
 * jo:hh()'s "<0.45" test (pico8/lua.lua:408). state->bias is threaded by
 * the caller across repeated calls (mirroring jo.ir's own one-time-created,
 * session-long closure shape, and rng_t's own injection pattern in
 * planetgen_generate) -- it is NOT reset per call.
 *
 * Q16.16 throughout:
 *   span = FX_ONE - |state->bias|              (1-abs(b))
 *   drawn = rng_next_range(rng, 0, span)        (rnd(1-abs(b)))
 *     -- reuses planetgen.c:33-34's exact idiom: a fixed_t span's raw
 *     int32_t value used directly as rng_next_range's exclusive bound.
 *   v = drawn + max(state->bias, 0)
 *   state->bias += fixed_mul(FX_ONE/2 - v, 0.8 as fixed_t)   (n_=0.8)
 *   return v < 0.45*FX_ONE
 *
 * state->bias can and does go negative (the walk is unbounded within the
 * range this formula naturally settles into), so max(state->bias,0) is a
 * real, exercised branch, not a formality. */
bool rules_discovery_roll_empty(rules_discovery_empty_roll_t *state, rng_t *rng);

#endif
