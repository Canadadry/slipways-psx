#ifndef PICO8_ECONOMY_SCORE_H
#define PICO8_ECONOMY_SCORE_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "game/ui_button.h"

/* Throughout this module an absent or wrong-kind field reads as its
 * zero/false default rather than crashing, and a NULL instance is a safe
 * no-op. */

pico8_fixed_t pico8_da(entity_t *pt);

/* Lua passes the raw method value `jo.gg` as `jp`'s predicate, which calls
 * it as `e:gg()`; the direct pico8_jo_gg() call here is that same call, not
 * an approximation of it. */
pico8_fixed_t pico8_cz(void);

pico8_fixed_t pico8_db(const entity_t *pt);

/* `ky()`'s six results, `s[1..6]` in the cart, in that order. */
typedef struct {
    pico8_fixed_t di;
    pico8_fixed_t gl;
    pico8_fixed_t el;
    char *happiness; /* valid until the next reset of the frame zone */
    pico8_fixed_t total;
    pico8_fixed_t tier;
} pico8_ky_t;

pico8_ky_t pico8_ky(void);

pico8_fixed_t pico8_bf(void);

/* The difficulty button `eu()` was given, borrowed; NULL until `eu()`
 * (pico8_cart.c) calls the setter. Once that button leaves (the `jl()` of
 * `eu()` drops it), `dk` designates a copy of what is read from it: `px`,
 * `trade`, `en` and their flags; any other member is nil there. */
const pico8_js_t *pico8_dk(void);
void pico8_economy_set_dk(const pico8_js_t *dk);

/* Called by the exit queue before `e` is given back. */
void pico8_economy_dk_quitte(const entity_t *e);

/* Writes `kr` into planet `p` (asserted) and returns it. */
pico8_fixed_t pico8_kr(entity_t *p);

#endif
