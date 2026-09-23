#ifndef PICO8_STR_H
#define PICO8_STR_H

/* PICO-8's sub(s,i,j): 1-indexed and both bounds included, so sub(s,1,1) is
 * the first character alone. `i` clamps up to 1 and `j` down to strlen(s);
 * `j < i` after clamping yields an empty string, never an error. The
 * two-argument form sub(s,i) is not implemented, no call site uses it.
 *
 * Returns a newly allocated, NUL-terminated string the caller owns and
 * must free() -- never a view into `s`. */
char *pico8_sub(const char *s, int i, int j);

#endif
