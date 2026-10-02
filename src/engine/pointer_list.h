#ifndef ENGINE_POINTER_LIST_H
#define ENGINE_POINTER_LIST_H

#include "engine/allocator.h"

/* PICO-8's add()/del()/all()/foreach() over plain pointers. The container
 * owns only `items`, grown in and given back to `zone`: what an element
 * points at is the caller's. NULL is a storable element. */
typedef struct {
    void **items;
    int count;
    int capacity;
    const allocator_t *zone;
} pointer_list_t;

/* `zone` must outlive `ptrs`. */
void pointer_list_init(pointer_list_t *pointer_list, const allocator_t *zone);
/* Takes `items` for `capacity` elements from `zone` at once; adding past it
 * grows through `zone` as usual -- in an arena sized to fit, a pico8_fatal. */
void pointer_list_init_reserve(pointer_list_t *pointer_list, const allocator_t *zone, int capacity);
/* Gives `items` back to the zone and leaves `ptrs` empty, same zone. */
void pointer_list_free(pointer_list_t *pointer_list);
/* An iterator holds `ptrs`, not `items`: it survives the growth. */
void pointer_list_add(pointer_list_t *pointer_list, void *e);
/* Removes the FIRST element identical to `e` (pointer equality) and shifts
 * the later ones down; 1 if one was removed, 0 (no change) otherwise. */
int pointer_list_del(pointer_list_t *pointer_list, const void *e);

/* all(): recovers when the element it last returned is deleted during the
 * walk (it re-reads that slot once the count drops), not when an earlier
 * one is -- the documented PICO-8 caveat, kept as is. */
typedef struct {
    const pointer_list_t *pointer_list;
    int index;      /* last index returned, -1 before the first call */
    int last_count; /* ptrs->count right after that return (or at init) */
} pointer_list_iter_t;
pointer_list_iter_t pointer_list_all(const pointer_list_t *pointer_list);
/* 1 and *out set, or 0 once exhausted (*out untouched). */
int pointer_list_iter_next(pointer_list_iter_t *iter, void **out);

/* No deletion tolerance: `fn` must not add to or delete from `ptrs`. */
typedef void (*pointer_list_foreach_fn_t)(void *e, void *ctx);
void pointer_list_foreach(const pointer_list_t *pointer_list, pointer_list_foreach_fn_t fn, void *ctx);

#endif
