#ifndef ENGINE_ENTITY_LIST_H
#define ENGINE_ENTITY_LIST_H

#include "engine/entity_class.h"
#include "engine/allocator.h"

/* pico8_ptrs_t for entities only. Owns only `items`, grown in and given back
 * to `zone`; NULL is storable. */
typedef struct {
    entity_t **items;
    int count;
    int capacity;
    const allocator_t *zone;
} entity_list_t;

/* `zone` must outlive `ents`. */
void entity_list_init(entity_list_t *entity_list, const allocator_t *zone);
/* Takes `items` for `capacity` elements from `zone` at once (0 takes nothing);
 * adding past it grows through `zone` as usual. */
void entity_list_init_reserve(entity_list_t *entity_list, const allocator_t *zone, int capacity);
/* Gives `items` back to the zone and leaves `ents` empty, same zone. */
void entity_list_free(entity_list_t *entity_list);
/* Count back to 0; `items`, its capacity and the zone stay, so later adds
 * reuse them. Gives nothing back to the zone. */
void entity_list_clear(entity_list_t *entity_list);
void entity_list_add(entity_list_t *entity_list, entity_t *e);
/* Removes the FIRST element identical to `e`; 1 if one was removed. */
int entity_list_del(entity_list_t *entity_list, const entity_t *e);
/* 1 if `e` is an element (pointer identity); `ents` NULL holds nothing. */
int entity_list_has(const entity_list_t *entity_list, const entity_t *e);
/* A fresh list in `zone`: `a` then `b`; the caller releases it with
 * pico8_ents_free(). */
entity_list_t entity_list_concat(const allocator_t *zone, const entity_list_t *a,
                            const entity_list_t *b);

/* As pico8_jp_ptrs: a fresh list in `zone` of the elements `ow` keeps, in
 * order; the caller releases it with pico8_ents_free(). A NULL `qk` gives an
 * empty list. */
typedef int (*filter_entity_list_predicate_fn_t)(entity_t *e, void *ctx);
entity_list_t entity_list_filter(const allocator_t *zone, const entity_list_t *qk,
                           filter_entity_list_predicate_fn_t ow, void *ctx);

/* As pico8_ptrs_iter_t. */
typedef struct {
    const entity_list_t *entity_list;
    int index;
    int last_count;
} entity_list_iter_t;
entity_list_iter_t entity_list_all(const entity_list_t *entity_list);
int entity_list_iter_next(entity_list_iter_t *iter, entity_t **out);

#endif
