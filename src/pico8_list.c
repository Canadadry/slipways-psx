#include "pico8_list.h"

#include <stdlib.h>

#define PICO8_LIST_INITIAL_CAPACITY 4

void pico8_list_init(pico8_list_t *list) {
    list->tag = PICO8_TYPE_TAG_LIST;
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void pico8_list_free(pico8_list_t *list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Simple capacity-doubling growth, starting at a small initial capacity.
 * abort()s on allocation failure rather than returning a partially-valid
 * list -- this phase's own scope (a handful of small fake-entity lists in
 * a unit test / smoke test) never approaches a size where this matters in
 * practice; not a documented PICO-8 behavior, just an ordinary C growable-
 * array choice. */
static void pico8_list_grow(pico8_list_t *list) {
    int new_capacity = list->capacity == 0 ? PICO8_LIST_INITIAL_CAPACITY : list->capacity * 2;
    pico8_value_t *new_items = realloc(list->items, (size_t)new_capacity * sizeof(pico8_value_t));
    if (new_items == NULL) {
        abort();
    }
    list->items = new_items;
    list->capacity = new_capacity;
}

void pico8_list_add(pico8_list_t *list, pico8_value_t value) {
    if (list->count == list->capacity) {
        pico8_list_grow(list);
    }
    list->items[list->count] = value;
    list->count++;
}

int pico8_list_del(pico8_list_t *list, pico8_value_t value) {
    int i;
    for (i = 0; i < list->count; i++) {
        if (pico8_value_equal(list->items[i], value)) {
            int j;
            for (j = i; j < list->count - 1; j++) {
                list->items[j] = list->items[j + 1];
            }
            list->count--;
            return 1;
        }
    }
    return 0;
}

pico8_list_iter_t pico8_list_all(const pico8_list_t *list) {
    pico8_list_iter_t it;
    it.list = list;
    it.index = -1;
    it.last_count = list->count;
    return it;
}

int pico8_list_iter_next(pico8_list_iter_t *iter, pico8_value_t *out_value) {
    int next_index;

    if (iter->index < 0) {
        /* First call: nothing returned yet, so nothing to compare the
         * current count against -- just start at the first element. */
        next_index = 0;
    } else if (iter->list->count < iter->last_count) {
        /* The list shrank since we last returned an element: assume the
         * element at iter->index (the one we just returned) is what was
         * deleted. del() shifts everything after it down by one, so the
         * next real element has slid into that same slot -- re-read it
         * instead of advancing past it (see pico8_list.h's comment on
         * pico8_list_all for the documented scope of this tolerance). */
        next_index = iter->index;
    } else {
        next_index = iter->index + 1;
    }

    if (next_index >= iter->list->count) {
        return 0;
    }

    *out_value = iter->list->items[next_index];
    iter->index = next_index;
    iter->last_count = iter->list->count;
    return 1;
}

void pico8_list_foreach(const pico8_list_t *list, pico8_list_foreach_fn fn, void *ctx) {
    int i;
    for (i = 0; i < list->count; i++) {
        fn(list->items[i], ctx);
    }
}
