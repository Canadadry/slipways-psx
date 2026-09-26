#ifndef PICO8_STRUCT_H
#define PICO8_STRUCT_H

#include <string.h>

#include "pico8_type_tag.h"
#include "pico8_value.h"

typedef enum {
    PICO8_METHOD_OL,
    PICO8_METHOD_PH,
    PICO8_METHOD_KJ,
    PICO8_METHOD_EV,
    PICO8_METHOD_LDOWN,
    PICO8_METHOD_MD,
    PICO8_METHOD_HQ,
    PICO8_METHOD_EY,
    PICO8_METHOD_KZ,
    PICO8_METHOD_PR,
    PICO8_METHOD_RDOWN,
    PICO8_METHOD_EB,
    PICO8_METHOD_HD,
    PICO8_METHOD_HB,
    PICO8_METHOD_GG,
    PICO8_METHOD_GV,
    PICO8_METHOD_HK,
    PICO8_METHOD_KF,
    PICO8_METHOD_CN,
    PICO8_METHOD_IB,
    PICO8_METHOD_LL,
    PICO8_METHOD_LY,
    PICO8_METHOD_COUNT
} pico8_method_t;

/* The method named `name`, or -1 when no struct has a method by that name:
 * the name is then looked up as a field. */
static inline int pico8_method_of(const char *name) {
    static const char *const names[PICO8_METHOD_COUNT] = {
        [PICO8_METHOD_OL] = "ol",
        [PICO8_METHOD_PH] = "ph",
        [PICO8_METHOD_KJ] = "kj",
        [PICO8_METHOD_EV] = "ev",
        [PICO8_METHOD_LDOWN] = "ldown",
        [PICO8_METHOD_MD] = "md",
        [PICO8_METHOD_HQ] = "hq",
        [PICO8_METHOD_EY] = "ey",
        [PICO8_METHOD_KZ] = "kz",
        [PICO8_METHOD_PR] = "pr",
        [PICO8_METHOD_RDOWN] = "rdown",
        [PICO8_METHOD_EB] = "eb",
        [PICO8_METHOD_HD] = "hd",
        [PICO8_METHOD_HB] = "hb",
        [PICO8_METHOD_GG] = "gg",
        [PICO8_METHOD_GV] = "gv",
        [PICO8_METHOD_HK] = "hk",
        [PICO8_METHOD_KF] = "kf",
        [PICO8_METHOD_CN] = "cn",
        [PICO8_METHOD_IB] = "ib",
        [PICO8_METHOD_LL] = "ll",
        [PICO8_METHOD_LY] = "ly",
    };
    int i;

    for (i = 0; i < PICO8_METHOD_COUNT; i++) {
        if (strcmp(names[i], name) == 0) {
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

/* One `static const` per converted class. `methods` is indexed by
 * pico8_method_t, NULL where the class has no such method. `field` is NULL
 * only for a class no generic consumer ever reaches, `set_field` for a
 * class no generic writer ever reaches. */
typedef struct {
    pico8_value_function_t methods[PICO8_METHOD_COUNT];
    pico8_struct_field_fn field;
    pico8_struct_set_field_fn set_field;
} pico8_struct_class_t;

/* First member of every entity struct, and of the parent struct a child
 * embeds first: the tag stays at offset 0 for pico8_type_tag_of(). `tag` is
 * always PICO8_TYPE_TAG_STRUCT. */
typedef struct {
    pico8_type_tag_t tag;
    const pico8_struct_class_t *cls;
} pico8_struct_header_t;

#endif
