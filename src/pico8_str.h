#ifndef PICO8_STR_H
#define PICO8_STR_H

/* PICO-8's sub(s,i,j): a 1-indexed, INCLUSIVE substring (index 1 is the
 * first character of s, and both i and j are included in the result) --
 * distinct from stock Lua's string.sub only in that PICO-8's manual states
 * this contract directly rather than leaving it to Lua's own (also
 * 1-indexed-inclusive, so behaviorally the same here) string.sub docs.
 * Source: PICO-8 manual, https://www.lexaloffle.com/pico8_manual.txt
 *
 * This module operates on plain C strings, not on the shared pico8_value_t
 * (pico8_value.h) -- sub()'s own PICO-8 signature takes and returns a bare
 * string, never the dynamic-value type, so there is nothing here for
 * pico8_value_t to add; see pico8_value.h's pico8_value_tostr for the
 * function that *does* need the shared value type.
 *
 * Every sub() call site found by grepping pico8/lua.lua in this phase's
 * scope (8 sites: lines 57, 64, 66 (x4), 76, 80 -- inside ob()/fi()'s own
 * parser and unstash()) always passes both i and j explicitly; PICO-8's
 * real two-arg sub(s,i) form ("i to end of string") is never used anywhere
 * in scope, so it is deliberately not implemented here -- a real future
 * need for it is evidence to add it then, not to guess at now.
 *
 * i clamped up to 1 and j clamped down to strlen(s) mirror PICO-8's own
 * documented clamping of out-of-range indices; j < i (after clamping)
 * yields an empty string, matching the empty-range case every caller in
 * scope can produce (e.g. an empty quoted string "" in ob()'s grammar).
 * Negative indices (PICO-8's "count from the end") are not evidenced by
 * any in-scope call site and are not implemented.
 *
 * Returns a newly heap-allocated, NUL-terminated string the caller owns
 * and must free() -- sub() produces a new, independent Lua string value,
 * never a view into s, so the C port's ownership mirrors that. */
char *pico8_sub(const char *s, int i, int j);

#endif
