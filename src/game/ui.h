#ifndef GAME_UI_H
#define GAME_UI_H

#include "game/background.h"
#include "engine/entity_list.h"
#include "engine/entity_id_list.h"
#include "game/game_data.h"
#include "engine/geom.h"
#include "p8/gfx.h"
#include "p8/vram.h"
#include "game/mouse.h"
#include "engine/entity_class.h"

enum { UI_HOVER_LAYERS = 16 };

typedef enum { MOUSE_SIDE_LEFT, MOUSE_SIDE_RIGHT } mouse_side_t;

typedef struct {
    int layer_count;
    entity_id_list_t layers[UI_HOVER_LAYERS];
} ui_hovered_by_layer_t;

#define UI_HAS_MOUSE_WORLD (1u << 0)
#define UI_HAS_HOVERED_BY_LAYER (1u << 1)
#define UI_HAS_OWN_EPHEMERALS (1u << 2)

typedef struct {
    entity_t header;
    const ui_archetype_t *archetype;
    uint16_t has;
    uint16_t ephemerals_serial;
    entity_id_list_t ephemerals;
    vec_t mouse_world;
    ui_hovered_by_layer_t hovered_by_layer;
    entity_id_t hovered;
    entity_id_t previous_hovered;
    entity_id_t left_held;
    entity_id_t right_held;
} ui_t;

void ui_init(void);

void ui_shutdown(void);

int ui_ready(void);

const entity_class_t *ui_class(void);

const box_t *ui_panel_box(void);

/* An empty list whose items live in a fresh ui-zone slot; ui_set_ephemerals
 * hands the slot to the ui. */
entity_id_list_t ui_ephemerals_new(void);

/* Takes the list's slot; a slot the ui owned before goes to the exit queue. */
void ui_set_ephemerals(ui_t *self, entity_id_list_t list);

ui_t *ui_new(void);

/* Each NULL when absent or once its entity was flushed. */
entity_t *ui_hovered(const ui_t *self);
entity_t *ui_previous_hovered(const ui_t *self);
entity_t *ui_left_held(const ui_t *self);
background_t *ui_right_held(const ui_t *self);

/* Empties the ui's own ephemerals list (kept, not replaced) and its hover
 * layers: their ids name entities of the world reset_world() abandons. */
void ui_forget_world(ui_t *self);

entity_t *main_mouse(void);

mouse_t *main_mouse_typed(void);

const entity_class_t *main_mouse_placeholder_class(void);

void ui_set_main_mouse(entity_t *instance);

ui_t *main_ui(void);

void ui_set_main_ui(ui_t *ui);

/* Called once per frame by `cart_update60()`. TODO: `mem` is no longer
 * read; drop it. */
void ui_update(p8_vram_t *mem, ui_t *self);

void ui_draw_overlay(const p8_graphics_t *gfx, p8_vram_t *mem, const ui_t *self);

void ui_update_hover(ui_t *self);

void ui_handle_button(ui_t *self, mouse_side_t side);

typedef enum { SEND_TO_HOVERED_LEFT_PRESS, SEND_TO_HOVERED_RIGHT_PRESS, SEND_TO_HOVERED_ACCEPT_DROP } send_to_hovered_slot_t;

int ui_send_to_hovered(ui_t *self, send_to_hovered_slot_t message, mouse_pos_t mouse, const entity_t *held,
                entity_t **out_target, down_t *out_down);

void ui_clear_ephemerals(ui_t *self);

#endif
