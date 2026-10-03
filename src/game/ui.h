#ifndef GAME_UI_H
#define GAME_UI_H

#include "game/background.h"
#include "engine/entity_list.h"
#include "game/game_data.h"
#include "engine/geom.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/mouse.h"
#include "engine/entity_class.h"

/* ui_update, ui_draw_overlay, ui_update_hover, ui_handle_button,
 * ui_send_to_hovered and ui_clear_ephemerals return early on a NULL
 * `ui_t`; ui_set_ephemerals does not. An absent main mouse or camera reads
 * as the position (0, 0). */

enum { UI_HOVER_LAYERS = 16 };

/* Not a P8_MOUSE_* bit: the right side reads the right or middle button. */
typedef enum { MOUSE_SIDE_LEFT, MOUSE_SIDE_RIGHT } mouse_side_t;

/* Each of `layers` lists the hovered entities of its layer in the order
 * they were found; `layer_count` is always UI_HOVER_LAYERS. */
typedef struct {
    int layer_count;
    entity_list_t layers[UI_HOVER_LAYERS];
} ui_hovered_by_layer_t;

/* `ephemerals` of a new `ui_t` is `ui_ephemerals_shared()`, until
 * ui_clear_ephemerals or a button list replaces it with a list from
 * `ui_ephemerals_new()`, given back when it is replaced in turn or when its
 * `ui_t` is released (from `reset_world()`). `hovered_by_layer` is NULL
 * until the first ui_update_hover, then points at `hovered_by_layer_own`,
 * whose layers every ui_update_hover remakes in the frame zone.
 * `mouse_world` is NULL until the first ui_update, then points at
 * `mouse_world_own`, rewritten in place by each ui_update. Never copy the
 * struct. */
typedef struct {
    entity_t header;
    const ui_archetype_t *archetype;
    entity_list_t *ephemerals;
    vec_t *mouse_world;
    vec_t mouse_world_own;
    ui_hovered_by_layer_t *hovered_by_layer;
    ui_hovered_by_layer_t hovered_by_layer_own;
    entity_t *hovered; /* borrowed; NULL once the exit queue gives it back */
    entity_t *previous_hovered; /* borrowed; NULL once the exit queue gives it back */
    entity_t *left_held; /* borrowed, never freed */
    background_t *right_held; /* borrowed, never freed */
} ui_t;

/* Empties `ui_ephemerals_shared()`. Also
 * puts back the member-less placeholder main mouse that
 * `ui_set_main_mouse()` later replaces. */
void ui_init(void);

/* Forgets `main_ui()` and `main_mouse()` without reading them: the zone
 * reset that precedes this call has already taken them, and the exit queue
 * gave back the ephemerals of `main_ui()` before it. The shared ephemerals give
 * their items back to the loading zone. Idempotent, and safe before the
 * first `ui_init()`. */
void ui_shutdown(void);

/* 1 from ui_init() to ui_shutdown(), 0 otherwise. */
int ui_ready(void);

const entity_class_t *ui_class(void);

/* The box of the class, read by targeting_track: `&ui_archetype.panel_box`,
 * NULL before `ui_init()` and after `ui_shutdown()`. */
const box_t *ui_panel_box(void);

/* The empty container every new `ui_t` starts with as `ephemerals`:
 * static, never freed, emptied (not freed) by each `ui_init()`. */
entity_list_t *ui_ephemerals_shared(void);

/* An empty list in an ephemerals slot of its own, its items grown there. */
entity_list_t *ui_ephemerals_new(void);

/* A list from ui_ephemerals_new() becomes `self->ephemerals`; the list it
 * replaces goes to the exit queue (the frame still compares it in
 * ui_update), unless it is the shared one. */
void ui_set_ephemerals(ui_t *self, entity_list_t *le);

/* The `ui_t` registers itself through entity_register into the live
 * entities and the draw_overlay message, under no tag; it lives in the
 * current game-zone generation. Asserts before the init. */
ui_t *ui_new(void);

/* Between `ui_init()` and `ui_set_main_mouse()` this is the placeholder
 * struct, with no member and no method; after `reset_world()` it is a
 * `mouse_t` (game/mouse.h). Its members are read through
 * `main_mouse_typed()`. */
entity_t *main_mouse(void);

/* The main mouse as a `mouse_t`: NULL when there is none and on the
 * placeholder. */
mouse_t *main_mouse_typed(void);

const entity_class_t *main_mouse_placeholder_class(void);

/* Makes `instance` the new main mouse. The outgoing instance is NOT freed
 * -- see the comment at the definition. NULL goes back to no main mouse,
 * and passing back the current `main_mouse()` is a no-op. */
void ui_set_main_mouse(entity_t *instance);

/* NULL until `reset_world()` (engine/bootstrap.c) calls `ui_set_main_ui()`,
 * and again after `ui_shutdown()`. */
ui_t *main_ui(void);

/* Same ownership rule as `ui_set_main_mouse()`. */
void ui_set_main_ui(ui_t *ui);

/* Called once per frame by `cart_update60()`. TODO: `mem` is no longer
 * read; drop it with the typed PICO-8 memory. */
void ui_update(p8_memory_t *mem, ui_t *self);

void ui_draw_overlay(const p8_graphics_t *gfx, p8_memory_t *mem, const ui_t *self);

void ui_update_hover(ui_t *self);

/* The side picks the member `left_held`/`right_held` and the method
 * `left_press`/`right_press`. */
void ui_handle_button(ui_t *self, mouse_side_t side);

/* The method ui_send_to_hovered asks each hovered entity: `left_press`
 * and `right_press` get the world and screen mouse positions, `accept_drop`
 * gets the held entity. */
typedef enum { SEND_TO_HOVERED_LEFT_PRESS, SEND_TO_HOVERED_RIGHT_PRESS, SEND_TO_HOVERED_ACCEPT_DROP } send_to_hovered_slot_t;

/* Returns 1 and fills `*out_target` and `*out_down` on the first answer
 * whose `stop` is set, or 0 leaving both untouched when nothing answers. */
int ui_send_to_hovered(ui_t *self, send_to_hovered_slot_t message, vec_t *mouse_world, vec_t *mouse_screen, const entity_t *held,
                entity_t **out_target, down_t *out_down);

void ui_clear_ephemerals(ui_t *self);

#endif
