#ifndef GAME_STRUCTURE_ICON_H
#define GAME_STRUCTURE_ICON_H

#include "game/game_data.h"
#include "p8/gfx.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* structure_icon_refresh() and structure_icon_draw() do nothing on a NULL
 * icon, and the draw nothing on a NULL graphics state or memory;
 * structure_icon_left_press() on a NULL icon still bumps the shared counter.
 * An absent field of the held kind reads as its zero default.
 *
 * `kind` holds an `entity_kind_t`, not an entity: its fields are read off a
 * `structure_icon_kind_t` view, never through a struct header on the kind
 * pointer. */

/* `pos` starts on the archetype's, shared by every icon; the refresh
 * rebinds it to the shared cursor. The cursor and the counter are no
 * members: the module's shared state, read by structure_icon_cursor() and
 * structure_icon_id_counter(). The struct lives in the game zone current
 * at its construction, and dies with it. */
typedef struct {
    entity_t header;
    const structure_icon_archetype_t *archetype;
    const vec_t *pos;
    const entity_kind_t *kind;
    tooltip_t *tooltip;
    int is_revealed;
    int has_is_revealed; /* 0: absent, and then never read */
    int is_hovered;
} structure_icon_t;

/* The held kind read as a class: no parent, never an entity, never
 * written. */
typedef struct {
    entity_t header;
    const char *required_tech;
    p8_fixed_t sprite;
    int has_sprite; /* 0: absent, and then never read */
    state_t state;
    p8_fixed_t cost;
    int has_cost; /* 0: absent, and then never read */
} structure_icon_kind_t;

/* Calls structure_icon_shutdown(); puts the
 * shared counter back to 0 and unsets the shared cursor. */
void structure_icon_init(void);

/* Unsets the shared cursor, frees nothing. Idempotent. */
void structure_icon_shutdown(void);

/* 1 from structure_icon_init() to structure_icon_shutdown(), 0 otherwise. */
int structure_icon_ready(void);
const entity_class_t *structure_icon_class(void);

/* The keys of structure_icon_new(), read, not kept; NULL is none. `kind` (a
 * planet-upgrade kind) and `tooltip` (a tooltip struct) are borrowed. */
typedef struct {
    const entity_kind_t *kind;
    tooltip_t *tooltip;
} structure_icon_keys_t;

/* `keys` NULL means none. The struct, made in the current game zone,
 * registers itself as a live entity through entity_construct(). */
structure_icon_t *structure_icon_new(const structure_icon_keys_t *keys);

/* `structure_icon_archetype.rest_cursor`, for cart_init() to point the
 * shared cursor at: an alias, not a copy. NULL before structure_icon_init()
 * and after the shutdown. */
const vec_t *structure_icon_rest_cursor(void);

/* The shared cursor, borrowed; NULL while it is unset. */
const vec_t *structure_icon_cursor(void);

/* The shared counter: 0 while it is unset, else 1 with the value written. */
int structure_icon_id_counter(p8_fixed_t *out);

/* NULL unsets the shared cursor. */
void structure_icon_set_cursor(const vec_t *cursor);

/* Reveals the icon once, claiming the next slot off the shared cursor: a
 * box made in the current game zone. Requires structure_icon_init(). */
void structure_icon_refresh(structure_icon_t *self);

/* Returns a fresh instance of the held kind, for the caller to hold as a
 * placement in progress, or an absent value when the icon holds no kind;
 * bumps the shared counter either way. The instance and its `planet_id`
 * share a grab arena. Requires structure_icon_init(). */
down_t structure_icon_left_press(structure_icon_t *self);

/* By value, one per read: each member copied off the held kind's record,
 * absent when absent there or when the icon holds no kind. `required_tech`
 * is borrowed from the kind's defaults. */
structure_icon_kind_t structure_icon_kind_view(const structure_icon_t *self);
const entity_class_t *structure_icon_kind_view_class(void);

void structure_icon_draw(p8_graphics_t *gfx, p8_memory_t *mem, structure_icon_t *self, vec_t p);

#endif
