#ifndef PICO8_ENTS_H
#define PICO8_ENTS_H

#include "pico8_struct.h"
#include "pico8_zone_choisie.h"

/* pico8_ptrs_t for entities only. Owns only `items`, grown in and given back
 * to `zone`; NULL is storable. */
typedef struct {
    pico8_struct_header_t **items;
    int count;
    int capacity;
    const pico8_zone_choisie_t *zone;
} pico8_ents_t;

/* `zone` must outlive `ents`. */
void pico8_ents_init(pico8_ents_t *ents, const pico8_zone_choisie_t *zone);
/* Gives `items` back to the zone and leaves `ents` empty, same zone. */
void pico8_ents_free(pico8_ents_t *ents);
/* Count back to 0; `items`, its capacity and the zone stay, so later adds
 * reuse them. Gives nothing back to the zone. */
void pico8_ents_clear(pico8_ents_t *ents);
void pico8_ents_add(pico8_ents_t *ents, pico8_struct_header_t *e);
/* Removes the FIRST element identical to `e`; 1 if one was removed. */
int pico8_ents_del(pico8_ents_t *ents, const pico8_struct_header_t *e);
/* 1 if `e` is an element (pointer identity); `ents` NULL holds nothing. */
int pico8_ents_has(const pico8_ents_t *ents, const pico8_struct_header_t *e);
/* A fresh list in `zone`: `a` then `b`; the caller releases it with
 * pico8_ents_free(). */
pico8_ents_t pico8_ents_cat(const pico8_zone_choisie_t *zone, const pico8_ents_t *a,
                            const pico8_ents_t *b);

/* As pico8_jp_ptrs: a fresh list in `zone` of the elements `ow` keeps, in
 * order; the caller releases it with pico8_ents_free(). A NULL `qk` gives an
 * empty list. */
typedef int (*pico8_jp_ents_predicate_fn)(pico8_struct_header_t *e, void *ctx);
pico8_ents_t pico8_jp_ents(const pico8_zone_choisie_t *zone, const pico8_ents_t *qk,
                           pico8_jp_ents_predicate_fn ow, void *ctx);

/* As pico8_ptrs_iter_t. */
typedef struct {
    const pico8_ents_t *ents;
    int index;
    int last_count;
} pico8_ents_iter_t;
pico8_ents_iter_t pico8_ents_all(const pico8_ents_t *ents);
int pico8_ents_iter_next(pico8_ents_iter_t *iter, pico8_struct_header_t **out);

#endif
