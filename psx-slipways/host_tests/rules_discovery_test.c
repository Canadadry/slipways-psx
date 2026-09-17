#include "t.h"
#include "../rules_discovery.h"

static void test_probe_cost_credits_matches_traced_value(void) {
    T_CHECK(RULES_DISCOVERY_PROBE_COST_CREDITS == 3);
}

/* This asserts 1, NOT the 3 that docs/plan.md's Phase 3 step 10 one-liner
 * and docs/design/slipways-mechanics.md SS4 both state in prose ("3 mois"/
 * "3 months"). The source data (mz's fp=1, pico8/lua.lua:758, flowing
 * through d_()'s fm:nx(self.fp or 0) at line 571 and hy:nx(hx)'s months
 * semantics at lines 1204-1209 -- see rules_discovery.h's comment on this
 * constant for the full trail) contradicts both docs, and source wins.
 * This test exists specifically so that correction is asserted in code,
 * not just narrated in the PRD. */
static void test_probe_cost_months_is_1_not_3(void) {
    T_CHECK(RULES_DISCOVERY_PROBE_COST_MONTHS == 1);
}

int main(void) {
    t_run("probe_cost_credits_matches_traced_value", test_probe_cost_credits_matches_traced_value);
    t_run("probe_cost_months_is_1_not_3", test_probe_cost_months_is_1_not_3);
    return t_report();
}
