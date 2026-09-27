#ifndef PICO8_LAYOUT_H
#define PICO8_LAYOUT_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Never freed, nor is the list `ls` points at: TROU-7. */
typedef struct {
    pico8_struct_header_t header;
    pico8_value_t ls;
} pico8_jb_t;

/* `items` is the cart's `self[1..n]` in ascending index order, holes
 * closed up: the sequence `lq:pq` walks. Its values are borrowed from the
 * overrides, tables shared as the cart's copy shares them. Nothing frees
 * the row, `items`, or `fs` and its entries: TROU-7. */
typedef struct {
    pico8_struct_header_t header;
    pico8_list_t items;
    pico8_value_t c;
    pico8_value_t bg;
    pico8_value_t score;
    pico8_value_t ra;
    pico8_value_t fs;
    pico8_value_t w;
} pico8_lq_t;

/* `lq` and `jb` are root classes declared with no spec string and are not
 * ke-descended, so neither ever self-registers into hv/cv/cg. */
void pico8_layout_init(void);
void pico8_layout_shutdown(void);

/* NULL before pico8_layout_init() and after pico8_layout_shutdown(). Neither
 * class is retrievable through pico8_class_lookup(): both register under the
 * shared "" name-registry slot. */
pico8_class_t *pico8_lq_class(void);
pico8_class_t *pico8_jb_class(void);

const pico8_struct_class_t *pico8_lq_struct_class(void);

pico8_lq_t *pico8_lq_new(const pico8_ob_table_t *overrides);

/* A fresh table the caller frees with pico8_ob_table_free(), its values
 * shared with the row, never copied. Keys in the order the cart's copy
 * `mw` finds them: the positionals `pq` saw at 1..k (k = the count of
 * `fs`), then `c`, `bg`, `score`, `ra` when not nil, `fs`, `w`, then the
 * positionals added after `pq` at k+1... */
pico8_ob_table_t *pico8_lq_table_view(const pico8_lq_t *self);
pico8_jb_t *pico8_jb_new(const pico8_ob_table_t *overrides);

/* Omitting `a` is numerically the same as `a` of 0, so pico8_lq_ol_default
 * passes 0 rather than carrying a "was it provided" flag. A NULL self draws
 * nothing. */
void pico8_lq_ol(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_lq_t *self, pico8_vec_t p, pico8_fixed_t a);
void pico8_lq_ol_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_lq_t *self, pico8_vec_t p);

/* A NULL self, or one whose `ls` is nil, draws nothing -- not even the
 * frame. */
void pico8_jb_ol(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_jb_t *self, pico8_fixed_t om, pico8_fixed_t a);

#endif
