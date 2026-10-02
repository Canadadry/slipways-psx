#ifndef ENGINE_BOOTSTRAP_H
#define ENGINE_BOOTSTRAP_H

/* Rebuilds `hv`/`cv`/`cg` and `mo`/`je`/`ok` from scratch on every call --
 * the cart calls it once from `_init()` and again from `eu()`. It builds
 * `ke`/`qb`/`nf`/`ui` only when they are missing, so a caller that skipped
 * pico8_cart_load() still gets them; what it creates registers itself into
 * `hv`/`cv`/`cg` through `lt`. Every entity still in `hv` whose class has
 * `rendre` goes to the exit queue first (pico8_file_sortie.h), released at
 * the next flush, never during this call. `mo` and `je` are never freed,
 * `ok` not until pico8_bootstrap_shutdown(). */
void reset_world(void);

/* Frees the `ok` instance through its owning module; `mo` and `je` are
 * never freed.
 * Idempotent, and safe before the first pico8_jl(). No Lua equivalent. */
void bootstrap_shutdown(void);

/* One frame of entity logic. Requires pico8_jl() (or at least
 * pico8_entity_init()); an empty `hv` is a no-op. */
void update_entities(void);

#endif
