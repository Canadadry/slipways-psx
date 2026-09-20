#ifndef PICO8_VEC_H
#define PICO8_VEC_H

#include "pico8_fixed.h"

/* PICO-8's `km` vector metatable (pico8/lua.lua lines 98-126): a plain
 * two-field table {x=..., y=...} with metamethods for +, -, scalar * and
 * /, unary -, plus a handful of named methods (rg/nn/pa/qx) and the __len
 * metamethod (PICO-8's `#` operator). Ported here as a plain two-field
 * fixed-point struct plus one C function per Lua function/metamethod.
 *
 * Naming convention (this is the first chunk in this codebase porting a
 * Lua operator-overload method to a plain C function, since C has no
 * operator overloading): every function keeps the minified Lua name it
 * ports for traceability (v, rg, nn, pa, qx, qm); the metamethods are
 * spelled out with their plain-English meaning instead of their `__foo`
 * spelling, since that reads far more clearly at every C call site:
 * pico8_vec_add/sub/mul/div/neg (for __add/__sub/__mul/__div/__unm) and
 * pico8_vec_len_sq (for __len, PICO-8's `#` operator -- named for what it
 * returns, since C has no `#`-operator overload to mirror). */
typedef struct {
    pico8_fixed_t x;
    pico8_fixed_t y;
} pico8_vec_t;

/* function v(x,y)
 *   return setmetatable({x=x,y=y
 *   },km)end */
pico8_vec_t pico8_vec_v(pico8_fixed_t x, pico8_fixed_t y);

/* function km:__add(b)
 *   return v(self.x+b.x,self.y+b.y)end */
pico8_vec_t pico8_vec_add(pico8_vec_t a, pico8_vec_t b);

/* function km:__sub(b)
 *   return v(self.x-b.x,self.y-b.y)end */
pico8_vec_t pico8_vec_sub(pico8_vec_t a, pico8_vec_t b);

/* function km:__mul(m)
 *   return v(self.x*m,self.y*m)end
 * Vector-by-scalar only, matching every call site in lua.lua -- no
 * vector-by-vector multiply is ever used. */
pico8_vec_t pico8_vec_mul(pico8_vec_t v, pico8_fixed_t m);

/* function km:__div(d)
 *   return v(self.x/d,self.y/d)end
 * Vector-by-scalar only (see pico8_vec_mul above). Division-by-zero
 * saturation on each component is inherited from pico8_fixed_div, not
 * special-cased here -- see pico8_vec_nn below, which is where this is
 * actually exercised (normalizing a zero-length vector). */
pico8_vec_t pico8_vec_div(pico8_vec_t v, pico8_fixed_t d);

/* function km:__unm()
 *   return v(-self.x,-self.y)end */
pico8_vec_t pico8_vec_neg(pico8_vec_t v);

/* function km:rg(v2)
 *   return self.x*v2.x+self.y*v2.y
 * end */
pico8_fixed_t pico8_vec_rg(pico8_vec_t a, pico8_vec_t b);

/* function km:__len()
 *   return self:rg(self)end
 * PICO-8's `#` operator on a km table: squared length, i.e. rg(v,v).
 * Ported as its own function (not just calling pico8_vec_rg(v,v) at every
 * call site) since it's used directly as an optimization at call sites
 * that only need squared distance, exactly as the original does. */
pico8_fixed_t pico8_vec_len_sq(pico8_vec_t v);

/* function km:qx()
 *   return sqrt(#self)end */
pico8_fixed_t pico8_vec_qx(pico8_vec_t v);

/* function km:nn()
 *   return self/self:qx()end
 * Normalize: v / qx(v). For a zero-length vector, qx(v) == 0, so this
 * inherits pico8_fixed_div's divide-by-zero saturation behavior on each
 * component (saturating to +0x7fff.ffff, since 0's own dividend is
 * non-negative) rather than special-casing the zero-length vector --
 * matching the original, which has no such special case either. */
pico8_vec_t pico8_vec_nn(pico8_vec_t v);

/* function km:pa()
 *   return v(-self.y,self.x)end */
pico8_vec_t pico8_vec_pa(pico8_vec_t v);

/* function qm(fy,lx)
 *   return v(cos(lx),sin(lx))*fy
 * end
 * Polar-to-Cartesian: a unit vector at angle lx (in turns, see
 * pico8_fixed_cos/pico8_fixed_sin), scaled to length fy. */
pico8_vec_t pico8_vec_qm(pico8_fixed_t fy, pico8_fixed_t lx);

#endif
