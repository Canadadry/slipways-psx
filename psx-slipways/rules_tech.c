#include "rules_tech.h"

/* The 4 Era-1 records, traced to the raw nb table (unstash 10457,
 * pico8/map.json), re-decoded directly rather than trusted from
 * docs/design/slipways-mechanics.md's own SS12 table/prose -- see
 * rules_tech.h for the full citation and the doc's own era-assignment
 * correction (xenofoods is Era 2, not Era 1, per raw lv). name/base_cost
 * (od) values, in raw nb table order: space folding(4), geneseeds(5),
 * nanomaterials(6), drillbots(7). */
static const tech_t g_tech_table[TECH_COUNT] = {
    [TECH_SPACE_FOLDING] = {
        .name = "space folding",
        .base_cost = 4,
    },
    [TECH_GENESEEDS] = {
        .name = "geneseeds",
        .base_cost = 5,
    },
    [TECH_NANOMATERIALS] = {
        .name = "nanomaterials",
        .base_cost = 6,
    },
    [TECH_DRILLBOTS] = {
        .name = "drillbots",
        .base_cost = 7,
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
