#ifndef GAME_UI_BUTTON_H
#define GAME_UI_BUTTON_H

#include "game/game_data.h"
#include "p8/gfx.h"
#include "game/layout.h"
#include "game/map_tables.h"
#include "p8/vram.h"
#include "game/planet.h"
#include "game/research.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "engine/allocator.h"

typedef struct button button_t;

typedef void (*button_fn_t)(button_t *self);

struct button {
    entity_t header;
    const allocator_t *slot;
    vec_t pos_box;
    vec_t slide_target_box;
    const button_archetype_t *archetype;
    p8_fixed_t layer;
    layout_row_t *layout_row;
    const allocator_t *layout_row_slot;
    p8_fixed_t width;
    layout_row_t *side_tooltip;
    int has_side_tooltip;
    tooltip_t *tooltip;
    const vec_t *pos;
    vec_t *slide_target;
    button_fn_t on_click;
    int has_on_click;
    int is_hovered;
    button_fn_t refresh;
    const char *name;
    p8_fixed_t trade;
    int has_trade;
    p8_fixed_t type_bias;
    int has_type_bias;
    const technologies_entry_t *technology;
    planet_t *project_planet;
    research_project_t *project;
    p8_fixed_t project_cost;
    int has_project_cost;
};

void button_init(void);
void button_shutdown(void);

int button_ready(void);
const entity_class_t *button_class(void);

typedef struct {
    int has_layer;
    p8_fixed_t layer;
    const layout_row_t *layout_row_copy;
    const layout_row_keys_t *layout_row_keys;
    /* GAP-23 */
    int has_side_tooltip;
    int side_tooltip_is_false;
    const layout_row_keys_t *side_tooltip_row;
    /* GAP-23 */
    int has_tooltip;
    const data_row_t *tooltip_row;
    const vec_t *pos;
    const vec_t *slide_target;
    int has_on_click;
    button_fn_t on_click;
    button_fn_t refresh;
    const char *name;
    int has_trade;
    p8_fixed_t trade;
    int has_type_bias;
    p8_fixed_t type_bias;
    const technologies_entry_t *technology;
    planet_t *project_planet;
    research_project_t *project;
    int has_project_cost;
    p8_fixed_t project_cost;
} button_keys_t;

button_t *button_new(const button_keys_t *keys);

void button_construct(button_t *self, const button_keys_t *keys);

void button_update(button_t *self);

void button_draw_overlay(p8_graphics_t *gfx, p8_vram_t *mem, const button_t *self);

void button_draw(p8_graphics_t *graphics, p8_vram_t *memory, const button_t *self, vec_t draw_pos);

down_t button_left_press(button_t *self);

typedef struct modal_backdrop modal_backdrop_t;

typedef int (*modal_backdrop_on_click_fn_t)(modal_backdrop_t *self);

struct modal_backdrop {
    entity_t header;
    const allocator_t *slot;
    const modal_backdrop_archetype_t *archetype;
    modal_backdrop_on_click_fn_t on_click;
    int is_dead;
    int is_hovered;
};

void modal_backdrop_init(void);
void modal_backdrop_shutdown(void);

int modal_backdrop_ready(void);
const entity_class_t *modal_backdrop_class(void);

typedef struct {
    modal_backdrop_on_click_fn_t on_click;
} modal_backdrop_keys_t;

modal_backdrop_t *modal_backdrop_new(const modal_backdrop_keys_t *keys);

down_t modal_backdrop_left_press(modal_backdrop_t *self);

#endif
