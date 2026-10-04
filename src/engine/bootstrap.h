#ifndef ENGINE_BOOTSTRAP_H
#define ENGINE_BOOTSTRAP_H

#include "engine/entity_registry.h"

/* Rebuilds the registries and the main camera, mouse and user interface
 * from scratch on every call: cart_init() calls it, and cart_start_game()
 * again through reset_world_with_reserve(). It initialises the entity, camera, mouse and user-interface modules
 * only when they are not ready, so a caller that skipped cart_load() still
 * gets them; what it creates registers itself through entity_register().
 * Every live entity whose class has a release goes to the exit queue first
 * (engine/exit_queue.h), released at the next flush, never during this
 * call. The three instances live in the game zone: never freed. */
void reset_world(void);
/* reset_world(), the registries taken at their reserve. */
void reset_world_with_reserve(const entity_registry_reserve_t *reserve);

/* Shuts down every module reset_world() initialises; the main camera,
 * mouse and user interface stay in the game zone, never freed. Idempotent,
 * and safe before the first reset_world(). */
void bootstrap_shutdown(void);

/* One frame of entity logic. Requires reset_world() (or at least
 * entity_registry_init()); an empty live list is a no-op. */
void update_entities(void);

#endif
