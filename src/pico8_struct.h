#ifndef PICO8_STRUCT_H
#define PICO8_STRUCT_H

#include <string.h>

#include "pico8_coverage.h"
#include "pico8_fixed.h"
#include "pico8_geom.h"
#include "pico8_vec.h"

/* The states `dg` runs as `qc[qc.me](qc,qc.t)`. */
typedef enum {
    PICO8_STATE_PH,
    PICO8_STATE_LY,
    PICO8_STATE_LL,
    PICO8_STATE_KZ,
    PICO8_STATE_PR,
    PICO8_STATE_COUNT
} pico8_state_t;

/* The state named `me`, or -1 for any other name (`unknown`, `known`). */
static inline int pico8_state_of(const char *me) {
    static const char *const names[PICO8_STATE_COUNT] = {
        [PICO8_STATE_PH] = "ph",
        [PICO8_STATE_LY] = "ly",
        [PICO8_STATE_LL] = "ll",
        [PICO8_STATE_KZ] = "kz",
        [PICO8_STATE_PR] = "pr",
    };
    int i;

    for (i = 0; i < PICO8_STATE_COUNT; i++) {
        if (strcmp(names[i], me) == 0) {
            return i;
        }
    }
    return -1;
}

/* The ten tags `cg` indexes entities under, in the order
 * every class lists its own. */
typedef enum {
    PICO8_TAG_QH,
    PICO8_TAG_HJ,
    PICO8_TAG_NR,
    PICO8_TAG_G_,
    PICO8_TAG_MH,
    PICO8_TAG_JC,
    PICO8_TAG_MZ,
    PICO8_TAG_JO,
    PICO8_TAG_NP,
    PICO8_TAG_PROBER,
    PICO8_TAG_COUNT
} pico8_tag_t;

#define PICO8_TAG(t) (1u << (t))

/* The four classes whose descendants the code asks after. */
typedef enum {
    PICO8_LINEAGE_JO,
    PICO8_LINEAGE_QH,
    PICO8_LINEAGE_NL,
    PICO8_LINEAGE_G_,
} pico8_lineage_t;

#define PICO8_LINEAGE(l) (1u << (l))

/* The tag named `name`, or -1 for any other name. */
static inline int pico8_tag_named(const char *name) {
    static const char *const names[PICO8_TAG_COUNT] = {
        [PICO8_TAG_QH] = "qh", [PICO8_TAG_HJ] = "hj", [PICO8_TAG_NR] = "nr",
        [PICO8_TAG_G_] = "g_", [PICO8_TAG_MH] = "mh", [PICO8_TAG_JC] = "jc",
        [PICO8_TAG_MZ] = "mz", [PICO8_TAG_JO] = "jo", [PICO8_TAG_NP] = "np",
        [PICO8_TAG_PROBER] = "prober",
    };
    int i;

    for (i = 0; i < PICO8_TAG_COUNT; i++) {
        if (strcmp(names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

typedef struct pico8_struct_class pico8_struct_class_t;
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
    const pico8_struct_class_t *cls;
    const char *me;
    pico8_fixed_t t;
    const pico8_fixed_t *fa;
    const pico8_vec_t *const *ra;
    const pico8_vec_t *const *p;
    const pico8_qz_box_t *kq;
    const pico8_fixed_t *ql;
    const pico8_fixed_t *fg;
    const pico8_fixed_t *mt;
    pico8_fixed_t *ma;
    const pico8_fixed_t *fx;
    int *p_;
    int *m_;
    struct pico8_emitter *emitter;
    struct pico8_probe *probe;
} pico8_struct_header_t;

/* Every constructor calls it first: its `malloc` leaves the header
 * indeterminate, and a reader would follow a link left unset. `me` is nil,
 * `t` 0 and every link NULL until the constructor sets its own. */
static inline void pico8_struct_header_init(pico8_struct_header_t *self, const pico8_struct_class_t *cls) {
    *self = (pico8_struct_header_t){.cls = cls};
}

/* What `ldown`, `rdown` and `gv` return: `stop` is the Lua truth of the
 * cart's return value -- 0 for nil as well as for false: a `stop` that only
 * caught false swallowed every click near an unrevealed planet --, `held`
 * the entity it returned, NULL otherwise. `held` != NULL implies `stop`. */
typedef struct {
    int stop;
    pico8_struct_header_t *held;
} pico8_down_t;

typedef void (*pico8_state_fn)(void *self, pico8_fixed_t t);

struct pico8_qy;
struct pico8_jo_trade_proposal;
struct pico8_str_seq;
struct pico8_kind_rec;

/* One `static const` per converted class. A NULL member: the class has no
 * such method. `kj`, `ev` and `hd` read the vectors they get and never keep
 * them; `ldown`, `rdown` and `hq` may keep theirs. NULL `ra`: nil. `has_hb`
 * is NULL when a non-NULL `hb` is enough for `e.hb` to exist; otherwise the
 * instance decides. `mm` returns a fresh particle the caller owns. */
struct pico8_struct_class {
    void (*kj)(void *self, const pico8_vec_t *ra);
    void (*ev)(void *self, const pico8_vec_t *ra);
    int (*hd)(pico8_struct_header_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
    void (*hk)(void *self, pico8_fixed_t yr);
    void (*hb)(void *self);
    int (*has_hb)(const void *self);
    pico8_down_t (*ldown)(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
    pico8_down_t (*rdown)(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
    pico8_down_t (*gv)(void *self, const pico8_struct_header_t *s);
    void (*hq)(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
    void (*ey)(void *self, pico8_struct_header_t *kv);
    void (*kf)(void *self, pico8_struct_header_t *sw);
    void (*cn)(void *self, struct pico8_jo_trade_proposal *lw);
    void (*ol)(void *self, pico8_vec_t p, pico8_fixed_t a);
    struct pico8_qy *(*mm)(pico8_struct_header_t *self);
    pico8_state_fn states[PICO8_STATE_COUNT];
    /* `PICO8_TAG(t)` set: lt() registers every instance under tag t. */
    unsigned tags;
    /* `PICO8_LINEAGE(l)` set: the class is l's or descends from it. */
    unsigned lineage;
};

/* The construction keys of the jo tree (the cart's `mw(ob)` in `kv:jr`).
 * NULL: the key is absent, which is what a nil key is in the cart. The
 * constructor reads the struct, never keeps it, and adopts the string and
 * vectors it points at, without copying them. */
typedef struct {
    const char *id;
    pico8_vec_t *p;
    pico8_vec_t *ra;
} pico8_jo_keys_t;

/* A kind the cart handles as a value (`gx[]`, `ie.ke`, `kv[hp.cw]`): one
 * `static const` per kind, never freed. Not an entity: it has no
 * pico8_struct_header_t. `new_entity` builds one of the kind and returns
 * its header (pico8_X_kind_new). `rec`, never NULL, holds the kind's `rr`,
 * `s`, `me` and `od` whether its modules are up or down. */
typedef struct {
    pico8_struct_header_t *(*new_entity)(const pico8_jo_keys_t *keys);
    const struct pico8_kind_rec *rec;
} pico8_entity_kind_t;

/* Here, not in pico8_slipway.h / pico8_probe.h: ui.c compares against
 * them, and including either header from ui.c makes an include pair. */
const pico8_struct_class_t *pico8_jc_struct_class(void);
const pico8_struct_class_t *pico8_g__struct_class(void);

#endif
