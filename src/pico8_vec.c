#include "pico8_vec.h"

/* function v(x,y)
 *   return setmetatable({x=x,y=y
 *   },km)end
 * The metatable itself has no C equivalent (pico8_vec_t is a plain
 * struct, not a tagged/dispatchable value), so only the {x=x,y=y} table
 * construction is ported. */
pico8_vec_t pico8_vec_v(pico8_fixed_t x, pico8_fixed_t y) {
    pico8_vec_t result;
    result.x = x;
    result.y = y;
    return result;
}

/* function km:__add(b)
 *   return v(self.x+b.x,self.y+b.y)end */
pico8_vec_t pico8_vec_add(pico8_vec_t a, pico8_vec_t b) {
    return pico8_vec_v(pico8_fixed_add(a.x, b.x), pico8_fixed_add(a.y, b.y));
}

/* function km:__sub(b)
 *   return v(self.x-b.x,self.y-b.y)end */
pico8_vec_t pico8_vec_sub(pico8_vec_t a, pico8_vec_t b) {
    return pico8_vec_v(pico8_fixed_sub(a.x, b.x), pico8_fixed_sub(a.y, b.y));
}

/* function km:__mul(m)
 *   return v(self.x*m,self.y*m)end
 * Vector-by-scalar only, matching every call site in lua.lua. */
pico8_vec_t pico8_vec_mul(pico8_vec_t v, pico8_fixed_t m) {
    return pico8_vec_v(pico8_fixed_mul(v.x, m), pico8_fixed_mul(v.y, m));
}

/* function km:__div(d)
 *   return v(self.x/d,self.y/d)end
 * Vector-by-scalar only. Division-by-zero saturation on each component is
 * inherited from pico8_fixed_div, exercised for real by pico8_vec_nn
 * below on a zero-length vector. */
pico8_vec_t pico8_vec_div(pico8_vec_t v, pico8_fixed_t d) {
    return pico8_vec_v(pico8_fixed_div(v.x, d), pico8_fixed_div(v.y, d));
}

/* function km:__unm()
 *   return v(-self.x,-self.y)end */
pico8_vec_t pico8_vec_neg(pico8_vec_t v) {
    return pico8_vec_v(pico8_fixed_neg(v.x), pico8_fixed_neg(v.y));
}

/* function km:rg(v2)
 *   return self.x*v2.x+self.y*v2.y
 * end */
pico8_fixed_t pico8_vec_rg(pico8_vec_t a, pico8_vec_t b) {
    return pico8_fixed_add(pico8_fixed_mul(a.x, b.x), pico8_fixed_mul(a.y, b.y));
}

/* function km:__len()
 *   return self:rg(self)end
 * PICO-8's `#` operator on a km table: squared length, i.e. rg(v,v). */
pico8_fixed_t pico8_vec_len_sq(pico8_vec_t v) {
    return pico8_vec_rg(v, v);
}

/* function km:qx()
 *   return sqrt(#self)end */
pico8_fixed_t pico8_vec_qx(pico8_vec_t v) {
    return pico8_fixed_sqrt(pico8_vec_len_sq(v));
}

/* function km:pa()
 *   return v(-self.y,self.x)end */
pico8_vec_t pico8_vec_pa(pico8_vec_t v) {
    return pico8_vec_v(pico8_fixed_neg(v.y), v.x);
}

/* function km:nn()
 *   return self/self:qx()end
 * For a zero-length vector, qx(v) == 0, so this inherits
 * pico8_fixed_div's divide-by-zero saturation on each component rather
 * than special-casing the zero-length vector -- matching the original,
 * which has no such special case either. */
pico8_vec_t pico8_vec_nn(pico8_vec_t v) {
    return pico8_vec_div(v, pico8_vec_qx(v));
}

/* function qm(fy,lx)
 *   return v(cos(lx),sin(lx))*fy
 * end
 * Polar-to-Cartesian: a unit vector at angle lx (in turns, see
 * pico8_fixed_cos/pico8_fixed_sin's sign-inverted-sin convention), scaled
 * to length fy. */
pico8_vec_t pico8_vec_qm(pico8_fixed_t fy, pico8_fixed_t lx) {
    pico8_vec_t unit = pico8_vec_v(pico8_fixed_cos(lx), pico8_fixed_sin(lx));
    return pico8_vec_mul(unit, fy);
}
