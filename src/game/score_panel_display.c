#include "game/score_panel_display.h"

#include <assert.h>
#include <string.h>

#include "game/economy_score.h"
#include "game/main_economy.h"
#include "game/layout.h"
#include "game/message_box.h"
#include "game/score_panel.h"
#include "p8/str.h"
#include "game/ui_button.h"
#include "engine/frame_zone.h"


/* The captured `t` of `pj:hk`'s `iw.lh` closure. One static is enough:
 * `pj.ea.lh` is replaced wholesale by every call that sets it, so only one
 * such closure is ever live. */
static int g_score_panel_new_year_keep_open_t = 0;

static int score_panel_new_year_keep_open_method(message_box_t *self) {
    (void)self;
    g_score_panel_new_year_keep_open_t -= 1;
    return g_score_panel_new_year_keep_open_t > 0;
}

void score_panel_new_year(p8_fixed_t year) {
    score_panel_year_banner_t *year_banner = score_panel_year_banner();
    const data_row_t *countdown_rows = score_panel_countdown_rows();
    const data_row_t *message = NULL;
    char *year_str;
    int idx;

    if (year_banner == NULL) {
        return;
    }

    idx = 3426 - p8_fixed_flr_int(year);
    if (idx >= 1 && idx <= countdown_rows->count) {
        const data_item_t *item = &countdown_rows->items[idx - 1];

        assert((item->row != NULL || item->is_false) && "gs holds rows and false only");
        message = item->row;
    }

    year_str = p8_tostr_fixed(frame_zone(), year);
    assert(strlen("year ") + strlen(year_str) <= SCORE_PANEL_TEXT_CAP && "a year is at most a 5-digit integer");
    strcpy(year_banner->year, "year ");
    strcat(year_banner->year, year_str);

    year_banner->second_items[0] = (data_item_t){.text = year_banner->year};

    if (message != NULL) {
        year_banner->items[2] = (data_item_t){.row = message};
        year_banner->row.count = 3;
        g_score_panel_new_year_keep_open_t = 90;
        year_banner->keep_open = score_panel_new_year_keep_open_method;
    } else {
        year_banner->row.count = 2;
    }

    {
        const data_row_t *items[3];
        message_box_keys_t keys = {0};
        message_box_t *made;
        int i;

        for (i = 0; i < year_banner->row.count; i++) {
            items[i] = year_banner->row.items[i].row;
        }
        keys.items = items;
        keys.count = year_banner->row.count;
        keys.keep_open = year_banner->keep_open;
        made = message_box_new(&keys);
        (void)made;
    }
}

/* `de.my`'s body: `pj:ph` stores it in a button, `fq(de)` in a veil, each
 * through the variant of its own callback type. */
static void score_panel_toggle_visibility(void) {
    score_panel_t *ju = main_score_panel();
    if (ju != NULL) {
        ju->is_shown = !(ju->has_is_shown && ju->is_shown);
        ju->has_is_shown = 1;
    }
}

static int score_panel_toggle_on_click_method(modal_backdrop_t *self) {
    (void)self;
    score_panel_toggle_visibility();
    return 1;
}

static void score_panel_toggle_button_on_click_method(button_t *self) {
    (void)self;
    score_panel_toggle_visibility();
}

/* `self` inside `pj:ph` is always `ju` -- `pj()` is constructed exactly once
 * in the cart -- so reading `ju.c_` here IS reading the captured upvalue. */
static int score_panel_update_keep_open_method(message_box_t *self) {
    const score_panel_t *ju = main_score_panel();
    (void)self;
    return ju != NULL && ju->has_is_shown && ju->is_shown;
}

typedef struct {
    const final_score_t *final_score;
} score_panel_update_ctx_t;

