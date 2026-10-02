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
static int g_pj_hk_lh_t = 0;

static int pico8_pj_hk_lh_dispatch(pico8_mh_t *self) {
    (void)self;
    g_pj_hk_lh_t -= 1;
    return g_pj_hk_lh_t > 0;
}

void pico8_pj_hk(pico8_fixed_t yr) {
    pico8_pj_ea_t *iw = pico8_pj_ea();
    const pico8_jb_row_t *gs = pico8_pj_gs();
    const pico8_jb_row_t *pp = NULL;
    char *yr_str;
    int idx;

    if (iw == NULL) {
        return;
    }

    idx = 3426 - pico8_fixed_flr_int(yr);
    if (idx >= 1 && idx <= gs->count) {
        const pico8_jb_item_t *item = &gs->items[idx - 1];

        assert((item->row != NULL || item->is_false) && "gs holds rows and false only");
        pp = item->row;
    }

    yr_str = pico8_tostr_fixed(pico8_zone_frame(), yr);
    assert(strlen("year ") + strlen(yr_str) <= PICO8_PJ_TEXT_CAP && "a year is at most a 5-digit integer");
    strcpy(iw->year, "year ");
    strcat(iw->year, yr_str);

    iw->second_items[0] = (pico8_jb_item_t){.s = iw->year};

    if (pp != NULL) {
        iw->items[2] = (pico8_jb_item_t){.row = pp};
        iw->row.count = 3;
        g_pj_hk_lh_t = 90;
        iw->lh = pico8_pj_hk_lh_dispatch;
    } else {
        iw->row.count = 2;
    }

    {
        const pico8_jb_row_t *items[3];
        pico8_mh_keys_t keys = {0};
        pico8_mh_t *made;
        int i;

        for (i = 0; i < iw->row.count; i++) {
            items[i] = iw->row.items[i].row;
        }
        keys.items = items;
        keys.count = iw->row.count;
        keys.lh = iw->lh;
        made = pico8_mh_new(&keys);
        (void)made;
    }
}

/* `de.my`'s body: `pj:ph` stores it in a button, `fq(de)` in a veil, each
 * through the variant of its own callback type. */
static void pico8_pj_de_toggle(void) {
    pico8_pj_t *ju = pico8_ju();
    if (ju != NULL) {
        ju->c_ = !(ju->has_c_ && ju->c_);
        ju->has_c_ = 1;
    }
}

static int pico8_pj_de_my_dispatch(pico8_fq_t *self) {
    (void)self;
    pico8_pj_de_toggle();
    return 1;
}

static void pico8_pj_de_js_my_dispatch(pico8_js_t *self) {
    (void)self;
    pico8_pj_de_toggle();
}

/* `self` inside `pj:ph` is always `ju` -- `pj()` is constructed exactly once
 * in the cart -- so reading `ju.c_` here IS reading the captured upvalue. */
static int pico8_pj_ph_lh_dispatch(pico8_mh_t *self) {
    const pico8_pj_t *ju = pico8_ju();
    (void)self;
    return ju != NULL && ju->has_c_ && ju->c_;
}

typedef struct {
    const pico8_ky_t *s;
} pico8_pj_ph_ctx_t;

/* `tostr(s[i])`, in the frame zone. */
static char *pico8_pj_ph_s_str(const pico8_ky_t *s, int i) {
    switch (i) {
    case 1:
        return pico8_tostr_fixed(pico8_zone_frame(), s->di);
    case 2:
        return pico8_tostr_fixed(pico8_zone_frame(), s->gl);
    case 3:
        return pico8_tostr_fixed(pico8_zone_frame(), s->el);
    case 4:
        return pico8_concat2(pico8_zone_frame(), s->happiness, "");
    case 5:
        return pico8_tostr_fixed(pico8_zone_frame(), s->total);
    case 6:
        return pico8_tostr_fixed(pico8_zone_frame(), s->tier);
    default:
        /* TROU-13 */
        return pico8_concat2(pico8_zone_frame(), "", "");
    }
}

