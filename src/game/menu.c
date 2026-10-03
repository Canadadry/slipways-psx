#include "game/menu.h"

#include <assert.h>
#include <string.h>

#include "p8/audio.h"
#include "game/main_economy.h"
#include "engine/entity_class.h"
#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "p8/fixed.h"
#include "game/game_data.h"
#include "p8/input.h"
#include "game/layout.h"
#include "game/message_box.h"
#include "game/draw_functions.h"
#include "game/score_panel.h"
#include "game/score_panel_display.h"
#include "game/tech_gate.h"
#include "game/ui_button.h"
#include "engine/vec.h"
#include "p8/str.h"
#include "engine/frame_zone.h"
#include "game/ui_zone.h"

/* The most items a row of the galaxy menu or the pause menu holds. */
#define MENU_ROW_CAP 4

static int g_menu_up = 0;
/* The galaxy menu's positions and the pause menu's fixed_pos and row_step,
 * reset by each init: the buttons borrow them. */
static vec_t g_galaxy_menu_pos[GALAXY_MENU_COUNT];
static vec_t g_pause_menu_fixed_pos;
static vec_t g_pause_menu_row_step;

static void galaxy_menu_button4_on_click_method(button_t *self);

typedef struct {
    entity_t header;
    vec_t *fixed_pos;
    vec_t *row_step;
    p8_fixed_t back_color;
    int has_back_color; /* 0: no back color, and back_color is then never read */
} pause_menu_open_box_t;

static const entity_class_t pause_menu_open_box_class_def = {0};

/* Never allocated nor freed: menu_init() points its fixed_pos and row_step at
 * the pause menu's values and sets its back color; menu_shutdown() clears. */
static pause_menu_open_box_t g_pause_menu_open_box = {
    {.class = &pause_menu_open_box_class_def},
    NULL,
    NULL,
    0,
    0,
};

/* The technology field of the button keys is where tech_tree_screen_step
 * stashes the row's captured tech, as its entry of technologies_table, read
 * back here off the button the keys were copied to. */
static void tech_tree_screen_row_on_click_method(button_t *self) {
    if (self == NULL) {
        return;
    }
    if (self->technology != NULL) {
        economy_invent(main_economy_typed(), self->technology);
    }
}

/* One tech-tree screen button: its keys and the layout rows they read. */
typedef struct {
    button_keys_t row;
    layout_row_keys_t layout_row;
    layout_row_cell_t layout_row_items[2];
    layout_row_keys_t cost_keys;
    layout_row_cell_t cost_items[2];
} tech_tree_screen_row_t;

/* An unaffordable tech gets on_click present but NULL, not absent: the
 * affordable flag is a comparison, and a false one leaves no callback. */
static void tech_tree_screen_step(const technologies_entry_t *t, tech_tree_screen_row_t *b) {
    static const data_item_t pair_items[] = {{.number = 24}, {.number = 5}};
    static const data_row_t pair = {.count = 2, .items = pair_items};
    p8_fixed_t research_cost_result;
    char *research_cost_str;

    b->layout_row_items[0] = layout_row_cell_fixed(p8_fixed_from_int(t->icon));
    b->layout_row_items[1] = layout_row_cell_string(t->name);
    b->layout_row.items = b->layout_row_items;
    b->layout_row.count = 2;
    b->layout_row.has_color = 1;
    b->layout_row.color = p8_fixed_from_int(6);
    b->row.layout_row_keys = &b->layout_row;

    research_cost_result = tech_gate_research_cost(t);
    research_cost_str = p8_tostr_fixed(frame_zone(), research_cost_result);

    b->cost_items[0] = layout_row_cell_row(&pair);
    b->cost_items[1] = layout_row_cell_string(research_cost_str);
    b->cost_keys.items = b->cost_items;
    b->cost_keys.count = 2;
    b->row.has_side_tooltip = 1;
    b->row.side_tooltip_row = &b->cost_keys;

    b->row.has_tooltip = 1;
    b->row.tooltip_row = t->tooltip_rows;

    b->row.has_on_click = 1;
    if (tech_gate_affordable_flags(t)) {
        b->row.technology = t;
        b->row.on_click = tech_tree_screen_row_on_click_method;
    }
}

/* Tooltip of the invented-count row: one line per invented tech, its name
 * (NULL an empty line), in one block of the frame zone. */
