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

/* `js.my`, `js.hb`: the cart calls them as `self:my()`, `self:hb()`, and
 * drops what they return. */
typedef void (*button_fn_t)(button_t *self);

/* A button: every key of its construction table, copied (`js(b)`), then
 * what `js:pq` built from them. `my` and `hb` are closures the table
 * carried, called as fields; `_cr_t` and `_jo_fe_pt`/`_b`/`_od` are what
 * the `my` closures of `cr` and `jo:fe` captured, not cart fields. `ra` and
 * `qf` are NULL (nil) or point at `ra_box`/`qf_box`, copies of the values the
 * keys carried. The struct, its `kq`, `lq`, `h_` and `jb` live in `place`,
 * given back by the exit queue once `he` takes the button out -- but for an
 * `lq` a `hb` rebuilt, which lives in `lq_place` and goes with the button. */
struct button {
    entity_t header;
    const allocator_t *slot;
    vec_t pos_box;
    vec_t slide_target_box;
    const button_archetype_t *archetype;
    p8_fixed_t layer;
    layout_row_t *layout_row;
    const allocator_t *layout_row_slot; /* NULL: `lq` is in `place` */
    p8_fixed_t width;
    layout_row_t *side_tooltip;
    int has_side_tooltip; /* 0: nil; 1: present, and a NULL `h_` is `false` */
    tooltip_t *tooltip;
    const vec_t *pos;
    vec_t *slide_target;
    button_fn_t on_click;
    int has_on_click; /* 0: nil; 1: present, and a NULL `my` is `false` */
    int is_hovered;
    button_fn_t refresh;
    p8_fixed_t gpio_signal;
    int has_gpio_signal; /* 0: `fg` is nil, and is then never read */
    const char *name;
    p8_fixed_t trade;
    int has_trade; /* 0: `trade` is nil, and is then never read */
    p8_fixed_t type_bias;
    int has_type_bias; /* 0: `en` is nil, and is then never read */
    const technologies_entry_t *technology; /* an entry of pico8_nb_table */
    planet_t *project_planet;
    research_project_t *project;
    p8_fixed_t project_cost;
    int has_project_cost; /* 0: `_jo_fe_od` is nil, and is then never read */
};

void button_init(void);
void button_shutdown(void);

/* 1 from pico8_js_init() to pico8_js_shutdown(), 0 otherwise. */
int button_ready(void);
const entity_class_t *button_class(void);

/* The cart's keys of a button, each typed as the member it fills (NULL
 * or an unset flag: absent). The rows `lq_row`, `h_row`, `jb_row` and the
 * `lq` `lq_copy` are read during the construction, not kept. `js.lq`: the
 * row `lq_row`, else a copy of `lq_copy`, else an empty row. */
typedef struct {
    int has_layer; /* 0: `fa` keeps the archetype's */
    p8_fixed_t layer;
    const layout_row_t *layout_row_copy;
    const layout_row_keys_t *layout_row_keys;
    int has_side_tooltip; /* present without `h_row` nor `h_is_false`: an empty row (TROU-23) */
    int side_tooltip_is_false;
    const layout_row_keys_t *side_tooltip_row;
    int has_tooltip; /* present without `jb_row`: an empty `jb` (TROU-23) */
    const data_row_t *tooltip_row;
    const vec_t *pos;
    const vec_t *slide_target;
    int has_on_click; /* present, and a NULL `my` is `false` */
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
 * through `lt`. */
button_t *button_new(const button_keys_t *keys);

/* `keys` is read, not kept (NULL: none). */
void button_construct(button_t *self, const button_keys_t *keys);

void button_update(button_t *self);

void button_draw_overlay(p8_graphics_t *gfx, p8_memory_t *mem, const button_t *self);

void button_draw(p8_graphics_t *graphics, p8_memory_t *memory, const button_t *self, vec_t draw_pos);

/* Always returns true: the click is consumed whether or not a `my` callback
 * was there to run. */
down_t button_left_press(button_t *self);

typedef struct modal_backdrop modal_backdrop_t;

/* `fq.my`: the Lua truth of what it returns, which `fq:ldown` keeps as `p_`. */
typedef int (*modal_backdrop_on_click_fn_t)(modal_backdrop_t *self);

/* `my` is the closure the construction table carried, which `mv` calls as
 * a field, not a method. The struct lives in `place`, given back by the
 * exit queue once `he` takes the veil out. */
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

/* 1 from pico8_fq_init() to pico8_fq_shutdown(), 0 otherwise. */
int modal_backdrop_ready(void);
const entity_class_t *modal_backdrop_class(void);

/* The cart's key `my` of a veil: NULL is nil. */
typedef struct {
    modal_backdrop_on_click_fn_t on_click;
} modal_backdrop_keys_t;

/* `keys` is read, not kept (NULL: none). The struct registers itself
 * through `lt`. */
modal_backdrop_t *modal_backdrop_new(const modal_backdrop_keys_t *keys);

down_t modal_backdrop_left_press(modal_backdrop_t *self);

#endif
