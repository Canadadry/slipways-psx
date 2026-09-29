#include "pico8_score_panel_display.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_economy_score.h"
#include "pico8_entity.h"
#include "pico8_layout.h"
#include "pico8_list.h"
#include "pico8_msgbox.h"
#include "pico8_probe.h"
#include "pico8_score_panel.h"
#include "pico8_ui_button.h"


/* Returns a newly allocated string the caller owns and must free(). */
static char *pj_concat2(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char *out = malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

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
    char *year_label;
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

    yr_str = pico8_value_tostr(pico8_value_fixed(yr));
    year_label = pj_concat2("year ", yr_str);
    free(yr_str);

    iw->second_items[0] = (pico8_jb_item_t){.s = year_label};

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

/* A position of a row of `bz` as the value `lq` borrows. */
static pico8_value_t pico8_pj_ph_item_value(const pico8_jb_item_t *item) {
    if (item->s != NULL) {
        return pico8_value_string(item->s);
    }
    if (item->row != NULL) {
        return pico8_value_ref((void *)item->row);
    }
    if (item->is_false) {
        return pico8_value_bool(0);
    }
    if (item->is_fixed) {
        return pico8_value_fixed(item->fixed);
    }
    return pico8_value_fixed(pico8_fixed_from_int(item->n));
}

/* `s[i]`: absent outside 1..6. */
static pico8_value_t pico8_pj_ph_s_at(const pico8_ky_t *s, int i) {
    switch (i) {
    case 1:
        return pico8_value_fixed(s->di);
    case 2:
        return pico8_value_fixed(s->gl);
    case 3:
        return pico8_value_fixed(s->el);
    case 4:
        return pico8_value_string(s->happiness);
    case 5:
        return pico8_value_fixed(s->total);
    case 6:
        return pico8_value_fixed(s->tier);
    default:
        return pico8_value_ref(NULL);
    }
}

/* An article `ld` of `bl` and its items, in one block, never freed (TROU-7). */
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
    pico8_value_t s_val;
    char *s_str;
    pico8_value_t items[5];
    pico8_lq_keys_t keys = {0};
    pico8_lq_t *lq_inst;
    pico8_pj_ph_ld_t *wrapped;
    int k;

    if (!st->e->has_score) {
        return &st->row;
    }

    score_i = st->e->score;

    s_val = pico8_pj_ph_s_at(ctx->s, score_i);
    /* `s[0]` is genuinely absent for score 0, and this port has no nil to
     * format: `pico8_value_tostr` renders the absent filler as "", which
     * PICO-8's own tostr would not. */
    s_str = pico8_value_tostr(s_val);
    /* TROU-7 */
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
        piece1 = pj_concat2(hl_str, " (");
        piece2 = pj_concat2(piece1, px);
        free(piece1);
        label = pj_concat2(piece2, ")");
        free(piece2);
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
            items[keys.count++] = pico8_pj_ph_item_value(&st->items[k]);
        }
    }
    keys.items = items;
    keys.has_c = st->row.has_c;
    keys.c = pico8_fixed_from_int(st->row.c);
    keys.has_score = 1;
    keys.score = pico8_fixed_from_int(score_i);
    keys.ra = st->e->has_ra ? &st->ra : NULL;
    /* TROU-7 */
    lq_inst = pico8_lq_new(&keys);
    wrapped = malloc(sizeof *wrapped);
    assert(wrapped != NULL);
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
        pico8_list_t lq1;
        pico8_list_t lq2;
        pico8_js_t *made1;
        pico8_js_t *made2;

        (void)pico8_fq_new(NULL);

        pico8_list_init(&lq1);
        pico8_list_add(&lq1, pico8_value_string(" again? "));
        spec1.has_fa = 1;
        spec1.fa = pico8_fixed_from_int(15);
        /* TROU-7 */
        spec1.ra = malloc(sizeof *spec1.ra);
        *spec1.ra = pico8_vec_v(pico8_fixed_from_int(51), pico8_fixed_from_int(100));
        spec1.lq = pico8_value_ref(&lq1);
        spec1.has_my = 1;
        spec1.my = pico8_init_field_value();
        made1 = pico8_js_new(&spec1);
        (void)made1;
        pico8_list_free(&lq1);

        pico8_list_init(&lq2);
        pico8_list_add(&lq2, pico8_value_ref((void *)&back));
        spec2.has_fa = 1;
        spec2.fa = pico8_fixed_from_int(15);
        /* TROU-7 */
        spec2.ra = malloc(sizeof *spec2.ra);
        *spec2.ra = pico8_vec_v(pico8_fixed_from_int(39), pico8_fixed_from_int(100));
        spec2.lq = pico8_value_ref(&lq2);
        spec2.has_my = 1;
        spec2.my = pico8_pj_de_js_my_dispatch;
        made2 = pico8_js_new(&spec2);
        (void)made2;
        pico8_list_free(&lq2);

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

        /* `self.ed`'s class-level default is the number 1, never a REF, and a
         * non-REF `ed` is never `kc`: the block has to fire on the first
         * eligible tick. Comparing only a REF-valued `ed` silently disables it. */
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
