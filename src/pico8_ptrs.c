#include "pico8_ptrs.h"

#include <assert.h>
#include <stddef.h>

#define PICO8_PTRS_INITIAL_CAPACITY 4

void pico8_ptrs_init(pico8_ptrs_t *ptrs, const pico8_zone_choisie_t *zone) {
    assert(zone != NULL && "ptrs_init: a list without a zone would grow nowhere");
    ptrs->items = NULL;
    ptrs->count = 0;
    ptrs->capacity = 0;
    ptrs->zone = zone;
}

void pico8_ptrs_free(pico8_ptrs_t *ptrs) {
    assert(ptrs != NULL && "ptrs primitive called on a NULL container");
    if (ptrs->items != NULL) {
        pico8_zone_release(ptrs->zone, (unsigned char *)ptrs->items);
    }
    ptrs->items = NULL;
    ptrs->count = 0;
    ptrs->capacity = 0;
}

static void pico8_ptrs_grow(pico8_ptrs_t *ptrs) {
    int new_capacity = ptrs->capacity == 0 ? PICO8_PTRS_INITIAL_CAPACITY : ptrs->capacity * 2;
    assert(ptrs->zone != NULL && "ptrs_add on a list pico8_ptrs_init never gave a zone");
    ptrs->items = (void **)pico8_zone_grow(ptrs->zone, (unsigned char *)ptrs->items,
                                           (size_t)ptrs->capacity * sizeof(void *),
                                           (size_t)new_capacity * sizeof(void *));
    ptrs->capacity = new_capacity;
}

void pico8_ptrs_add(pico8_ptrs_t *ptrs, void *e) {
    assert(ptrs != NULL && "ptrs primitive called on a NULL container");
    if (ptrs->count == ptrs->capacity) {
        pico8_ptrs_grow(ptrs);
    }
    ptrs->items[ptrs->count] = e;
    ptrs->count++;
}

int pico8_ptrs_del(pico8_ptrs_t *ptrs, const void *e) {
    int i;
    assert(ptrs != NULL && "ptrs primitive called on a NULL container");
    for (i = 0; i < ptrs->count; i++) {
        if (ptrs->items[i] == e) {
            int j;
            for (j = i; j < ptrs->count - 1; j++) {
                ptrs->items[j] = ptrs->items[j + 1];
            }
            ptrs->count--;
            return 1;
        }
    }
    return 0;
}

pico8_ptrs_iter_t pico8_ptrs_all(const pico8_ptrs_t *ptrs) {
    pico8_ptrs_iter_t it;
    assert(ptrs != NULL && "ptrs primitive called on a NULL container");
    it.ptrs = ptrs;
    it.index = -1;
    it.last_count = ptrs->count;
    return it;
}

int pico8_ptrs_iter_next(pico8_ptrs_iter_t *iter, void **out) {
    int next_index;

    if (iter->index < 0) {
        next_index = 0;
    } else if (iter->ptrs->count < iter->last_count) {
        next_index = iter->index;
    } else {
        next_index = iter->index + 1;
    }

    if (next_index >= iter->ptrs->count) {
        return 0;
    }

    *out = iter->ptrs->items[next_index];
    iter->index = next_index;
    iter->last_count = iter->ptrs->count;
    return 1;
}

void pico8_ptrs_foreach(const pico8_ptrs_t *ptrs, pico8_ptrs_foreach_fn fn, void *ctx) {
    int i;
    assert(ptrs != NULL && "ptrs primitive called on a NULL container");
    for (i = 0; i < ptrs->count; i++) {
        fn(ptrs->items[i], ctx);
    }
}
