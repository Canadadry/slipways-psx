#ifndef DEVTOOLS_FIXED_TEST_DOUBLE_H
#define DEVTOOLS_FIXED_TEST_DOUBLE_H

#include "p8/fixed.h"

/* Rounds to the nearest representable 16.16 value. PICO-8 truncates
 * instead, and P8_FIXED_LIT takes the bits PICO-8 itself produced: 0.1 is
 * 0x00001999 there, where this rounds it to 0x0000199a. */
p8_fixed_t fixed_test_double_from_double(double d);

/* Exact. */
double fixed_test_double_to_double(p8_fixed_t x);

#endif
