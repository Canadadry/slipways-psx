#ifndef GAME_TARGETING_H
#define GAME_TARGETING_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/vram.h"
#include "engine/entity_class.h"
#include "engine/entity_id_type.h"
#include "engine/vec.h"

typedef enum {
    TARGETING_TARGET_NONE,
    TARGETING_TARGET_ENTITY,
    TARGETING_TARGET_TRACK_THROWAWAY,
    TARGETING_TARGET_SLIPWAY_TOO_FAR,
    TARGETING_TARGET_WORMHOLE_TOO_FAR,
} targeting_target_kind_t;

typedef struct {
    targeting_target_kind_t kind;
    entity_id_t id;
} targeting_target_t;

typedef struct targeting {
    const targeting_archetype_t *archetype;
    targeting_target_t target;
    p8_fixed_t cost;
    int has_cost;
    vec_t track_pos;
    vec_t track_map_pos;
} targeting_t;

/* The entity, the static sentinel, or NULL once the entity was flushed. */
entity_t *targeting_target(const targeting_t *self);
void targeting_set_target(targeting_t *self, entity_t *target);

void targeting_track(entity_t *self, vec_t *mp, vec_t *rp);

void targeting_track_method(entity_t *self, vec_t *mouse_world, vec_t *mouse_screen);

void probe_link_target(entity_t *et, entity_t *self);

typedef struct {
    entity_t header;
    entity_id_t targeted_by;
} targeting_track_target_t;

const entity_class_t *targeting_track_target_class(void);

void targeting_tooltip(const p8_graphics_t *gfx, p8_vram_t *mem, entity_t *self);

void targeting_tooltip_show(const p8_graphics_t *gfx, p8_vram_t *mem, state_t state, const targeting_t *pb);

void targeting_tooltip_method(entity_t *self, const vec_t *screen_pos);

int targeting_confirm(entity_t *self);

void targeting_confirm_method(entity_t *self, entity_t *dropped);

#endif