static const data_row_t *invented_lines(const pointer_list_t *list) {
    int n = list != NULL ? list->count : 0;
    size_t size = sizeof(data_row_t) + (size_t)n * sizeof(data_item_t);
    data_row_t *row = (data_row_t *)allocator_alloc(frame_zone(), size);
    data_item_t *items = (data_item_t *)(row + 1);
    int i;

    memset(row, 0, size);
    for (i = 0; i < n; i++) {
        items[i].text = list->items[i];
    }
    row->count = n;
    row->items = items;
    return row;
}

void menu_show_tech_tree_screen(void) {
    const economy_t *economy = main_economy_typed();
    p8_fixed_t current_era = p8_fixed_from_int(0);
    int invented_count = 0;
    p8_fixed_t next_threshold = p8_fixed_from_int(0);
    int tier_thresholds;
    available_technologies_kept_t available_technologies_list;
    tech_tree_screen_row_t *rows;
    button_keys_t *buttons;
    int row_count = 0;
    int index;
    score_panel_layout_block_t *layout_block;
    open_box_frame_t frame = {0};

    if (economy != NULL) {
        current_era = economy->era;
    }
    if (economy != NULL && economy->invented != NULL) {
        invented_count = economy->invented->count;
    }

    if (int_seq_at(tier_thresholds_table, TIER_THRESHOLDS_COUNT, p8_fixed_flr_int(current_era) - 1, &tier_thresholds)) {
        next_threshold = p8_fixed_from_int(tier_thresholds);
    }

    /* 1 is the ignore-cost flag: this screen lists every tech of the era,
     * affordable or not. The second galaxy row's refresh wants only the
     * affordable ones. The two sites do not merge. */
    available_technologies_list = tech_gate_available_technologies(1);
    rows = (tech_tree_screen_row_t *)allocator_alloc(frame_zone(), ((size_t)available_technologies_list.count + 1) * sizeof *rows);
    buttons = (button_keys_t *)allocator_alloc(frame_zone(), ((size_t)available_technologies_list.count + 1) * sizeof *buttons);
    memset(rows, 0, ((size_t)available_technologies_list.count + 1) * sizeof *rows);
    memset(buttons, 0, ((size_t)available_technologies_list.count + 1) * sizeof *buttons);
    for (index = 0; index < available_technologies_list.count; index++) {
        tech_tree_screen_step(available_technologies_list.items[index], &rows[row_count++]);
    }

    if (invented_count > 0) {
        tech_tree_screen_row_t *extra = &rows[row_count++];
        char *invented_text = p8_tostr_fixed(frame_zone(), p8_fixed_from_int(invented_count));
        char *label = p8_concat2(frame_zone(), invented_text, " invented");
        extra->layout_row_items[0] = layout_row_cell_fixed(p8_fixed_from_int(30));
        extra->layout_row_items[1] = layout_row_cell_string(label);
        extra->layout_row.items = extra->layout_row_items;
        extra->layout_row.count = 2;
        extra->layout_row.has_color = 1;
        extra->layout_row.color = p8_fixed_from_int(3);
        extra->row.layout_row_keys = &extra->layout_row;
        extra->row.has_tooltip = 1;
        extra->row.tooltip_row = invented_lines(economy->invented);
    }
    for (index = 0; index < row_count; index++) {
        buttons[index] = rows[index].row;
    }

    layout_block = score_panel_layout_block();
    if (layout_block != NULL) {
        p8_fixed_t tier_thresholds_era = p8_fixed_from_int(0);
        char *era_str;
        char *diff1_str;
        char *diff2_str;
        char *tmp1;
        char *tmp2;
        char *tmp3;
        char *ep;

        if (int_seq_at(tier_thresholds_table, TIER_THRESHOLDS_COUNT, p8_fixed_flr_int(current_era), &tier_thresholds)) {
            tier_thresholds_era = p8_fixed_from_int(tier_thresholds);
        }

        era_str = p8_tostr_fixed(frame_zone(), current_era);
        diff1_str = p8_tostr_fixed(frame_zone(), p8_fixed_sub(p8_fixed_from_int(invented_count), next_threshold));
        diff2_str = p8_tostr_fixed(frame_zone(), p8_fixed_sub(tier_thresholds_era, next_threshold));

        tmp1 = p8_concat2(frame_zone(), "level ", era_str);
        tmp2 = p8_concat2(frame_zone(), tmp1, " (");
        tmp3 = p8_concat2(frame_zone(), tmp2, diff1_str);
        tmp1 = p8_concat2(frame_zone(), tmp3, "/");
        tmp2 = p8_concat2(frame_zone(), tmp1, diff2_str);
        ep = p8_concat2(frame_zone(), tmp2, ")");

        layout_block->description = ep;
        frame.description = layout_block->description;
        frame.fixed_pos = layout_block->fixed_pos;
        frame.has_back_color = 1;
        frame.back_color = layout_block->back_color;
        frame.row_step = layout_block->row_step;
        message_box_open_box(&frame, buttons, row_count);
    }
}

