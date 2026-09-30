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

/* First member of every entity struct, and of the parent struct a child
 * embeds first. `me` and `t` are ke's (`me` NULL is nil):
 * pico8_ke_struct_init sets them; on an entity that never went through it
 * they are left unset and never read. */
typedef struct {
    const pico8_struct_class_t *cls;
    const char *me;
    pico8_fixed_t t;
} pico8_struct_header_t;

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
struct pico8_emitter;
struct pico8_probe;
struct pico8_str_seq;
struct pico8_kind_rec;

/* One `static const` per converted class. A NULL member: the class has no
 * such method. `kj`, `ev` and `hd` read the vectors they get and never keep
 * them; `ldown`, `rdown` and `hq` may keep theirs. NULL `ra`: nil. `has_hb`
 * is NULL when a non-NULL `hb` is enough for `e.hb` to exist; otherwise the
 * instance decides. `mm` returns a fresh particle the caller owns. `parent` returns the class
 * whose struct this one embeds first; the member is NULL for a root.
 * `emitter_of` and `probe_of` return the particle emitter and the probe the
 * instance stores in itself; the member is NULL for a class that carries
 * none. A class that sets one descends from ke; each is only ever
 * called as `e->cls->emitter_of(e)`, `e->cls->probe_of(e)`. */
struct pico8_struct_class {
    void (*kj)(void *self, const pico8_vec_t *ra);
    void (*ev)(void *self, const pico8_vec_t *ra);
    int (*hd)(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
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
    struct pico8_qy *(*mm)(void *self);
    pico8_state_fn states[PICO8_STATE_COUNT];
    /* `PICO8_TAG(t)` set: lt() registers every instance under tag t. */
    unsigned tags;
    const pico8_struct_class_t *(*parent)(void);
    struct pico8_emitter *(*emitter_of)(void *self);
    struct pico8_probe *(*probe_of)(void *self);
    /* A NULL member: the class has no such field, for every instance. A
     * reader returns NULL, or 0 leaving *out untouched, when the field is
     * nil; `get_fx` and `get_p_` return its Lua truth. `set_m_` also marks
     * `m_` present. Each is only ever called as `e->cls->get_X(e, ...)`. */
    int (*get_fa)(const void *self, pico8_fixed_t *out);
    const pico8_vec_t *(*get_ra)(const void *self);
    const pico8_vec_t *(*get_p)(const void *self);
    const pico8_qz_box_t *(*get_kq)(const void *self);
    int (*get_ql)(const void *self, pico8_fixed_t *out);
    int (*get_fg)(const void *self, pico8_fixed_t *out);
    int (*get_mt)(const void *self, pico8_fixed_t *out);
    int (*get_fx)(const void *self);
    int (*get_p_)(const void *self);
    void (*set_m_)(void *self, int m_);
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

#endif
