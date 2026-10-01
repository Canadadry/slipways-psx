#ifndef PICO8_PTRS_H
#define PICO8_PTRS_H

#include "pico8_zone_choisie.h"

/* PICO-8's add()/del()/all()/foreach() over plain pointers. The container
 * owns only `items`, grown in and given back to `zone`: what an element
 * points at is the caller's. NULL is a storable element. */
typedef struct {
    void **items;
    int count;
    int capacity;
    const pico8_zone_choisie_t *zone;
} pico8_ptrs_t;

/* `zone` must outlive `ptrs`. */
void pico8_ptrs_init(pico8_ptrs_t *ptrs, const pico8_zone_choisie_t *zone);
/* Takes `items` for `capacity` elements from `zone` at once; adding past it
 * grows through `zone` as usual -- in an arena sized to fit, a pico8_fatal. */
void pico8_ptrs_init_reserve(pico8_ptrs_t *ptrs, const pico8_zone_choisie_t *zone, int capacity);
/* Gives `items` back to the zone and leaves `ptrs` empty, same zone. */
void pico8_ptrs_free(pico8_ptrs_t *ptrs);
/* An iterator holds `ptrs`, not `items`: it survives the growth. */
void pico8_ptrs_add(pico8_ptrs_t *ptrs, void *e);
/* Removes the FIRST element identical to `e` (pointer equality) and shifts
 * the later ones down; 1 if one was removed, 0 (no change) otherwise. */
int pico8_ptrs_del(pico8_ptrs_t *ptrs, const void *e);

/* all(): recovers when the element it last returned is deleted during the
 * walk (it re-reads that slot once the count drops), not when an earlier
 * one is -- the documented PICO-8 caveat, kept as is. */
typedef struct {
    const pico8_ptrs_t *ptrs;
    int index;      /* last index returned, -1 before the first call */
    int last_count; /* ptrs->count right after that return (or at init) */
} pico8_ptrs_iter_t;
pico8_ptrs_iter_t pico8_ptrs_all(const pico8_ptrs_t *ptrs);
/* 1 and *out set, or 0 once exhausted (*out untouched). */
int pico8_ptrs_iter_next(pico8_ptrs_iter_t *iter, void **out);

/* No deletion tolerance: `fn` must not add to or delete from `ptrs`. */
typedef void (*pico8_ptrs_foreach_fn)(void *e, void *ctx);
void pico8_ptrs_foreach(const pico8_ptrs_t *ptrs, pico8_ptrs_foreach_fn fn, void *ctx);

#endif
