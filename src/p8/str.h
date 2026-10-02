#ifndef P8_STR_H
#define P8_STR_H

#include "p8/fixed.h"
#include "engine/allocator.h"

/* PICO-8's sub(s,i,j): 1-indexed and both bounds included, so sub(s,1,1) is
 * the first character alone. `i` clamps up to 1 and `j` down to strlen(s);
 * `j < i` after clamping yields an empty string, never an error. The
 * two-argument form sub(s,i) is not implemented, no call site uses it.
 *
 * Returns a NUL-terminated string in `zone`, released by that zone --
 * never a view into `s`. */
char *p8_sub(const allocator_t *zone, const char *s, int i, int j);

/* tostr(x) of a number: 4 fractional digits rounded half to even, then
 * trailing zeros and a bare trailing '.' are trimmed, so an exact 3
 * renders "3" and not "3.0000". This reproduces 13 of the 14 block-C
 * witnesses of tools/pico8_reference_probe_out.txt; magnitudes below 0.0001 are TROU-22.
 *
 * Returns a NUL-terminated string in `zone`, as pico8_sub does. */
char *p8_tostr_fixed(const allocator_t *zone, p8_fixed_t x);

/* Lua's `a..b` of two strings, in `zone`, as pico8_sub does. */
char *p8_concat2(const allocator_t *zone, const char *a, const char *b);

#endif
