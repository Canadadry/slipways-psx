#include "engine/vec.h"

vec_t vec_make(p8_fixed_t x, p8_fixed_t y) {
    vec_t result;
    result.x = x;
    result.y = y;
    return result;
}

vec_t vec_add(vec_t a, vec_t b) {
    return vec_make(p8_fixed_add(a.x, b.x), p8_fixed_add(a.y, b.y));
}

vec_t vec_sub(vec_t a, vec_t b) {
    return vec_make(p8_fixed_sub(a.x, b.x), p8_fixed_sub(a.y, b.y));
}

vec_t vec_mul(vec_t v, p8_fixed_t m) {
    return vec_make(p8_fixed_mul(v.x, m), p8_fixed_mul(v.y, m));
}

vec_t vec_div(vec_t v, p8_fixed_t d) {
    return vec_make(p8_fixed_div(v.x, d), p8_fixed_div(v.y, d));
}

vec_t vec_neg(vec_t v) {
    return vec_make(p8_fixed_neg(v.x), p8_fixed_neg(v.y));
}

p8_fixed_t vec_dot(vec_t a, vec_t b) {
    return p8_fixed_add(p8_fixed_mul(a.x, b.x), p8_fixed_mul(a.y, b.y));
}

/* Squared length, not length. */
p8_fixed_t vec_length_squared(vec_t v) {
    return vec_dot(v, v);
}

p8_fixed_t vec_length(vec_t v) {
    return p8_fixed_sqrt(vec_length_squared(v));
}

vec_t vec_perpendicular(vec_t v) {
    return vec_make(p8_fixed_neg(v.y), v.x);
}

/* A zero-length vector saturates each component through p8_fixed_div
 * rather than being special-cased. */
vec_t vec_normalize(vec_t v) {
    return vec_div(v, vec_length(v));
}

vec_t vec_from_polar(p8_fixed_t fy, p8_fixed_t lx) {
    vec_t unit = vec_make(p8_fixed_cos(lx), p8_fixed_sin(lx));
    return vec_mul(unit, fy);
}
