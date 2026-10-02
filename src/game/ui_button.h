#ifndef GAME_UI_BUTTON_H
#define GAME_UI_BUTTON_H

#include "game/game_data.h"
#include "p8/gfx.h"
#include "game/layout.h"
#include "game/map_tables.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "game/research.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "engine/allocator.h"

typedef struct button button_t;

/* `on_click` and `refresh` of a button: each is called with the button it
 * belongs to. */
typedef void (*button_fn_t)(button_t *self);

/* A button: every key it was built from, copied, then what button_construct
 * built from them. `on_click` and `refresh` are callbacks the keys carried;
 * `technology` and `project_planet`/`project`/`project_cost` are what the
 * `on_click` of the tech tree screen and of the projects panel read.
 * `pos` and `slide_target` are NULL (absent) or point at
 * `pos_box`/`slide_target_box`, copies of the values the keys carried. The
 * struct, its `hit_box`, `layout_row`, `side_tooltip` and `tooltip` live in
 * `slot`, given back by the exit queue once entity_unregister takes the
 * button out -- but for a `layout_row` a `refresh` rebuilt, which lives in
 * `layout_row_slot` and goes with the button. */
struct button {
    entity_t header;
    const allocator_t *slot;
    vec_t pos_box;
    vec_t slide_target_box;
    const button_archetype_t *archetype;
    p8_fixed_t layer;
    layout_row_t *layout_row;
    const allocator_t *layout_row_slot; /* NULL: `layout_row` is in `slot` */
    p8_fixed_t width;
    layout_row_t *side_tooltip;
    int has_side_tooltip; /* 0: absent; 1: present, and a NULL `side_tooltip` is false */
    tooltip_t *tooltip;
    const vec_t *pos;
    vec_t *slide_target;
    button_fn_t on_click;
    int has_on_click; /* 0: absent; 1: present, and a NULL `on_click` is false */
    int is_hovered;
    button_fn_t refresh;
    p8_fixed_t gpio_signal;
    int has_gpio_signal; /* 0: `gpio_signal` is absent, and is then never read */
    const char *name;
    p8_fixed_t trade;
    int has_trade; /* 0: `trade` is absent, and is then never read */
    p8_fixed_t type_bias;
    int has_type_bias; /* 0: `type_bias` is absent, and is then never read */
    const technologies_entry_t *technology; /* an entry of technologies_table */
    planet_t *project_planet;
    research_project_t *project;
    p8_fixed_t project_cost;
    int has_project_cost; /* 0: `project_cost` is absent, and is then never read */
};

void button_init(void);
void button_shutdown(void);

/* 1 from button_init() to button_shutdown(), 0 otherwise. */
int button_ready(void);
const entity_class_t *button_class(void);

/* The keys of a button, each typed as the member it fills (NULL or an
 * unset flag: absent). The rows `layout_row_keys`, `side_tooltip_row`,
 * `tooltip_row` and the layout row `layout_row_copy` are read during the
 * construction, not kept. The button's `layout_row`: built from
 * `layout_row_keys`, else a copy of `layout_row_copy`, else an empty row. */
typedef struct {
    int has_layer; /* 0: `layer` keeps the archetype's */
    p8_fixed_t layer;
    const layout_row_t *layout_row_copy;
    const layout_row_keys_t *layout_row_keys;
    int has_side_tooltip; /* present without `side_tooltip_row` nor `side_tooltip_is_false`: an empty row (TROU-23) */
    int side_tooltip_is_false;
    const layout_row_keys_t *side_tooltip_row;
    int has_tooltip; /* present without `tooltip_row`: an empty tooltip (TROU-23) */
    const data_row_t *tooltip_row;
    const vec_t *pos;
    const vec_t *slide_target;
    int has_on_click; /* present, and a NULL `on_click` is false */
    button_fn_t on_click;
    button_fn_t refresh;
    int has_gpio_signal;
    p8_fixed_t gpio_signal;
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

/* `keys` is read, not kept (NULL: none). The struct registers itself
 * through entity_register. */
button_t *button_new(const button_keys_t *keys);

/* `keys` is read, not kept (NULL: none). */
void button_construct(button_t *self, const button_keys_t *keys);

void button_update(button_t *self);

void button_draw_overlay(p8_graphics_t *gfx, p8_memory_t *mem, const button_t *self);

void button_draw(p8_graphics_t *graphics, p8_memory_t *memory, const button_t *self, vec_t draw_pos);

/* Always stops the dispatch: the click is consumed whether or not an
 * `on_click` callback was there to run. */
down_t button_left_press(button_t *self);

typedef struct modal_backdrop modal_backdrop_t;

/* modal_backdrop_left_press keeps whether it returns non-zero as `is_dead`. */
typedef int (*modal_backdrop_on_click_fn_t)(modal_backdrop_t *self);

/* `on_click` is the callback the keys carried. The struct lives in `slot`,
 * given back by the exit queue once entity_unregister takes the backdrop
 * out. */
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

/* 1 from modal_backdrop_init() to modal_backdrop_shutdown(), 0 otherwise. */
int modal_backdrop_ready(void);
const entity_class_t *modal_backdrop_class(void);

/* The key `on_click` of a modal backdrop: NULL is absent. */
typedef struct {
    modal_backdrop_on_click_fn_t on_click;
} modal_backdrop_keys_t;

/* `keys` is read, not kept (NULL: none). The struct registers itself
 * through entity_register. */
modal_backdrop_t *modal_backdrop_new(const modal_backdrop_keys_t *keys);

down_t modal_backdrop_left_press(modal_backdrop_t *self);

#endif
