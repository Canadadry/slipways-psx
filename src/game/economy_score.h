#ifndef GAME_ECONOMY_SCORE_H
#define GAME_ECONOMY_SCORE_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "game/ui_button.h"

/* planet_upkeep() and economy_score_trade_income() answer 0 for a NULL
 * entity; economy_score_net_income() asserts a planet. While the main
 * economy is not the game's, its production and happiness read as absent. */

p8_fixed_t planet_upkeep(entity_t *pt);

p8_fixed_t economy_score_colony_upkeep(void);

p8_fixed_t economy_score_trade_income(const entity_t *entity);

/* The six results of economy_score_final(). */
typedef struct {
    p8_fixed_t development;
    p8_fixed_t population;
    p8_fixed_t technology;
    char *happiness; /* valid until the next reset of the frame zone */
    p8_fixed_t total;
    p8_fixed_t tier;
} final_score_t;

final_score_t economy_score_final(void);

p8_fixed_t economy_score_happiness(void);

/* The difficulty button cart_start_game() was given, borrowed; NULL until
 * cart_start_game() calls the setter. Once that button leaves (the
 * reset_world() of cart_start_game() drops it), the result is a static copy
 * of what is read from it: `name`, `trade`, `type_bias` and their flags;
 * any other member is zero there. */
const button_t *economy_score_difficulty(void);
void economy_score_set_difficulty(const button_t *dk);

/* Called by the exit queue before the entity is given back. */
void economy_score_difficulty_quits(const entity_t *e);

/* Writes `net_income` into the planet (asserted) and returns it. */
p8_fixed_t economy_score_net_income(entity_t *p);

#endif
