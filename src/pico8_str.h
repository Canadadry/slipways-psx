#ifndef PICO8_STR_H
#define PICO8_STR_H

#include "pico8_fixed.h"

/* PICO-8's sub(s,i,j): 1-indexed and both bounds included, so sub(s,1,1) is
 * the first character alone. `i` clamps up to 1 and `j` down to strlen(s);
 * `j < i` after clamping yields an empty string, never an error. The
 * two-argument form sub(s,i) is not implemented, no call site uses it.
 *
 * Returns a newly allocated, NUL-terminated string the caller owns and
 * must free() -- never a view into `s`. */
char *pico8_sub(const char *s, int i, int j);

/* tostr(x) of a number: 4 fractional digits rounded half to even, then
 * trailing zeros and a bare trailing '.' are trimmed, so an exact 3
 * renders "3" and not "3.0000". This reproduces 13 of the 14 block-C
 * witnesses of tools/probe_out.txt; magnitudes below 0.0001 are TROU-22.
 *
 * Returns a newly allocated, NUL-terminated string the caller owns and
 * must free(), as pico8_sub does. */
char *pico8_tostr_fixed(pico8_fixed_t x);

#endif
