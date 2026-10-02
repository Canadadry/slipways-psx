#ifndef GAME_LAYOUT_H
#define GAME_LAYOUT_H

#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/map_tables.h"
#include "p8/mem.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "engine/allocator.h"

/* What row_measure_item() decides for one item, kept for the draw. */
typedef enum {
    LAYOUT_ROW_ITEM_NONE,
    LAYOUT_ROW_ITEM_TEXT,
    LAYOUT_ROW_ITEM_SPRITE
} layout_row_item_kind_t;

typedef struct {
    layout_row_item_kind_t kind;
    const char *text;        /* TEXT only; a copy in the row's zone */
    p8_fixed_t color;     /* TEXT only: the row's color, or 6 */
    p8_fixed_t sprite_id; /* SPRITE only */
    p8_fixed_t sprite_palette_block; /* SPRITE only: the block hud_select_palette_block() activates */
    vec_t offset;           /* offset within the line; set by layout_row_construct(), not by row_measure_item() */
} layout_row_item_t;

/* The five cases row_measure_item() tells a positional apart by, written by
 * the producers of the keys: absent and false are both NOTHING, the union
 * does not tell them apart. TRUE is any other true value that is not a
 * number, a string or a row. */
typedef enum {
    LAYOUT_ROW_CELL_NOTHING,
    LAYOUT_ROW_CELL_FIXED,
    LAYOUT_ROW_CELL_STRING,
    LAYOUT_ROW_CELL_ROW,
    LAYOUT_ROW_CELL_TRUE
} layout_row_cell_kind_t;

/* In a row, `text` is a copy in the row's zone. `row` is borrowed: it is
 * read only by the layout, and by a layout_row_copy() in the same frame (a
 * research project's layout row copied into a button's). A ROW is read as a
 * tuple (sprite number, width, palette block, spacing) or as a resource,
 * whatever it points at. */
typedef struct {
    layout_row_cell_kind_t kind;
    union {
        p8_fixed_t fixed;
        const char *text;
        const data_row_t *row;
    } as;
} layout_row_cell_t;

/* `items` are the positionals in ascending index order, holes closed up:
 * the sequence layout_row_construct() walks, `items_count` of them
 * (layout_row_add() appends without laying out again, so
 * `placed_items_count` may be smaller). `pos` is the vector the keys held,
 * borrowed, never copied. The row, `items`, `placed_items` and the texts are
 * in `zone`, given by the creator: they go with it. */
typedef struct {
    entity_t header;
    const allocator_t *zone;
    layout_row_cell_t *items;
    int items_count;
    p8_fixed_t color;
    int has_color; /* 0: `color` is absent, and then never read */
    p8_fixed_t background;
    int has_background;     /* 0: `background` is absent, and then never read */
    int background_is_bool; /* 1: `background` holds a boolean (0/1), 0: a fixed; read only when has_background */
    p8_fixed_t score;
    int has_score; /* 0: `score` is absent, and then never read */
    const vec_t *pos;
    layout_row_item_t *placed_items; /* NULL: not laid out yet */
    int placed_items_count;
    p8_fixed_t width;
} layout_row_t;

/* `rows` NULL: not laid out yet. The tooltip, `rows` and its lines are in
 * `zone`, given by the creator: they go with it. */
typedef struct {
    entity_t header;
    const allocator_t *zone;
    layout_row_t **rows;
    int rows_count;
} tooltip_t;

/* The layout row and the tooltip are root classes with no defaults, not
 * built through entity_construct(), so neither ever registers itself in the
 * live list or the registries. */
void layout_init(void);
void layout_shutdown(void);

/* 1 from layout_init() to layout_shutdown(), 0 otherwise. */
int layout_row_ready(void);

/* 1 from layout_init() to layout_shutdown(), 0 otherwise. */
int tooltip_ready(void);

const entity_class_t *layout_row_class(void);
const entity_class_t *tooltip_class(void);

/* The keys of a row. `items` are its positionals in order, each a cell
 * copied into the row's `items` (a text copied too, a ROW borrowed); color,
 * background and score count under their flags, a NULL `pos` is absent. */
typedef struct {
    const layout_row_cell_t *items;
    int count;
    int has_color;
    p8_fixed_t color;
    int has_background;
    int background_is_bool; /* read only when has_background, as in layout_row_t */
    p8_fixed_t background;
    int has_score;
    p8_fixed_t score;
    vec_t *pos;
} layout_row_keys_t;

/* `keys` is read, not kept (NULL: none). `zone` holds the row, its arrays
 * and its texts, is kept for layout_row_add(), and must outlive the row. */
layout_row_t *layout_row_new(const allocator_t *zone, const layout_row_keys_t *keys);

/* Cells as the producers of keys write them. */
layout_row_cell_t layout_row_cell_nothing(void);
layout_row_cell_t layout_row_cell_fixed(p8_fixed_t x);
/* The string is borrowed until a row copies it; a NULL one stops the run:
 * no producer writes one. */
layout_row_cell_t layout_row_cell_string(const char *s);
/* `row` borrowed; NULL is NOTHING. */
layout_row_cell_t layout_row_cell_row(const data_row_t *row);
/* An item of the data tables as row_measure_item() tells it apart: a row,
 * a string, a number; false and a NULL `ref` are NOTHING. What it points at
 * is borrowed. */
layout_row_cell_t layout_row_cell_item(const data_item_t *item);

/* The keys of a row of the data tables, its items written into `cells`
 * (`cap` of them at most; a nested row, a tuple, as a ROW cell) and its
 * color. `cells` must outlive the keys. */
layout_row_keys_t layout_row_keys_of_row(const data_row_t *row, layout_row_cell_t *cells, int cap);

/* A copy of `row`, laid out anew: every cell in order (texts copied, ROWs
 * borrowed), its color, background, score and position. It is in `zone`, as
 * with layout_row_new(). */
layout_row_t *layout_row_copy(const allocator_t *zone, const layout_row_t *row);

/* Appends `cell` (its text copied) to a row already laid out, growing
 * `items` in the row's zone; `placed_items` and `width` are left as they
 * were. */
void layout_row_add(layout_row_t *self, layout_row_cell_t cell);

/* One line per item of `row` (NULL: no line): a string makes a one-item
 * row, a row its scalar items and its color, any other item an empty row.
 * `row` is read, not kept: the tooltip, its lines and their texts are in
 * `zone`, which must outlive the tooltip. */
tooltip_t *tooltip_new(const allocator_t *zone, const data_row_t *row);

/* Omitting the alignment factor is numerically the same as passing 0, so
 * layout_row_draw_at_default passes 0 rather than carrying a "was it
 * provided" flag. A NULL self draws nothing. */
void layout_row_draw_at(const p8_graphics_t *gfx, p8_memory_t *mem, const layout_row_t *self, vec_t p, p8_fixed_t a);
void layout_row_draw_at_default(const p8_graphics_t *gfx, p8_memory_t *mem, const layout_row_t *self, vec_t p);

/* A NULL self, or one whose `rows` is NULL, draws nothing -- not even the
 * frame. */
void tooltip_draw_at(p8_graphics_t *gfx, p8_memory_t *mem, const tooltip_t *self, p8_fixed_t om, p8_fixed_t a);

#endif
