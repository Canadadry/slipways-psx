#ifndef RULES_TECH_H
#define RULES_TECH_H

/* The 4 (of 19 documented) technologies this step ports: the real,
 * zero-prerequisite, always-available Era-1 slice. See
 * docs/prd/triage/psx-rules-tech.md, Implementation Decisions ("central
 * scoping finding").
 *
 * Traced by re-decoding the raw nb table (unstash 10457, pico8/map.json)
 * directly, rather than trusting docs/design/slipways-mechanics.md's own
 * SS12 table/prose at face value -- the same discipline rules_planets.h's
 * own PRD already applied to fd. Field names are cross-checked against
 * hp.mr-style positional parsing (pico8/lua.lua:1442,
 * foreach(nb,ho(nb,ob([["px","lv","nk","od","oy",]]),{"nk"}))); each raw
 * record has shape o(px, lv, nk, od, oy).
 *
 * The re-decode surfaces a load-bearing correction: the raw nb table has
 * exactly 19 tech records (not 20), and Era 1 (lv==1) has exactly 4 techs
 * (not the 5 a naive reading of the design doc's SS12 prose implies) --
 * `xenofoods`, which the design doc's own SS12 table lists under Era 1,
 * actually carries raw lv==2 in the unstash data (it belongs to Era 2, not
 * Era 1). This mirrors rules_planets.h's own precedent of documenting a
 * design-doc discrepancy discovered by re-verifying raw unstash data rather
 * than trusting the doc's table uncritically.
 *
 * The 4 Era-1 records, in raw nb table order (name, od == base_cost):
 * space folding (4), geneseeds (5), nanomaterials (6), drillbots (7). */
typedef enum {
    TECH_SPACE_FOLDING,
    TECH_GENESEEDS,
    TECH_NANOMATERIALS,
    TECH_DRILLBOTS,
    TECH_COUNT,
} tech_id_t;

/* One entry of the traced nb table (pico8/lua.lua, unstash 10457): a
 * tech's display name and its static base cost (the raw od field). lv/nk
 * are documented above but not stored as struct fields -- lv is implicitly
 * 1 for every entry in this subset by construction, and nk/fu/gi (the live
 * cost-discount formula) are wholly deferred (see the PRD's Implementation
 * Decisions / Out of Scope). */
typedef struct {
    const char *name;
    int base_cost;
} tech_t;

/* Plain accessor into the static Era-1 tech table. id must be a valid enum
 * value in range (less than TECH_COUNT) -- mirrors
 * rules_planets_type()/rules_planets_job()'s own simplicity, no elaborate
 * error handling. */
const tech_t *rules_tech_get(tech_id_t id);

#endif
