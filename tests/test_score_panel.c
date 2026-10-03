/* The score panel, built by its only road (cart_start_game(),
 * score_panel_new(), on a game started directly: no events file), read on
 * its members and on the class's shared readouts, year banner and layout
 * block, its defaults read on its archetype, and driven through the real
 * on_click, menu_show_tech_tree_screen() and panel toggle. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "p8/assets.h"
#include "game/cart.h"
#include "engine/dispatch_context.h"
#include "game/economy_score.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/input.h"
#include "game/layout.h"
#include "p8/mem.h"
#include "game/menu.h"
#include "game/message_box.h"
#include "game/score_panel.h"
#include "game/score_panel_display.h"
#include "engine/entity_class.h"
#include "game/ui.h"
#include "game/ui_button.h"
#include "engine/vec.h"

#define BUILT_HASH 0xC419CD23u
#define SHARED_HASH 0x1F82BA04u
#define LIFE_HASH 0xDDBCC765u
#define TECH_TREE_SCREEN_HASH 0x7D1C5B1Fu

static p8_memory_t memory;
static p8_graphics_t graphics;
static p8_input_t input;

static const tag_t tags[] = {TAG_PLANET, TAG_PROBE, TAG_STRUCTURE, TAG_SLIPWAY, TAG_WORMHOLE, TAG_MESSAGE_BOX, TAG_NODE, TAG_ASCENSION, TAG_TARGETING, TAG_SLIPGATE};

static uint32_t fnv(uint32_t h, uint32_t word) {
    int i;
    for (i = 0; i < 4; i++) {
        h ^= (word >> (8 * i)) & 0xFFu;
        h *= 16777619u;
    }
    return h;
}

static uint32_t fnv_str(uint32_t h, const char *s) {
    for (; *s != '\0'; s++) {
        h = fnv(h, (uint32_t)(unsigned char)*s);
    }
    return fnv(h, 0xFFu);
}

static int entity_list_rank_in(const entity_list_t *list, const void *e) {
    int i;
    for (i = 0; list != NULL && i < list->count; i++) {
        if (list->items[i] == e) {
            return i;
        }
    }
    return -1;
}

static int entity_list_listed(const entity_list_t *list, const void *e) {
    return entity_list_rank_in(list, e) >= 0;
}

static int entity_list_count_of(const entity_list_t *list) {
    return list == NULL ? 0 : list->count;
}

/* 0 absent, 1 false, 2 true. */
static uint32_t is_shown_state(const score_panel_t *pj) {
    if (!pj->has_is_shown) {
        return 0;
    }
    return pj->is_shown ? 2 : 1;
}

/* A NULL last_box reads as the class's default, the number 1. */
static p8_fixed_t last_box_fixed(const score_panel_t *pj) {
    assert(pj->last_box == NULL);
    return p8_fixed_from_int(1);
}

static int is_layout_row(const void *e) {
    return e != NULL &&
           ((const entity_t *)e)->class == layout_row_class();
}

static int row_count(const void *lq) {
    return ((const layout_row_t *)lq)->items_count;
}

static layout_row_cell_t row_item(const void *lq, int i) {
    assert(i >= 0 && i < row_count(lq));
    return ((const layout_row_t *)lq)->items[i];
}

/* The words the frozen hashes fold before a value: 0xF1 a number, 0xB0 a
 * boolean (then 0 or 1), 0x57 a string; any other moves every *_HASH. */
static uint32_t fold_fixed(uint32_t h, p8_fixed_t x) {
    return fnv(fnv(h, 0xF1u), (uint32_t)x);
}

static uint32_t fold_bool(uint32_t h, int b) {
    return fnv(fnv(h, 0xB0u), b ? 1u : 0u);
}

static uint32_t fold_string(uint32_t h, const char *s) {
    return fnv_str(fnv(h, 0x57u), s);
}

static uint32_t fold_ref(uint32_t h, const void *ref);
static uint32_t fold_row(uint32_t h, const data_row_t *row);

static uint32_t fold_item(uint32_t h, const data_item_t *item) {
    if (item->text != NULL) {
        return fold_string(h, item->text);
    }
    if (item->is_false) {
        return fold_bool(h, 0);
    }
    if (item->is_ref) {
        return fold_ref(h, item->ref);
    }
    return fold_fixed(h, item->is_fixed ? item->fixed : p8_fixed_from_int(item->number));
}

