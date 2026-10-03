#include "game/layout.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "game/hud.h"

static int g_layout_up = 0;
static const entity_class_t layout_row_class_def;

layout_row_cell_t layout_row_cell_nothing(void) {
    layout_row_cell_t cell = {.kind = LAYOUT_ROW_CELL_NOTHING};
    return cell;
}

layout_row_cell_t layout_row_cell_fixed(p8_fixed_t x) {
    layout_row_cell_t cell = {.kind = LAYOUT_ROW_CELL_FIXED, .as.fixed = x};
    return cell;
}

layout_row_cell_t layout_row_cell_string(const char *s) {
    layout_row_cell_t cell = {.kind = LAYOUT_ROW_CELL_STRING, .as.text = s};

    assert(s != NULL && "layout_row_cell_string: no producer writes a NULL string; its kind would have no reader in row_measure_item");
    return cell;
}

layout_row_cell_t layout_row_cell_row(const data_row_t *row) {
    layout_row_cell_t cell = {.kind = LAYOUT_ROW_CELL_ROW, .as.row = row};

    /* Not optional: a NULL read as anything but NOTHING reaches the sprite
     * branch of row_measure_item() and draws sprite 0, a block of raw data
     * on screen (test_layout_nil_item_draws_nothing). */
    if (row == NULL) {
        return layout_row_cell_nothing();
    }
    return cell;
}

layout_row_cell_t layout_row_cell_item(const data_item_t *item) {
    if (item->row != NULL) {
        return layout_row_cell_row(item->row);
    }
    if (item->text != NULL) {
        return layout_row_cell_string(item->text);
    }
    if (item->is_false) {
        return layout_row_cell_nothing();
    }
    if (item->is_ref) {
        return layout_row_cell_row(item->ref);
    }
    if (item->is_fixed) {
        return layout_row_cell_fixed(item->fixed);
    }
    return layout_row_cell_fixed(p8_fixed_from_int(item->number));
}

/* One of a tuple's four items, or an item of a line. */
static layout_row_cell_t row_scalar_item(const data_item_t *item) {
    assert(item->row == NULL && "a tuple or a line holds scalar items only; a row inside one has no reader here");
    return layout_row_cell_item(item);
}

static const char *row_copy_text(const allocator_t *zone, const char *s) {
    size_t n = strlen(s) + 1;
    char *copy = (char *)allocator_alloc(zone, n);

    memcpy(copy, s, n);
    return copy;
}

/* A STRING cell points at its copy in `zone`; any other cell is kept as is. */
static layout_row_cell_t row_own_cell(const allocator_t *zone, layout_row_cell_t cell) {
    if (cell.kind == LAYOUT_ROW_CELL_STRING) {
        cell.as.text = row_copy_text(zone, cell.as.text);
    }
    return cell;
}

/* Writes out_item plus the item's width and spacing (out_w, out_spacing).
 * `self` may be NULL; it is only read for the TEXT branch's color. A
 * non-FIXED second, third or fourth tuple item reads as 0. */
static void row_measure_item(const layout_row_t *self, layout_row_cell_t cell, layout_row_item_t *out_item,
                         p8_fixed_t *out_w, p8_fixed_t *out_spacing) {
    p8_fixed_t width = p8_fixed_from_int(6);
    p8_fixed_t palette_block = p8_fixed_from_int(0);
    int have_spacing = 0;
    p8_fixed_t spacing = p8_fixed_from_int(0);

    if (cell.kind == LAYOUT_ROW_CELL_ROW) {
        const data_row_t *t = cell.as.row;
        const data_item_t wrap[] = {{.is_fixed = 1, .fixed = t->resource}, {.number = 5}};
        const data_item_t *items = t->has_resource ? wrap : t->items;
        int count = t->has_resource ? 2 : t->count;
        layout_row_cell_t f1, f2, f3, f4;
        int have_f1 = count >= 1;
        int have_f2 = count >= 2;
        int have_f3 = count >= 3;
        int have_f4 = count >= 4;

        f1 = have_f1 ? row_scalar_item(&items[0]) : layout_row_cell_nothing();
        f2 = have_f2 ? row_scalar_item(&items[1]) : layout_row_cell_nothing();
        f3 = have_f3 ? row_scalar_item(&items[2]) : layout_row_cell_nothing();
        f4 = have_f4 ? row_scalar_item(&items[3]) : layout_row_cell_nothing();

        cell = f1;
        if (have_f2 && f2.kind == LAYOUT_ROW_CELL_FIXED) {
            width = f2.as.fixed;
        }
        if (have_f3 && f3.kind == LAYOUT_ROW_CELL_FIXED) {
            palette_block = f3.as.fixed;
        }
        if (have_f4 && f4.kind != LAYOUT_ROW_CELL_NOTHING) {
            have_spacing = 1;
            spacing = (f4.kind == LAYOUT_ROW_CELL_FIXED) ? f4.as.fixed : p8_fixed_from_int(0);
        }
    }

    if (cell.kind == LAYOUT_ROW_CELL_STRING) {
        p8_fixed_t c = p8_fixed_from_int(6);

        if (self != NULL && self->has_color) {
            c = self->color;
        }
        out_item->kind = LAYOUT_ROW_ITEM_TEXT;
        out_item->text = cell.as.text;
        out_item->color = c;
        *out_w = p8_fixed_from_int((int)strlen(cell.as.text) * 4 + 3);
        *out_spacing = p8_fixed_from_int(1);
        return;
    }

    if (cell.kind != LAYOUT_ROW_CELL_NOTHING) {
        out_item->kind = LAYOUT_ROW_ITEM_SPRITE;
        out_item->sprite_id = (cell.kind == LAYOUT_ROW_CELL_FIXED) ? cell.as.fixed : p8_fixed_from_int(0);
        out_item->sprite_palette_block = palette_block;
    } else {
        out_item->kind = LAYOUT_ROW_ITEM_NONE;
    }
    *out_w = width;
    *out_spacing = have_spacing ? spacing : p8_fixed_from_int(2);
}

