#ifndef PICO8_LAYOUT_H
#define PICO8_LAYOUT_H

#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/map_tables.h"
#include "p8/mem.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "engine/allocator.h"

/* C has no closures, so lq:mn bakes the draw its Lua counterpart returns as a
 * closure into this tagged record instead. */
typedef enum {
    PICO8_LQ_ITEM_NONE,
    PICO8_LQ_ITEM_TEXT,
    PICO8_LQ_ITEM_SPRITE
} pico8_lq_item_kind_t;

typedef struct {
    pico8_lq_item_kind_t kind;
    const char *text;        /* TEXT only; a copy in the row's zone */
    p8_fixed_t color;     /* TEXT only: self.c or 6 */
    p8_fixed_t sprite_id; /* SPRITE only: f */
    p8_fixed_t sprite_qw; /* SPRITE only: dv()'s palette-block arg */
    pico8_vec_t d;           /* offset within the line; set by lq:pq, not by lq:mn */
} pico8_lq_item_t;

/* The five cases `lq:mn` tells a positional apart by, written by the
 * producers of the keys: nil and false are both NOTHING, the union does not
 * tell them apart. TRUE is any other truthy value that is not a number, a
 * string or a row. */
typedef enum {
    PICO8_LQ_CELL_NOTHING,
    PICO8_LQ_CELL_FIXED,
    PICO8_LQ_CELL_STRING,
    PICO8_LQ_CELL_ROW,
    PICO8_LQ_CELL_TRUE
} pico8_lq_cell_kind_t;

/* In a row, `s` is a copy in the row's zone. `row` is borrowed: it is read
 * only by the layout, and by a pico8_lq_copy in the same frame (`hp:lq` to
 * `js:pq`). A ROW is read as a tuple `{f,w,qw,qv}` or a `{qe=x}` whatever
 * it points at. */
typedef struct {
    pico8_lq_cell_kind_t kind;
    union {
        p8_fixed_t fixed;
        const char *s;
        const pico8_jb_row_t *row;
    } as;
} pico8_lq_cell_t;

/* `items` is the cart's `self[1..n]` in ascending index order, holes
 * closed up: the sequence `lq:pq` walks, `items_count` of them (pico8_lq_add
 * appends without laying out again, so `fs_count` may be smaller). `ra` is
 * the vector the keys held, borrowed, never copied. The row, `items`, `fs`
 * and the texts are in `zone`, given by the creator: they go with it. */
typedef struct {
    entity_t header;
    const pico8_zone_choisie_t *zone;
    pico8_lq_cell_t *items;
    int items_count;
    p8_fixed_t c;
    int has_c; /* 0: `c` is nil, and `c` is then never read */
    p8_fixed_t bg;
    int has_bg;     /* 0: `bg` is nil, and `bg` is then never read */
    int bg_is_bool; /* 1: `bg` holds a boolean (0/1), 0: a fixed; read only when `has_bg` */
    p8_fixed_t score;
    int has_score; /* 0: `score` is nil, and `score` is then never read */
    const pico8_vec_t *ra;
    pico8_lq_item_t *fs; /* NULL: not laid out yet */
    int fs_count;
    p8_fixed_t w;
} pico8_lq_t;

/* `ls` NULL: not laid out yet. The bubble, `ls` and its lines are in
 * `zone`, given by the creator: they go with it. */
typedef struct {
    entity_t header;
    const pico8_zone_choisie_t *zone;
    pico8_lq_t **ls;
    int ls_count;
} pico8_jb_t;

/* `lq` and `jb` are root classes with no defaults and are not
 * ke-descended, so neither ever self-registers into hv/cv/cg. */
void pico8_layout_init(void);
void pico8_layout_shutdown(void);

/* 1 from pico8_layout_init() to pico8_layout_shutdown(), 0 otherwise. */
int pico8_lq_ready(void);

/* 1 from pico8_layout_init() to pico8_layout_shutdown(), 0 otherwise. */
int pico8_jb_ready(void);

