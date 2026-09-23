#ifndef PICO8_GEOM_H
#define PICO8_GEOM_H

#include "pico8_fixed.h"
#include "pico8_vec.h"

/* Three values, not a bool: `cl` needs "on the line" (the Lua nil `ee`
 * returns for a zero cross product) told apart from "off it, on this
 * side". A bool, or a bare sign with 0 doing double duty, loses one half
 * of that. */
typedef enum {
    PICO8_EE_ON_LINE = 0,
    PICO8_EE_POSITIVE = 1,
    PICO8_EE_NEGATIVE = -1
} pico8_ee_side_t;

/* The sign is a genuine mathematical sign here: sgn is only reached once
 * the cross product is known non-zero, never through sgn(0) == 1. */
pico8_ee_side_t pico8_geom_ee(pico8_vec_t a, pico8_vec_t b, pico8_vec_t c);

/* Returns 1/0, this codebase's int-as-bool convention. */
int pico8_geom_cl(pico8_vec_t p1, pico8_vec_t p2, pico8_vec_t q1, pico8_vec_t q2);

/* Returns 1/0. `fh` is a squared distance, and so is the distance it is
 * compared against. */
int pico8_geom_be(pico8_vec_t a, pico8_vec_t b, pico8_vec_t c, pico8_fixed_t fh);

typedef struct {
    pico8_fixed_t xl;
    pico8_fixed_t yt;
    pico8_fixed_t xr;
    pico8_fixed_t yb;
} pico8_qz_box_t;

/* C has no closures, so the predicate the cart returns becomes this box
 * plus pico8_qz_call. The box is heap-allocated and belongs to the caller,
 * which usually parks it in a field as a PICO8_VALUE_REF and never frees
 * it (see pico8_value.h's borrowed-pointer convention). */
pico8_qz_box_t *pico8_qz(pico8_fixed_t xl, pico8_fixed_t yt, pico8_fixed_t xr, pico8_fixed_t yb);

/* True iff p is inside [xl,xr]x[yt,yb], by mid()'s clamp-then-compare
 * idiom (pico8_fixed_mid). `box` must not be NULL: the caller guards it. */
int pico8_qz_call(const pico8_qz_box_t *box, pico8_vec_t p);

#endif