/* The names the frozen hashes folded, before draw functions were an enum. */
static const char *const draw_function_names[DRAW_FUNCTION_COUNT] = {
    [DRAW_FUNCTION_DRAW_BAR] = "draw_bar",
    [DRAW_FUNCTION_PRINT_OUTLINED] = "print_outlined",
    [DRAW_FUNCTION_DRAW_HELP_FRAME] = "draw_help_frame",
    [DRAW_FUNCTION_SPR] = "spr",
    [DRAW_FUNCTION_DRAW_ROW] = "draw_row",
};

/* A row's items (a nested row folded the same way), then its color, then
 * its draw function; more entries may follow, the caller's. */
static uint32_t fold_row_entries(uint32_t h, const data_row_t *row) {
    int i;

    for (i = 0; i < row->count; i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = row->items[i].row != NULL ? fold_row(fnv(h, 0x7Au), row->items[i].row)
                                      : fold_item(h, &row->items[i]);
    }
    if (row->has_color) {
        h = fold_fixed(fnv_str(h, "color"), p8_fixed_from_int(row->color));
    }
    if (row->draw_function != DRAW_FUNCTION_NONE) {
        h = fold_string(fnv_str(h, "draw_function"), draw_function_names[row->draw_function]);
    }
    return h;
}

static int row_entries(const data_row_t *row) {
    return row->count + (row->has_color != 0) + (row->draw_function != DRAW_FUNCTION_NONE);
}

static uint32_t fold_row(uint32_t h, const data_row_t *row) {
    return fnv(fold_row_entries(h, row), (uint32_t)row_entries(row));
}

/* The year banner: its items, then keep_open. */
static uint32_t fold_year_banner(uint32_t h, const score_panel_year_banner_t *ea) {
    h = fold_row_entries(h, &ea->row);
    if (ea->keep_open != NULL) {
        h = fnv(fnv_str(h, "keep_open"), 0xFCu);
    }
    return fnv(h, (uint32_t)(row_entries(&ea->row) + (ea->keep_open != NULL)));
}

static uint32_t fold_ref(uint32_t h, const void *ref) {
    if (ref == NULL) {
        return fnv(h, 0xDEADu);
    }
    if (entity_list_listed(live_entities(), ref)) {
        return fnv(fnv(h, 0x4Eu), (uint32_t)entity_list_rank_in(live_entities(), ref));
    }
    if (ref == (const void *)&score_panel_countdown_rows_row || ref == (const void *)&score_panel_frame_rows_row) {
        return fold_row(fnv(h, 0x7Au), ref);
    }
    if (ref == (const void *)score_panel_readouts()) {
        return fold_row(fnv(h, 0x7Au), &score_panel_readouts()->row);
    }
    if (ref == (const void *)score_panel_year_banner()) {
        return fold_year_banner(fnv(h, 0x7Au), ref);
    }
    return fnv(fnv(h, 0xEEEEu), 0u);
}

/* A present member: bit 1, then the value as the frozen hashes fold it.
 * The 1 of a member the struct always holds, and the 0xF1 of a fixed, are
 * constant by the struct's shape, kept so that the hashes still prove the
 * typed reads. */
static uint32_t fold_fixed_member(uint32_t h, p8_fixed_t f) {
    return fnv(fnv(fnv(h, 1u), 0xF1u), (uint32_t)f);
}

static uint32_t fold_opt_fixed(uint32_t h, int has, p8_fixed_t f) {
    return has ? fold_fixed_member(h, f) : fnv(h, 0u);
}

/* A NULL reference folds as an absent member. */
static uint32_t fold_ref_member(uint32_t h, const void *ref) {
    return ref != NULL ? fold_ref(fnv(h, 1u), ref) : fnv(h, 0u);
}

static uint32_t fold_last_box(uint32_t h, const score_panel_t *pj) {
    assert(pj->last_box == NULL && "fold_last_box: no test folds a score panel holding a last box: to fold one, fold 1 then its rank in the live entities (0x4E, rank) as fold_ref does, or outside them 0xEEEE then the frozen struct tag 0x9E5C57C7");
    return fold_fixed_member(h, last_box_fixed(pj));
}

