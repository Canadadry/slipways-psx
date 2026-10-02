#ifndef PICO8_FIXED_TEST_DOUBLE_H
#define PICO8_FIXED_TEST_DOUBLE_H

#include "p8/fixed.h"

/* Rounds to the nearest representable 16.16 value -- not PICO-8's
 * truncation, which PICO8_FIXED_LIT carries. */
p8_fixed_t pico8_fixed_from_double(double d);

/* Exact. */
double pico8_fixed_to_double(p8_fixed_t x);

#endif
