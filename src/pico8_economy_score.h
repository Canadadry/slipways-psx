#ifndef PICO8_ECONOMY_SCORE_H
#define PICO8_ECONOMY_SCORE_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"

/* Throughout this module an absent or wrong-kind field reads as its
 * zero/false default rather than crashing, and a NULL instance is a safe
 * no-op. */

/* `pt` is non-const, unlike this module's other formulas, because `pt:gg()`
 * goes through pico8_mv(), which takes a mutable instance. */
pico8_fixed_t pico8_da(pico8_instance_t *pt);

/* Lua passes the raw method value `jo.gg` as `jp`'s predicate, which calls
 * it as `e:gg()`; the mv() dispatch here is that same call, not an
 * approximation of it. */
pico8_fixed_t pico8_cz(void);

pico8_fixed_t pico8_db(const pico8_instance_t *pt);

/* Returns a fresh 6-entry positional table the caller owns and must
 * release with pico8_ob_table_free(). Entry 4 is a heap string that the
 * table does not own and freeing the table does not release. */
pico8_ob_table_t *pico8_ky(void);

pico8_fixed_t pico8_bf(void);

/* Borrowed, never freed here: it is the field table of the difficulty
 * instance `eu()` was given, and that instance owns it. NULL until `eu()`
 * (pico8_cart.c) calls the setter. */
const pico8_ob_table_t *pico8_dk(void);
void pico8_economy_set_dk(const pico8_ob_table_t *dk);

#endif
