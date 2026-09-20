#include "pico8_geom.h"

#include <stdlib.h>

/* function ee(a,b,c)
 *   local kl=
 *   (b.y-a.y)*(c.x-b.x)-
 *   (b.x-a.x)*(c.y-b.y)
 *   return kl~=0 and sgn(kl)
 * end
 * See pico8_geom.h's pico8_ee_side_t comment for the tri-state design:
 * kl==0 maps to PICO8_EE_ON_LINE (the Lua nil/falsy case); otherwise
 * sgn(kl) (never 0 here, since kl~=0 is already known) maps to
 * PICO8_EE_POSITIVE/PICO8_EE_NEGATIVE. */
pico8_ee_side_t pico8_geom_ee(pico8_vec_t a, pico8_vec_t b, pico8_vec_t c) {
    pico8_fixed_t kl = pico8_fixed_sub(
        pico8_fixed_mul(pico8_fixed_sub(b.y, a.y), pico8_fixed_sub(c.x, b.x)),
        pico8_fixed_mul(pico8_fixed_sub(b.x, a.x), pico8_fixed_sub(c.y, b.y)));
    if (kl == 0) {
        return PICO8_EE_ON_LINE;
    }
    return (pico8_fixed_sgn(kl) > 0) ? PICO8_EE_POSITIVE : PICO8_EE_NEGATIVE;
}

/* function cl(p1,p2,q1,q2)
 *   local oq1,oq2,op1,op2=
 *   ee(p1,p2,q1),ee(p1,p2,q2),
 *   ee(q1,q2,p1),ee(q1,q2,p2)
 *   return oq1 and oq2 and op1 and op2
 *   and oq1~=oq2 and op1~=op2
 * end
 * The Lua `oq1 and oq2 and op1 and op2` requires all four to be non-nil
 * (not PICO8_EE_ON_LINE); `oq1~=oq2`/`op1~=op2` then requires each pair to
 * differ (one POSITIVE, one NEGATIVE) -- ported directly as enum
 * (in)equality checks now that PICO8_EE_ON_LINE is a real third value
 * distinct from either sign. */
int pico8_geom_cl(pico8_vec_t p1, pico8_vec_t p2, pico8_vec_t q1, pico8_vec_t q2) {
    pico8_ee_side_t oq1 = pico8_geom_ee(p1, p2, q1);
    pico8_ee_side_t oq2 = pico8_geom_ee(p1, p2, q2);
    pico8_ee_side_t op1 = pico8_geom_ee(q1, q2, p1);
    pico8_ee_side_t op2 = pico8_geom_ee(q1, q2, p2);
    return oq1 != PICO8_EE_ON_LINE && oq2 != PICO8_EE_ON_LINE &&
           op1 != PICO8_EE_ON_LINE && op2 != PICO8_EE_ON_LINE &&
           oq1 != oq2 && op1 != op2;
}

/* function be(a,b,c,fh)
 *   local d=b-a
 *   local l_=d:rg(c-a)/#d
 *   return l_>0.01
 *   and l_<0.99
 *   and #(a+d*l_-c)<fh
 * end
 * `#d`/`#(...)` is PICO-8's `__len` on a km vector -- squared length (see
 * pico8_vec_len_sq), not qx's actual length -- ported verbatim as such:
 * `l` is dot(d,c-a)/lenSq(d) (the projection fraction of c onto segment
 * a-b), and the final comparison is a *squared*-distance-to-fh test, not
 * an actual-distance one. The 0.01/0.99 margin bounds are copied verbatim
 * from the original, unchanged. */
int pico8_geom_be(pico8_vec_t a, pico8_vec_t b, pico8_vec_t c, pico8_fixed_t fh) {
    pico8_vec_t d = pico8_vec_sub(b, a);
    pico8_fixed_t l = pico8_fixed_div(pico8_vec_rg(d, pico8_vec_sub(c, a)), pico8_vec_len_sq(d));
    pico8_fixed_t lo = pico8_fixed_from_double(0.01);
    pico8_fixed_t hi = pico8_fixed_from_double(0.99);
    pico8_vec_t projected = pico8_vec_sub(pico8_vec_add(a, pico8_vec_mul(d, l)), c);
    pico8_fixed_t dist_sq = pico8_vec_len_sq(projected);
    return l > lo && l < hi && dist_sq < fh;
}

/* function qz(xl,yt,xr,yb)
 * return function(p)
 * return mid(xl,xr,p.x)==p.x
 * and mid(yt,yb,p.y)==p.y
 * end
 * end
 * See pico8_geom.h's own comment on this declaration for why this returns
 * a heap box rather than a native closure. */
pico8_qz_box_t *pico8_qz(pico8_fixed_t xl, pico8_fixed_t yt, pico8_fixed_t xr, pico8_fixed_t yb) {
    pico8_qz_box_t *box = malloc(sizeof *box);
    box->xl = xl;
    box->yt = yt;
    box->xr = xr;
    box->yb = yb;
    return box;
}

int pico8_qz_call(const pico8_qz_box_t *box, pico8_vec_t p) {
    return pico8_fixed_mid(box->xl, box->xr, p.x) == p.x && pico8_fixed_mid(box->yt, box->yb, p.y) == p.y;
}
