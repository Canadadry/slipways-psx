#ifndef GAME_MESSAGE_BOX_H
#define GAME_MESSAGE_BOX_H

#include "engine/entity_class.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/vram.h"
#include "game/ui_button.h"
#include "engine/vec.h"

typedef struct message_box message_box_t;

typedef int (*message_box_keep_open_fn_t)(message_box_t *self);

enum { MESSAGE_BOX_ITEMS_CAP = SCORE_ROWS_COUNT };

struct message_box {
    entity_t header;
    const allocator_t *slot;
    const message_box_archetype_t *archetype;
    p8_fixed_t duration_scale;
    message_box_keep_open_fn_t keep_open;
    int is_dead;
    const data_row_t *items[MESSAGE_BOX_ITEMS_CAP];
    int items_count;
};

message_box_t *shown_message_box(void);
void message_box_set_shown(message_box_t *mh);

void message_box_init(void);

void message_box_shutdown(void);

int message_box_ready(void);

typedef struct {
    const data_row_t *const *items;
    int count;
    int has_duration_scale;
    p8_fixed_t duration_scale;
    message_box_keep_open_fn_t keep_open;
} message_box_keys_t;

message_box_t *message_box_new(const message_box_keys_t *keys);

void message_box_construct(message_box_t *self);

void message_box_draw_overlay(p8_graphics_t *gfx, p8_vram_t *mem, message_box_t *self);

void toast_init(void);

enum { TOAST_TEXT_CAP = 16 };

typedef struct {
    entity_t header;
    const allocator_t *slot;
    vec_t pos_box;
    char text_buf[TOAST_TEXT_CAP + 1];
    const toast_archetype_t *archetype;
    p8_fixed_t fade;
    const vec_t *pos;
    int is_dead;
    const char *text;
    p8_fixed_t color;
} toast_t;

void toast_shutdown(void);

int toast_ready(void);

typedef struct {
    const vec_t *pos;
    const char *text;
    p8_fixed_t color;
} toast_keys_t;

toast_t *toast_new(const toast_keys_t *keys);

void toast_construct(toast_t *self);
void toast_update(toast_t *self);

void toast_draw(const p8_graphics_t *gfx, p8_vram_t *mem, const toast_t *self, vec_t p);

typedef struct {
    const char *description;
    const vec_t *fixed_pos;
    int clear_ephemerals_after;
    int has_back_color;
    p8_fixed_t back_color;
    const vec_t *row_step;
    const vec_t *pos;
} open_box_frame_t;

void message_box_open_box(const open_box_frame_t *frame, const button_keys_t *buttons, int button_count);

#endif
