#ifndef ENGINE_COLLECT_H
#define ENGINE_COLLECT_H

#include "engine/geom.h"
#include "engine/pointer_list.h"

/* `ow` gets each element plus `ctx`, this port's stand-in for the captured
 * upvalue of the original's closure. Returns a new container in `zone` the
 * caller releases with pico8_ptrs_free(); a NULL `qk` gives an empty one. */
typedef int (*filter_pointer_list_predicate_fn_t)(void *e, void *ctx);
pointer_list_t collect_filter_pointer_list(const allocator_t *zone, const pointer_list_t *qk,
                           filter_pointer_list_predicate_fn_t ow, void *ctx);

/* `a`'s elements then `b`'s, in a new container in `zone` the caller
 * releases with pico8_ptrs_free(). Neither list may be NULL. */
pointer_list_t collect_concat_pointer_list(const allocator_t *zone, const pointer_list_t *a,
                           const pointer_list_t *b);

/* `kp` writes *out and returns 1 to contribute an element, 0 to contribute
 * nothing -- this port has no nil for `add(m,kp(e)or nil)` to drop. Returns
 * a new container in `zone` the caller releases with pico8_ptrs_free(). `qk`
 * must not be NULL. */
typedef int (*map_pointer_list_fn_t)(void *e, void *ctx, void **out);
pointer_list_t collect_map_pointer_list(const allocator_t *zone, const pointer_list_t *qk,
                           map_pointer_list_fn_t kp, void *ctx);

/* TROU-3 */
/* Membership by string content: every element of `qk` is a string (NULL is
 * skipped); a NULL `qk` is a member of nothing. */
int collect_contains_string(const pointer_list_t *qk, const char *kb);

/* Returns a string in `zone`, released by that zone, and writes the
 * color into *out_color: `c` when d >= 0, 8 when d < 0. `ku` may be NULL
 * (no suffix). A caller that wants only the string may pass any `c`. */
char *collect_format_signed(const allocator_t *zone, p8_fixed_t d, int c, const char *ku,
               int *out_color);

#endif