/* The last item's spacing starts at 0, so a row with no item gets width 0:
 * empty rows are built, for a button with no row and for a tooltip line that
 * is neither a string nor a row. */
static void layout_row_construct(layout_row_t *self) {
    p8_fixed_t w = p8_fixed_from_int(0);
    p8_fixed_t fs = p8_fixed_from_int(0);
    int n = self->items_count;
    int i;

    self->placed_items = (layout_row_item_t *)allocator_alloc(self->zone, (size_t)(n > 0 ? n : 1) * sizeof *self->placed_items);
    for (i = 0; i < n; i++) {
        p8_fixed_t w_i = p8_fixed_from_int(0);
        p8_fixed_t spacing_i = p8_fixed_from_int(0);
        p8_fixed_t x = w;

        row_measure_item(self, self->items[i], &self->placed_items[i], &w_i, &spacing_i);
        /* A tuple is borrowed until this layout only: its text is copied. */
        if (self->items[i].kind == LAYOUT_ROW_CELL_ROW && self->placed_items[i].kind == LAYOUT_ROW_ITEM_TEXT) {
            self->placed_items[i].text = row_copy_text(self->zone, self->placed_items[i].text);
        }
        self->placed_items[i].offset = vec_make(x, p8_fixed_from_int(0));
        w = p8_fixed_add(w, p8_fixed_add(w_i, spacing_i));
        fs = spacing_i;
    }
    self->placed_items_count = n;
    self->width = p8_fixed_sub(w, fs);
}

static void row_draw_at_item_draw(const p8_graphics_t *gfx, p8_vram_t *mem, const layout_row_item_t *item,
                                   vec_t p) {
    vec_t pos = vec_add(p, item->offset);

    switch (item->kind) {
    case LAYOUT_ROW_ITEM_TEXT:
        hud_print_outlined_default(gfx, mem, item->text, p8_fixed_add(pos.x, p8_fixed_from_int(2)),
                          p8_fixed_add(pos.y, p8_fixed_from_int(1)),
                          p8_fixed_flr_int(item->color));
        break;
    case LAYOUT_ROW_ITEM_SPRITE:
        hud_select_palette_block(mem, item->sprite_palette_block);
        p8_spr_default(gfx, mem, p8_fixed_flr_int(item->sprite_id),
                           p8_fixed_flr_int(pos.x),
                           p8_fixed_flr_int(pos.y));
        hud_select_palette_block_default(mem);
        break;
    case LAYOUT_ROW_ITEM_NONE:
    default:
        break;
    }
}

void layout_row_draw_at(const p8_graphics_t *gfx, p8_vram_t *mem, const layout_row_t *self, vec_t p, p8_fixed_t a) {
    p8_fixed_t w;
    int i;

    if (self == NULL) {
        return;
    }
    assert(self->header.class == &layout_row_class_def &&
           "layout_row_draw_at: every row reaching it (a planet's or a button's layout_row, a button's side_tooltip, "
           "a tooltip's rows, the draw_row dispatch) is a layout row; anything else would be read through the wrong layout");
    w = self->width;
    p = vec_sub(p, vec_make(p8_fixed_mul(a, w), p8_fixed_from_int(0)));

    if (self->placed_items == NULL) {
        return;
    }
    for (i = 0; i < self->placed_items_count; i++) {
        row_draw_at_item_draw(gfx, mem, &self->placed_items[i], p);
    }
}

