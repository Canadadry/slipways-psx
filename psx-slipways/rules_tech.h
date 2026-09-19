#ifndef RULES_TECH_H
#define RULES_TECH_H

#include <stdint.h>

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
    /* Era 2 (od 10-18), raw nb table order, per
     * docs/prd/triage/psx-full-planets-and-tech.md's traced full 19-tech
     * table (chunk 11). */
    TECH_XENOFOODS,
    TECH_SLIPGATES,
    TECH_GEOHARVESTING,
    TECH_TRADE_LEAGUE,
    TECH_REPLICATION,
    /* Era 3 (od 19-31). */
    TECH_AUTOASSEMBLERS,
    TECH_SKILL_IMPLANTS,
    TECH_STARBIRTH,
    TECH_BIOME_HACKING,
    TECH_INFRASPACE,
    /* Era 4 (od 34-50). */
    TECH_SUPERHUMAN_AI,
    TECH_VOID_SYNTHESIS,
    TECH_GENE_REWRITING,
    TECH_TIME_COMPRESSION,
    TECH_ASCENSION,
    TECH_COUNT,
} tech_id_t;

/* One entry of the traced nb table (pico8/lua.lua, unstash 10457): a
 * tech's display name, its static base cost (the raw od field), and its
 * era (the raw lv field, 1-4). lv/nk are documented above but nk/fu/gi
 * (the live cost-discount formula) are wholly deferred (see the PRD's
 * Implementation Decisions / Out of Scope). era is appended after the two
 * original fields rather than inserted between them, so the 4 existing
 * Era-1 entries' name/base_cost initializers stay byte-identical and only
 * gain one new `.era = 1` line each (regression-safe extension, matching
 * this PRD's own "zero-value-change" discipline for pre-existing data).
 *
 * icon (docs/prd/triage/psx-sprites-audio-extraction.md, Implementation
 * Chunks chunk 8): the resolved resource-code sprite id t.nk already
 * carries after nb's own load-time ho() letter-to-resource-code transform
 * (pico8/lua.lua:17-23, invoked at :1442) -- not a raw nk letter and not a
 * new lookup table, just a place to hold the value this port already
 * decodes elsewhere (docs/design/slipways-mechanics.md's Annexe A).
 * Populated for the 4 Era-1 entries only (sprite_atlas.h's
 * SPRITE_RESOURCE_* constants); appended last so it, too, is a
 * regression-safe extension -- every other existing field/initializer
 * stays byte-identical. */
typedef struct {
    const char *name;
    int base_cost;
    int era;
    int icon;
} tech_t;

/* Plain accessor into the static full 19-tech table. id must be a valid
 * enum value in range (less than TECH_COUNT) -- mirrors
 * rules_planets_type()/rules_planets_job()'s own simplicity, no elaborate
 * error handling. */
const tech_t *rules_tech_get(tech_id_t id);

/* The fixed number of tiles game_draw.c's placeholder tech-menu render
 * loop draws: exactly the 4 real, zero-prerequisite Era-1 techs this port
 * originally shipped (TECH_SPACE_FOLDING..TECH_DRILLBOTS), not
 * TECH_COUNT. Deliberately decoupled from TECH_COUNT so growing the tech
 * table to the full 19-entry roster (chunk 11) doesn't silently balloon
 * that placeholder's visual output from 4 tiles to 19 -- the placeholder
 * has no live "researched"/era-progression state to justify rendering
 * anything beyond the original 4-tile mock, per this PRD's own
 * "behavior-preserving" requirement for the game_draw.c change. */
#define RULES_TECH_ERA1_TILE_COUNT 4

/* Select's bit in the raw PSn00bSDK pad bitmask: bit 0, PAD_SELECT,
 * standard published PS1 SIO0 digital-button layout (psxpad.h's own
 * PAD_* macros), per docs/prd/psx-select-start-mapping.md's
 * already-settled citation. This module stays host-testable with no PSX
 * headers (per this PRD's "no PSX headers" requirement for the pure
 * core), so the bit is re-declared locally here rather than pulled in via
 * psxpad.h -- following cursor.h/cursor.c's own established convention of
 * defining its own local CURSOR_PAD_* macros instead of including
 * psxpad.h directly. Active-low: 0 = pressed. */
#define RULES_TECH_PAD_SELECT (1 << 0)

/* Pure core: no statics, no I/O, zero PSX headers -- mirrors cursor.h's
 * cursor_step() shape exactly.
 *
 * Computes Select's press edge (pressed = (~buttons) & prev_buttons, the
 * same idiom cursor_step() already uses) and flips prev_open on that
 * edge. A held or released Select does nothing further: this is a pure
 * edge-triggered toggle, no held-state semantics (unlike Circle's
 * press/held/release trio in cursor.c) -- Select's job here is simpler by
 * design, matching its "simple, unconditional button press" framing in
 * docs/plan.md step 7. prev_open/the return value are a plain int
 * boolean (0 = closed, 1 = open), matching cursor.c's own int
 * mouse_mode field convention, per this PRD's own declared signature. */
int rules_tech_step(int prev_open, uint16_t prev_buttons, uint16_t buttons);

/* Stateful wrapper, mirroring cursor.h/cursor.c's own
 * init/update/get_state shape exactly. rules_tech_update() reads the raw
 * buttons value directly (the caller passes input_get_state().buttons) --
 * this module has no dependency on cursor.c whatsoever, per this PRD's
 * Implementation Decisions ("Select's edge detection and gating"). */
typedef struct {
    int open; /* 0 = closed, 1 = open */
} rules_tech_state_t;

void rules_tech_init(void);
void rules_tech_update(uint16_t buttons);
rules_tech_state_t rules_tech_get_state(void);

#endif
