#include "engine/geom.h"

#include <assert.h>

side_of_line_t geom_side_of_line(vec_t a, vec_t b, vec_t c) {
    p8_fixed_t kl = p8_fixed_sub(
        p8_fixed_mul(p8_fixed_sub(b.y, a.y), p8_fixed_sub(c.x, b.x)),
        p8_fixed_mul(p8_fixed_sub(b.x, a.x), p8_fixed_sub(c.y, b.y)));
    if (kl == 0) {
        return SIDE_ON_LINE;
    }
    return (p8_fixed_sgn(kl) > 0) ? SIDE_POSITIVE : SIDE_NEGATIVE;
}

/* Lua's `oq1 and oq2 and op1 and op2` is the "none of the four is on the
 * line" half; the two inequalities are the "each pair straddles" half. */
int geom_segments_cross(vec_t p1, vec_t p2, vec_t q1, vec_t q2) {
    side_of_line_t q1_side = geom_side_of_line(p1, p2, q1);
    side_of_line_t q2_side = geom_side_of_line(p1, p2, q2);
    side_of_line_t p1_side = geom_side_of_line(q1, q2, p1);
    side_of_line_t p2_side = geom_side_of_line(q1, q2, p2);
    return q1_side != SIDE_ON_LINE && q2_side != SIDE_ON_LINE &&
           p1_side != SIDE_ON_LINE && p2_side != SIDE_ON_LINE &&
           q1_side != q2_side && p1_side != p2_side;
}

/* `l` is the projection fraction of c onto a-b, dot(d,c-a) over the
 * SQUARED length of d; the final test compares squared distances too. The
 * 0.01/0.99 bounds are the cart's own. */
int geom_near_segment(vec_t a, vec_t b, vec_t c, p8_fixed_t fh) {
    vec_t d = vec_sub(b, a);
    p8_fixed_t l = p8_fixed_div(vec_dot(d, vec_sub(c, a)), vec_length_squared(d));
    p8_fixed_t lo = P8_FIXED_LIT(0x0000028f, 0.01);
    p8_fixed_t hi = P8_FIXED_LIT(0x0000fd70, 0.99);
    vec_t projected = vec_sub(vec_add(a, vec_mul(d, l)), c);
    p8_fixed_t distance_squared = vec_length_squared(projected);
    return l > lo && l < hi && distance_squared < fh;
}

box_t *box_make(const allocator_t *zone, p8_fixed_t xl, p8_fixed_t yt,
                         p8_fixed_t xr, p8_fixed_t yb) {
    box_t *box = (box_t *)allocator_alloc(zone, sizeof *box);
    box->left = xl;
    box->top = yt;
    box->right = xr;
    box->bottom = yb;
    return box;
}

int box_contains(const box_t *box, vec_t p) {
    assert(box != NULL && "qz: the box is the caller's to check, not this call's");
    return p8_fixed_mid(box->left, box->right, p.x) == p.x && p8_fixed_mid(box->top, box->bottom, p.y) == p.y;
}
