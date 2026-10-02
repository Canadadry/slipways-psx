#ifndef ENGINE_VEC_H
#define ENGINE_VEC_H

#include "p8/fixed.h"

/* Naming: the metamethods C has no operator overloading for are spelled
 * out (pico8_vec_add/sub/mul/div/neg for __add/__sub/__mul/__div/__unm,
 * pico8_vec_len_sq for __len); the named Lua methods keep their own name. */
typedef struct {
    p8_fixed_t x;
    p8_fixed_t y;
} vec_t;

vec_t vec_make(p8_fixed_t x, p8_fixed_t y);

vec_t vec_add(vec_t a, vec_t b);

vec_t vec_sub(vec_t a, vec_t b);

/* Vector by scalar only; no call site multiplies two vectors. */
vec_t vec_mul(vec_t v, p8_fixed_t m);

/* Vector by scalar only. Division by zero saturates per component, as
 * pico8_fixed_div does -- see pico8_vec_nn. */
vec_t vec_div(vec_t v, p8_fixed_t d);

vec_t vec_neg(vec_t v);

p8_fixed_t vec_dot(vec_t a, vec_t b);

/* PICO-8's `#` on a km vector: the SQUARED length. Call sites that only
 * compare distances use it directly, as the cart does. */
p8_fixed_t vec_length_squared(vec_t v);

p8_fixed_t vec_length(vec_t v);

/* A zero-length vector is not special-cased, here or in the cart: qx is
 * then 0 and each component saturates through pico8_fixed_div. */
vec_t vec_normalize(vec_t v);

vec_t vec_perpendicular(vec_t v);

/* Angle lx is in turns; the y component follows pico8_fixed_sin's
 * inverted sign. */
vec_t vec_from_polar(p8_fixed_t fy, p8_fixed_t lx);

#endif
