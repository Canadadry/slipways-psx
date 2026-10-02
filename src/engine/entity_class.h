#ifndef ENGINE_ENTITY_CLASS_H
#define ENGINE_ENTITY_CLASS_H

#include <string.h>

#include "devtools/class_coverage.h"
#include "p8/fixed.h"
#include "engine/geom.h"
#include "engine/vec.h"

/* The states update_entities() runs: the method of `states` that `state`
 * names, called with `state_time`. */
typedef enum {
    ENTITY_STATE_NORMAL,
    ENTITY_STATE_SETTLED,
    ENTITY_STATE_VANISHING,
    ENTITY_STATE_PLACING,
    ENTITY_STATE_UNFOLDING,
    STATE_COUNT
} state_t;

/* The state with this name, or -1 for any other name ("unknown", "known"). */
static inline int entity_state_of(const char *me) {
    static const char *const names[STATE_COUNT] = {
        [ENTITY_STATE_NORMAL] = "normal",
        [ENTITY_STATE_SETTLED] = "settled",
        [ENTITY_STATE_VANISHING] = "vanishing",
        [ENTITY_STATE_PLACING] = "placing",
        [ENTITY_STATE_UNFOLDING] = "unfolding",
    };
    int i;

    for (i = 0; i < STATE_COUNT; i++) {
        if (strcmp(names[i], me) == 0) {
            return i;
        }
    }
    return -1;
}

/* The ten tags the registry indexes entities under, in the order
 * every class lists its own. */
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

/* The four classes whose descendants the code asks after. */
typedef enum {
    LINEAGE_PLANET,
    LINEAGE_STRUCTURE,
    LINEAGE_TRADING_HUB,
    LINEAGE_PROBE,
} lineage_t;

#define LINEAGE(l) (1u << (l))

/* The tag named `name`, or -1 for any other name. */
static inline int entity_tag_named(const char *name) {
    static const char *const names[TAG_COUNT] = {
        [TAG_STRUCTURE] = "structure", [TAG_ASCENSION] = "ascension", [TAG_SLIPGATE] = "slipgate",
        [TAG_PROBE] = "probe", [TAG_MESSAGE_BOX] = "message_box", [TAG_SLIPWAY] = "slipway",
        [TAG_WORMHOLE] = "wormhole", [TAG_PLANET] = "planet", [TAG_NODE] = "node",
        [TAG_TARGETING] = "targeting",
    };
    int i;

    for (i = 0; i < TAG_COUNT; i++) {
        if (strcmp(names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

typedef struct entity_class entity_class_t;
struct emitter;
struct targeting;

/* First member of every entity struct, and of the parent struct a child
 * embeds first. `state` NULL: no state. A component link points at the
 * field's storage, the archetype's for a field the class never writes;
 * NULL: the class has no such field, or it is absent. Written through:
 * `shake` by entity_geom_shake(), `is_dead` by its class, `is_hovered` by
 * the hover tracking of game/ui.c, and `emitter` and `targeting`, which
 * the instance stores in itself, by their modules. `*fold_anchor` 0:
 * absent. */
typedef struct {
    const entity_class_t *class;
    const char *state;
    p8_fixed_t state_time;
    const p8_fixed_t *layer;
    const vec_t *const *pos;
    const vec_t *const *map_pos;
    const box_t *hit_box;
    const p8_fixed_t *cursor_sprite;
    const p8_fixed_t *gpio_signal;
    const p8_fixed_t *infraspace;
    p8_fixed_t *shake;
    const p8_fixed_t *fold_anchor;
    int *is_dead;
    int *is_hovered;
    struct emitter *emitter;
    struct targeting *targeting;
} entity_t;

/* Every constructor calls it first: its zone block leaves the header
 * indeterminate, and a reader would follow a link left unset. `state` is
 * NULL, `state_time` 0 and every link NULL until the constructor sets its
 * own. */
static inline void entity_init_header(entity_t *self, const entity_class_t *cls) {
    *self = (entity_t){.class = cls};
}

/* What `left_press`, `right_press` and `accept_drop` return: `stop`
 * nonzero when the handler takes the event, 0 when it declines or has
 * nothing to say; `held` the entity it hands back, NULL otherwise. `held`
 * != NULL implies `stop`. */
typedef struct {
    int stop;
    entity_t *held;
} down_t;

typedef void (*state_fn_t)(entity_t *self, p8_fixed_t t);

struct particle;
struct trade_offer;
struct str_seq;
struct kind_record;

/* One `static const` per class. A NULL member: the class has no such
 * method. `draw`, `draw_overlay` and `hit_test` read the vectors they get
 * and never keep them; `left_press`, `right_press` and `drag` may keep
 * theirs. A NULL screen position: none given. `has_refresh` is NULL when a
 * non-NULL `refresh` is enough for the entity to have one; otherwise the
 * instance decides. `spawn_particle` returns a fresh particle the caller
 * owns. */
struct entity_class {
    void (*draw)(entity_t *self, const vec_t *ra);
    void (*draw_overlay)(entity_t *self, const vec_t *ra);
    int (*hit_test)(entity_t *self, const vec_t *qa, const vec_t *ra);
    void (*new_year)(entity_t *self, p8_fixed_t yr);
    void (*refresh)(entity_t *self);
    int (*has_refresh)(const entity_t *self);
    down_t (*left_press)(entity_t *self, vec_t *qa, vec_t *ra);
    down_t (*right_press)(entity_t *self, vec_t *qa, vec_t *ra);
    down_t (*accept_drop)(entity_t *self, const entity_t *s);
    void (*drag)(entity_t *self, vec_t *qa, vec_t *ra);
    void (*drop)(entity_t *self, entity_t *kv);
    void (*attach_slipway)(entity_t *self, entity_t *sw);
    void (*offer_trade)(entity_t *self, struct trade_offer *lw);
    void (*draw_at)(entity_t *self, vec_t p, p8_fixed_t a);
    struct particle *(*spawn_particle)(entity_t *self);
    state_fn_t states[STATE_COUNT];
    /* Set of `TAG(t)`: entity_register() files every instance under each
     * such tag. */
    unsigned tags;
    /* Set of `LINEAGE(l)`: the class is that lineage's or descends from
     * it. */
    unsigned lineage;
    void (*apply_camera)(entity_t *self, p8_fixed_t fy);
    /* Releases what the instance holds in the interface and particle
     * zones; only exit_queue_flush() calls it. */
    void (*release)(entity_t *self);
};

/* The construction keys of the planet family. NULL: the key is absent.
 * The constructor reads the struct, never keeps it, and adopts the string
 * and vectors it points at, without copying them. */
typedef struct {
    const char *planet_id;
    vec_t *map_pos;
    vec_t *pos;
} planet_keys_t;

/* A kind handled as a value (the kind table, a structure icon's `kind`, a
 * research project's `class_name`): one `static const` per kind, never
 * freed. Not an entity: it has no `entity_t` header. `new_entity` builds
 * one entity of the kind and returns its header. `record`, never NULL,
 * holds the kind's `required_tech`, `sprite`, `state` and `cost` whether
 * its modules are up or down. */
typedef struct {
    entity_t *(*new_entity)(const planet_keys_t *keys);
    const struct kind_record *record;
} entity_kind_t;

/* Here, not in game/slipway.h / game/probe.h: ui.c compares against them,
 * and slipway.c and probe.c include game/ui.h, so either header in ui.c
 * would make a mutual include. */
const entity_class_t *slipway_class(void);
const entity_class_t *probe_class(void);

#endif