/* The layout block as the spec table it was: its entries in spec order,
 * then their count; the 0xF1 of back_color and the 0x57 of description are
 * constant by the struct's shape. */
static uint32_t fold_tech_tree_screen(uint32_t h, const score_panel_layout_block_t *cr) {
    int count = 0;

    assert(cr != NULL);
    h = fnv(fnv(h, 1u), 0x7Au);
    if (cr->fixed_pos != NULL) {
        h = fold_ref(fnv_str(h, "fixed_pos"), cr->fixed_pos);
        count++;
    }
    if (cr->row_step != NULL) {
        h = fold_ref(fnv_str(h, "row_step"), cr->row_step);
        count++;
    }
    h = fnv(fnv(fnv_str(h, "back_color"), 0xF1u), (uint32_t)cr->back_color);
    count++;
    if (cr->description != NULL) {
        h = fnv_str(fnv(fnv_str(h, "description"), 0x57u), cr->description);
        count++;
    }
    return fnv(h, (uint32_t)count);
}

static void check(const char *label, uint32_t got, uint32_t want) {
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got == want);
}

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)p8_peek(&memory, P8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
}

static const entity_class_t *class_of(const void *e) {
    return ((const entity_t *)e)->class;
}

/* A game started as cart_start_game() starts it, with no difficulty picked:
 * a NULL difficulty reads as difficulty 0. */
static score_panel_t *start_game(void) {
    p8_input_init(&input);
    dispatch_context_set_input(&input);
    cart_init();
    cart_flush_exit_queue();
    cart_start_game(NULL);
    cart_flush_exit_queue();
    assert(main_score_panel() != NULL);
    return main_score_panel();
}

static const data_row_t *row_of(const data_row_t *t, int i) {
    assert(i <= t->count && t->items[i - 1].row != NULL);
    return t->items[i - 1].row;
}

static const char *string_at(const data_row_t *t, int i) {
    assert(i <= t->count && t->items[i - 1].text != NULL);
    return t->items[i - 1].text;
}

/* The first item of each of the 7 readouts, as refresh wrote them. */
static uint32_t fold_readouts_heads(uint32_t h) {
    const score_panel_readouts_t *ou = score_panel_readouts();
    int i;

    assert(ou != NULL);
    for (i = 1; i <= 7; i++) {
        h = fold_item(h, &row_of(&ou->row, i)->items[0]);
    }
    return h;
}

static int is_of(const void *e, const entity_class_t *cls) {
    return e != NULL && ((const entity_t *)e)->class == cls;
}

static int modal_backdrop_count(void) {
    const entity_list_t *hv = live_entities();
    int n = 0;
    int i;
    for (i = 0; i < hv->count; i++) {
        n += is_of(hv->items[i], modal_backdrop_class());
    }
    return n;
}

static int layer15_button_count(void) {
    const entity_list_t *hv = live_entities();
    int n = 0;
    int i;
    for (i = 0; i < hv->count; i++) {
        const void *e = hv->items[i];
        n += is_of(e, button_class()) && ((const button_t *)e)->layer == p8_fixed_from_int(15);
    }
    return n;
}

static int panel_toggle_on_click(void) {
    return score_panel_toggle_on_click()(NULL);
}

/* score_panel_update()'s second button, the one whose on_click is the
 * panel toggle's. */
static button_t *panel_toggle_button(void) {
    const entity_list_t *hv = live_entities();
    int i;
    for (i = hv->count - 1; i >= 0; i--) {
        button_t *e = (button_t *)hv->items[i];
        if (is_of(e, button_class()) && e->layer == p8_fixed_from_int(15) &&
            e->on_click != score_panel_init_field_value()) {
            return e;
        }
    }
    return NULL;
}

/* The game's panel as cart_start_game() and entity_register() left it:
 * where it registered, its defaults, the names only read elsewhere, its 4
 * methods, and the 5 tables of the class, by identity and content. */
