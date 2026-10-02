#ifndef DEVTOOLS_FIXED_TEST_DOUBLE_H
#define DEVTOOLS_FIXED_TEST_DOUBLE_H

#include "p8/fixed.h"

/* Rounds to the nearest representable 16.16 value -- not PICO-8's
 * truncation, which PICO8_FIXED_LIT carries. */
p8_fixed_t fixed_test_double_from_double(double d);

/* Exact. */
double fixed_test_double_to_double(p8_fixed_t x);

#endif
