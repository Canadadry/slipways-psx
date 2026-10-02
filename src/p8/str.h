#ifndef P8_STR_H
#define P8_STR_H

#include "p8/fixed.h"
#include "engine/allocator.h"

/* PICO-8's sub(): 1-indexed and both bounds included, so start and end 1
 * give the first character alone. The start index clamps up to 1 and the
 * end index down to the source length; an end below the start after
 * clamping yields an empty string, never an error. The two-argument form,
 * without an end index, is not implemented: no call site uses it.
 *
 * Returns a NUL-terminated string in `zone`, released by that zone --
 * never a view into the source string. */
char *p8_sub(const allocator_t *zone, const char *s, int i, int j);

/* PICO-8's tostr() of a number: 4 fractional digits rounded half to even,
 * then trailing zeros and a bare trailing '.' are trimmed, so an exact 3
 * renders "3" and not "3.0000". This reproduces 13 of the 14 block-C
 * witnesses of tools/pico8_reference_probe_out.txt; magnitudes below
 * 0.0001 are TROU-22.
 *
 * Returns a NUL-terminated string in `zone`, as p8_sub does. */
char *p8_tostr_fixed(const allocator_t *zone, p8_fixed_t x);

/* PICO-8's `..` concatenation of two strings, in `zone`, as p8_sub does. */
char *p8_concat2(const allocator_t *zone, const char *a, const char *b);

#endif
