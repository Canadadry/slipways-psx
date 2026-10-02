#ifndef PICO8_VEC_H
#define PICO8_VEC_H

#include "p8/fixed.h"

/* Naming: the metamethods C has no operator overloading for are spelled
 * out (pico8_vec_add/sub/mul/div/neg for __add/__sub/__mul/__div/__unm,
 * pico8_vec_len_sq for __len); the named Lua methods keep their own name. */
typedef struct {
    p8_fixed_t x;
    p8_fixed_t y;
} pico8_vec_t;

pico8_vec_t pico8_vec_v(p8_fixed_t x, p8_fixed_t y);

pico8_vec_t pico8_vec_add(pico8_vec_t a, pico8_vec_t b);

pico8_vec_t pico8_vec_sub(pico8_vec_t a, pico8_vec_t b);

/* Vector by scalar only; no call site multiplies two vectors. */
pico8_vec_t pico8_vec_mul(pico8_vec_t v, p8_fixed_t m);

/* Vector by scalar only. Division by zero saturates per component, as
 * pico8_fixed_div does -- see pico8_vec_nn. */
pico8_vec_t pico8_vec_div(pico8_vec_t v, p8_fixed_t d);

pico8_vec_t pico8_vec_neg(pico8_vec_t v);

p8_fixed_t pico8_vec_rg(pico8_vec_t a, pico8_vec_t b);

/* PICO-8's `#` on a km vector: the SQUARED length. Call sites that only
 * compare distances use it directly, as the cart does. */
p8_fixed_t pico8_vec_len_sq(pico8_vec_t v);

p8_fixed_t pico8_vec_qx(pico8_vec_t v);

/* A zero-length vector is not special-cased, here or in the cart: qx is
 * then 0 and each component saturates through pico8_fixed_div. */
pico8_vec_t pico8_vec_nn(pico8_vec_t v);

pico8_vec_t pico8_vec_pa(pico8_vec_t v);

/* Angle lx is in turns; the y component follows pico8_fixed_sin's
 * inverted sign. */
pico8_vec_t pico8_vec_qm(p8_fixed_t fy, p8_fixed_t lx);

#endif
