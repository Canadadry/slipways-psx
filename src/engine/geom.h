#ifndef ENGINE_GEOM_H
#define ENGINE_GEOM_H

#include "p8/fixed.h"
#include "engine/vec.h"
#include "engine/allocator.h"

/* Three values, not a bool: `cl` needs "on the line" (the Lua nil `ee`
 * returns for a zero cross product) told apart from "off it, on this
 * side". A bool, or a bare sign, where 0 would have to mean both, loses
 * one half of that. */
typedef enum {
    SIDE_ON_LINE = 0,
    SIDE_POSITIVE = 1,
    SIDE_NEGATIVE = -1
} side_of_line_t;

/* The sign is a genuine mathematical sign here: sgn is only reached once
 * the cross product is known non-zero, never through sgn(0) == 1. */
side_of_line_t geom_side_of_line(vec_t a, vec_t b, vec_t c);

/* Returns 1/0, this codebase's int-as-bool convention. */
int geom_segments_cross(vec_t p1, vec_t p2, vec_t q1, vec_t q2);

/* Returns 1/0. `fh` is a squared distance, and so is the distance it is
 * compared against. */
int geom_near_segment(vec_t a, vec_t b, vec_t c, p8_fixed_t fh);

typedef struct {
    p8_fixed_t left;
    p8_fixed_t top;
    p8_fixed_t right;
    p8_fixed_t bottom;
} box_t;

/* C has no closures, so the predicate the cart returns becomes this box
 * plus pico8_qz_call. The box lives in `zone` and belongs to the caller,
 * which parks it in a link (`js`'s `header.kq`) and never releases it. */
box_t *box_make(const allocator_t *zone, p8_fixed_t xl, p8_fixed_t yt, p8_fixed_t xr, p8_fixed_t yb);

/* True iff p is inside [xl,xr]x[yt,yb], by mid()'s clamp-then-compare
 * idiom (pico8_fixed_mid). `box` must not be NULL: the caller guards it. */
int box_contains(const box_t *box, vec_t p);

#endif
