#ifndef FIXED_H
#define FIXED_H

#include <stdint.h>

typedef int32_t fixed_t;

#define FX_ONE ((fixed_t)(1 << 16))

static inline fixed_t fixed_add(fixed_t a, fixed_t b) {
    return a + b;
}

static inline fixed_t fixed_sub(fixed_t a, fixed_t b) {
    return a - b;
}

static inline fixed_t fixed_mul(fixed_t a, fixed_t b) {
    return (fixed_t)(((int64_t)a * (int64_t)b) >> 16);
}

static inline fixed_t fixed_from_double(double d) {
    return (fixed_t)(d * (double)FX_ONE);
}

static inline double fixed_to_double(fixed_t f) {
    return (double)f / (double)FX_ONE;
}

#endif
