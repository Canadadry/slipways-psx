#include "rules_tech.h"

/* The full 19-tech table, traced to the raw nb table (unstash 10457,
 * pico8/map.json), re-decoded directly rather than trusted from
 * docs/design/slipways-mechanics.md's own SS12 table/prose -- see
 * rules_tech.h for the full citation and the doc's own era-assignment
 * correction (xenofoods is Era 2, not Era 1, per raw lv). name/base_cost
 * (od) values, in raw nb table order.
 *
 * The 4 Era-1 entries (TECH_SPACE_FOLDING..TECH_DRILLBOTS) are unchanged
 * from this module's original PRD -- only the new `.era = 1` line was
 * added to each; name/base_cost stay byte-identical, re-asserted by
 * host_tests/rules_tech_test.c's own regression checks. */
static const tech_t g_tech_table[TECH_COUNT] = {
    [TECH_SPACE_FOLDING] = {
        .name = "space folding",
        .base_cost = 4,
        .era = 1,
    },
    [TECH_GENESEEDS] = {
        .name = "geneseeds",
        .base_cost = 5,
        .era = 1,
    },
    [TECH_NANOMATERIALS] = {
        .name = "nanomaterials",
        .base_cost = 6,
        .era = 1,
    },
    [TECH_DRILLBOTS] = {
        .name = "drillbots",
        .base_cost = 7,
        .era = 1,
    },
    [TECH_XENOFOODS] = {
        .name = "xenofoods",
        .base_cost = 10,
        .era = 2,
    },
    [TECH_SLIPGATES] = {
        .name = "slipgates",
        .base_cost = 12,
        .era = 2,
    },
    [TECH_GEOHARVESTING] = {
        .name = "geoharvesting",
        .base_cost = 14,
        .era = 2,
    },
    [TECH_TRADE_LEAGUE] = {
        .name = "trade league",
        .base_cost = 16,
        .era = 2,
    },
    [TECH_REPLICATION] = {
        .name = "replication",
        .base_cost = 18,
        .era = 2,
    },
    [TECH_AUTOASSEMBLERS] = {
        .name = "autoassemblers",
        .base_cost = 19,
        .era = 3,
    },
    [TECH_SKILL_IMPLANTS] = {
        .name = "skill implants",
        .base_cost = 22,
        .era = 3,
    },
    [TECH_STARBIRTH] = {
        .name = "starbirth",
        .base_cost = 25,
        .era = 3,
    },
    [TECH_BIOME_HACKING] = {
        .name = "biome hacking",
        .base_cost = 28,
        .era = 3,
    },
    [TECH_INFRASPACE] = {
        .name = "infraspace",
        .base_cost = 31,
        .era = 3,
    },
    [TECH_SUPERHUMAN_AI] = {
        .name = "superhuman ai",
        .base_cost = 34,
        .era = 4,
    },
    [TECH_VOID_SYNTHESIS] = {
        .name = "void synthesis",
        .base_cost = 38,
        .era = 4,
    },
    [TECH_GENE_REWRITING] = {
        .name = "gene rewriting",
        .base_cost = 42,
        .era = 4,
    },
    [TECH_TIME_COMPRESSION] = {
        .name = "time compression",
        .base_cost = 46,
        .era = 4,
    },
    [TECH_ASCENSION] = {
        .name = "ascension",
        .base_cost = 50,
        .era = 4,
    },
};

const tech_t *rules_tech_get(tech_id_t id) {
    return &g_tech_table[id];
}

int rules_tech_step(int prev_open, uint16_t prev_buttons, uint16_t buttons) {
    uint16_t pressed = (uint16_t) (~buttons) & prev_buttons;

    int next_open = prev_open;

    if (pressed & RULES_TECH_PAD_SELECT) {
        next_open = !next_open;
    }

    return next_open;
}

/* Stateful wrapper -- mirrors cursor.c's g_cursor_mouse_mode/
 * g_cursor_prev_buttons shape exactly: flat, non-static, module-owned
 * globals; rules_tech_init() resets them; rules_tech_update() reads them
 * into locals, calls the pure step function, writes the result back out,
 * then updates prev_buttons last (matching cursor_update()'s own
 * read-into-locals/call-pure-fn/write-back-out/update-prev_buttons-last
 * sequencing). */
int g_rules_tech_open = 0;
uint16_t g_rules_tech_prev_buttons = 0xffff;

void rules_tech_init(void) {
    g_rules_tech_open = 0;
    g_rules_tech_prev_buttons = 0xffff;
}

void rules_tech_update(uint16_t buttons) {
    int prev_open = g_rules_tech_open;

    int next_open = rules_tech_step(prev_open, g_rules_tech_prev_buttons, buttons);

    g_rules_tech_open = next_open;
    g_rules_tech_prev_buttons = buttons;
}

rules_tech_state_t rules_tech_get_state(void) {
    rules_tech_state_t state;
    state.open = g_rules_tech_open;
    return state;
}