static void test_built(void) {
    uint32_t hash = 2166136261u;
    score_panel_t *ju = start_game();
    size_t index;

    hash = fnv(hash, (uint32_t)entity_list_rank_in(live_entities(), ju));
    hash = fnv(hash, (uint32_t)live_entities()->count);
    assert(entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW), ju) && entity_list_listed(entities_by_message(ENTITY_MESSAGE_NEW_YEAR), ju) && entity_list_listed(entities_by_message(ENTITY_MESSAGE_REFRESH), ju));
    assert(!entity_list_listed(entities_by_message(ENTITY_MESSAGE_DRAW_OVERLAY), ju) && !entity_list_listed(entities_by_message(ENTITY_MESSAGE_HIT_TEST), ju));
    hash = fnv(hash, (uint32_t)entity_list_rank_in(entities_by_message(ENTITY_MESSAGE_DRAW), ju));
    hash = fnv(hash, (uint32_t)entity_list_rank_in(entities_by_message(ENTITY_MESSAGE_NEW_YEAR), ju));
    hash = fnv(hash, (uint32_t)entity_list_rank_in(entities_by_message(ENTITY_MESSAGE_REFRESH), ju));
    for (index = 0; index < sizeof tags / sizeof tags[0]; index++) {
        assert(!entity_list_listed(entities_by_tag(tags[index]), ju));
    }

    assert(ju->layer == p8_fixed_from_int(10));
    assert(ju->archetype == &score_panel_archetype && ju->layer == ju->archetype->layer);
    assert(ju->last_box == NULL);
    assert(ju->header.state != NULL && strcmp(ju->header.state, "normal") == 0);
    hash = fold_fixed_member(hash, ju->header.state_time);
    assert(!ju->has_is_shown && !ju->has_curtain);
    assert(class_of(ju)->draw != NULL && class_of(ju)->states[ENTITY_STATE_NORMAL] != NULL && class_of(ju)->refresh != NULL &&
           class_of(ju)->new_year != NULL);
    assert(class_of(ju)->hit_test == NULL && class_of(ju)->draw_overlay == NULL);
    hash = fold_ref_member(hash, score_panel_year_banner());
    assert(ju->archetype->countdown_rows == &score_panel_countdown_rows_row);
    hash = fold_ref_member(hash, ju->archetype->countdown_rows);
    hash = fold_tech_tree_screen(hash, score_panel_layout_block());
    assert(ju->archetype->frame_rows == &score_panel_frame_rows_row);
    hash = fold_ref_member(hash, ju->archetype->frame_rows);
    hash = fold_ref_member(hash, score_panel_readouts());
    check("built", hash, BUILT_HASH);
}

/* A second panel shares the class's readouts, year banner, layout block,
 * countdown rows and frame rows, and sees what refresh and new_year wrote
 * into them through the first; layer, last_box, is_shown and curtain stay
 * each panel's own. */
static void test_shared(void) {
    uint32_t h = 2166136261u;
    score_panel_t *ju = start_game();
    score_panel_t *other = score_panel_new();
    const score_panel_year_banner_t *ea;

    assert(other != NULL && other != ju);
    assert(other->archetype == ju->archetype);
    h = fnv(h, (uint32_t)entity_list_rank_in(live_entities(), other));

    class_of(ju)->refresh(&ju->header);
    h = fold_readouts_heads(h);

    class_of(ju)->new_year(&ju->header, p8_fixed_from_int(3425));
    ea = score_panel_year_banner();
    assert(strcmp(string_at(row_of(&ea->row, 2), 1), "year 3425") == 0);
    assert(ea->row.count == 3 && ea->keep_open != NULL);
    h = fold_ref_member(h, score_panel_year_banner());

    main_economy_typed()->end_status = "final score";
    main_economy_typed()->has_end_status = 1;
    class_of(ju)->states[ENTITY_STATE_NORMAL](&ju->header, p8_fixed_from_int(0));
    assert(ju->layer == p8_fixed_from_int(13));
    assert(is_shown_state(ju) == 2);
    assert(other->layer == p8_fixed_from_int(10));
    assert(last_box_fixed(other) == p8_fixed_from_int(1));
    assert(!other->has_is_shown && !other->has_curtain);
    h = fold_fixed_member(h, other->layer);
    h = fold_last_box(h, other);
    h = fold_opt_fixed(h, ju->has_curtain, ju->curtain);
    check("shared", h, SHARED_HASH);
}

/* refresh; new_year on a year with a message and two without; the tech-tree
 * screen; draw closed; the game's end through the update, twice; draw open;
 * the panel toggle twice. */
