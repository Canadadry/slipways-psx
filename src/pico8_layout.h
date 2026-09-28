#ifndef PICO8_LAYOUT_H
#define PICO8_LAYOUT_H

#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_list.h"
#include "pico8_map_tables.h"
#include "pico8_mem.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Never freed, nor is the list `ls` points at: TROU-7. */
typedef struct {
    pico8_struct_header_t header;
    pico8_list_t *ls;
} pico8_jb_t;

/* `items` is the cart's `self[1..n]` in ascending index order, holes
 * closed up: the sequence `lq:pq` walks. Its values are borrowed from the
 * keys, tables shared as the cart's copy shares them. `ra` is the vector
 * the keys held, borrowed, never copied. Nothing frees the row,
 * `items`, or `fs` and its entries: TROU-7. */
typedef struct {
    pico8_struct_header_t header;
    pico8_list_t items;
    pico8_fixed_t c;
    int has_c; /* 0: `c` is nil, and `c` is then never read */
    pico8_fixed_t bg;
    int has_bg;     /* 0: `bg` is nil, and `bg` is then never read */
    int bg_is_bool; /* 1: `bg` holds a boolean (0/1), 0: a fixed; read only when `has_bg` */
    pico8_fixed_t score;
    int has_score; /* 0: `score` is nil, and `score` is then never read */
    pico8_vec_t *ra;
    pico8_list_t *fs;
    pico8_fixed_t w;
} pico8_lq_t;

/* `lq` and `jb` are root classes with no defaults and are not
 * ke-descended, so neither ever self-registers into hv/cv/cg. */
void pico8_layout_init(void);
void pico8_layout_shutdown(void);

/* 1 from pico8_layout_init() to pico8_layout_shutdown(), 0 otherwise. */
int pico8_lq_ready(void);

/* 1 from pico8_layout_init() to pico8_layout_shutdown(), 0 otherwise. */
int pico8_jb_ready(void);

const pico8_struct_class_t *pico8_lq_struct_class(void);
const pico8_struct_class_t *pico8_jb_struct_class(void);

/* The cart's keys of a row. `items` are its positionals in order, copied
 * into `lq.items` (the values themselves borrowed); `c`, `bg` and `score`
 * count under their flags, `ra` NULL is nil. */
typedef struct {
    const pico8_value_t *items;
    int count;
    int has_c;
    pico8_fixed_t c;
    int has_bg;
    int bg_is_bool; /* read only when `has_bg`, as in `pico8_lq_t` */
    pico8_fixed_t bg;
    int has_score;
    pico8_fixed_t score;
    pico8_vec_t *ra;
} pico8_lq_keys_t;

/* `keys` is read, not kept (NULL: none). */
pico8_lq_t *pico8_lq_new(const pico8_lq_keys_t *keys);

/* The keys of a generated row, its items written into `items` (`cap` of
 * them at most; a nested row, a tuple, as a REF to it) and its `c`. `items`
 * must outlive the keys. */
pico8_lq_keys_t pico8_lq_keys_of_row(const pico8_jb_row_t *row, pico8_value_t *items, int cap);

/* The keys the copy `lq(row)` is built from, by value, nothing allocated:
 * every item of `row` in order (borrowed), its `c`, `bg`, `score`, `ra`. */
pico8_lq_keys_t pico8_lq_keys_of(const pico8_lq_t *row);

/* "bg" as 1a held it, a FIXED or a BOOL; nil (REF NULL) for a nil "bg" or a
 * NULL self. */
pico8_value_t pico8_lq_bg(const pico8_lq_t *self);

/* One line per item of `row` (NULL: no line): a string makes a one-item
 * row, a row its scalar items and its `c`, any other item an empty row. */
pico8_jb_t *pico8_jb_new(const pico8_jb_row_t *row);

/* Omitting `a` is numerically the same as `a` of 0, so pico8_lq_ol_default
 * passes 0 rather than carrying a "was it provided" flag. A NULL self draws
 * nothing. */
void pico8_lq_ol(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_lq_t *self, pico8_vec_t p, pico8_fixed_t a);
void pico8_lq_ol_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_lq_t *self, pico8_vec_t p);

/* A NULL self, or one whose `ls` is nil, draws nothing -- not even the
 * frame. */
void pico8_jb_ol(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_jb_t *self, pico8_fixed_t om, pico8_fixed_t a);

#endif
