#include "pico8_menu.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_struct.h"
#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_input.h"
#include "pico8_layout.h"
#include "pico8_msgbox.h"
#include "pico8_pi.h"
#include "pico8_probe.h"
#include "pico8_score_panel.h"
#include "pico8_score_panel_display.h"
#include "pico8_tech_gate.h"
#include "pico8_ui_button.h"
#include "pico8_value.h"
#include "pico8_vec.h"
#include "pico8_ref_audit.h"

/* The most items a row of `ix` or `ha` holds. */
#define PICO8_MENU_ROW_CAP 4

static int g_menu_up = 0;
/* `ix[i].ra` and `ha.hi`/`ha.gq`, reset by each init: the buttons borrow
 * them. */
static pico8_vec_t g_ix_ra[PICO8_IX_COUNT];
static pico8_vec_t g_ha_hi;
static pico8_vec_t g_ha_gq;

static void pico8_ix4_my_dispatch(pico8_js_t *self);

typedef struct {
    pico8_struct_header_t header;
    pico8_vec_t *hi;
    pico8_vec_t *gq;
    pico8_fixed_t ca;
    int has_ca; /* 0: `ca` is nil, and `ca` is then never read */
} pico8_ha_pz_t;

static const pico8_struct_class_t g_ha_pz_struct_class = {0};

/* Never allocated nor freed: `pico8_menu_init()` points its `hi`/`gq`/`ca`
 * at `ha`'s own values, `pico8_menu_shutdown()` resets them to nil. */
static pico8_ha_pz_t g_ha_pz = {
    {PICO8_TYPE_TAG_STRUCT, &g_ha_pz_struct_class},
    NULL,
    NULL,
    0,
    0,
};

