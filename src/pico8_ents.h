#ifndef PICO8_ENTS_H
#define PICO8_ENTS_H

#include "pico8_struct.h"

/* pico8_ptrs_t for entities only. Owns only `items`; NULL is storable. */
typedef struct {
    pico8_struct_header_t **items;
    int count;
    int capacity;
} pico8_ents_t;

void pico8_ents_init(pico8_ents_t *ents);
/* Releases `items` only and leaves `ents` empty. */
void pico8_ents_free(pico8_ents_t *ents);
void pico8_ents_add(pico8_ents_t *ents, pico8_struct_header_t *e);
/* Removes the FIRST element identical to `e`; 1 if one was removed. */
int pico8_ents_del(pico8_ents_t *ents, const pico8_struct_header_t *e);
/* 1 if `e` is an element (pointer identity); `ents` NULL holds nothing. */
int pico8_ents_has(const pico8_ents_t *ents, const pico8_struct_header_t *e);
/* A fresh list: `a` then `b`; the caller frees its `items`. */
pico8_ents_t pico8_ents_cat(const pico8_ents_t *a, const pico8_ents_t *b);

/* As pico8_ptrs_iter_t. */
typedef struct {
    const pico8_ents_t *ents;
    int index;
    int last_count;
} pico8_ents_iter_t;
pico8_ents_iter_t pico8_ents_all(const pico8_ents_t *ents);
int pico8_ents_iter_next(pico8_ents_iter_t *iter, pico8_struct_header_t **out);

#endif
