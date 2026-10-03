#ifndef GAME_SCORE_PANEL_H
#define GAME_SCORE_PANEL_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/map_tables.h"
#include "game/message_box.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* score_panel_draw() on a NULL panel still draws the rows and skips only
 * the curtain. */

/* A NULL last_box reads as the class's default, the number 1, never as
 * absent. */
typedef struct {
    entity_t header;
    const score_panel_archetype_t *archetype;
    p8_fixed_t layer;
    message_box_t *last_box;
    int is_shown;
    int has_is_shown;
    p8_fixed_t curtain;
    int has_curtain;
} score_panel_t;

/* The tech-tree screen's layout block: no parent, never an entity, and no
 * writer. Trap: description is a frame-zone text, dangling after the next
 * reset; only menu_show_tech_tree_screen(), which has just written it,
 * reads it. */
typedef struct {
    entity_t header;
    const vec_t *fixed_pos;
    const vec_t *row_step;
    p8_fixed_t back_color;
    const char *description;
} score_panel_layout_block_t;

/* The longest text refresh or new_year writes: "year " and a year, or a
 * formatted price or number with its sign, its four decimals and a suffix. */
enum { SCORE_PANEL_TEXT_CAP = 16 };

/* The readouts: a copy of score_panel_readouts_row whose sub-rows refresh
 * rewrites (item 1 of each), each an article of draw_functions_draw_rows();
 * a text refresh writes is a copy in its entry of texts. */
typedef struct {
    data_row_t row;
    data_item_t items[7];
    data_row_t subs[7];
    data_item_t sub_items[7][4];
    char texts[7][SCORE_PANEL_TEXT_CAP + 1];
} score_panel_readouts_t;

/* The year banner: the row field holds its first article, its second (a
 * copy of its generated row whose item 1 new_year rewrites, to a copy in
 * year) and its third when set (a countdown row); keep_open NULL for none. */
typedef struct {
    data_row_t row;
    data_item_t items[3];
    data_row_t second;
    data_item_t second_items[5];
    char year[SCORE_PANEL_TEXT_CAP + 1];
    message_box_keep_open_fn_t keep_open;
} score_panel_year_banner_t;

/* A score row, a copy of its entry of score_rows_table. Without a score it
 * is an article of draw_functions_draw_rows() (the plain row, never
 * rewritten); with one, score_panel_update() rewrites its positions,
 * has_value saying which hold a value, and makes it a line (pos its
 * position). Trap: the texts it writes there are frame-zone texts, dangling
 * after the next reset; only score_panel_update(), which has just written
 * them, reads them. */
typedef struct {
    data_row_t row;
    data_item_t items[5];
    int has_value[5];
    vec_t pos;
    const score_rows_entry_t *entry;
} score_row_t;

/* Calls draw_functions_init() first. Resets the score rows, the readouts
 * and the year banner to their generated rows. */
void score_panel_init(void);

/* Resets the main score panel to NULL without freeing the instance it
 * named: it is a bookmark, the live entities own the entity. */
void score_panel_shutdown(void);

/* 1 from score_panel_init() to score_panel_shutdown(), 0 otherwise. */
int score_panel_ready(void);
const entity_class_t *score_panel_class(void);

/* Shared by every panel, past and future: refresh rewrites the readouts,
 * new_year the year banner, in place; both are statics of this module. The
 * layout block is the one static score_panel_layout_block_t, never freed,
 * whose description menu_show_tech_tree_screen() rewrites and the init
 * resets to NULL. All NULL before score_panel_init() and after the
 * shutdown. */
score_panel_readouts_t *score_panel_readouts(void);
score_panel_year_banner_t *score_panel_year_banner(void);
score_panel_layout_block_t *score_panel_layout_block(void);
const entity_class_t *score_panel_layout_block_class(void);
/* The archetype's frame rows and countdown rows: never NULL. */
const data_row_t *score_panel_frame_rows(void);
const data_row_t *score_panel_countdown_rows(void);

/* The panel registers itself through entity_register() into the live
 * entities and the draw, new_year and refresh message lists, under no tag;
 * it lives in the game-zone generation cart_start_game() opens. Asserts
 * when called before the init. */
score_panel_t *score_panel_new(void);

/* The SCORE_ROWS_COUNT score rows, statics of this module: every
 * score_panel_update() run rewrites them in place. NULL before the init and
 * after the shutdown. */
score_row_t *score_panel_rows(void);

/* Never NULL: an inert placeholder until the real init bridge
 * (cart_init_method) is installed, which a NULL argument resets. A button
 * built earlier keeps whichever value it captured then. */
button_fn_t score_panel_init_field_value(void);
void score_panel_set_init(button_fn_t fn);

/* NULL until cart_start_game() builds the one panel instance and sets it
 * here. The setter takes no ownership. */
score_panel_t *main_score_panel(void);
void score_panel_set_main(score_panel_t *instance);

void score_panel_refresh(void);

void score_panel_draw(p8_graphics_t *gfx, p8_memory_t *mem, score_panel_t *self);

void score_panel_refresh_palette(void);

void score_panel_new_year_method(entity_t *self, p8_fixed_t yr);
void score_panel_update_method(entity_t *self, p8_fixed_t t);

void score_panel_display_init(void);
void score_panel_display_shutdown(void);

#endif
