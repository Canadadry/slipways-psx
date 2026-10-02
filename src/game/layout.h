#ifndef GAME_LAYOUT_H
#define GAME_LAYOUT_H

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
    LAYOUT_ROW_ITEM_NONE,
    LAYOUT_ROW_ITEM_TEXT,
    LAYOUT_ROW_ITEM_SPRITE
} layout_row_item_kind_t;

typedef struct {
    layout_row_item_kind_t kind;
    const char *text;        /* TEXT only; a copy in the row's zone */
    p8_fixed_t color;     /* TEXT only: self.c or 6 */
    p8_fixed_t sprite_id; /* SPRITE only: f */
    p8_fixed_t sprite_palette_block; /* SPRITE only: dv()'s palette-block arg */
    vec_t offset;           /* offset within the line; set by lq:pq, not by lq:mn */
} layout_row_item_t;

/* The five cases `lq:mn` tells a positional apart by, written by the
 * producers of the keys: nil and false are both NOTHING, the union does not
 * tell them apart. TRUE is any other truthy value that is not a number, a
 * string or a row. */
typedef enum {
    LAYOUT_ROW_CELL_NOTHING,
    LAYOUT_ROW_CELL_FIXED,
    LAYOUT_ROW_CELL_STRING,
    LAYOUT_ROW_CELL_ROW,
    LAYOUT_ROW_CELL_TRUE
} layout_row_cell_kind_t;

/* In a row, `s` is a copy in the row's zone. `row` is borrowed: it is read
 * only by the layout, and by a pico8_lq_copy in the same frame (`hp:lq` to
 * `js:pq`). A ROW is read as a tuple `{f,w,qw,qv}` or a `{qe=x}` whatever
 * it points at. */
typedef struct {
    layout_row_cell_kind_t kind;
    union {
        p8_fixed_t fixed;
        const char *text;
        const data_row_t *row;
    } as;
} layout_row_cell_t;

/* `items` is the cart's `self[1..n]` in ascending index order, holes
 * closed up: the sequence `lq:pq` walks, `items_count` of them (pico8_lq_add
 * appends without laying out again, so `fs_count` may be smaller). `ra` is
 * the vector the keys held, borrowed, never copied. The row, `items`, `fs`
 * and the texts are in `zone`, given by the creator: they go with it. */
typedef struct {
    entity_t header;
    const allocator_t *zone;
    layout_row_cell_t *items;
    int items_count;
    p8_fixed_t color;
    int has_color; /* 0: `c` is nil, and `c` is then never read */
    p8_fixed_t background;
    int has_background;     /* 0: `bg` is nil, and `bg` is then never read */
    int background_is_bool; /* 1: `bg` holds a boolean (0/1), 0: a fixed; read only when `has_bg` */
    p8_fixed_t score;
    int has_score; /* 0: `score` is nil, and `score` is then never read */
    const vec_t *pos;
    layout_row_item_t *placed_items; /* NULL: not laid out yet */
    int placed_items_count;
    p8_fixed_t width;
} layout_row_t;

/* `ls` NULL: not laid out yet. The bubble, `ls` and its lines are in
 * `zone`, given by the creator: they go with it. */
typedef struct {
    entity_t header;
    const allocator_t *zone;
    layout_row_t **rows;
    int rows_count;
} tooltip_t;

/* `lq` and `jb` are root classes with no defaults and are not
 * ke-descended, so neither ever self-registers into hv/cv/cg. */
void layout_init(void);
void layout_shutdown(void);

/* 1 from pico8_layout_init() to pico8_layout_shutdown(), 0 otherwise. */
int layout_row_ready(void);

/* 1 from pico8_layout_init() to pico8_layout_shutdown(), 0 otherwise. */
int tooltip_ready(void);

const entity_class_t *layout_row_class(void);
const entity_class_t *tooltip_class(void);

/* The cart's keys of a row. `items` are its positionals in order, each a
 * cell copied into `lq.items` (a text copied too, a ROW borrowed); `c`, `bg` and
 * `score` count under their flags, `ra` NULL is nil. */
typedef struct {
    const layout_row_cell_t *items;
    int count;
    int has_color;
    p8_fixed_t color;
    int has_background;
    int background_is_bool; /* read only when `has_bg`, as in `pico8_lq_t` */
    p8_fixed_t background;
    int has_score;
    p8_fixed_t score;
    vec_t *pos;
} layout_row_keys_t;

/* `keys` is read, not kept (NULL: none). `zone` holds the row, its arrays
 * and its texts, is kept for pico8_lq_add, and must outlive the row. */
layout_row_t *layout_row_new(const allocator_t *zone, const layout_row_keys_t *keys);

/* Cells as the producers of keys write them. */
layout_row_cell_t layout_row_cell_nothing(void);
layout_row_cell_t layout_row_cell_fixed(p8_fixed_t x);
/* `s` borrowed until a row copies it; a NULL `s` stops the run: no
 * producer writes one. */
layout_row_cell_t layout_row_cell_string(const char *s);
/* `row` borrowed; NULL is nil: NOTHING. */
layout_row_cell_t layout_row_cell_row(const data_row_t *row);
/* A generated item as `lq:mn` tells it apart: a row, a string, a number;
 * false and a nil `ref` are NOTHING. What it points at is borrowed. */
layout_row_cell_t layout_row_cell_item(const data_item_t *item);

/* The keys of a generated row, its items written into `cells` (`cap` of
 * them at most; a nested row, a tuple, as a ROW cell) and its `c`. `cells`
 * must outlive the keys. */
layout_row_keys_t layout_row_keys_of_row(const data_row_t *row, layout_row_cell_t *cells, int cap);

/* The copy `lq(row)`, laid out anew: every cell of `row` in order (texts
 * copied, ROWs borrowed), its `c`, `bg`, `score`, `ra`. It is in `zone`, as
 * pico8_lq_new. */
layout_row_t *layout_row_copy(const allocator_t *zone, const layout_row_t *row);

/* The cart's `add(l, v)` on a row already laid out: `cell` (its text
 * copied) is appended to `items`, grown in the row's zone; `fs` and `w` are
 * left as they were. */
void layout_row_add(layout_row_t *self, layout_row_cell_t cell);

/* One line per item of `row` (NULL: no line): a string makes a one-item
 * row, a row its scalar items and its `c`, any other item an empty row.
 * `row` is read, not kept: the bubble, its lines and their texts are in
 * `zone`, which must outlive the `jb`. */
tooltip_t *tooltip_new(const allocator_t *zone, const data_row_t *row);

/* Omitting `a` is numerically the same as `a` of 0, so layout_row_draw_at_default
 * passes 0 rather than carrying a "was it provided" flag. A NULL self draws
 * nothing. */
void layout_row_draw_at(const p8_graphics_t *gfx, p8_memory_t *mem, const layout_row_t *self, vec_t p, p8_fixed_t a);
void layout_row_draw_at_default(const p8_graphics_t *gfx, p8_memory_t *mem, const layout_row_t *self, vec_t p);

/* A NULL self, or one whose `ls` is nil, draws nothing -- not even the
 * frame. */
void tooltip_draw_at(p8_graphics_t *gfx, p8_memory_t *mem, const tooltip_t *self, p8_fixed_t om, p8_fixed_t a);

#endif
