#ifndef PICO8_CART_H
#define PICO8_CART_H

#include "pico8_class.h"
#include "pico8_value.h"

/* Declares every class and data table the cart's top-level statements build,
 * which real Lua runs once before `_init()`. Reads the cart memory through
 * pico8_dispatch_mem(): the caller must have set a `mem` with
 * pico8_load_map_json() already run against it, otherwise the cart data
 * tables are simply not built and every other class still is. A second call
 * frees and rebuilds every class, so it dangles the `cls` pointer of every
 * instance the previous one left alive.
 *
 * The asserts in its body cover the ordering constraints that were found;
 * an adjacency they do not cover is not thereby free. Re-derive before
 * inserting an `_init()`. */
void pico8_cart_load(void);

/* Frees everything pico8_cart_load() built, in reverse order so a class is
 * never freed while a later one still holds a `.parent` into it. Idempotent,
 * and safe before the first pico8_cart_load(). No Lua equivalent. */
void pico8_cart_shutdown(void);

/* Requires pico8_cart_load(). */
void pico8__init(void);

/* `_init` as a bare function value, for a button's `my` field. `self`, the
 * button, is discarded: `_init()` declares no parameter. */
pico8_value_t pico8_init_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* Starts a game from the clicked difficulty button `nj`, whose own fields
 * table becomes `dk` (borrowed, not copied); a NULL `nj` gives a NULL `dk`.
 * Requires pico8_cart_load(). */
void pico8_eu(pico8_instance_t *nj);

/* `eu` as a bare function value, for a `bo` entry's `my` field: the dispatch
 * `self` IS `nj`, never an `mv()` extra argument. */
pico8_value_t pico8_eu_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* Requires pico8_cart_load() and at least one prior pico8__init()/pico8_eu();
 * a not-yet-built `ok` or `hv` is a no-op rather than an error. */
void pico8__update60(void);

/* Reads the cart memory through pico8_dispatch_mem(); a NULL mem makes the
 * drawing calls that need it no-ops. */
void pico8__draw(void);

#endif
