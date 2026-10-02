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

/* The most items a row of `ix` or `ha` holds. */
#define PICO8_MENU_ROW_CAP 4

static int g_menu_up = 0;
/* `ix[i].ra` and `ha.hi`/`ha.gq`, reset by each init: the buttons borrow
 * them. */
static vec_t g_ix_ra[GALAXY_MENU_COUNT];
static vec_t g_ha_hi;
static vec_t g_ha_gq;

static void pico8_ix4_my_dispatch(pico8_js_t *self);

typedef struct {
    entity_t header;
    vec_t *hi;
    vec_t *gq;
    p8_fixed_t ca;
    int has_ca; /* 0: `ca` is nil, and `ca` is then never read */
} pico8_ha_pz_t;

static const entity_class_t g_ha_pz_struct_class = {0};

/* Never allocated nor freed: `pico8_menu_init()` points its `hi`/`gq`/`ca`
 * at `ha`'s own values, `pico8_menu_shutdown()` resets them to nil. */
static pico8_ha_pz_t g_ha_pz = {
    {.class = &g_ha_pz_struct_class},
    NULL,
    NULL,
    0,
    0,
};

/* `_cr_t` is not a cart field: it is where pico8_cr_step stashes the row's
 * captured tech, as its entry of pico8_nb_table, read back here off the
 * button `js(b)` copied it to. */
static void pico8_cr_row_my_dispatch(pico8_js_t *self) {
    if (self == NULL) {
        return;
    }
    if (self->_cr_t != NULL) {
        economy_invent(main_economy_typed(), self->_cr_t);
    }
}

/* One row of `cr`'s `bs`: the keys and the rows its `lq`/`h_` read. */
typedef struct {
    pico8_js_keys_t row;
    layout_row_keys_t lq;
    layout_row_cell_t lq_items[2];
    layout_row_keys_t h;
    layout_row_cell_t h_items[2];
} pico8_cr_row_t;

/* An unaffordable tech gets "my" = false, not an absent field: `t.e_` is a
 * comparison result in `cb()`, so Lua's `and` yields the boolean here. */
static void pico8_cr_step(const technologies_entry_t *t, pico8_cr_row_t *b) {
    static const data_item_t pair_items[] = {{.number = 24}, {.number = 5}};
    static const data_row_t pair = {.count = 2, .items = pair_items};
    p8_fixed_t gi_result;
    char *gi_str;

    b->lq_items[0] = layout_row_cell_fixed(p8_fixed_from_int(t->icon));
    b->lq_items[1] = layout_row_cell_string(t->name);
    b->lq.items = b->lq_items;
    b->lq.count = 2;
    b->lq.has_color = 1;
    b->lq.color = p8_fixed_from_int(6);
    b->row.lq_row = &b->lq;

    gi_result = tech_gate_research_cost(t);
    gi_str = p8_tostr_fixed(frame_zone(), gi_result);

    b->h_items[0] = layout_row_cell_row(&pair);
    b->h_items[1] = layout_row_cell_string(gi_str);
    b->h.items = b->h_items;
    b->h.count = 2;
    b->row.has_h_ = 1;
    b->row.h_row = &b->h;

    b->row.has_jb = 1;
    b->row.jb_row = t->tooltip_rows;

    b->row.has_my = 1;
    if (tech_gate_affordable_flags(t)) {
        b->row._cr_t = t;
        b->row.my = pico8_cr_row_my_dispatch;
    }
}

/* `jb` of the "n invented" row: one line per invented tech, its name (NULL
 * an empty line), in one block of the frame zone. */