/* `tostr(s[i])`, in the frame zone. */
static char *score_panel_final_score_text(const final_score_t *s, int i) {
    switch (i) {
    case 1:
        return p8_tostr_fixed(frame_zone(), s->development);
    case 2:
        return p8_tostr_fixed(frame_zone(), s->population);
    case 3:
        return p8_tostr_fixed(frame_zone(), s->technology);
    case 4:
        return p8_concat2(frame_zone(), s->happiness, "");
    case 5:
        return p8_tostr_fixed(frame_zone(), s->total);
    case 6:
        return p8_tostr_fixed(frame_zone(), s->tier);
    default:
        /* TROU-13 */
        return p8_concat2(frame_zone(), "", "");
    }
}

/* An article `ld` of `bl` and its items, in one block. */
typedef struct {
    data_row_t row;
    data_item_t items[3];
} score_panel_update_draw_row_t;

static void score_panel_update_set(score_rows_row_t *st, int i, data_item_t item) {
    st->items[i - 1] = item;
    st->has_value[i - 1] = 1;
}

/* One row of `bz` as `ph` shows it: a row without a score is its own
 * article; one with a score gets `s[score]` at 3 (and more for 0 and 6),
 * then becomes the line of an article `ld`. */
static const data_row_t *score_panel_update_map(score_rows_row_t *score_row, const score_panel_update_ctx_t *ctx) {
    int score_i;
    char *s_str;
    layout_row_cell_t items[5];
    layout_row_keys_t keys = {0};
    layout_row_t *layout_row_inst;
    score_panel_update_draw_row_t *wrapped;
    int index;

    if (!score_row->entry->has_score) {
        return &score_row->row;
    }

    score_i = score_row->entry->score;

    s_str = score_panel_final_score_text(ctx->final_score, score_i);
    score_panel_update_set(score_row, 3, (data_item_t){.text = s_str});

    if (score_i == 0) {
        const economy_t *fm = main_economy_typed();
        const char *end_status_str = fm != NULL && fm->has_end_status && fm->end_status != NULL ? fm->end_status : "current score";
        const button_t *dk = economy_score_difficulty();
        const char *px = "";
        char *piece1;
        char *piece2;
        char *label;

        if (dk != NULL && dk->name != NULL) {
            px = dk->name;
        }
        piece1 = p8_concat2(frame_zone(), end_status_str, " (");
        piece2 = p8_concat2(frame_zone(), piece1, px);
        label = p8_concat2(frame_zone(), piece2, ")");
        score_panel_update_set(score_row, 1, (data_item_t){.text = label});
    }

    if (score_i == 6) {
        int i;
        for (i = 1; i <= 5; i++) {
            int icon = (i <= p8_fixed_flr_int(ctx->final_score->tier)) ? 56 : 57;
            score_panel_update_set(score_row, i, (data_item_t){.number = icon});
        }
    }

    /* TROU-25 */
    for (index = 0; index < 5; index++) {
        if (score_row->has_value[index]) {
            items[keys.count++] = layout_row_cell_item(&score_row->items[index]);
        }
    }
    keys.items = items;
    keys.has_color = score_row->row.has_color;
    keys.color = p8_fixed_from_int(score_row->row.color);
    keys.has_score = 1;
    keys.score = p8_fixed_from_int(score_i);
    keys.pos = score_row->entry->has_pos ? &score_row->pos : NULL;
    layout_row_inst = layout_row_new(frame_zone(), &keys);
    wrapped = (score_panel_update_draw_row_t *)allocator_alloc(frame_zone(), sizeof *wrapped);
    wrapped->items[0] = (data_item_t){.is_ref = 1, .ref = layout_row_inst};
    wrapped->items[1] = (data_item_t){.is_ref = 1, .ref = keys.pos};
    wrapped->items[2] = (data_item_t){.is_fixed = 1, .fixed = P8_FIXED_LIT(0x00008000, 0.5)};
    wrapped->row = (data_row_t){.count = 3, .items = wrapped->items, .draw_function = "draw_row"};
    return &wrapped->row;
}