/* An article `ld` of `bl` and its items, in one block. */
typedef struct {
    pico8_jb_row_t row;
    pico8_jb_item_t items[3];
} pico8_pj_ph_ld_t;

static void pico8_pj_ph_set(pico8_bz_row_t *st, int i, pico8_jb_item_t item) {
    st->items[i - 1] = item;
    st->has[i - 1] = 1;
}

/* One row of `bz` as `ph` shows it: a row without a score is its own
 * article; one with a score gets `s[score]` at 3 (and more for 0 and 6),
 * then becomes the line of an article `ld`. */
static const pico8_jb_row_t *pico8_pj_ph_map(pico8_bz_row_t *st, const pico8_pj_ph_ctx_t *ctx) {
    int score_i;
    char *s_str;
    pico8_lq_cell_t items[5];
    pico8_lq_keys_t keys = {0};
    pico8_lq_t *lq_inst;
    pico8_pj_ph_ld_t *wrapped;
    int k;

    if (!st->e->has_score) {
        return &st->row;
    }

    score_i = st->e->score;

    s_str = pico8_pj_ph_s_str(ctx->s, score_i);
    pico8_pj_ph_set(st, 3, (pico8_jb_item_t){.s = s_str});

    if (score_i == 0) {
        const pico8_hy_t *fm = pico8_fm_hy();
        const char *hl_str = fm != NULL && fm->has_hl && fm->hl != NULL ? fm->hl : "current score";
        const pico8_js_t *dk = pico8_dk();
        const char *px = "";
        char *piece1;
        char *piece2;
        char *label;

        if (dk != NULL && dk->px != NULL) {
            px = dk->px;
        }
        piece1 = pico8_concat2(pico8_zone_frame(), hl_str, " (");
        piece2 = pico8_concat2(pico8_zone_frame(), piece1, px);
        label = pico8_concat2(pico8_zone_frame(), piece2, ")");
        pico8_pj_ph_set(st, 1, (pico8_jb_item_t){.s = label});
    }

    if (score_i == 6) {
        int i;
        for (i = 1; i <= 5; i++) {
            int icon = (i <= pico8_fixed_flr_int(ctx->s->tier)) ? 56 : 57;
            pico8_pj_ph_set(st, i, (pico8_jb_item_t){.n = icon});
        }
    }

    /* TROU-25 */
    for (k = 0; k < 5; k++) {
        if (st->has[k]) {
            items[keys.count++] = pico8_lq_cell_item(&st->items[k]);
        }
    }
    keys.items = items;
    keys.has_c = st->row.has_c;
    keys.c = pico8_fixed_from_int(st->row.c);
    keys.has_score = 1;
    keys.score = pico8_fixed_from_int(score_i);
    keys.ra = st->e->has_ra ? &st->ra : NULL;
    lq_inst = pico8_lq_new(pico8_zone_frame(), &keys);
    wrapped = (pico8_pj_ph_ld_t *)pico8_zone_alloc(pico8_zone_frame(), sizeof *wrapped);
    wrapped->items[0] = (pico8_jb_item_t){.is_ref = 1, .ref = lq_inst};
    wrapped->items[1] = (pico8_jb_item_t){.is_ref = 1, .ref = keys.ra};
    wrapped->items[2] = (pico8_jb_item_t){.is_fixed = 1, .fixed = PICO8_FIXED_LIT(0x00008000, 0.5)};
    wrapped->row = (pico8_jb_row_t){.count = 3, .items = wrapped->items, .fn = "ld"};
    return &wrapped->row;
}

