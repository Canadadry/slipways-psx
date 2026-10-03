#ifndef GAME_MESSAGE_BOX_H
#define GAME_MESSAGE_BOX_H

#include "engine/entity_class.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/ui_button.h"
#include "engine/vec.h"

/* message_box_construct(), message_box_draw_overlay(), toast_construct(),
 * toast_update() and toast_draw() return at once on a NULL instance. */

typedef struct message_box message_box_t;

/* keep_open: non zero holds the box open, tested once its time is up. */
typedef int (*message_box_keep_open_fn_t)(message_box_t *self);

/* The game builds a box at two sites: score_panel_new_year() (up to 3
 * articles) and score_panel_update() (one per score row). */
enum { MESSAGE_BOX_ITEMS_CAP = SCORE_ROWS_COUNT };

/* items are the box's rows, in the order the keys gave them, which the
 * overlay draw lays out through draw_functions_draw_rows(). A draw_row
 * article is a copy in the slot, its layout row and vector included; any
 * other stays shared with the caller (the year banner, the score rows).
 * The struct lives in its slot, given back by the exit queue once
 * entity_unregister() takes the box out. */
struct message_box {
    entity_t header;
    const allocator_t *slot;
    const message_box_archetype_t *archetype;
    p8_fixed_t duration_scale;
    message_box_keep_open_fn_t keep_open; /* NULL: none */
    int is_dead;
    const data_row_t *items[MESSAGE_BOX_ITEMS_CAP];
    int items_count;
};

/* The message box currently open, or NULL for none. The setter never frees
 * the box it replaces: it borrows a struct that entity_register() filed in
 * the live entities, and the exit queue resets it to NULL as it gives that
 * struct's slot back. */
message_box_t *shown_message_box(void);
void message_box_set_shown(message_box_t *mh);

void message_box_init(void);

/* Clears the shown message box, never frees any box. Idempotent, and safe
 * before the first message_box_init(). */
void message_box_shutdown(void);

/* 1 from message_box_init() to message_box_shutdown(), 0 otherwise. */
int message_box_ready(void);

/* The keys of a message box: items its rows in order, each an article of
 * draw_functions_draw_rows() (a draw_row article copied, any other borrowed
 * for the box's life), duration_scale under its flag, keep_open NULL for
 * none. */
typedef struct {
    const data_row_t *const *items;
    int count;
    int has_duration_scale; /* 0: the archetype's is kept */
    p8_fixed_t duration_scale;
    message_box_keep_open_fn_t keep_open;
} message_box_keys_t;

/* The keys are read, not kept (NULL: none), but for the articles they lend.
 * The struct registers itself through entity_construct(). */
message_box_t *message_box_new(const message_box_keys_t *keys);

void message_box_construct(message_box_t *self);

/* NULL graphics or memory skips the drawing and the camera reset only; the
 * closing timer and the shown-box bookkeeping still run. */
void message_box_draw_overlay(p8_graphics_t *gfx, p8_memory_t *mem, message_box_t *self);

void toast_init(void);

enum { TOAST_TEXT_CAP = 16 };

/* text is NULL for none, else it points at text_buf, a copy of the key's
 * text; color is the forced color, 0 when the key gives none. pos
 * deliberately stays the key's vector, borrowed, until the first
 * toast_update(): planet_new_year() passes its planet's position, which
 * planet_bob() moves in place before that update, and the toast reads it
 * moved. From then on, and when toast_construct() places the toast itself,
 * pos points at pos_box. The struct lives in its slot, given back by the
 * exit queue once entity_unregister() takes the toast out. */
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

/* 1 from toast_init() to toast_shutdown(), 0 otherwise. */
int toast_ready(void);

/* The keys of a toast: its position, text and forced color. NULL or 0 is an
 * absent key, which leaves the member's default. */
typedef struct {
    const vec_t *pos;
    const char *text;
    p8_fixed_t color;
} toast_keys_t;

/* The keys are read, not kept (NULL: none), but for the position, borrowed
 * until the first toast_update(); the text is copied. The struct registers
 * itself through entity_construct(). */
toast_t *toast_new(const toast_keys_t *keys);

void toast_construct(toast_t *self);
void toast_update(toast_t *self);

/* NULL graphics or memory makes the whole call a no-op. */
void toast_draw(const p8_graphics_t *gfx, p8_memory_t *mem, const toast_t *self, vec_t p);

/* What message_box_open_box() reads of its frame, filled by the caller from
 * its own members: a NULL pointer or a 0 has_* is absent, and
 * clear_ephemerals_after is non zero for true. The frame is read during the
 * call and none of it is kept. */
typedef struct {
    const char *description;
    const vec_t *fixed_pos;
    int clear_ephemerals_after;
    int has_back_color;
    p8_fixed_t back_color;
    const vec_t *row_step;
    const vec_t *pos;
} open_box_frame_t;

/* The button rows are the keys of main_ui()'s ephemeral buttons, each read
 * on a copy that gets the box position, a slide_target and the layer; the
 * rows stay the caller's, unchanged. The ephemerals they replace go to the
 * exit queue. A NULL frame is a no-op. */
void message_box_open_box(const open_box_frame_t *frame, const button_keys_t *buttons, int button_count);

#endif