const entity_class_t *pico8_lq_struct_class(void);
const entity_class_t *pico8_jb_struct_class(void);

/* The cart's keys of a row. `items` are its positionals in order, each a
 * cell copied into `lq.items` (a text copied too, a ROW borrowed); `c`, `bg` and
 * `score` count under their flags, `ra` NULL is nil. */
typedef struct {
    const pico8_lq_cell_t *items;
    int count;
    int has_c;
    p8_fixed_t c;
    int has_bg;
    int bg_is_bool; /* read only when `has_bg`, as in `pico8_lq_t` */
    p8_fixed_t bg;
    int has_score;
    p8_fixed_t score;
    pico8_vec_t *ra;
} pico8_lq_keys_t;

/* `keys` is read, not kept (NULL: none). `zone` holds the row, its arrays
 * and its texts, is kept for pico8_lq_add, and must outlive the row. */
pico8_lq_t *pico8_lq_new(const pico8_zone_choisie_t *zone, const pico8_lq_keys_t *keys);

/* Cells as the producers of keys write them. */
pico8_lq_cell_t pico8_lq_cell_nothing(void);
pico8_lq_cell_t pico8_lq_cell_fixed(p8_fixed_t x);
/* `s` borrowed until a row copies it; a NULL `s` stops the run: no
 * producer writes one. */
pico8_lq_cell_t pico8_lq_cell_string(const char *s);
/* `row` borrowed; NULL is nil: NOTHING. */
pico8_lq_cell_t pico8_lq_cell_row(const pico8_jb_row_t *row);
/* A generated item as `lq:mn` tells it apart: a row, a string, a number;
 * false and a nil `ref` are NOTHING. What it points at is borrowed. */
pico8_lq_cell_t pico8_lq_cell_item(const pico8_jb_item_t *item);

/* The keys of a generated row, its items written into `cells` (`cap` of
 * them at most; a nested row, a tuple, as a ROW cell) and its `c`. `cells`
 * must outlive the keys. */
pico8_lq_keys_t pico8_lq_keys_of_row(const pico8_jb_row_t *row, pico8_lq_cell_t *cells, int cap);

/* The copy `lq(row)`, laid out anew: every cell of `row` in order (texts
 * copied, ROWs borrowed), its `c`, `bg`, `score`, `ra`. It is in `zone`, as
 * pico8_lq_new. */
pico8_lq_t *pico8_lq_copy(const pico8_zone_choisie_t *zone, const pico8_lq_t *row);

/* The cart's `add(l, v)` on a row already laid out: `cell` (its text
 * copied) is appended to `items`, grown in the row's zone; `fs` and `w` are
 * left as they were. */
void pico8_lq_add(pico8_lq_t *self, pico8_lq_cell_t cell);

/* One line per item of `row` (NULL: no line): a string makes a one-item
 * row, a row its scalar items and its `c`, any other item an empty row.
 * `row` is read, not kept: the bubble, its lines and their texts are in
 * `zone`, which must outlive the `jb`. */
pico8_jb_t *pico8_jb_new(const pico8_zone_choisie_t *zone, const pico8_jb_row_t *row);

/* Omitting `a` is numerically the same as `a` of 0, so pico8_lq_ol_default
 * passes 0 rather than carrying a "was it provided" flag. A NULL self draws
 * nothing. */
void pico8_lq_ol(const p8_graphics_t *gfx, p8_memory_t *mem, const pico8_lq_t *self, pico8_vec_t p, p8_fixed_t a);
void pico8_lq_ol_default(const p8_graphics_t *gfx, p8_memory_t *mem, const pico8_lq_t *self, pico8_vec_t p);

/* A NULL self, or one whose `ls` is nil, draws nothing -- not even the
 * frame. */
void pico8_jb_ol(p8_graphics_t *gfx, p8_memory_t *mem, const pico8_jb_t *self, p8_fixed_t om, p8_fixed_t a);

#endif