static void galaxy_menu_button1_on_click_method(button_t *self) {
    economy_t *fm = main_economy_typed();
    (void)self;
    if (fm != NULL) {
        fm->year_ticks = p8_fixed_from_int(1);
    }
    economy_new_year(fm);
}

static void galaxy_menu_button2_on_click_method(button_t *self) {
    (void)self;
    menu_show_tech_tree_screen();
}

/* Without the flag, only affordable techs count. menu_show_tech_tree_screen()
 * passes it, the other, non-interchangeable site: passing it here would light
 * the icon whenever any tech of the era is left. */
static void galaxy_menu_button2_refresh_method(button_t *self) {
    available_technologies_kept_t available_technologies_list;
    int available_technologies_count;
    layout_row_cell_t item;
    layout_row_keys_t keys = {0};
    const allocator_t *slot;

    if (self == NULL) {
        return;
    }

    available_technologies_list = tech_gate_available_technologies(0);
    available_technologies_count = available_technologies_list.count;

    item = layout_row_cell_fixed(p8_fixed_from_int(available_technologies_count > 0 ? 24 : 37));
    keys.items = &item;
    keys.count = 1;
    slot = ui_zone_slot(SLOT_KIND_ROW);
    self->layout_row = layout_row_new(slot, &keys);
    if (self->layout_row_slot != NULL) {
        arena_release(self->layout_row_slot);
    }
    self->layout_row_slot = slot;
}

/* The modal backdrop takes the on_click of the one panel toggle of
 * score_panel_display.c, the one score_panel_update() uses. */
static void galaxy_menu_button3_on_click_method(button_t *self) {
    score_panel_t *ju = main_score_panel();
    modal_backdrop_keys_t keys = {0};
    modal_backdrop_t *built;
    (void)self;
    if (ju != NULL) {
        ju->is_shown = 1;
        ju->has_is_shown = 1;
    }
    keys.on_click = score_panel_toggle_on_click();
    built = modal_backdrop_new(&keys);
    (void)built;
}

/* TROU-10 */
static void pause_menu_row2_on_click_method(button_t *self) {
    int stat_val;
    p8_fixed_t s;
    (void)self;
    stat_val = p8_stat(dispatch_context_input(), 20);
    s = p8_fixed_sgn(p8_fixed_from_int(stat_val));
    p8_music(-p8_fixed_flr_int(s));
}

/* TROU-10 */
/* The mute state is the real global of p8/audio.c, which p8_play_sfx() reads;
 * a private copy here would leave that mute check on the old value. */
static void pause_menu_row3_on_click_method(button_t *self) {
    (void)self;
    p8_set_sfx_muted(!p8_sfx_muted());
}

static void pause_menu_row4_on_click_method(button_t *self) {
    (void)self;
}

static const button_fn_t g_galaxy_menu_on_click[GALAXY_MENU_COUNT] = {
    galaxy_menu_button1_on_click_method,
    galaxy_menu_button2_on_click_method,
    galaxy_menu_button3_on_click_method,
    galaxy_menu_button4_on_click_method,
};
static const button_fn_t g_galaxy_menu_refresh[GALAXY_MENU_COUNT] = {NULL, galaxy_menu_button2_refresh_method, NULL, NULL};

/* Opens the pause menu: its rows go in as buttons, their on_click installed
 * on every run, in the frame the pause-menu struct carries. */
