#include "pico8_geom.h"

#include <assert.h>

pico8_ee_side_t pico8_geom_ee(pico8_vec_t a, pico8_vec_t b, pico8_vec_t c) {
    pico8_fixed_t kl = pico8_fixed_sub(
        pico8_fixed_mul(pico8_fixed_sub(b.y, a.y), pico8_fixed_sub(c.x, b.x)),
        pico8_fixed_mul(pico8_fixed_sub(b.x, a.x), pico8_fixed_sub(c.y, b.y)));
    if (kl == 0) {
        return PICO8_EE_ON_LINE;
    }
    return (pico8_fixed_sgn(kl) > 0) ? PICO8_EE_POSITIVE : PICO8_EE_NEGATIVE;
}

/* Lua's `oq1 and oq2 and op1 and op2` is the "none of the four is on the
 * line" half; the two inequalities are the "each pair straddles" half. */
int pico8_geom_cl(pico8_vec_t p1, pico8_vec_t p2, pico8_vec_t q1, pico8_vec_t q2) {
    pico8_ee_side_t oq1 = pico8_geom_ee(p1, p2, q1);
    pico8_ee_side_t oq2 = pico8_geom_ee(p1, p2, q2);
    pico8_ee_side_t op1 = pico8_geom_ee(q1, q2, p1);
    pico8_ee_side_t op2 = pico8_geom_ee(q1, q2, p2);
    return oq1 != PICO8_EE_ON_LINE && oq2 != PICO8_EE_ON_LINE &&
           op1 != PICO8_EE_ON_LINE && op2 != PICO8_EE_ON_LINE &&
           oq1 != oq2 && op1 != op2;
}

/* `l` is the projection fraction of c onto a-b, dot(d,c-a) over the
 * SQUARED length of d; the final test compares squared distances too. The
 * 0.01/0.99 bounds are the cart's own. */
int pico8_geom_be(pico8_vec_t a, pico8_vec_t b, pico8_vec_t c, pico8_fixed_t fh) {
    pico8_vec_t d = pico8_vec_sub(b, a);
    pico8_fixed_t l = pico8_fixed_div(pico8_vec_rg(d, pico8_vec_sub(c, a)), pico8_vec_len_sq(d));
    pico8_fixed_t lo = PICO8_FIXED_LIT(0x0000028f, 0.01);
    pico8_fixed_t hi = PICO8_FIXED_LIT(0x0000fd70, 0.99);
    pico8_vec_t projected = pico8_vec_sub(pico8_vec_add(a, pico8_vec_mul(d, l)), c);
    pico8_fixed_t dist_sq = pico8_vec_len_sq(projected);
    return l > lo && l < hi && dist_sq < fh;
}

pico8_qz_box_t *pico8_qz(const pico8_zone_choisie_t *zone, pico8_fixed_t xl, pico8_fixed_t yt,
                         pico8_fixed_t xr, pico8_fixed_t yb) {
    pico8_qz_box_t *box = (pico8_qz_box_t *)pico8_zone_alloc(zone, sizeof *box);
    box->xl = xl;
    box->yt = yt;
    box->xr = xr;
    box->yb = yb;
    return box;
}

int pico8_qz_call(const pico8_qz_box_t *box, pico8_vec_t p) {
    assert(box != NULL && "qz: the box is the caller's to check, not this call's");
    return pico8_fixed_mid(box->xl, box->xr, p.x) == p.x && pico8_fixed_mid(box->yt, box->yb, p.y) == p.y;
}