void score_panel_update(score_panel_t *self) {
    const economy_t *economy = main_economy_typed();
    int has_end_status = economy != NULL && economy->has_end_status && economy->end_status != NULL;
    int has_curtain = self != NULL && self->has_curtain;

    if (has_end_status && !has_curtain) {
        static const data_item_t back_items[] = {{.number = 10}, {.number = 7}};
        static const data_row_t back = {.count = 2, .items = back_items};
        button_keys_t spec1 = {0};
        button_keys_t spec2 = {0};
        layout_row_cell_t first_line_item;
        layout_row_cell_t second_line_item;
        layout_row_keys_t first_line = {0};
        layout_row_keys_t second_line = {0};
        button_t *made1;
        button_t *made2;
        const vec_t first_line_pos = vec_make(p8_fixed_from_int(51), p8_fixed_from_int(100));
        const vec_t second_line_pos = vec_make(p8_fixed_from_int(39), p8_fixed_from_int(100));

        (void)modal_backdrop_new(NULL);

        first_line_item = layout_row_cell_string(" again? ");
        first_line.items = &first_line_item;
        first_line.count = 1;
        spec1.has_layer = 1;
        spec1.layer = p8_fixed_from_int(15);
        spec1.pos = &first_line_pos;
        spec1.layout_row_keys = &first_line;
        spec1.has_on_click = 1;
        spec1.on_click = score_panel_init_field_value();
        made1 = button_new(&spec1);
        (void)made1;

        second_line_item = layout_row_cell_row(&back);
        second_line.items = &second_line_item;
        second_line.count = 1;
        spec2.has_layer = 1;
        spec2.layer = p8_fixed_from_int(15);
        spec2.pos = &second_line_pos;
        spec2.layout_row_keys = &second_line;
        spec2.has_on_click = 1;
        spec2.on_click = score_panel_toggle_button_on_click_method;
        made2 = button_new(&spec2);
        (void)made2;

        if (self != NULL) {
            self->is_shown = 1;
            self->has_is_shown = 1;
            self->curtain = 0;
            self->has_curtain = 1;
            self->layer = p8_fixed_from_int(13);
        }
    }

    {
        int c_truthy = self != NULL && self->has_is_shown && self->is_shown;
        int last_box_equals_shown_message_box = 0;

        /* A NULL `ed` is the class default, the number 1, which is never
         * `kc`, not even a nil one: without the `ed != NULL` test, NULL == NULL
         * would keep the block from firing on the first eligible tick. */
        if (self != NULL) {
            last_box_equals_shown_message_box = self->last_box != NULL && shown_message_box() == self->last_box;
        }

        if (c_truthy && !last_box_equals_shown_message_box) {
            final_score_t s = economy_score_final();
            score_rows_row_t *bz = score_panel_score_rows();
            score_panel_update_ctx_t ctx;
            const data_row_t *iw[SCORE_ROWS_COUNT];
            message_box_keys_t keys = {0};
            message_box_t *made;
            int i;

            ctx.final_score = &s;
            for (i = 0; bz != NULL && i < SCORE_ROWS_COUNT; i++) {
                iw[i] = score_panel_update_map(&bz[i], &ctx);
            }

            keys.items = iw;
            keys.count = bz != NULL ? SCORE_ROWS_COUNT : 0;
            keys.has_duration_scale = 1;
            keys.duration_scale = p8_fixed_from_int(3);
            keys.keep_open = score_panel_update_keep_open_method;
            made = message_box_new(&keys);

            if (self != NULL) {
                self->last_box = made;
            }
        }
    }
}

void score_panel_display_init(void) {
    score_panel_display_shutdown();
    g_score_panel_new_year_keep_open_t = 0;
}

void score_panel_display_shutdown(void) {
}

modal_backdrop_on_click_fn_t score_panel_toggle_on_click(void) {
    return score_panel_toggle_on_click_method;
}

void score_panel_new_year_method(entity_t *self, p8_fixed_t yr) {
    (void)self;
    score_panel_new_year(yr);
}

void score_panel_update_method(entity_t *self, p8_fixed_t t) {
    (void)t;
    score_panel_update((score_panel_t *)self);
}