void layout_row_draw_at_default(const p8_graphics_t *gfx, p8_vram_t *mem, const layout_row_t *self, vec_t p) {
    layout_row_draw_at(gfx, mem, self, p, p8_fixed_from_int(0));
}

/* The longest tooltip line of the data, technologies_9_tooltip_rows_1. */
#define TOOLTIP_LINE_CAP 7

/* The line tooltip_construct() builds from one item of its row. */
static layout_row_t *tooltip_line(const allocator_t *zone, const data_item_t *item) {
    layout_row_keys_t keys = {0};
    layout_row_cell_t s;
    layout_row_cell_t items[TOOLTIP_LINE_CAP];
    int i;

    if (item->text != NULL) {
        s = layout_row_cell_string(item->text);
        keys.items = &s;
        keys.count = 1;
        return layout_row_new(zone, &keys);
    }
    if (item->row == NULL) {
        return layout_row_new(zone, NULL);
    }
    assert(item->row->draw_function == DRAW_FUNCTION_NONE && "tooltip_line: a line's only named key is its color; a draw_function has no member -- stop, do not add one");
    assert(item->row->count <= TOOLTIP_LINE_CAP &&
           "tooltip_line: the longest tooltip line of the data (technologies_9_tooltip_rows_1) has 7 items; a longer one needs a bigger array");
    for (i = 0; i < item->row->count; i++) {
        items[i] = row_scalar_item(&item->row->items[i]);
    }
    keys.items = items;
    keys.count = item->row->count;
    keys.has_color = item->row->has_color;
    keys.color = p8_fixed_from_int(item->row->color);
    return layout_row_new(zone, &keys);
}

/* `rows` is allocated even for no line: tooltip_draw_at() draws the frame of
 * an empty tooltip, and skips one whose `rows` is NULL. */
static void tooltip_construct(tooltip_t *self, const data_row_t *row) {
    int n = row != NULL ? row->count : 0;
    int i;

    self->rows = (layout_row_t **)allocator_alloc(self->zone, (size_t)(n > 0 ? n : 1) * sizeof *self->rows);
    for (i = 0; i < n; i++) {
        self->rows[i] = tooltip_line(self->zone, &row->items[i]);
    }
    self->rows_count = n;
}

/* The line count is rows_count: tooltip_construct() makes exactly one line
 * per positional item of its row, so the two counts never differ. */
void tooltip_draw_at(p8_graphics_t *gfx, p8_vram_t *mem, const tooltip_t *self, p8_fixed_t om,
                  p8_fixed_t a) {
    p8_fixed_t h;
    p8_fixed_t dn;
    int i;

    if (self == NULL || self->rows == NULL) {
        return;
    }

    h = p8_fixed_add(p8_fixed_mul(p8_fixed_from_int(self->rows_count), p8_fixed_from_int(8)),
                         p8_fixed_from_int(2));
    om = p8_fixed_sub(om, p8_fixed_mul(h, a));
    dn = p8_fixed_add(om, h);

    p8_fillp(gfx, 0x0F0F);
    hud_draw_bar(gfx, mem, om, dn, 0x10);
    p8_fillp_reset(gfx);

    for (i = 0; i < self->rows_count; i++) {
        vec_t pos = vec_make(p8_fixed_from_int(64), p8_fixed_add(om, p8_fixed_from_int(2)));
        layout_row_draw_at(gfx, mem, self->rows[i], pos, P8_FIXED_LIT(0x00008000, 0.5));
        om = p8_fixed_add(om, p8_fixed_from_int(8));
    }
}

static void layout_row_draw_at_method(entity_t *self, vec_t p, p8_fixed_t a) {
    layout_row_draw_at(dispatch_context_gfx(), dispatch_context_mem(), (const layout_row_t *)self, p, a);
}

/* Only a planet's layout row is drawn through its class, never a tooltip; the position is ignored, the top read as 0. */
static void tooltip_draw_at_method(entity_t *self, vec_t p, p8_fixed_t a) {
    (void)p;
    tooltip_draw_at(dispatch_context_gfx(), dispatch_context_mem(), (const tooltip_t *)self, p8_fixed_from_int(0), a);
}

static const entity_class_t layout_row_class_def = {
    .draw_at = layout_row_draw_at_method,
};

/* A row with room for that many cells, members unset, not laid out. */
static layout_row_t *row_struct_alloc(const allocator_t *zone, int n) {
    layout_row_t *self;

    assert(zone != NULL && "row_struct_alloc: a row without a zone would have nowhere to keep its cells");
    self = (layout_row_t *)allocator_alloc(zone, sizeof *self);

    entity_init_header(&self->header, &layout_row_class_def);
    self->header.pos = &self->pos;
    self->color = 0;
    self->has_color = 0;
    self->background = 0;
    self->has_background = 0;
    self->background_is_bool = 0;
    self->score = 0;
    self->has_score = 0;
    self->pos = NULL;
    self->placed_items = NULL;
    self->placed_items_count = 0;
    self->width = 0;
    self->zone = zone;
    self->items = (layout_row_cell_t *)allocator_alloc(zone, (size_t)(n > 0 ? n : 1) * sizeof *self->items);
    self->items_count = n;
    return self;
}

