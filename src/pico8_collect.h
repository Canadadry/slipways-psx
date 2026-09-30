#ifndef PICO8_COLLECT_H
#define PICO8_COLLECT_H

#include "pico8_game_data.h"
#include "pico8_geom.h"
#include "pico8_ptrs.h"

/* `ow` gets each element plus `ctx`, this port's stand-in for the captured
 * upvalue of the original's closure. Returns a new container the caller
 * releases with pico8_ptrs_free(); a NULL `qk` gives an empty one. */
typedef int (*pico8_jp_ptrs_predicate_fn)(void *e, void *ctx);
pico8_ptrs_t pico8_jp_ptrs(const pico8_ptrs_t *qk, pico8_jp_ptrs_predicate_fn ow, void *ctx);

/* `a`'s elements then `b`'s, in a new container the caller releases with
 * pico8_ptrs_free(). Neither argument may be NULL. */
pico8_ptrs_t pico8_la_ptrs(const pico8_ptrs_t *a, const pico8_ptrs_t *b);

/* `kp` writes *out and returns 1 to contribute an element, 0 to contribute
 * nothing -- this port has no nil for `add(m,kp(e)or nil)` to drop. Returns
 * a new container the caller releases with pico8_ptrs_free(). `qk` must not
 * be NULL. */
typedef int (*pico8_nv_ptrs_fn)(void *e, void *ctx, void **out);
pico8_ptrs_t pico8_nv_ptrs(const pico8_ptrs_t *qk, pico8_nv_ptrs_fn kp, void *ctx);

/* TROU-3 */
/* Membership by string content: every element of `qk` is a string (NULL is
 * skipped); a NULL `qk` is a member of nothing. */
int pico8_gu_content_ptrs(const pico8_ptrs_t *qk, const char *kb);

/* Returns a newly malloc'd string the caller must free(), and writes the
 * color into *out_color: `c` when d >= 0, 8 when d < 0. `ku` may be NULL
 * (no suffix). A caller that wants only the string may pass any `c`. */
char *pico8_ne(pico8_fixed_t d, int c, const char *ku, int *out_color);

#endif
