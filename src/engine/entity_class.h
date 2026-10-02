#ifndef ENGINE_ENTITY_CLASS_H
#define ENGINE_ENTITY_CLASS_H

#include <string.h>

#include "devtools/class_coverage.h"
#include "p8/fixed.h"
#include "engine/geom.h"
#include "engine/vec.h"

/* The states `dg` runs as `qc[qc.me](qc,qc.t)`. */
typedef enum {
    ENTITY_STATE_NORMAL,
    ENTITY_STATE_SETTLED,
    ENTITY_STATE_VANISHING,
    ENTITY_STATE_PLACING,
    ENTITY_STATE_UNFOLDING,
    STATE_COUNT
} state_t;

/* The state named `me`, or -1 for any other name (`unknown`, `known`). */
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

/* The ten tags `cg` indexes entities under, in the order
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
struct pico8_emitter;
struct pico8_probe;

/* First member of every entity struct, and of the parent struct a child
 * embeds first. `me` and `t` are ke's (`me` NULL is nil). A component link
 * points at the field's storage, the archetype's for a field the class
 * never writes; NULL: the class has no such field, or it is nil. `ma` is
 * written through, by `pico8_ma`, `p_`, by `d_`, `m_`, by `ui`'s hover,
 * and the particle emitter and the probe, which the instance stores in
 * itself, by their modules. `*fx` 0 is nil. */
typedef struct {
    const entity_class_t *class;
    const char *state;
    p8_fixed_t state_time;
    const p8_fixed_t *layer;
    const pico8_vec_t *const *pos;
    const pico8_vec_t *const *map_pos;
    const pico8_qz_box_t *hit_box;
    const p8_fixed_t *cursor_sprite;
    const p8_fixed_t *gpio_signal;
    const p8_fixed_t *infraspace;
    p8_fixed_t *shake;
    const p8_fixed_t *fold_anchor;
    int *is_dead;
    int *is_hovered;
    struct pico8_emitter *emitter;
    struct pico8_probe *targeting;
} entity_t;

/* Every constructor calls it first: its zone block leaves the header
 * indeterminate, and a reader would follow a link left unset. `me` is nil,
 * `t` 0 and every link NULL until the constructor sets its own. */
static inline void entity_init_header(entity_t *self, const entity_class_t *cls) {
    *self = (entity_t){.class = cls};
}

/* What `ldown`, `rdown` and `gv` return: `stop` is the Lua truth of the
 * cart's return value -- 0 for nil as well as for false: a `stop` that only
 * caught false swallowed every click near an unrevealed planet --, `held`
 * the entity it returned, NULL otherwise. `held` != NULL implies `stop`. */
typedef struct {
    int stop;
    entity_t *held;
} down_t;

typedef void (*state_fn_t)(entity_t *self, p8_fixed_t t);

struct pico8_qy;
struct pico8_jo_trade_proposal;
struct pico8_str_seq;
struct pico8_kind_rec;

/* One `static const` per converted class. A NULL member: the class has no
 * such method. `kj`, `ev` and `hd` read the vectors they get and never keep
 * them; `ldown`, `rdown` and `hq` may keep theirs. NULL `ra`: nil. `has_hb`
 * is NULL when a non-NULL `hb` is enough for `e.hb` to exist; otherwise the
 * instance decides. `mm` returns a fresh particle the caller owns. */
struct entity_class {
    void (*draw)(entity_t *self, const pico8_vec_t *ra);
    void (*draw_overlay)(entity_t *self, const pico8_vec_t *ra);
    int (*hit_test)(entity_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
    void (*new_year)(entity_t *self, p8_fixed_t yr);
    void (*refresh)(entity_t *self);
    int (*has_refresh)(const entity_t *self);
    down_t (*left_press)(entity_t *self, pico8_vec_t *qa, pico8_vec_t *ra);
    down_t (*right_press)(entity_t *self, pico8_vec_t *qa, pico8_vec_t *ra);
    down_t (*accept_drop)(entity_t *self, const entity_t *s);
    void (*drag)(entity_t *self, pico8_vec_t *qa, pico8_vec_t *ra);
    void (*drop)(entity_t *self, entity_t *kv);
    void (*attach_slipway)(entity_t *self, entity_t *sw);
    void (*offer_trade)(entity_t *self, struct pico8_jo_trade_proposal *lw);
    void (*draw_at)(entity_t *self, pico8_vec_t p, p8_fixed_t a);
    struct pico8_qy *(*spawn_particle)(entity_t *self);
    state_fn_t states[STATE_COUNT];
    /* `PICO8_TAG(t)` set: lt() registers every instance under tag t. */
    unsigned tags;
    /* `PICO8_LINEAGE(l)` set: the class is l's or descends from it. */
    unsigned lineage;
    void (*apply_camera)(entity_t *self, p8_fixed_t fy);
    /* Releases what the instance holds in the interface and particle
     * zones; only the exit queue's flush calls it (pico8_file_sortie.h). */
    void (*release)(entity_t *self);
};

/* The construction keys of the jo tree (the cart's `mw(ob)` in `kv:jr`).
 * NULL: the key is absent, which is what a nil key is in the cart. The
 * constructor reads the struct, never keeps it, and adopts the string and
 * vectors it points at, without copying them. */
typedef struct {
    const char *planet_id;
    pico8_vec_t *map_pos;
    pico8_vec_t *pos;
} planet_keys_t;

/* A kind the cart handles as a value (`gx[]`, `ie.ke`, `kv[hp.cw]`): one
 * `static const` per kind, never freed. Not an entity: it has no
 * pico8_struct_header_t. `new_entity` builds one of the kind and returns
 * its header (pico8_X_kind_new). `rec`, never NULL, holds the kind's `rr`,
 * `s`, `me` and `od` whether its modules are up or down. */
typedef struct {
    entity_t *(*new_entity)(const planet_keys_t *keys);
    const struct pico8_kind_rec *record;
} entity_kind_t;

/* Here, not in pico8_slipway.h / pico8_probe.h: ui.c compares against
 * them, and including either header from ui.c makes an include pair. */
const entity_class_t *pico8_jc_struct_class(void);
const entity_class_t *pico8_g__struct_class(void);

#endif
