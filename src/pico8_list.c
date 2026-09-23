#include "pico8_list.h"

#include <assert.h>
#include <stdlib.h>

#include "pico8_ref_audit.h"

#define PICO8_LIST_INITIAL_CAPACITY 4

/* The five entry points below are where EVERY list operation in the port
 * eventually arrives, whatever path the pointer took to get there, so
 * they are the accessor-entry half of the reverse REF-cast audit
 * (src/pico8_ref_audit.h, docs/audit-ref-cast-reverse.md): under
 * -DPICO8_REF_AUDIT, PICO8_REF_AUDIT_EXPECT_LIST asks each incoming
 * pointer whether it really is a pico8_list_t, catching a REF read as a
 * list through a route the cast-site macros never enumerated.
 *
 * pico8_list_init is deliberately NOT checked: it is the function that
 * WRITES the tag, and its argument is legitimately uninitialised.
 *
 * The assert is the other half, and is live in an ordinary build:
 * PICO8_REF_AS_LIST passes a NULL ref through on purpose (a NULL is not a
 * wrong tag), so a list primitive is where an absent field finally has to
 * be refused. */

void pico8_list_init(pico8_list_t *list) {
    list->tag = PICO8_TYPE_TAG_LIST;
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void pico8_list_free(pico8_list_t *list) {
    PICO8_REF_AUDIT_EXPECT_LIST(list);
    assert(list != NULL && "list primitive called on a NULL list");
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Capacity doubling from a small initial capacity. abort()s on allocation
 * failure rather than returning a partially-valid list. */
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
    PICO8_REF_AUDIT_EXPECT_LIST(list);
    assert(list != NULL && "list primitive called on a NULL list");
    if (list->count == list->capacity) {
        pico8_list_grow(list);
    }
    list->items[list->count] = value;
    list->count++;
}

int pico8_list_del(pico8_list_t *list, pico8_value_t value) {
    int i;
    PICO8_REF_AUDIT_EXPECT_LIST(list);
    assert(list != NULL && "list primitive called on a NULL list");
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
    PICO8_REF_AUDIT_EXPECT_LIST(list);
    assert(list != NULL && "list primitive called on a NULL list");
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
         * element at iter->index is what was deleted, and re-read that
         * same slot (see pico8_list_all in pico8_list.h for the scope of
         * this tolerance). */
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
    PICO8_REF_AUDIT_EXPECT_LIST(list);
    assert(list != NULL && "list primitive called on a NULL list");
    for (i = 0; i < list->count; i++) {
        fn(list->items[i], ctx);
    }
}
