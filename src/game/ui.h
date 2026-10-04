#ifndef GAME_UI_H
#define GAME_UI_H

#include "game/background.h"
#include "engine/entity_list.h"
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
    entity_list_t layers[UI_HOVER_LAYERS];
} ui_hovered_by_layer_t;

typedef struct {
    entity_t header;
    const ui_archetype_t *archetype;
    entity_list_t *ephemerals;
    vec_t *mouse_world;
    vec_t mouse_world_own;
    ui_hovered_by_layer_t *hovered_by_layer;
    ui_hovered_by_layer_t hovered_by_layer_own;
    entity_t *hovered;
    entity_t *previous_hovered;
    entity_t *left_held;
    background_t *right_held;
} ui_t;

void ui_init(void);

void ui_shutdown(void);

int ui_ready(void);

const entity_class_t *ui_class(void);

const box_t *ui_panel_box(void);

entity_list_t *ui_ephemerals_shared(void);

entity_list_t *ui_ephemerals_new(void);

void ui_set_ephemerals(ui_t *self, entity_list_t *le);

ui_t *ui_new(void);

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

int ui_send_to_hovered(ui_t *self, send_to_hovered_slot_t message, vec_t *mouse_world, vec_t *mouse_screen, const entity_t *held,
                entity_t **out_target, down_t *out_down);

void ui_clear_ephemerals(ui_t *self);

#endif