static void test_life(void) {
    uint32_t hash = 2166136261u;
    score_panel_t *score_panel = start_game();
    const score_panel_year_banner_t *year_banner = score_panel_year_banner();
    const score_panel_layout_block_t *layout_block = score_panel_layout_block();
    static const int years[] = {3425, 3400, 3401};
    int modal_backdrop_before;
    int button_before;
    int boxes;
    size_t index;

    class_of(score_panel)->refresh(&score_panel->header);
    hash = fold_readouts_heads(hash);

    for (index = 0; index < sizeof years / sizeof years[0]; index++) {
        class_of(score_panel)->new_year(&score_panel->header, p8_fixed_from_int(years[index]));
        hash = fnv(hash, (uint32_t)(year_banner->row.count >= 3));
        hash = fnv(hash, (uint32_t)(year_banner->keep_open != NULL));
        hash = fnv_str(hash, string_at(row_of(&year_banner->row, 2), 1));
        hash = fnv(hash, (uint32_t)entity_list_count_of(entities_by_tag(TAG_MESSAGE_BOX)));
        hash = fnv(hash, (uint32_t)entity_list_rank_in(live_entities(), shown_message_box()));
    }
    assert(year_banner->row.count == 2 && year_banner->keep_open != NULL);

    menu_show_tech_tree_screen();
    assert(layout_block == score_panel_layout_block());
    assert(layout_block->description != NULL);
    hash = fnv_str(hash, layout_block->description);

    for (index = 0; index < 2; index++) {
        class_of(score_panel)->draw(&score_panel->header, NULL);
        hash = fnv(hash, screen_hash());
    }
    assert(!score_panel->has_curtain);

    modal_backdrop_before = modal_backdrop_count();
    button_before = layer15_button_count();
    main_economy_typed()->end_status = "final score";
    main_economy_typed()->has_end_status = 1;
    class_of(score_panel)->states[ENTITY_STATE_NORMAL](&score_panel->header, p8_fixed_from_int(0));
    assert(is_shown_state(score_panel) == 2);
    assert(score_panel->has_curtain && score_panel->curtain == 0);
    assert(score_panel->layer == p8_fixed_from_int(13));
    assert(modal_backdrop_count() == modal_backdrop_before + 1 && layer15_button_count() == button_before + 2);
    assert(score_panel->last_box == shown_message_box() && shown_message_box() != NULL);
    boxes = entity_list_count_of(entities_by_tag(TAG_MESSAGE_BOX));
    hash = fnv(hash, (uint32_t)entity_list_rank_in(live_entities(), shown_message_box()));
    hash = fnv(hash, (uint32_t)live_entities()->count);

    class_of(score_panel)->states[ENTITY_STATE_NORMAL](&score_panel->header, p8_fixed_from_int(0));
    assert(entity_list_count_of(entities_by_tag(TAG_MESSAGE_BOX)) == boxes);
    assert(score_panel->last_box == shown_message_box());

    for (index = 0; index < 3; index++) {
        class_of(score_panel)->draw(&score_panel->header, NULL);
        hash = fold_opt_fixed(hash, score_panel->has_curtain, score_panel->curtain);
        hash = fnv(hash, screen_hash());
    }
    assert(score_panel->has_curtain && score_panel->curtain == P8_FIXED_LIT(0x00009999, 0.6));

    assert(panel_toggle_on_click() == 1);
    assert(is_shown_state(score_panel) == 1);
    assert(panel_toggle_on_click() == 1);
    assert(is_shown_state(score_panel) == 2);
    {
        button_t *de = panel_toggle_button();
        assert(de != NULL && de->has_on_click && de->on_click != NULL);
        de->on_click(de);
        assert(is_shown_state(score_panel) == 1);
        de->on_click(de);
        assert(is_shown_state(score_panel) == 2);
    }
    hash = fold_fixed_member(hash, score_panel->layer);
    check("life", hash, LIFE_HASH);
}

static const button_t *as_button(const void *e) {
    assert(is_of(e, button_class()));
    return e;
}

static vec_t vec_of(const vec_t *v) {
    assert(v != NULL);
    return *v;
}

/* A cell as the value it was sorted from folded: a row as any REF, NOTHING
 * as false (no row folded here holds an absent cell at the top level). */