void pico8_pj_ph(pico8_pj_t *self) {
    const pico8_hy_t *fm = pico8_fm_hy();
    int has_hl = fm != NULL && fm->has_hl && fm->hl != NULL;
    int has_gz = self != NULL && self->has_gz;

    if (has_hl && !has_gz) {
        static const pico8_jb_item_t back_items[] = {{.n = 10}, {.n = 7}};
        static const pico8_jb_row_t back = {.count = 2, .items = back_items};
        pico8_js_keys_t spec1 = {0};
        pico8_js_keys_t spec2 = {0};
        pico8_lq_cell_t lq1_item;
        pico8_lq_cell_t lq2_item;
        pico8_lq_keys_t lq1 = {0};
        pico8_lq_keys_t lq2 = {0};
        pico8_js_t *made1;
        pico8_js_t *made2;
        const pico8_vec_t ra1 = pico8_vec_v(pico8_fixed_from_int(51), pico8_fixed_from_int(100));
        const pico8_vec_t ra2 = pico8_vec_v(pico8_fixed_from_int(39), pico8_fixed_from_int(100));

        (void)pico8_fq_new(NULL);

        lq1_item = pico8_lq_cell_string(" again? ");
        lq1.items = &lq1_item;
        lq1.count = 1;
        spec1.has_fa = 1;
        spec1.fa = pico8_fixed_from_int(15);
        spec1.ra = &ra1;
        spec1.lq_row = &lq1;
        spec1.has_my = 1;
        spec1.my = pico8_init_field_value();
        made1 = pico8_js_new(&spec1);
        (void)made1;

        lq2_item = pico8_lq_cell_row(&back);
        lq2.items = &lq2_item;
        lq2.count = 1;
        spec2.has_fa = 1;
        spec2.fa = pico8_fixed_from_int(15);
        spec2.ra = &ra2;
        spec2.lq_row = &lq2;
        spec2.has_my = 1;
        spec2.my = pico8_pj_de_js_my_dispatch;
        made2 = pico8_js_new(&spec2);
        (void)made2;

        if (self != NULL) {
            self->c_ = 1;
            self->has_c_ = 1;
            self->gz = 0;
            self->has_gz = 1;
            self->fa = pico8_fixed_from_int(13);
        }
    }

    {
        int c_truthy = self != NULL && self->has_c_ && self->c_;
        int ed_equals_kc = 0;

        /* A NULL `ed` is the class default, the number 1, which is never
         * `kc`, not even a nil one: without the `ed != NULL` test, NULL == NULL
         * would keep the block from firing on the first eligible tick. */
        if (self != NULL) {
            ed_equals_kc = self->ed != NULL && pico8_kc() == self->ed;
        }

        if (c_truthy && !ed_equals_kc) {
            pico8_ky_t s = pico8_ky();
            pico8_bz_row_t *bz = pico8_pj_bz();
            pico8_pj_ph_ctx_t ctx;
            const pico8_jb_row_t *iw[PICO8_BZ_COUNT];
            pico8_mh_keys_t keys = {0};
            pico8_mh_t *made;
            int i;

            ctx.s = &s;
            for (i = 0; bz != NULL && i < PICO8_BZ_COUNT; i++) {
                iw[i] = pico8_pj_ph_map(&bz[i], &ctx);
            }

            keys.items = iw;
            keys.count = bz != NULL ? PICO8_BZ_COUNT : 0;
            keys.has_rh = 1;
            keys.rh = pico8_fixed_from_int(3);
            keys.lh = pico8_pj_ph_lh_dispatch;
            made = pico8_mh_new(&keys);

            if (self != NULL) {
                self->ed = made;
            }
        }
    }
}

void pico8_score_panel_display_init(void) {
    pico8_score_panel_display_shutdown();
    g_pj_hk_lh_t = 0;
}

void pico8_score_panel_display_shutdown(void) {
}

pico8_fq_my_fn pico8_de_my(void) {
    return pico8_pj_de_my_dispatch;
}

void pico8_pj_hk_dispatch(entity_t *self, pico8_fixed_t yr) {
    (void)self;
    pico8_pj_hk(yr);
}

void pico8_pj_ph_dispatch(entity_t *self, pico8_fixed_t t) {
    (void)t;
    pico8_pj_ph((pico8_pj_t *)self);
}