static layout_row_t *row_struct_finish(layout_row_t *self) {
    layout_row_construct(self);
    return self;
}

static layout_row_t *row_create(const allocator_t *zone, const layout_row_keys_t *keys) {
    layout_row_t *self = row_struct_alloc(zone, keys != NULL ? keys->count : 0);
    int k;

    if (keys != NULL) {
        for (k = 0; k < keys->count; k++) {
            self->items[k] = row_own_cell(zone, keys->items[k]);
        }
        if (keys->has_color) {
            self->color = keys->color;
            self->has_color = 1;
        }
        if (keys->has_background) {
            self->background = keys->background;
            self->has_background = 1;
            self->background_is_bool = keys->background_is_bool;
        }
        if (keys->has_score) {
            self->score = keys->score;
            self->has_score = 1;
        }
        self->pos = keys->pos;
    }
    return row_struct_finish(self);
}

layout_row_keys_t layout_row_keys_of_row(const data_row_t *row, layout_row_cell_t *cells, int cap) {
    layout_row_keys_t keys = {0};
    int i;

    assert(row->count <= cap && row->draw_function == DRAW_FUNCTION_NONE && !row->has_resource &&
           "layout_row_keys_of_row: a row passes positional items and a color; a draw_function or a resource has no member -- stop, do not add one");
    for (i = 0; i < row->count; i++) {
        cells[i] = layout_row_cell_item(&row->items[i]);
    }
    keys.items = cells;
    keys.count = row->count;
    keys.has_color = row->has_color;
    keys.color = p8_fixed_from_int(row->color);
    return keys;
}

layout_row_t *layout_row_copy(const allocator_t *zone, const layout_row_t *row) {
    layout_row_t *self = row_struct_alloc(zone, row->items_count);
    int k;

    for (k = 0; k < row->items_count; k++) {
        self->items[k] = row_own_cell(zone, row->items[k]);
    }
    self->color = row->color;
    self->has_color = row->has_color;
    self->background = row->background;
    self->has_background = row->has_background;
    self->background_is_bool = row->background_is_bool;
    self->score = row->score;
    self->has_score = row->has_score;
    self->pos = row->pos;
    return row_struct_finish(self);
}

void layout_row_add(layout_row_t *self, layout_row_cell_t cell) {
    self->items = (layout_row_cell_t *)allocator_grow(self->zone, (unsigned char *)self->items,
                                                     (size_t)self->items_count * sizeof *self->items,
                                                     (size_t)(self->items_count + 1) * sizeof *self->items);
    self->items[self->items_count] = row_own_cell(self->zone, cell);
    self->items_count++;
}

static const entity_class_t tooltip_class_def = {
    .draw_at = tooltip_draw_at_method,
};

/* The tooltip itself, before its lines. */
static tooltip_t *tooltip_struct_alloc(const allocator_t *zone) {
    tooltip_t *self;

    assert(zone != NULL && "tooltip_struct_alloc: a tooltip without a zone would have nowhere to keep its lines");
    self = (tooltip_t *)allocator_alloc(zone, sizeof *self);

    entity_init_header(&self->header, &tooltip_class_def);
    self->zone = zone;
    self->rows = NULL;
    self->rows_count = 0;
    return self;
}

static tooltip_t *tooltip_create(const allocator_t *zone, const data_row_t *row) {
    tooltip_t *self;

    assert((row == NULL || (!row->has_color && row->draw_function == DRAW_FUNCTION_NONE)) &&
           "tooltip_create: every line is positional; a named key has no member -- stop, do not add one");
    self = tooltip_struct_alloc(zone);
    tooltip_construct(self, row);
    return self;
}

void layout_init(void) {
    layout_shutdown();
    g_layout_up = 1;
}

void layout_shutdown(void) {
    g_layout_up = 0;
}

int layout_row_ready(void) {
    return g_layout_up;
}

int tooltip_ready(void) {
    return g_layout_up;
}

const entity_class_t *layout_row_class(void) {
    return &layout_row_class_def;
}

const entity_class_t *tooltip_class(void) {
    return &tooltip_class_def;
}

layout_row_t *layout_row_new(const allocator_t *zone, const layout_row_keys_t *keys) {
    return row_create(zone, keys);
}

tooltip_t *tooltip_new(const allocator_t *zone, const data_row_t *row) {
    return tooltip_create(zone, row);
}