static uint32_t fold_cell(uint32_t h, layout_row_cell_t c) {
    switch (c.kind) {
    case LAYOUT_ROW_CELL_ROW:
        return fnv(h, 0x12u);
    case LAYOUT_ROW_CELL_FIXED:
        return fold_fixed(h, c.as.fixed);
    case LAYOUT_ROW_CELL_STRING:
        return fold_string(h, c.as.text);
    default:
        return fold_bool(h, c.kind == LAYOUT_ROW_CELL_TRUE);
    }
}

/* A button's layout row as its construction built it: its width, its color,
 * its items. */
static uint32_t fold_layout_row(uint32_t h, const layout_row_t *lq) {
    int i;

    assert(is_layout_row(lq));
    h = fold_fixed_member(h, lq->width);
    h = fold_opt_fixed(h, lq->has_color, lq->color);
    for (i = 0; i < row_count(lq); i++) {
        h = fnv(h, (uint32_t)(i + 1));
        h = fold_cell(h, row_item(lq, i));
    }
    return h;
}

/* What the tech-tree screen left in main_ui()'s ephemerals: the description
 * head row, then one row per tech, all at the layer of the back color, each
 * at the fixed position, its slide_target stepped by the row step. */
static uint32_t fold_tech_tree_screen_ephemerals(uint32_t h, const char **head) {
    const entity_list_t *le;
    const layout_row_t *lq;
    const vec_t *ra;
    int i;

    assert(main_ui() != NULL && main_ui()->ephemerals != NULL);
    le = main_ui()->ephemerals;
    assert(le->count >= 2);
    h = fnv(h, (uint32_t)le->count);
    lq = as_button(le->items[0])->layout_row;
    assert(lq != NULL && lq->has_color && lq->color == p8_fixed_from_int(13));
    assert(row_count(lq) > 0 && row_item(lq, 0).kind == LAYOUT_ROW_CELL_STRING);
    *head = row_item(lq, 0).as.text;
    h = fnv_str(h, *head);
    ra = as_button(le->items[0])->pos;
    for (i = 0; i < le->count; i++) {
        const button_t *b = as_button(le->items[i]);
        vec_t qf = vec_of(b->slide_target);
        assert(b->pos != NULL && b->pos->x == ra->x && b->pos->y == ra->y);
        assert(b->layer == p8_fixed_from_int(10));
        assert(qf.x == 0 && qf.y == p8_fixed_from_int(11 * (i + 1)));
        h = fold_fixed_member(h, b->layer);
        h = fnv(fnv(h, (uint32_t)qf.x), (uint32_t)qf.y);
        assert(b->layout_row != NULL);
        h = fold_layout_row(h, b->layout_row);
    }
    assert(vec_of(ra).x == 0 && vec_of(ra).y == p8_fixed_from_int(11));
    return h;
}

/* The tech-tree screen twice on one game, then on the next game started in
 * the same process: one shared layout block, its description absent until
 * the first screen and kept across games, the same head row each time. */
static void test_tech_tree_screen(void) {
    uint32_t h = 2166136261u;
    const score_panel_layout_block_t *cr;
    const char *head1;
    const char *head2;

    (void)start_game();
    cr = score_panel_layout_block();
    assert(cr != NULL && cr->description == NULL);
    menu_show_tech_tree_screen();
    h = fold_tech_tree_screen_ephemerals(h, &head1);
    assert(cr->description != NULL && strcmp(cr->description, head1) == 0);
    menu_show_tech_tree_screen();
    h = fold_tech_tree_screen_ephemerals(h, &head2);
    assert(strcmp(head1, head2) == 0);
    assert(score_panel_new() != NULL && score_panel_layout_block() == cr);

    (void)start_game();
    assert(score_panel_layout_block() == cr);
    assert(cr->description != NULL && strcmp(cr->description, head2) == 0);
    menu_show_tech_tree_screen();
    h = fold_tech_tree_screen_ephemerals(h, &head1);
    check("tech_tree_screen", h, TECH_TREE_SCREEN_HASH);
}

static int score_panel_year_banner_keep_open_witness(message_box_t *self) {
    (void)self;
    return 1;
}

/* Right after the init, before any refresh or new_year, the readouts and
 * the year banner hold what their generated rows hold, and are copies of
 * them; each init resets what refresh and new_year wrote. */