static const data_row_t *pico8_menu_invented_lines(const pointer_list_t *list) {
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

void pico8_cr(void) {
    const economy_t *fm = main_economy_typed();
    p8_fixed_t tl = p8_fixed_from_int(0);
    int gr = 0;
    p8_fixed_t qi = p8_fixed_from_int(0);
    int ek;
    available_technologies_kept_t cb_list;
    pico8_cr_row_t *rows;
    pico8_js_keys_t *bs;
    int n = 0;
    int i;
    pico8_pj_cr_t *cr;
    pico8_pz_frame_t frame = {0};

    if (fm != NULL) {
        tl = fm->era;
    }
    if (fm != NULL && fm->invented != NULL) {
        gr = fm->invented->count;
    }

    if (int_seq_at(tier_thresholds_table, TIER_THRESHOLDS_COUNT, p8_fixed_flr_int(tl) - 1, &ek)) {
        qi = p8_fixed_from_int(ek);
    }

    /* `true` is the ignore-cost flag: this screen lists every tech of the era,
     * affordable or not. `ix[2].hb`'s bare `cb()` wants only the affordable
     * ones. The two sites do not merge. */
    cb_list = tech_gate_available_technologies(1);
    rows = (pico8_cr_row_t *)allocator_alloc(frame_zone(), ((size_t)cb_list.count + 1) * sizeof *rows);
    bs = (pico8_js_keys_t *)allocator_alloc(frame_zone(), ((size_t)cb_list.count + 1) * sizeof *bs);
    memset(rows, 0, ((size_t)cb_list.count + 1) * sizeof *rows);
    memset(bs, 0, ((size_t)cb_list.count + 1) * sizeof *bs);
    for (i = 0; i < cb_list.count; i++) {
        pico8_cr_step(cb_list.items[i], &rows[n++]);
    }

    if (gr > 0) {
        pico8_cr_row_t *extra = &rows[n++];
        char *gr_str = p8_tostr_fixed(frame_zone(), p8_fixed_from_int(gr));
        char *label = p8_concat2(frame_zone(), gr_str, " invented");
        extra->lq_items[0] = layout_row_cell_fixed(p8_fixed_from_int(30));
        extra->lq_items[1] = layout_row_cell_string(label);
        extra->lq.items = extra->lq_items;
        extra->lq.count = 2;
        extra->lq.has_color = 1;
        extra->lq.color = p8_fixed_from_int(3);
        extra->row.lq_row = &extra->lq;
        extra->row.has_jb = 1;
        extra->row.jb_row = pico8_menu_invented_lines(fm->invented);
    }
    for (i = 0; i < n; i++) {
        bs[i] = rows[i].row;
    }

    cr = pico8_pj_cr();
    if (cr != NULL) {
        p8_fixed_t ek_tl = p8_fixed_from_int(0);
        char *tl_str;
        char *diff1_str;
        char *diff2_str;
        char *tmp1;
        char *tmp2;
        char *tmp3;
        char *ep;

        if (int_seq_at(tier_thresholds_table, TIER_THRESHOLDS_COUNT, p8_fixed_flr_int(tl), &ek)) {
            ek_tl = p8_fixed_from_int(ek);
        }

        tl_str = p8_tostr_fixed(frame_zone(), tl);
        diff1_str = p8_tostr_fixed(frame_zone(), p8_fixed_sub(p8_fixed_from_int(gr), qi));
        diff2_str = p8_tostr_fixed(frame_zone(), p8_fixed_sub(ek_tl, qi));

        tmp1 = p8_concat2(frame_zone(), "level ", tl_str);
        tmp2 = p8_concat2(frame_zone(), tmp1, " (");
        tmp3 = p8_concat2(frame_zone(), tmp2, diff1_str);
        tmp1 = p8_concat2(frame_zone(), tmp3, "/");
        tmp2 = p8_concat2(frame_zone(), tmp1, diff2_str);
        ep = p8_concat2(frame_zone(), tmp2, ")");

        cr->ep = ep;
        frame.ep = cr->ep;
        frame.hi = cr->hi;
        frame.has_ca = 1;
        frame.ca = cr->ca;
        frame.gq = cr->gq;
        pico8_pz(&frame, bs, n);
    }
}

static void pico8_ix1_my_dispatch(pico8_js_t *self) {
    economy_t *fm = main_economy_typed();
    (void)self;
    if (fm != NULL) {
        fm->year_ticks = p8_fixed_from_int(1);
    }
    economy_new_year(fm);
}

static void pico8_ix2_my_dispatch(pico8_js_t *self) {
    (void)self;
    pico8_cr();
}

/* The bare `cb()` counts only affordable techs. pico8_cr()'s `cb(true)` is the
 * other, non-interchangeable site: passing the flag here would light the icon
 * whenever any tech of the era is left. */
static void pico8_ix2_hb_dispatch(pico8_js_t *self) {
    available_technologies_kept_t cb_list;
    int cb_count;
    layout_row_cell_t item;
    layout_row_keys_t keys = {0};
    const allocator_t *place;

    if (self == NULL) {
        return;
    }

    cb_list = tech_gate_available_technologies(0);
    cb_count = cb_list.count;

    item = layout_row_cell_fixed(p8_fixed_from_int(cb_count > 0 ? 24 : 37));
    keys.items = &item;
    keys.count = 1;
    place = pico8_zone_interface_place(PICO8_SORTE_RANGEE);
    self->lq = layout_row_new(place, &keys);
    if (self->lq_place != NULL) {
        arena_release(self->lq_place);
    }
    self->lq_place = place;
}

/* `fq(de)` takes the `my` of the one `de` of pico8_score_panel_display.c,
 * the one `pj:ph()` uses. */
static void pico8_ix3_my_dispatch(pico8_js_t *self) {
    pico8_pj_t *ju = pico8_ju();
    pico8_fq_keys_t keys = {0};
    pico8_fq_t *built;
    (void)self;
    if (ju != NULL) {
        ju->c_ = 1;
        ju->has_c_ = 1;
    }
    keys.my = pico8_de_my();
    built = pico8_fq_new(&keys);
    (void)built;
}

/* TROU-10 */
static void pico8_ha2_my_dispatch(pico8_js_t *self) {
    int stat_val;
    p8_fixed_t s;
    (void)self;
    stat_val = p8_stat(dispatch_context_input(), 20);
    s = p8_fixed_sgn(p8_fixed_from_int(stat_val));
    p8_music(-p8_fixed_flr_int(s));
}

/* TROU-10 */
/* `ih` is the real global of pico8_audio.c, which pico8_rn() reads; a private
 * copy here would leave that mute check on the old value. */
static void pico8_ha3_my_dispatch(pico8_js_t *self) {
    (void)self;
    p8_set_sfx_muted(!p8_sfx_muted());
}

static void pico8_ha4_my_dispatch(pico8_js_t *self) {
    (void)self;
}

static const pico8_js_fn g_ix_my[GALAXY_MENU_COUNT] = {
    pico8_ix1_my_dispatch,
    pico8_ix2_my_dispatch,
    pico8_ix3_my_dispatch,
    pico8_ix4_my_dispatch,
};
static const pico8_js_fn g_ix_hb[GALAXY_MENU_COUNT] = {NULL, pico8_ix2_hb_dispatch, NULL, NULL};

/* `pz(ha,ha)`: `ha`'s rows go in as `bs`, their `my` installed on every
 * run, and `p` is the frame of `g_ha_pz`, which carries `ha`'s `hi`/`gq`/`ca`. */
static void pico8_ix4_my_dispatch(pico8_js_t *self) {
    const pico8_js_fn my[PAUSE_MENU_COUNT] = {
        pico8_init_field_value(),
        pico8_ha2_my_dispatch,
        pico8_ha3_my_dispatch,
        pico8_ha4_my_dispatch,
    };
    pico8_js_keys_t bs[PAUSE_MENU_COUNT];
    layout_row_keys_t lq[PAUSE_MENU_COUNT];
    layout_row_cell_t items[PAUSE_MENU_COUNT][PICO8_MENU_ROW_CAP];
    pico8_pz_frame_t frame = {0};
    int i;
    (void)self;

    if (!g_menu_up) {
        return;
    }
    for (i = 0; i < PAUSE_MENU_COUNT; i++) {
        const pause_menu_entry_t *e = &pause_menu_table[i];

        memset(&bs[i], 0, sizeof bs[i]);
        lq[i] = layout_row_keys_of_row(e->layout_row, items[i], PICO8_MENU_ROW_CAP);
        bs[i].lq_row = &lq[i];
        bs[i].has_fg = e->has_gpio_signal;
        bs[i].fg = p8_fixed_from_int(e->gpio_signal);
        bs[i].has_my = 1;
        bs[i].my = my[i];
    }
    frame.hi = g_ha_pz.hi;
    frame.has_ca = g_ha_pz.has_ca;
    frame.ca = g_ha_pz.ca;
    frame.gq = g_ha_pz.gq;
    pico8_pz(&frame, bs, PAUSE_MENU_COUNT);
}

pico8_js_t *pico8_ix_button(int i) {
    const galaxy_menu_entry_t *e;
    pico8_js_keys_t keys = {0};
    layout_row_keys_t lq;
    layout_row_cell_t items[PICO8_MENU_ROW_CAP];

    assert(g_menu_up && i >= 1 && i <= GALAXY_MENU_COUNT && "ix has PICO8_IX_COUNT rows, after pico8_menu_init()");
    e = &galaxy_menu_table[i - 1];
    lq = layout_row_keys_of_row(e->layout_row, items, PICO8_MENU_ROW_CAP);
    keys.lq_row = &lq;
    keys.ra = &g_ix_ra[i - 1];
    keys.has_my = 1;
    keys.my = g_ix_my[i - 1];
    keys.hb = g_ix_hb[i - 1];
    return pico8_js_new(&keys);
}

pico8_js_fn pico8_ix_my(int i) {
    assert(i >= 1 && i <= GALAXY_MENU_COUNT);
    return g_ix_my[i - 1];
}

const entity_t *pico8_ha_pz(void) {
    return g_menu_up ? &g_ha_pz.header : NULL;
}

const entity_class_t *pico8_ha_pz_struct_class(void) {
    return &g_ha_pz_struct_class;
}

void pico8_menu_init(void) {
    int i;

    pico8_menu_shutdown();

    draw_functions_init();

    for (i = 0; i < GALAXY_MENU_COUNT; i++) {
        g_ix_ra[i] = vec_make(p8_fixed_from_int(galaxy_menu_table[i].pos_x),
                                 p8_fixed_from_int(galaxy_menu_table[i].pos_y));
    }
    g_ha_hi = vec_make(p8_fixed_from_int(pause_menu_tail.fixed_pos_x), p8_fixed_from_int(pause_menu_tail.fixed_pos_y));
    g_ha_gq = vec_make(p8_fixed_from_int(pause_menu_tail.row_step_x), p8_fixed_from_int(pause_menu_tail.row_step_y));
    g_ha_pz.hi = &g_ha_hi;
    g_ha_pz.gq = &g_ha_gq;
    g_ha_pz.ca = p8_fixed_from_int(pause_menu_tail.back_color);
    g_ha_pz.has_ca = 1;
    g_menu_up = 1;
}

void pico8_menu_shutdown(void) {
    g_menu_up = 0;
    g_ha_pz.hi = NULL;
    g_ha_pz.gq = NULL;
    g_ha_pz.ca = 0;
    g_ha_pz.has_ca = 0;
    /* `ih` is deliberately not reset here: it belongs to pico8_audio.c, and
     * rebuilding this module's tables must not move it. */
}
