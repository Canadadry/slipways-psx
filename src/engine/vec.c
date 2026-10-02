#include "engine/vec.h"

pico8_vec_t pico8_vec_v(p8_fixed_t x, p8_fixed_t y) {
    pico8_vec_t result;
    result.x = x;
    result.y = y;
    return result;
}

pico8_vec_t pico8_vec_add(pico8_vec_t a, pico8_vec_t b) {
    return pico8_vec_v(p8_fixed_add(a.x, b.x), p8_fixed_add(a.y, b.y));
}

pico8_vec_t pico8_vec_sub(pico8_vec_t a, pico8_vec_t b) {
    return pico8_vec_v(p8_fixed_sub(a.x, b.x), p8_fixed_sub(a.y, b.y));
}

pico8_vec_t pico8_vec_mul(pico8_vec_t v, p8_fixed_t m) {
    return pico8_vec_v(p8_fixed_mul(v.x, m), p8_fixed_mul(v.y, m));
}

pico8_vec_t pico8_vec_div(pico8_vec_t v, p8_fixed_t d) {
    return pico8_vec_v(p8_fixed_div(v.x, d), p8_fixed_div(v.y, d));
}

pico8_vec_t pico8_vec_neg(pico8_vec_t v) {
    return pico8_vec_v(p8_fixed_neg(v.x), p8_fixed_neg(v.y));
}

p8_fixed_t pico8_vec_rg(pico8_vec_t a, pico8_vec_t b) {
    return p8_fixed_add(p8_fixed_mul(a.x, b.x), p8_fixed_mul(a.y, b.y));
}

/* Squared length, not length: PICO-8's `#`. */
p8_fixed_t pico8_vec_len_sq(pico8_vec_t v) {
    return pico8_vec_rg(v, v);
}

p8_fixed_t pico8_vec_qx(pico8_vec_t v) {
    return p8_fixed_sqrt(pico8_vec_len_sq(v));
}

pico8_vec_t pico8_vec_pa(pico8_vec_t v) {
    return pico8_vec_v(p8_fixed_neg(v.y), v.x);
}

/* A zero-length vector saturates each component through pico8_fixed_div
 * rather than being special-cased, as in the cart. */
pico8_vec_t pico8_vec_nn(pico8_vec_t v) {
    return pico8_vec_div(v, pico8_vec_qx(v));
}

pico8_vec_t pico8_vec_qm(p8_fixed_t fy, p8_fixed_t lx) {
    pico8_vec_t unit = pico8_vec_v(p8_fixed_cos(lx), p8_fixed_sin(lx));
    return pico8_vec_mul(unit, fy);
}
