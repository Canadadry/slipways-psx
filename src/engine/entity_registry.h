#ifndef ENGINE_ENTITY_REGISTRY_H
#define ENGINE_ENTITY_REGISTRY_H

#include "engine/entity_list.h"
#include "engine/entity_class.h"

/* (Re)builds the live list and the per-message and per-tag registries
 * empty, growing in the current game-zone generation (engine/game_zone.h),
 * after entity_registry_shutdown() dropped the previous build, and puts back
 * the placeholder main camera. Must run before anything below is used.
 * reset_world() (engine/bootstrap.c) calls it, then replaces the placeholder
 * through entity_registry_set_main_camera(). */
void entity_registry_init(void);

/* Drops the backing arrays of the live list and the registries, which go
 * back with their game-zone generation; the main camera is dropped unread:
 * the placeholder is static, a real camera lives in the game zone. The
 * registered entities stay owned by whoever instantiated them. A second
 * call, or a call before the first entity_registry_init(), is a no-op. */
void entity_registry_shutdown(void);

/* Every live entity whose class has a release goes to the exit queue
 * (engine/exit_queue.h); the live list itself is left as is. */
void entity_registry_abandon_live_entities(void);

/* Empties the live list and the registries without reading their arrays,
 * which a game-zone reset has just poisoned; the main camera and the
 * initialised state are kept. */
void entity_registry_forget_registries(void);

/* Every entity registered and not yet unregistered, in registration
 * order, as pointers to a struct that starts with an entity_t
 * (engine/entity_class.h). */
const entity_list_t *live_entities(void);

/* The entities registered under the message `name` ("draw",
 * "draw_overlay", "hit_test", "new_year" or "refresh"), or NULL if `name`
 * is not one of those five. The five buckets are fixed, so a returned
 * pointer stays valid for the module's whole lifetime. */
const entity_list_t *entities_by_message(const char *name);

/* The entities registered under `tag`, or NULL if no entity carrying that
 * tag has been registered since entity_registry_init() -- an unknown tag
 * included. A returned list holds until the next entity_registry_init(),
 * entity_registry_shutdown() or entity_registry_forget_registries(). */
const entity_list_t *entities_by_tag(const char *tag);

/* Only entity_construct calls it outside tests: every live entity has had
 * its state name and state time set by entity_init_instance, and
 * update_entities() reads them. */
entity_t *entity_register(entity_t *e);
/* An entity taken out of the live list here, with a class that has a
 * release, goes to the exit queue: once only, even if it is unregistered
 * again after reset_world(). */
void entity_unregister(entity_t *e);

/* The message is "draw" or "draw_overlay" (asserted). */
void draw_layers(const char *message);

/* The hit test of background and modal_backdrop: every point passes it. */
int entity_registry_hit_test_always(entity_t *self, const vec_t *mouse_world, const vec_t *mouse_screen);

/* Until reset_world() runs, the placeholder struct, with no member and no
 * apply_camera, which draw_layers() calls on a real camera only; afterwards
 * the real camera (game/scroll.h). Its members are read and written through
 * main_camera_typed(). Callers must not free it. */
entity_t *main_camera(void);

const entity_class_t *main_camera_placeholder_class(void);

/* Makes `instance` what main_camera() returns. The outgoing one lives on
 * with its generation: reset_world() can run from inside a dispatch chain
 * that still holds the old camera. NULL is a valid "no camera"; passing
 * back the current main_camera() is a no-op. */
void entity_registry_set_main_camera(entity_t *instance);

#endif
