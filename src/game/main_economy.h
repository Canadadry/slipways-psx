#ifndef GAME_MAIN_ECONOMY_H
#define GAME_MAIN_ECONOMY_H

#include "game/economy_formulas.h"
#include "p8/fixed.h"
#include "engine/pointer_list.h"
#include "engine/entity_class.h"

/* The economy's archetype, here and not with the others: this header does
 * not include game_data.h. */
typedef struct {
    p8_fixed_t credits;
    p8_fixed_t era;
    p8_fixed_t year_ticks;
    p8_fixed_t year;
} economy_archetype_t;

/* The economy, one per game, in that game's zone with `invented`; never
 * freed, it dies with its zone. `production`/`demand` are NULL before the
 * first economy_recompute(), then point at `production_own`/`demand_own`,
 * which every economy_recompute() refills in place. `end_status` is absent
 * before the first economy_recompute(). */
typedef struct {
    entity_t header;
    const economy_archetype_t *archetype;
    p8_fixed_t credits;
    p8_fixed_t income;
    p8_fixed_t research_points;
    p8_fixed_t era;
    pointer_list_t *invented;
    p8_fixed_t year_ticks;
    p8_fixed_t year;
    resource_map_t *production;
    resource_map_t *demand;
    resource_map_t production_own;
    resource_map_t demand_own;
    p8_fixed_t happiness;
    int has_happiness; /* 0: `happiness` is absent, and is then never read */
    const char *end_status;
    int has_end_status; /* 0: absent; 1 with `end_status` NULL: no end yet */
} economy_t;

/* The provisional main economy: one static struct, never allocated nor
 * freed, with no method and no writer; `research_points`/`credits` are
 * written only by the cheat code, and set back to absent by every
 * probe_init(). */
typedef struct {
    entity_t header;
    p8_fixed_t research_points;
    int has_research_points; /* 0: absent, and then never read */
    p8_fixed_t credits;
    int has_credits; /* 0: absent, and then never read */
} main_economy_placeholder_t;

const entity_class_t *main_economy_placeholder_class(void);

/* The main economy: the provisional, which probe_init() installs, then,
 * from cart_start_game() on, an economy; read it through
 * main_economy_typed() and main_economy_placeholder(). NULL before init and
 * after shutdown, which several callers test for. The setter only repoints:
 * from cart_init() to the next cart_start_game() the slot still holds the
 * quit game's economy, alive until the flush that follows. These functions
 * and their state are defined in probe.c. */
entity_t *main_economy(void);
void probe_set_main_economy(entity_t *instance);

/* The main economy as an economy, NULL when the slot is empty or holds the
 * provisional. */
economy_t *main_economy_typed(void);

/* The provisional, NULL when the slot is empty or holds an economy. */
const main_economy_placeholder_t *main_economy_placeholder(void);

#endif