static char *cr_concat2(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char *out = malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

/* `_cr_t` is not a cart field: it is where pico8_cr_step stashes the row's
 * captured tech, as its entry of pico8_nb_table, read back here off the
 * button `js(b)` copied it to. */
static void pico8_cr_row_my_dispatch(pico8_js_t *self) {
    if (self == NULL) {
        return;
    }
    if (self->_cr_t != NULL) {
        pico8_hy_kt(pico8_fm_hy(), self->_cr_t);
    }
}

/* One row of `cr`'s `bs`: the keys and the rows its `lq`/`h_` read. */
typedef struct {
    pico8_js_keys_t row;
    pico8_lq_keys_t lq;
    pico8_value_t lq_items[2];
    pico8_lq_keys_t h;
    pico8_value_t h_items[2];
} pico8_cr_row_t;

/* TROU-7 */
/* An unaffordable tech gets "my" = false, not an absent field: `t.e_` is a
 * comparison result in `cb()`, so Lua's `and` yields the boolean here. */
static void pico8_cr_step(const pico8_nb_entry_t *t, pico8_cr_row_t *b) {
    static const pico8_jb_item_t pair_items[] = {{.n = 24}, {.n = 5}};
    static const pico8_jb_row_t pair = {.count = 2, .items = pair_items};
    pico8_fixed_t gi_result;
    char *gi_str;

    b->lq_items[0] = pico8_value_fixed(pico8_fixed_from_int(t->nk));
    b->lq_items[1] = pico8_value_string(t->px);
    b->lq.items = b->lq_items;
    b->lq.count = 2;
    b->lq.has_c = 1;
    b->lq.c = pico8_fixed_from_int(6);
    b->row.lq = pico8_value_ref(NULL);
    b->row.lq_row = &b->lq;

    gi_result = pico8_gi_nb(t);
    gi_str = pico8_value_tostr(pico8_value_fixed(gi_result));

    b->h_items[0] = pico8_value_ref((void *)&pair);
    b->h_items[1] = pico8_value_string(gi_str);
    b->h.items = b->h_items;
    b->h.count = 2;
    b->row.has_h_ = 1;
    b->row.h_ = pico8_value_ref(NULL);
    b->row.h_row = &b->h;

    b->row.has_jb = 1;
    b->row.jb = pico8_value_ref(NULL);
    b->row.jb_row = t->oy;

    b->row.has_my = 1;
    if (pico8_nb_e_(t)) {
        b->row._cr_t = t;
        b->row.my = pico8_cr_row_my_dispatch;
    }
}

/* TROU-7 */
/* `jb` of the "n invented" row: one line per invented tech, its name (NULL
 * an empty line), in one fresh block. */
static const pico8_jb_row_t *pico8_menu_invented_lines(const pico8_ptrs_t *list) {
    int n = list != NULL ? list->count : 0;
    pico8_jb_row_t *row = calloc(1, sizeof *row + (size_t)n * sizeof(pico8_jb_item_t));
    pico8_jb_item_t *items = (pico8_jb_item_t *)(row + 1);
    int i;

    assert(row != NULL);
    for (i = 0; i < n; i++) {
        items[i].s = list->items[i];
    }
    row->count = n;
    row->items = items;
    return row;
}

void pico8_cr(void) {
    const pico8_hy_t *fm = pico8_fm_hy();
    pico8_fixed_t tl = pico8_fixed_from_int(0);
    int gr = 0;
    pico8_fixed_t qi = pico8_fixed_from_int(0);
    int ek;
    pico8_cb_kept_t cb_list;
    pico8_cr_row_t *rows;
    pico8_js_keys_t *bs;
    int n = 0;
    int i;
    pico8_pj_cr_t *cr;
    pico8_pz_frame_t frame = {0};

    if (fm != NULL) {
        tl = fm->ew;
    }
    if (fm != NULL && fm->invented != NULL) {
        gr = fm->invented->count;
    }

    if (pico8_int_seq_at(pico8_ek_table, PICO8_EK_COUNT, pico8_fixed_flr_int(tl) - 1, &ek)) {
        qi = pico8_fixed_from_int(ek);
    }

    /* `true` is the ignore-cost flag: this screen lists every tech of the era,
     * affordable or not. `ix[2].hb`'s bare `cb()` wants only the affordable
     * ones. The two sites do not merge. */
    cb_list = pico8_cb(pico8_value_bool(1));
    rows = calloc((size_t)cb_list.count + 1, sizeof *rows);
    bs = calloc((size_t)cb_list.count + 1, sizeof *bs);
    assert(rows != NULL && bs != NULL);
    for (i = 0; i < cb_list.count; i++) {
        pico8_cr_step(cb_list.items[i], &rows[n++]);
    }

    if (gr > 0) {
        pico8_cr_row_t *extra = &rows[n++];
        char *gr_str = pico8_value_tostr(pico8_value_fixed(pico8_fixed_from_int(gr)));
        char *label = cr_concat2(gr_str, " invented");
        free(gr_str);
        extra->lq_items[0] = pico8_value_fixed(pico8_fixed_from_int(30));
        extra->lq_items[1] = pico8_value_string(label);
        extra->lq.items = extra->lq_items;
        extra->lq.count = 2;
        extra->lq.has_c = 1;
        extra->lq.c = pico8_fixed_from_int(3);
        extra->row.lq = pico8_value_ref(NULL);
        extra->row.lq_row = &extra->lq;
        extra->row.has_jb = 1;
        extra->row.jb = pico8_value_ref(NULL);
        extra->row.jb_row = pico8_menu_invented_lines(fm->invented);
    }
    for (i = 0; i < n; i++) {
        bs[i] = rows[i].row;
    }

    cr = pico8_pj_cr();
    if (cr != NULL) {
        pico8_fixed_t ek_tl = pico8_fixed_from_int(0);
        char *tl_str;
        char *diff1_str;
        char *diff2_str;
        char *tmp1;
        char *tmp2;
        char *tmp3;
        char *ep;

        if (pico8_int_seq_at(pico8_ek_table, PICO8_EK_COUNT, pico8_fixed_flr_int(tl), &ek)) {
            ek_tl = pico8_fixed_from_int(ek);
        }

        tl_str = pico8_value_tostr(pico8_value_fixed(tl));
        diff1_str = pico8_value_tostr(pico8_value_fixed(pico8_fixed_sub(pico8_fixed_from_int(gr), qi)));
        diff2_str = pico8_value_tostr(pico8_value_fixed(pico8_fixed_sub(ek_tl, qi)));

        tmp1 = cr_concat2("level ", tl_str);
        tmp2 = cr_concat2(tmp1, " (");
        free(tmp1);
        tmp3 = cr_concat2(tmp2, diff1_str);
        free(tmp2);
        tmp1 = cr_concat2(tmp3, "/");
        free(tmp3);
        tmp2 = cr_concat2(tmp1, diff2_str);
        free(tmp1);
        ep = cr_concat2(tmp2, ")");
        free(tmp2);

        free(tl_str);
        free(diff1_str);
        free(diff2_str);

        cr->ep = ep;
        frame.ep = cr->ep;
        frame.hi = cr->hi;
        frame.has_ca = 1;
        frame.ca = cr->ca;
        frame.gq = cr->gq;
        pico8_pz(&frame, bs, n);
    }

    free(bs);
    free(rows);
}

static void pico8_ix1_my_dispatch(pico8_js_t *self) {
    pico8_hy_t *fm = pico8_fm_hy();
    (void)self;
    if (fm != NULL) {
        fm->lo = pico8_fixed_from_int(1);
    }
    pico8_hy_hk(fm);
}

static void pico8_ix2_my_dispatch(pico8_js_t *self) {
    (void)self;
    pico8_cr();
}

/* TROU-7 */
/* The bare `cb()` counts only affordable techs. pico8_cr()'s `cb(true)` is the
 * other, non-interchangeable site: passing the flag here would light the icon
 * whenever any tech of the era is left. */
static void pico8_ix2_hb_dispatch(pico8_js_t *self) {
    pico8_cb_kept_t cb_list;
    int cb_count;
    pico8_value_t item;
    pico8_lq_keys_t keys = {0};
    pico8_lq_t *built;

    if (self == NULL) {
        return;
    }

    cb_list = pico8_cb(pico8_value_ref(NULL));
    cb_count = cb_list.count;

    item = pico8_value_fixed(pico8_fixed_from_int(cb_count > 0 ? 24 : 37));
    keys.items = &item;
    keys.count = 1;
    built = pico8_lq_new(&keys);

    self->lq = built;
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
    pico8_fixed_t s;
    (void)self;
    stat_val = pico8_stat(pico8_dispatch_input(), 20);
    s = pico8_fixed_sgn(pico8_fixed_from_int(stat_val));
    pico8_music(-pico8_fixed_flr_int(s));
}

/* TROU-10 */
/* `ih` is the real global of pico8_audio.c, which pico8_rn() reads; a private
 * copy here would leave that mute check on the old value. */
static void pico8_ha3_my_dispatch(pico8_js_t *self) {
    (void)self;
    pico8_set_ih(!pico8_ih());
}

static void pico8_ha4_my_dispatch(pico8_js_t *self) {
    (void)self;
}

static const pico8_js_fn g_ix_my[PICO8_IX_COUNT] = {
    pico8_ix1_my_dispatch,
    pico8_ix2_my_dispatch,
    pico8_ix3_my_dispatch,
    pico8_ix4_my_dispatch,
};
static const pico8_js_fn g_ix_hb[PICO8_IX_COUNT] = {NULL, pico8_ix2_hb_dispatch, NULL, NULL};

/* `pz(ha,ha)`: `ha`'s rows go in as `bs`, their `my` installed on every
 * run, and `p` is the frame of `g_ha_pz`, which carries `ha`'s `hi`/`gq`/`ca`. */
static void pico8_ix4_my_dispatch(pico8_js_t *self) {
    const pico8_js_fn my[PICO8_HA_COUNT] = {
        pico8_init_field_value(),
        pico8_ha2_my_dispatch,
        pico8_ha3_my_dispatch,
        pico8_ha4_my_dispatch,
    };
    pico8_js_keys_t bs[PICO8_HA_COUNT];
    pico8_lq_keys_t lq[PICO8_HA_COUNT];
    pico8_value_t items[PICO8_HA_COUNT][PICO8_MENU_ROW_CAP];
    pico8_pz_frame_t frame = {0};
    int i;
    (void)self;

    if (!g_menu_up) {
        return;
    }
    for (i = 0; i < PICO8_HA_COUNT; i++) {
        const pico8_ha_entry_t *e = &pico8_ha_table[i];

        memset(&bs[i], 0, sizeof bs[i]);
        lq[i] = pico8_lq_keys_of_row(e->lq, items[i], PICO8_MENU_ROW_CAP);
        bs[i].lq = pico8_value_ref(NULL);
        bs[i].lq_row = &lq[i];
        bs[i].has_fg = e->has_fg;
        bs[i].fg = pico8_fixed_from_int(e->fg);
        bs[i].has_my = 1;
        bs[i].my = my[i];
    }
    frame.hi = g_ha_pz.hi;
    frame.has_ca = g_ha_pz.has_ca;
    frame.ca = g_ha_pz.ca;
    frame.gq = g_ha_pz.gq;
    pico8_pz(&frame, bs, PICO8_HA_COUNT);
}

pico8_js_t *pico8_ix_button(int i) {
    const pico8_ix_entry_t *e;
    pico8_js_keys_t keys = {0};
    pico8_lq_keys_t lq;
    pico8_value_t items[PICO8_MENU_ROW_CAP];

    assert(g_menu_up && i >= 1 && i <= PICO8_IX_COUNT && "ix has PICO8_IX_COUNT rows, after pico8_menu_init()");
    e = &pico8_ix_table[i - 1];
    lq = pico8_lq_keys_of_row(e->lq, items, PICO8_MENU_ROW_CAP);
    keys.lq = pico8_value_ref(NULL);
    keys.lq_row = &lq;
    keys.ra = &g_ix_ra[i - 1];
    keys.has_my = 1;
    keys.my = g_ix_my[i - 1];
    keys.hb = g_ix_hb[i - 1];
    return pico8_js_new(&keys);
}

pico8_js_fn pico8_ix_my(int i) {
    assert(i >= 1 && i <= PICO8_IX_COUNT);
    return g_ix_my[i - 1];
}

const void *pico8_ha_pz(void) {
    return g_menu_up ? &g_ha_pz : NULL;
}

const pico8_struct_class_t *pico8_ha_pz_struct_class(void) {
    return &g_ha_pz_struct_class;
}

void pico8_menu_init(void) {
    int i;

    pico8_menu_shutdown();

    pico8_pi_init();

    for (i = 0; i < PICO8_IX_COUNT; i++) {
        g_ix_ra[i] = pico8_vec_v(pico8_fixed_from_int(pico8_ix_table[i].ra_x),
                                 pico8_fixed_from_int(pico8_ix_table[i].ra_y));
    }
    g_ha_hi = pico8_vec_v(pico8_fixed_from_int(pico8_ha_tail.hi_x), pico8_fixed_from_int(pico8_ha_tail.hi_y));
    g_ha_gq = pico8_vec_v(pico8_fixed_from_int(pico8_ha_tail.gq_x), pico8_fixed_from_int(pico8_ha_tail.gq_y));
    g_ha_pz.hi = &g_ha_hi;
    g_ha_pz.gq = &g_ha_gq;
    g_ha_pz.ca = pico8_fixed_from_int(pico8_ha_tail.ca);
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
