#ifndef P8_FIXED_H
#define P8_FIXED_H

#include <stdint.h>

typedef int32_t p8_fixed_t;

#define P8_FIXED_SHIFT 16

#define P8_FIXED_DIV0_POS ((p8_fixed_t)0x7FFFFFFF)
#define P8_FIXED_DIV0_NEG ((p8_fixed_t)(-0x7FFFFFFF))

#define P8_FIXED_LIT(raw, decimal) ((p8_fixed_t)(uint32_t)(raw##u))

p8_fixed_t p8_fixed_from_int(int32_t i);

#define P8_FIXED_INT(i) ((p8_fixed_t)((uint32_t)(int32_t)(i) << P8_FIXED_SHIFT))

p8_fixed_t p8_fixed_add(p8_fixed_t a, p8_fixed_t b);
p8_fixed_t p8_fixed_sub(p8_fixed_t a, p8_fixed_t b);

p8_fixed_t p8_fixed_mul(p8_fixed_t a, p8_fixed_t b);

p8_fixed_t p8_fixed_div(p8_fixed_t a, p8_fixed_t b);

p8_fixed_t p8_fixed_neg(p8_fixed_t x);

p8_fixed_t p8_fixed_flr(p8_fixed_t x);

int32_t p8_fixed_flr_int(p8_fixed_t x);

p8_fixed_t p8_fixed_abs(p8_fixed_t x);

p8_fixed_t p8_fixed_sgn(p8_fixed_t x);

p8_fixed_t p8_fixed_mid(p8_fixed_t a, p8_fixed_t b, p8_fixed_t c);

p8_fixed_t p8_fixed_band(p8_fixed_t a, p8_fixed_t b);
p8_fixed_t p8_fixed_bor(p8_fixed_t a, p8_fixed_t b);

p8_fixed_t p8_fixed_mod(p8_fixed_t a, p8_fixed_t b);

p8_fixed_t p8_fixed_shl(p8_fixed_t x, int32_t n);

p8_fixed_t p8_fixed_sqrt(p8_fixed_t x);

p8_fixed_t p8_fixed_pow075(p8_fixed_t x);

p8_fixed_t p8_fixed_rnd(p8_fixed_t x);

p8_fixed_t p8_fixed_rnd_default(void);

p8_fixed_t p8_fixed_cos(p8_fixed_t x);

p8_fixed_t p8_fixed_sin(p8_fixed_t x);

#endif
