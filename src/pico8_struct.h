#ifndef PICO8_STRUCT_H
#define PICO8_STRUCT_H

#include <string.h>

#include "pico8_coverage.h"
#include "pico8_type_tag.h"
#include "pico8_value.h"
#include "pico8_vec.h"

typedef enum {
    PICO8_METHOD_OL,
    PICO8_METHOD_HQ,
    PICO8_METHOD_EY,
    PICO8_METHOD_EB,
    PICO8_METHOD_GG,
    PICO8_METHOD_KF,
    PICO8_METHOD_CN,
    PICO8_METHOD_IB,
    PICO8_METHOD_MM,
    PICO8_METHOD_COUNT
} pico8_method_t;

/* The method named `name`, or -1 when no struct has a method by that name:
 * the name is then looked up as a field. */
static inline int pico8_method_of(const char *name) {
    static const char *const names[PICO8_METHOD_COUNT] = {
        [PICO8_METHOD_OL] = "ol",
        [PICO8_METHOD_HQ] = "hq",
        [PICO8_METHOD_EY] = "ey",
        [PICO8_METHOD_EB] = "eb",
        [PICO8_METHOD_GG] = "gg",
        [PICO8_METHOD_KF] = "kf",
        [PICO8_METHOD_CN] = "cn",
        [PICO8_METHOD_IB] = "ib",
        [PICO8_METHOD_MM] = "mm",
    };
    int i;

    for (i = 0; i < PICO8_METHOD_COUNT; i++) {
        if (strcmp(names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

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

/* Returns 1 and writes *out_value when `self` has a non-nil field `name`;
 * returns 0, leaving *out_value untouched, otherwise. */
typedef int (*pico8_struct_field_fn)(const void *self, const char *name, pico8_value_t *out_value);

/* Returns 1 when `self` has a member `name` and has written `value` into
 * it; returns 0, writing nothing, otherwise. */
typedef int (*pico8_struct_set_field_fn)(void *self, const char *name, pico8_value_t value);

typedef struct pico8_struct_class pico8_struct_class_t;

/* First member of every entity struct, and of the parent struct a child
 * embeds first: the tag stays at offset 0 for pico8_type_tag_of(). `tag` is
 * always PICO8_TYPE_TAG_STRUCT. */
typedef struct {
    pico8_type_tag_t tag;
    const pico8_struct_class_t *cls;
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

/* One `static const` per converted class. A NULL member: the class has no
 * such method. `kj`, `ev` and `hd` read the vectors they get and never keep
 * them; `ldown` and `rdown` may keep theirs. NULL `ra`: nil. `has_hb` is
 * NULL when a non-NULL `hb` is enough for `e.hb` to exist; otherwise the
 * instance decides. `methods` is indexed by pico8_method_t. `field` is NULL
 * only for a class no generic consumer ever reaches, `set_field` for a
 * class no generic writer ever reaches. */
struct pico8_struct_class {
    pico8_value_function_t methods[PICO8_METHOD_COUNT];
    void (*kj)(void *self, const pico8_vec_t *ra);
    void (*ev)(void *self, const pico8_vec_t *ra);
    int (*hd)(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra);
    void (*hk)(void *self, pico8_fixed_t yr);
    void (*hb)(void *self);
    int (*has_hb)(const void *self);
    pico8_down_t (*ldown)(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
    pico8_down_t (*rdown)(void *self, pico8_vec_t *qa, pico8_vec_t *ra);
    pico8_down_t (*gv)(void *self, const pico8_struct_header_t *s);
    pico8_state_fn states[PICO8_STATE_COUNT];
    pico8_struct_field_fn field;
    pico8_struct_set_field_fn set_field;
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
 * `static const` per kind, never freed, whose first member is no tag, so
 * pico8_type_tag_of() reads it UNKNOWN. `new_entity` is the kind's
 * pico8_X_new(). `field` reads the kind's defaults; it returns 0, leaving
 * *out_value untouched, when `name` is absent from the whole chain. */
typedef struct {
    void *(*new_entity)(const pico8_jo_keys_t *keys);
    int (*field)(const char *name, pico8_value_t *out_value);
} pico8_entity_kind_t;

#endif
