#ifndef PICO8_BOOTSTRAP_H
#define PICO8_BOOTSTRAP_H

#include "pico8_class.h"

/* Rebuilds `hv`/`cv`/`cg` and the `mo`/`je`/`ok` instances from scratch on
 * every call -- the cart calls it once from `_init()` and again from
 * `eu()`. It builds `ke`/`qb`/`nf`/`ui` only when they are missing, so a
 * caller that skipped pico8_cart_load() still gets them; the instances it
 * creates register themselves into `hv`/`cv`/`cg` through `lt`, and nothing
 * frees them until pico8_bootstrap_shutdown(). */
void pico8_jl(void);

/* Frees the `mo`/`je`/`ok` instances through their owning modules, in
 * reverse order. Idempotent, and safe before the first pico8_jl(). No Lua
 * equivalent. */
void pico8_bootstrap_shutdown(void);

/* One frame of entity logic. Requires pico8_jl() (or at least
 * pico8_entity_init()); an empty `hv` is a no-op. */
void pico8_dg(void);

#endif