static void galaxy_menu_button4_on_click_method(button_t *self) {
    const button_fn_t my[PAUSE_MENU_COUNT] = {
        score_panel_init_field_value(),
        pause_menu_row2_on_click_method,
        pause_menu_row3_on_click_method,
        pause_menu_row4_on_click_method,
    };
    button_keys_t bs[PAUSE_MENU_COUNT];
    layout_row_keys_t lq[PAUSE_MENU_COUNT];
    layout_row_cell_t items[PAUSE_MENU_COUNT][MENU_ROW_CAP];
    open_box_frame_t frame = {0};
    int i;
    (void)self;

    if (!g_menu_up) {
        return;
    }
    for (i = 0; i < PAUSE_MENU_COUNT; i++) {
        const pause_menu_entry_t *e = &pause_menu_table[i];

        memset(&bs[i], 0, sizeof bs[i]);
        lq[i] = layout_row_keys_of_row(e->layout_row, items[i], MENU_ROW_CAP);
        bs[i].layout_row_keys = &lq[i];
        bs[i].has_gpio_signal = e->has_gpio_signal;
        bs[i].gpio_signal = p8_fixed_from_int(e->gpio_signal);
        bs[i].has_on_click = 1;
        bs[i].on_click = my[i];
    }
    frame.fixed_pos = g_pause_menu_open_box.fixed_pos;
    frame.has_back_color = g_pause_menu_open_box.has_back_color;
    frame.back_color = g_pause_menu_open_box.back_color;
    frame.row_step = g_pause_menu_open_box.row_step;
    message_box_open_box(&frame, bs, PAUSE_MENU_COUNT);
}

button_t *galaxy_menu_button(int i) {
    const galaxy_menu_entry_t *e;
    button_keys_t keys = {0};
    layout_row_keys_t lq;
    layout_row_cell_t items[MENU_ROW_CAP];

    assert(g_menu_up && i >= 1 && i <= GALAXY_MENU_COUNT && "galaxy_menu_button: the galaxy menu has GALAXY_MENU_COUNT rows, after menu_init()");
    e = &galaxy_menu_table[i - 1];
    lq = layout_row_keys_of_row(e->layout_row, items, MENU_ROW_CAP);
    keys.layout_row_keys = &lq;
    keys.pos = &g_galaxy_menu_pos[i - 1];
    keys.has_on_click = 1;
    keys.on_click = g_galaxy_menu_on_click[i - 1];
    keys.refresh = g_galaxy_menu_refresh[i - 1];
    return button_new(&keys);
}

button_fn_t galaxy_menu_on_click(int i) {
    assert(i >= 1 && i <= GALAXY_MENU_COUNT);
    return g_galaxy_menu_on_click[i - 1];
}

const entity_t *pause_menu_open_box(void) {
    return g_menu_up ? &g_pause_menu_open_box.header : NULL;
}

const entity_class_t *pause_menu_open_box_class(void) {
    return &pause_menu_open_box_class_def;
}

void menu_init(void) {
    int i;

    menu_shutdown();

    draw_functions_init();

    for (i = 0; i < GALAXY_MENU_COUNT; i++) {
        g_galaxy_menu_pos[i] = vec_make(p8_fixed_from_int(galaxy_menu_table[i].pos_x),
                                 p8_fixed_from_int(galaxy_menu_table[i].pos_y));
    }
    g_pause_menu_fixed_pos = vec_make(p8_fixed_from_int(pause_menu_tail.fixed_pos_x), p8_fixed_from_int(pause_menu_tail.fixed_pos_y));
    g_pause_menu_row_step = vec_make(p8_fixed_from_int(pause_menu_tail.row_step_x), p8_fixed_from_int(pause_menu_tail.row_step_y));
    g_pause_menu_open_box.fixed_pos = &g_pause_menu_fixed_pos;
    g_pause_menu_open_box.row_step = &g_pause_menu_row_step;
    g_pause_menu_open_box.back_color = p8_fixed_from_int(pause_menu_tail.back_color);
    g_pause_menu_open_box.has_back_color = 1;
    g_menu_up = 1;
}

void menu_shutdown(void) {
    g_menu_up = 0;
    g_pause_menu_open_box.fixed_pos = NULL;
    g_pause_menu_open_box.row_step = NULL;
    g_pause_menu_open_box.back_color = 0;
    g_pause_menu_open_box.has_back_color = 0;
    /* The mute state is deliberately not reset here: it belongs to p8/audio.c,
     * and rebuilding this module's tables must not move it. */
}
