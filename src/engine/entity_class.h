#ifndef ENGINE_ENTITY_CLASS_H
#define ENGINE_ENTITY_CLASS_H

#include "p8/fixed.h"
#include "engine/geom.h"
#include "engine/vec.h"
#include "engine/entity_id_type.h"

typedef enum {
    ENTITY_STATE_NONE,
    ENTITY_STATE_NORMAL,
    ENTITY_STATE_SETTLED,
    ENTITY_STATE_VANISHING,
    ENTITY_STATE_PLACING,
    ENTITY_STATE_UNFOLDING,
    ENTITY_STATE_UNKNOWN,
    ENTITY_STATE_KNOWN,
    STATE_COUNT
} state_t;

typedef enum {
    TAG_STRUCTURE,
    TAG_ASCENSION,
    TAG_SLIPGATE,
    TAG_PROBE,
    TAG_MESSAGE_BOX,
    TAG_SLIPWAY,
    TAG_WORMHOLE,
    TAG_PLANET,
    TAG_NODE,
    TAG_TARGETING,
    TAG_COUNT
} tag_t;

#define TAG(t) (1u << (t))

typedef enum {
    LINEAGE_PLANET,
    LINEAGE_STRUCTURE,
    LINEAGE_TRADING_HUB,
    LINEAGE_PROBE,
} lineage_t;

#define LINEAGE(l) (1u << (l))

typedef struct entity_class entity_class_t;
struct emitter;
struct targeting;

#define ENTITY_HAS_LAYER (1u << 0)
#define ENTITY_HAS_HIT_BOX (1u << 1)
#define ENTITY_HAS_CURSOR_SPRITE (1u << 2)
#define ENTITY_HAS_INFRASPACE (1u << 3)
#define ENTITY_HAS_FOLD_ANCHOR (1u << 4)

typedef struct {
    const entity_class_t *class;
    state_t state;
    p8_fixed_t state_time;
    uint16_t has;
    p8_fixed_t layer;
    const vec_t *const *pos;
    const vec_t *const *map_pos;
    box_t hit_box;
    p8_fixed_t cursor_sprite;
    p8_fixed_t infraspace;
    p8_fixed_t *shake;
    p8_fixed_t fold_anchor;
    int *is_dead;
    int *is_hovered;
    struct emitter *emitter;
    struct targeting *targeting;
    entity_id_t id;
} entity_t;

static inline void entity_init_header(entity_t *self, const entity_class_t *cls) {
    *self = (entity_t){.class = cls};
}

static inline int entity_has(const entity_t *self, unsigned bit) {
    return (self->has & bit) != 0;
}

typedef struct {
    int stop;
    entity_id_t held;
} down_t;

typedef void (*state_fn_t)(entity_t *self, p8_fixed_t t);

struct particle;
struct trade_offer;
struct str_seq;
struct kind_record;

struct entity_class {
    void (*draw)(entity_t *self, const vec_t *screen_pos);
    void (*draw_overlay)(entity_t *self, const vec_t *screen_pos);
    int (*hit_test)(entity_t *self, const vec_t *mouse_world, const vec_t *mouse_screen);
    void (*new_year)(entity_t *self, p8_fixed_t year);
    void (*refresh)(entity_t *self);
    int (*has_refresh)(const entity_t *self);
    down_t (*left_press)(entity_t *self, vec_t *mouse_world, vec_t *mouse_screen);
    down_t (*right_press)(entity_t *self, vec_t *mouse_world, vec_t *mouse_screen);
    down_t (*accept_drop)(entity_t *self, const entity_t *s);
    void (*drag)(entity_t *self, vec_t *mouse_world, vec_t *mouse_screen);
    void (*drop)(entity_t *self, entity_t *dropped);
    void (*attach_slipway)(entity_t *self, entity_t *slipway);
    void (*offer_trade)(entity_t *self, struct trade_offer *offer);
    void (*draw_at)(entity_t *self, vec_t p, p8_fixed_t a);
    struct particle *(*spawn_particle)(entity_t *self);
    state_fn_t states[STATE_COUNT];
    unsigned tags;
    unsigned lineage;
    void (*apply_camera)(entity_t *self, p8_fixed_t parallax);
    void (*release)(entity_t *self);
};

typedef struct {
    const char *planet_id;
    vec_t *map_pos;
    vec_t *pos;
} planet_keys_t;

typedef struct {
    entity_t *(*new_entity)(const planet_keys_t *keys);
    const struct kind_record *record;
} entity_kind_t;

const entity_class_t *slipway_class(void);
const entity_class_t *probe_class(void);

#endif
