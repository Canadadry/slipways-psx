#ifndef RULES_DISCOVERY_H
#define RULES_DISCOVERY_H

#include <stdbool.h>

#include "planetgen.h"

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

#endif