static void test_readouts_year_banner_start_as_their_rows(void) {
    score_panel_readouts_t *ou;

    score_panel_init();
    ou = score_panel_readouts();
    assert(ou != NULL && score_panel_year_banner() != NULL);
    assert(&ou->row != &score_panel_readouts_row && &score_panel_year_banner()->row != &score_panel_year_banner_row);
    assert(fold_row(0u, &ou->row) == fold_row(0u, &score_panel_readouts_row));
    assert(fold_row(0u, &score_panel_year_banner()->row) == fold_row(0u, &score_panel_year_banner_row) && score_panel_year_banner()->keep_open == NULL);
    ou->sub_items[0][0] = (data_item_t){.text = "written"};
    score_panel_year_banner()->second_items[0] = (data_item_t){.text = "year 3425"};
    score_panel_year_banner()->items[2] = (data_item_t){.row = &score_panel_countdown_rows_row};
    score_panel_year_banner()->row.count = 3;
    score_panel_year_banner()->keep_open = score_panel_year_banner_keep_open_witness;
    assert(fold_row(0u, &ou->row) != fold_row(0u, &score_panel_readouts_row));
    assert(fold_row(0u, &score_panel_year_banner()->row) != fold_row(0u, &score_panel_year_banner_row));
    score_panel_init();
    assert(score_panel_readouts() == ou);
    assert(fold_row(0u, &score_panel_readouts()->row) == fold_row(0u, &score_panel_readouts_row));
    assert(fold_row(0u, &score_panel_year_banner()->row) == fold_row(0u, &score_panel_year_banner_row) && score_panel_year_banner()->keep_open == NULL);
    score_panel_shutdown();
    assert(score_panel_readouts() == NULL && score_panel_year_banner() == NULL);
}

/* The game's panel is a struct whose descriptor holds exactly refresh,
 * draw, new_year and the update state; it takes no construction key. The
 * year banner, layout block and readouts are the class's shared state; the
 * countdown rows and frame rows are the generated rows. */
static void test_score_panel_is_a_struct(void) {
    const entity_class_t *cls = score_panel_class();
    score_panel_t *ju = start_game();
    int j;

    assert(is_of(ju, cls));
    assert(cls->drag == NULL && cls->drop == NULL && cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL &&
           cls->spawn_particle == NULL);
    assert(cls->refresh != NULL && cls->draw != NULL && cls->new_year != NULL && cls->states[ENTITY_STATE_NORMAL] != NULL);
    assert(cls->draw_overlay == NULL && cls->hit_test == NULL && cls->has_refresh == NULL && cls->left_press == NULL && cls->right_press == NULL &&
           cls->accept_drop == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert((cls->states[j] != NULL) == (j == ENTITY_STATE_NORMAL));
    }
    assert(score_panel_ready());
    assert(score_panel_readouts() != NULL && score_panel_year_banner() != NULL && score_panel_layout_block() != NULL);
    assert(score_panel_frame_rows() == &score_panel_frame_rows_row && score_panel_countdown_rows() == &score_panel_countdown_rows_row);
    assert(ju->archetype->countdown_rows == &score_panel_countdown_rows_row && ju->archetype->frame_rows == &score_panel_frame_rows_row);
}

/* The layout block is one static struct without method, holding fixed_pos,
 * row_step, back_color and, once the tech-tree screen ran, description; the
 * init resets description, the shutdown hides the block. */
static void test_tech_tree_screen_is_a_struct(void) {
    const entity_class_t *cls = score_panel_layout_block_class();
    score_panel_layout_block_t *cr;
    int j;

    (void)start_game();
    cr = score_panel_layout_block();
    assert(cr != NULL && is_of(cr, cls));
    assert(cls->draw == NULL && cls->draw_overlay == NULL && cls->hit_test == NULL && cls->new_year == NULL && cls->refresh == NULL &&
           cls->left_press == NULL && cls->right_press == NULL && cls->accept_drop == NULL);
    assert(cls->drag == NULL && cls->drop == NULL && cls->attach_slipway == NULL && cls->offer_trade == NULL && cls->draw_at == NULL &&
           cls->spawn_particle == NULL);
    for (j = 0; j < STATE_COUNT; j++) {
        assert(cls->states[j] == NULL);
    }
    assert(cr->fixed_pos != NULL);
    assert(cr->row_step != NULL && cr->row_step != cr->fixed_pos);
    assert(cr->back_color == p8_fixed_from_int(10));
    assert(cr->description == NULL);

    menu_show_tech_tree_screen();
    assert(cr->description != NULL);
    assert(score_panel_new() != NULL && score_panel_layout_block() == cr);

    score_panel_init();
    assert(score_panel_layout_block() == cr && cr->description == NULL);
    score_panel_shutdown();
    assert(score_panel_layout_block() == NULL);
}

