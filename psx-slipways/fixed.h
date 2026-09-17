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

/*
 * fixed_sqrt: Q16.16 fixed-point square root.
 *
 * For x < 0, returns 0. This is a defensive convention for a general
 * arithmetic primitive (like fixed_mul), not a traced game-design source
 * value -- callers that can legitimately hit negative inputs are
 * responsible for deciding what that means for them.
 *
 * For x >= 0: a fixed_t value x represents the real number x / FX_ONE, so
 * the fixed_t representing sqrt(x / FX_ONE) satisfies the identity
 * sqrt(x / FX_ONE) * FX_ONE == isqrt(x * FX_ONE), where isqrt is an
 * integer square root. x is widened to int64_t before shifting left by 16
 * (i.e. multiplying by FX_ONE) since x can be as large as INT32_MAX and
 * x << 16 would overflow a 32-bit type. The integer square root itself is
 * computed with the standard bit-by-bit ("digit by digit") method, using
 * only integer arithmetic (no float/double), matching this codebase's
 * no-FPU convention. The result is truncated back down to fixed_t.
 */
static inline fixed_t fixed_sqrt(fixed_t x) {
    if (x < 0) {
        return 0;
    }

    uint64_t n = (uint64_t)((int64_t)x << 16);
    uint64_t result = 0;
    /* Highest power of 4 that fits in a 64-bit value, halved each step. */
    uint64_t bit = (uint64_t)1 << 62;

    while (bit > n) {
        bit >>= 2;
    }

    while (bit != 0) {
        if (n >= result + bit) {
            n -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }

    return (fixed_t)result;
}

#endif
