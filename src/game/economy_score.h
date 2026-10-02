#ifndef GAME_ECONOMY_SCORE_H
#define GAME_ECONOMY_SCORE_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "game/ui_button.h"

/* Throughout this module an absent or wrong-kind field reads as its
 * zero/false default rather than crashing, and a NULL instance is a safe
 * no-op. */

p8_fixed_t planet_upkeep(entity_t *pt);

/* Lua passes the raw method value `jo.gg` as `jp`'s predicate, which calls
 * it as `e:gg()`; the direct pico8_jo_gg() call here is that same call, not
 * an approximation of it. */
p8_fixed_t economy_score_colony_upkeep(void);

p8_fixed_t economy_score_trade_income(const entity_t *entity);

/* `ky()`'s six results, `s[1..6]` in the cart, in that order. */
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

/* The difficulty button `eu()` was given, borrowed; NULL until `eu()`
 * (pico8_cart.c) calls the setter. Once that button leaves (the `jl()` of
 * `eu()` drops it), `dk` designates a copy of what is read from it: `px`,
 * `trade`, `en` and their flags; any other member is nil there. */
const button_t *economy_score_difficulty(void);
void economy_score_set_difficulty(const button_t *dk);

/* Called by the exit queue before `e` is given back. */
void economy_score_difficulty_quits(const entity_t *e);

/* Writes `kr` into planet `p` (asserted) and returns it. */
p8_fixed_t economy_score_net_income(entity_t *p);

#endif
