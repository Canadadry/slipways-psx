#ifndef RULES_DISCOVERY_H
#define RULES_DISCOVERY_H

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

#endif
