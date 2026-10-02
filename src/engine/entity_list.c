#include "engine/entity_list.h"

#include <assert.h>
#include <stddef.h>

#define PICO8_ENTS_INITIAL_CAPACITY 4

void pico8_ents_init(pico8_ents_t *ents, const pico8_zone_choisie_t *zone) {
    assert(zone != NULL && "ents_init: a list without a zone would grow nowhere");
    ents->items = NULL;
    ents->count = 0;
    ents->capacity = 0;
    ents->zone = zone;
}

void pico8_ents_init_reserve(pico8_ents_t *ents, const pico8_zone_choisie_t *zone, int capacity) {
    assert(capacity >= 0);
    pico8_ents_init(ents, zone);
    if (capacity == 0) {
        return;
    }
    ents->items = (entity_t **)pico8_zone_alloc(zone, (size_t)capacity * sizeof *ents->items);
    ents->capacity = capacity;
}

void pico8_ents_free(pico8_ents_t *ents) {
    assert(ents != NULL && "ents primitive called on a NULL container");
    if (ents->items != NULL) {
        pico8_zone_release(ents->zone, (unsigned char *)ents->items);
    }
    ents->items = NULL;
    ents->count = 0;
    ents->capacity = 0;
}

void pico8_ents_clear(pico8_ents_t *ents) {
    assert(ents != NULL && "ents primitive called on a NULL container");
    ents->count = 0;
}

static void pico8_ents_grow(pico8_ents_t *ents) {
    int new_capacity = ents->capacity == 0 ? PICO8_ENTS_INITIAL_CAPACITY : ents->capacity * 2;
    assert(ents->zone != NULL && "ents_add on a list pico8_ents_init never gave a zone");
    ents->items = (entity_t **)pico8_zone_grow(
        ents->zone, (unsigned char *)ents->items, (size_t)ents->capacity * sizeof *ents->items,
        (size_t)new_capacity * sizeof *ents->items);
    ents->capacity = new_capacity;
}

void pico8_ents_add(pico8_ents_t *ents, entity_t *e) {
    assert(ents != NULL && "ents primitive called on a NULL container");
    if (ents->count == ents->capacity) {
        pico8_ents_grow(ents);
    }
    ents->items[ents->count] = e;
    ents->count++;
}

int pico8_ents_del(pico8_ents_t *ents, const entity_t *e) {
    int i;
    assert(ents != NULL && "ents primitive called on a NULL container");
    for (i = 0; i < ents->count; i++) {
        if (ents->items[i] == e) {
            int j;
            for (j = i; j < ents->count - 1; j++) {
                ents->items[j] = ents->items[j + 1];
            }
            ents->count--;
            return 1;
        }
    }
    return 0;
}

int pico8_ents_has(const pico8_ents_t *ents, const entity_t *e) {
    int i;
    if (ents == NULL) {
        return 0;
    }
    for (i = 0; i < ents->count; i++) {
        if (ents->items[i] == e) {
            return 1;
        }
    }
    return 0;
}

pico8_ents_t pico8_ents_cat(const pico8_zone_choisie_t *zone, const pico8_ents_t *a,
                            const pico8_ents_t *b) {
    pico8_ents_t out;
    int i;
    assert(a != NULL && b != NULL && "cat: a NULL list is not tolerated, unlike has");
    pico8_ents_init(&out, zone);
    for (i = 0; i < a->count; i++) {
        pico8_ents_add(&out, a->items[i]);
    }
    for (i = 0; i < b->count; i++) {
        pico8_ents_add(&out, b->items[i]);
    }
    return out;
}

pico8_ents_t pico8_jp_ents(const pico8_zone_choisie_t *zone, const pico8_ents_t *qk,
                           pico8_jp_ents_predicate_fn ow, void *ctx) {
    pico8_ents_t f;
    pico8_ents_iter_t it;
    entity_t *e;
    pico8_ents_init(&f, zone);
    if (qk == NULL) {
        return f;
    }
    it = pico8_ents_all(qk);
    while (pico8_ents_iter_next(&it, &e)) {
        if (ow(e, ctx)) {
            pico8_ents_add(&f, e);
        }
    }
    return f;
}

pico8_ents_iter_t pico8_ents_all(const pico8_ents_t *ents) {
    pico8_ents_iter_t it;
    assert(ents != NULL && "ents primitive called on a NULL container");
    it.ents = ents;
    it.index = -1;
    it.last_count = ents->count;
    return it;
}

int pico8_ents_iter_next(pico8_ents_iter_t *iter, entity_t **out) {
    int next_index;

    if (iter->index < 0) {
        next_index = 0;
    } else if (iter->ents->count < iter->last_count) {
        next_index = iter->index;
    } else {
        next_index = iter->index + 1;
    }

    if (next_index >= iter->ents->count) {
        return 0;
    }

    *out = iter->ents->items[next_index];
    iter->index = next_index;
    iter->last_count = iter->ents->count;
    return 1;
}
