#ifndef ENGINE_COLLECT_H
#define ENGINE_COLLECT_H

#include "engine/geom.h"
#include "engine/pointer_list.h"

/* The predicate gets each element plus the context pointer, in place of a
 * closure's captured state. Returns a new container in `zone` the caller
 * releases with pointer_list_free(); a NULL source list gives an empty one. */
typedef int (*filter_pointer_list_predicate_fn_t)(void *e, void *ctx);
pointer_list_t collect_filter_pointer_list(const allocator_t *zone, const pointer_list_t *qk,
                           filter_pointer_list_predicate_fn_t ow, void *ctx);

/* Both lists' elements, the first's then the second's, in a new container
 * in `zone` the caller releases with pointer_list_free(). Neither list may
 * be NULL. */
pointer_list_t collect_concat_pointer_list(const allocator_t *zone, const pointer_list_t *a,
                           const pointer_list_t *b);

/* The mapper writes *out and returns 1 to contribute an element, 0 to
 * contribute nothing. Returns a new container in `zone` the caller releases
 * with pointer_list_free(). The source list must not be NULL. */
typedef int (*map_pointer_list_fn_t)(void *e, void *ctx, void **out);
pointer_list_t collect_map_pointer_list(const allocator_t *zone, const pointer_list_t *qk,
                           map_pointer_list_fn_t kp, void *ctx);

/* GAP-3 */
/* Membership by string content: every element of the list is a string
 * (NULL is skipped); a NULL list contains nothing. */
int collect_contains_string(const pointer_list_t *qk, const char *kb);

/* Returns the signed value as a string in `zone`, released by that zone,
 * and writes the color into *out_color: the given color when the value is
 * not negative, 8 otherwise. The suffix may be NULL (none). A caller that
 * wants only the string may pass any color. */
char *collect_format_signed(const allocator_t *zone, p8_fixed_t d, int c, const char *ku,
               int *out_color);

#endif
