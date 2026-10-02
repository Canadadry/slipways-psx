#ifndef GAME_STRUCTURE_ICON_H
#define GAME_STRUCTURE_ICON_H

#include "game/game_data.h"
#include "p8/gfx.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* Throughout this module a NULL `ie` struct, `gfx` or `mem` is a safe
 * no-op, and a nil member, or an absent field of `self.ke`, reads as its
 * zero/false default; a wrong construction key or field of `self.ke` stops
 * the run.
 *
 * `self.ke` holds a `pico8_entity_kind_t`, not an entity: its fields are
 * read off a `pico8_ie_ke_t`, never through a struct header on the kind
 * pointer. */

/* `ra` starts on the archetype's, shared by every `ie`; `hb` rebinds it to
 * the cursor. `gy` and `id` are no members: the module's shared state, read
 * by pico8_ie_gy()/pico8_ie_id(). The struct lives in the game generation
 * current at its construction, and dies with it. */
typedef struct {
    entity_t header;
    const structure_icon_archetype_t *archetype;
    const vec_t *pos;
    const entity_kind_t *kind;
    tooltip_t *tooltip;
    int is_revealed;
    int has_is_revealed; /* 0: `kh` is nil, and is then never read */
    int is_hovered;
} structure_icon_t;

/* `self.ke` read as Lua reads a class: no parent, never an entity, never
 * written. */
typedef struct {
    entity_t header;
    const char *required_tech;
    p8_fixed_t sprite;
    int has_sprite; /* 0: `s` is nil, and is then never read */
    const char *state;
    p8_fixed_t cost;
    int has_cost; /* 0: `od` is nil, and is then never read */
} structure_icon_kind_t;

/* Calls pico8_research_icon_shutdown(), then pico8_pi_init(); puts `ie.id`
 * back to 0. */
void structure_icon_init(void);

/* Clears `ie.gy`, frees nothing. Idempotent. */
void structure_icon_shutdown(void);

/* 1 from pico8_research_icon_init() to pico8_research_icon_shutdown(), 0 otherwise. */
int structure_icon_ready(void);
const entity_class_t *structure_icon_class(void);

/* The keys of `pico8_ie_new`, read, not kept; NULL is nil. `ke` (a
 * planet-upgrade kind) and `jb` (a tooltip struct) are borrowed. */
typedef struct {
    const entity_kind_t *kind;
    tooltip_t *tooltip;
} structure_icon_keys_t;

/* `keys` NULL means none. The struct, made in the current game generation,
 * registers itself into `hv`/`cv` through `lt`. */
structure_icon_t *structure_icon_new(const structure_icon_keys_t *keys);

/* `ie.eh`, `pico8_ie_def.eh`, for `_init` to point `ie.gy` at: an alias,
 * not a copy. NULL before pico8_research_icon_init() and after the
 * shutdown. */
const vec_t *structure_icon_rest_cursor(void);

/* The shared `ie.gy` cursor, borrowed; NULL while `gy` is nil. */
const vec_t *structure_icon_cursor(void);

/* The shared `ie.id` counter: 0 while `id` is nil, else 1 with `*out` set. */
int structure_icon_sprite_counter(p8_fixed_t *out);

/* NULL is a nil `gy`. */
void structure_icon_set_cursor(const vec_t *gy);

/* Reveals the icon once, claiming the next slot off the shared `ie.gy`
 * cursor: a box made in the current game generation. Requires
 * pico8_research_icon_init(). */
void structure_icon_refresh(structure_icon_t *self);

/* Returns a fresh instance of the class `self.ke` names, for the caller to
 * hold as a placement in progress, or an absent value when `self.ke` is not
 * a class; bumps the shared `ie.id` either way. The instance and its `id`
 * share a grab arena. Requires pico8_research_icon_init(). */
down_t structure_icon_left_press(structure_icon_t *self);

/* By value, one per read: each member copied off the chain of the class
 * `self.ke` holds, nil when absent there or when `self.ke` is. `rr` and
 * `me` are borrowed from the kind's defaults. */
structure_icon_kind_t structure_icon_kind_view(const structure_icon_t *self);
const entity_class_t *structure_icon_kind_view_class(void);

void structure_icon_draw(p8_graphics_t *gfx, p8_memory_t *mem, structure_icon_t *self, vec_t p);

#endif