/* Each scenario runs in a child forked right after cart_load(), as a fresh
 * process would start it: the random state is the same for all. */
static void in_fresh_child(void (*test)(void)) {
    pid_t child;
    int status;

    fflush(stdout);
    child = fork();
    assert(child >= 0);
    if (child == 0) {
        test();
        fflush(stdout);
        _exit(0);
    }
    assert(waitpid(child, &status, 0) == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/* economy_score_final() on a started game with ten techs invented and a
 * happiness of 1: no planet scores yet, so the total is the techs' part and
 * the tier follows from it. */
static void test_final_score_reads_invented_and_happiness(void) {
    economy_t *fm;
    final_score_t s;
    int i;

    (void)start_game();
    fm = main_economy_typed();
    for (i = 0; i < 10; i++) {
        pointer_list_add(fm->invented, "a tech");
    }
    fm->happiness = p8_fixed_from_int(1);
    fm->has_happiness = 1;
    s = economy_score_final();
    assert(s.development == 0 && s.population == 0 && s.technology == p8_fixed_from_int(1430));
    assert(strcmp(s.happiness, "1%") == 0);
    assert(s.total == p8_fixed_from_int(14) && s.tier == 0);
}

/* The given item (1-based) of the line that score_panel_update() put in the
 * box's given row (0-based), a draw_row article. */
static layout_row_cell_t update_row_item(const message_box_t *mh, int k, int i) {
    const data_row_t *ld;
    const layout_row_t *lq;

    assert(k < mh->items_count);
    ld = mh->items[k];
    assert(ld->draw_function == DRAW_FUNCTION_DRAW_ROW && ld->items[0].is_ref);
    lq = ld->items[0].ref;
    assert(i <= lq->items_count);
    return lq->items[i - 1];
}

/* score_panel_update() shows economy_score_final(): the score-5 row (box
 * row 10) carries the total, and the tier row (11) lights as many of its
 * five icons as the tier. */
static void test_update_rows_show_final_score(void) {
    void *ju = start_game();
    economy_t *fm = main_economy_typed();
    const message_box_t *mh;
    layout_row_cell_t total;
    layout_row_cell_t icon;
    int i;

    for (i = 0; i < 10; i++) {
        pointer_list_add(fm->invented, "a tech");
    }
    fm->happiness = p8_fixed_from_int(1);
    fm->has_happiness = 1;
    fm->end_status = "final score";
    fm->has_end_status = 1;
    class_of(ju)->states[ENTITY_STATE_NORMAL](ju, p8_fixed_from_int(0));
    mh = shown_message_box();
    assert(mh != NULL && mh->items_count == SCORE_ROWS_COUNT);
    total = update_row_item(mh, 10, 3);
    assert(total.kind == LAYOUT_ROW_CELL_STRING && strcmp(total.as.text, "14") == 0);
    for (i = 1; i <= 5; i++) {
        icon = update_row_item(mh, 11, i);
        assert(icon.kind == LAYOUT_ROW_CELL_FIXED && icon.as.fixed == p8_fixed_from_int(57));
    }
}

int main(void) {
    p8_mem_init(&memory);
    p8_gfx_init(&graphics);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    assert(p8_load_map_json(&memory, "pico8/map.json"));
    p8_input_init(&input);
    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    dispatch_context_set_input(&input);

    cart_load();
    in_fresh_child(test_built);
    in_fresh_child(test_shared);
    in_fresh_child(test_life);
    in_fresh_child(test_tech_tree_screen);
    in_fresh_child(test_readouts_year_banner_start_as_their_rows);
    in_fresh_child(test_score_panel_is_a_struct);
    in_fresh_child(test_tech_tree_screen_is_a_struct);
    in_fresh_child(test_final_score_reads_invented_and_happiness);
    in_fresh_child(test_update_rows_show_final_score);

    printf("test_score_panel: all tests passed\n");
    return 0;
}
