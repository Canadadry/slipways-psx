#ifndef GAME_LAYOUT_H
#define GAME_LAYOUT_H

#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/map_tables.h"
#include "p8/vram.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "engine/allocator.h"

typedef enum {
    LAYOUT_ROW_ITEM_NONE,
    LAYOUT_ROW_ITEM_TEXT,
    LAYOUT_ROW_ITEM_SPRITE
} layout_row_item_kind_t;

typedef struct {
    layout_row_item_kind_t kind;
    const char *text;
    p8_fixed_t color;
    p8_fixed_t sprite_id;
    p8_fixed_t sprite_palette_block;
    vec_t offset;
} layout_row_item_t;

typedef enum {
    LAYOUT_ROW_CELL_NOTHING,
    LAYOUT_ROW_CELL_FIXED,
    LAYOUT_ROW_CELL_STRING,
    LAYOUT_ROW_CELL_ROW,
    LAYOUT_ROW_CELL_TRUE
} layout_row_cell_kind_t;

typedef struct {
    layout_row_cell_kind_t kind;
    union {
        p8_fixed_t fixed;
        const char *text;
        const data_row_t *row;
    } as;
} layout_row_cell_t;

typedef struct {
    entity_t header;
    const allocator_t *zone;
    layout_row_cell_t *items;
    int items_count;
    p8_fixed_t color;
    int has_color;
    p8_fixed_t background;
    int has_background;
    int background_is_bool;
    p8_fixed_t score;
    int has_score;
    layout_row_item_t *placed_items;
    int placed_items_count;
    p8_fixed_t width;
} layout_row_t;

typedef struct {
    entity_t header;
    const allocator_t *zone;
    layout_row_t **rows;
    int rows_count;
} tooltip_t;

void layout_init(void);
void layout_shutdown(void);

int layout_row_ready(void);

int tooltip_ready(void);

const entity_class_t *layout_row_class(void);
const entity_class_t *tooltip_class(void);

typedef struct {
    const layout_row_cell_t *items;
    int count;
    int has_color;
    p8_fixed_t color;
    int has_background;
    int background_is_bool;
    p8_fixed_t background;
    int has_score;
    p8_fixed_t score;
    vec_t *pos;
} layout_row_keys_t;

layout_row_t *layout_row_new(const allocator_t *zone, const layout_row_keys_t *keys);

layout_row_cell_t layout_row_cell_nothing(void);
layout_row_cell_t layout_row_cell_fixed(p8_fixed_t x);
layout_row_cell_t layout_row_cell_string(const char *s);
layout_row_cell_t layout_row_cell_row(const data_row_t *row);
layout_row_cell_t layout_row_cell_item(const data_item_t *item);

layout_row_keys_t layout_row_keys_of_row(const data_row_t *row, layout_row_cell_t *cells, int cap);

layout_row_t *layout_row_copy(const allocator_t *zone, const layout_row_t *row);

void layout_row_add(layout_row_t *self, layout_row_cell_t cell);

tooltip_t *tooltip_new(const allocator_t *zone, const data_row_t *row);

void layout_row_draw_at(const p8_graphics_t *gfx, p8_vram_t *mem, const layout_row_t *self, vec_t p, p8_fixed_t a);
void layout_row_draw_at_default(const p8_graphics_t *gfx, p8_vram_t *mem, const layout_row_t *self, vec_t p);

void tooltip_draw_at(p8_graphics_t *gfx, p8_vram_t *mem, const tooltip_t *self, p8_fixed_t om, p8_fixed_t a);

#endif
