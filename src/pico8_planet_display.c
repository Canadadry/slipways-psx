#include "pico8_planet_display.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_hud.h"
#include "pico8_map_tables.h"
#include "pico8_msgbox.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_probe.h"
#include "pico8_research.h"
#include "pico8_research_display.h"
#include "pico8_slipway.h"
#include "pico8_ref_audit.h"
#include "pico8_type_tag.h"

/* Lua truthiness: only `false` and nil are falsy -- a fixed 0 is truthy. */
static int pico8_jo_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* `kw.qe` as a value: nil when absent. */
static pico8_value_t jo_kw_qe(const pico8_kw_t *kw) {
    if (kw == NULL || !kw->has_qe) {
        return pico8_value_ref(NULL);
    }
    return pico8_value_fixed(kw->qe);
}

/* `kw` as `lq:mn` reads it, `{qe=kw.qe}` (`mu` is never read there). A
 * fresh row, never freed (TROU-7). */
static const pico8_jb_row_t *jo_kw_tuple(const pico8_kw_t *kw) {
    pico8_jb_row_t *t = calloc(1, sizeof *t);

    assert(t != NULL);
    t->has_qe = kw->has_qe;
    t->qe = kw->qe;
    return t;
}

void pico8_jo_hb(pico8_jo_t *self) {
    const pico8_ptrs_t *jy_list;
    const pico8_ptrs_t *by_list;
    pico8_value_t qe = pico8_value_ref(NULL);
    pico8_fixed_t mu = pico8_fixed_from_int(0);
    pico8_lq_t *lq_result;

    if (self == NULL) {
        return;
    }

    jy_list = self->jy;
    if (self->kw != NULL) {
        qe = jo_kw_qe(self->kw);
        mu = self->kw->mu;
    }
    by_list = self->by;

    lq_result = pico8_jo_ex(jy_list, qe, mu, pico8_fixed_from_int(by_list != NULL ? by_list->count : 0),
                             pico8_fixed_from_int(0));
    self->lq = lq_result;
}

typedef struct {
    pico8_list_t *ji;
} pico8_jo_gh_ctx_t;

/* Filters on `gc == 0`, and also appends the element to the `ji` list in
 * `ctx` once per `gc`: the Lua predicate has that side effect. */
static int pico8_jo_gh_jp_step(void *e, void *ctx_ptr) {
    pico8_jo_gh_ctx_t *ctx = (pico8_jo_gh_ctx_t *)ctx_ptr;
    pico8_fixed_t gc = pico8_fixed_from_int(0);
    int count;
    int j;

    if (e != NULL) {
        const pico8_jy_t *i = e;
        if (i->has_gc) {
            gc = i->gc;
        }
    }
    count = pico8_fixed_flr_int(gc);
    for (j = 0; j < count; j++) {
        pico8_list_add(ctx->ji, pico8_value_ref(e));
    }
    return gc == pico8_fixed_from_int(0);
}

/* The most items an icon row of `jo:gh`, or an `lq` row of `jo:fe`, holds. */
#define PICO8_JO_ROW_CAP 8

/* The most rows `jo:gh` passes to `pz`: the icon, needs or wants more,
 * receives, makes or exports. */
#define PICO8_JO_GH_ROWS 4

/* TROU-7 */
/* `s` followed by `$`: a fresh string, never freed; `s` itself is freed. */
static char *jo_dollar(char *s) {
    size_t len = strlen(s);
    char *with_suffix = malloc(len + 2);

    assert(with_suffix != NULL);
    memcpy(with_suffix, s, len);
    with_suffix[len] = '$';
    with_suffix[len + 1] = '\0';
    free(s);
    return with_suffix;
}

/* A planet's `pz` frame: `ep`, `gq` and `ra` nil when NULL, `ca` always. */
static pico8_pz_frame_t jo_pz_frame(const pico8_jo_t *self) {
    pico8_pz_frame_t frame = {0};

    frame.ep = self->ep;
    frame.has_ca = 1;
    frame.ca = self->ca;
    frame.gq = self->gq;
    frame.ra = self->ra;
    return frame;
}

void pico8_jo_gh(const pico8_jo_t *self) {
    pico8_js_keys_t bs[PICO8_JO_GH_ROWS];
    pico8_lq_keys_t icon;
    pico8_value_t icon_items[PICO8_JO_ROW_CAP];
    pico8_lq_keys_t h_keys;
    pico8_value_t h_item;
    int n = 0;
    const pico8_qh_t *qh;
    const pico8_jb_row_t *icon_row = NULL;
    pico8_list_t *ji;
    pico8_jo_gh_ctx_t ctx;
    const pico8_ptrs_t *jy;
    pico8_ptrs_t jx;
    pico8_pz_frame_t frame;
    int i;

    if (self == NULL) {
        return;
    }

    memset(bs, 0, sizeof bs);
    qh = pico8_qh_of(&self->ke.header);
    if (qh != NULL && qh->px != NULL) {
        icon_row = qh->px;
    } else if (self->has_mi) {
        int mi = pico8_fixed_flr_int(self->mi);

        if (mi >= 1 && mi <= PICO8_JO_DZ_COUNT) {
            icon_row = &pico8_jo_dz()[mi - 1];
        }
    }
    bs[0].lq = pico8_value_ref(NULL);
    if (icon_row != NULL) {
        icon = pico8_lq_keys_of_row(icon_row, icon_items, PICO8_JO_ROW_CAP);
        bs[0].lq_row = &icon;
    }
    bs[0].has_h_ = 1;
    bs[0].h_ = pico8_value_ref(NULL);
    if (self->has_kr) {
        int discard_color = 0;

        h_item = pico8_value_string(jo_dollar(pico8_ne(self->kr, 0, NULL, &discard_color)));
        memset(&h_keys, 0, sizeof h_keys);
        h_keys.items = &h_item;
        h_keys.count = 1;
        h_keys.has_bg = 1;
        h_keys.bg_is_bool = self->kr < pico8_fixed_from_int(0);
        h_keys.bg = self->kr >= pico8_fixed_from_int(0) ? pico8_fixed_from_int(1) : 0;
        bs[0].h_row = &h_keys;
    }
    n = 1;

    ji = malloc(sizeof *ji);
    pico8_list_init(ji);
    pico8_list_add(ji, pico8_value_string("receives:"));
    ctx.ji = ji;

    jy = self->jy;
    jx = pico8_jp_ptrs(jy, pico8_jo_gh_jp_step, &ctx);

    if (jx.count > 0) {
        pico8_list_t *merged = malloc(sizeof *merged);
        pico8_list_init(merged);
        pico8_list_add(merged, pico8_value_string("needs:"));
        for (i = 0; i < jx.count; i++) {
            pico8_list_add(merged, pico8_value_ref(jx.items[i]));
        }
        bs[n++].lq = pico8_value_ref(merged);
    } else {
        pico8_fixed_t mi_fixed = self->has_mi ? self->mi : pico8_fixed_from_int(0);
        pico8_value_t kw_qe = jo_kw_qe(self->kw);
        int r_w;
        int qe_is_r_w;

        /* TROU-3 */
        qe_is_r_w = pico8_r_lookup("w", &r_w) && pico8_value_equal(kw_qe, pico8_value_fixed(pico8_fixed_from_int(r_w)));

        if (mi_fixed < pico8_fixed_from_int(4) && jy != NULL && jy->count > 0 && !qe_is_r_w) {
            pico8_list_t *merged = malloc(sizeof *merged);
            pico8_list_init(merged);
            pico8_list_add(merged, pico8_value_string("wants more:"));
            if (qh != NULL && pico8_jo_truthy(pico8_qh_qh(qh))) {
                pico8_list_add(merged, pico8_value_ref(jy->items[jy->count - 1]));
            } else {
                for (i = 0; i < jy->count; i++) {
                    pico8_list_add(merged, pico8_value_ref(jy->items[i]));
                }
            }
            bs[n++].lq = pico8_value_ref(merged);
        }
    }
    pico8_ptrs_free(&jx);

    /* A `ji` still holding only its header means the predicate appended
     * nothing; Lua's `add(t,nil)` is a no-op, so there is no row at all,
     * not a row holding false. */
    if (ji->count >= 2) {
        bs[n++].lq = pico8_value_ref(ji);
    } else {
        pico8_list_free(ji);
        free(ji);
    }

    {
        const pico8_kw_t *kw = self->kw;
        pico8_fixed_t mu = kw != NULL ? kw->mu : pico8_fixed_from_int(0);
        pico8_value_t qe_val = jo_kw_qe(kw);

        if (mu > pico8_fixed_from_int(0)) {
            pico8_list_t *lb = malloc(sizeof *lb);
            int makes = qe_val.kind == PICO8_VALUE_FIXED && pico8_int_seq_has_fixed(pico8_jo_cq(), qe_val.as.fixed);

            pico8_list_init(lb);

            if (makes) {
                char *mu_str = pico8_value_tostr(pico8_value_fixed(mu));
                pico8_list_add(lb, pico8_value_string("makes:"));
                pico8_list_add(lb, pico8_value_string(mu_str));
            } else {
                const pico8_ptrs_t *by = self->by;
                int by_count = (by != NULL) ? by->count : 0;
                char *count_str = pico8_value_tostr(pico8_value_fixed(pico8_fixed_from_int(by_count)));
                char *mu_str = pico8_value_tostr(pico8_value_fixed(mu));
                size_t count_len = strlen(count_str);
                size_t mu_len = strlen(mu_str);
                char *combined = malloc(count_len + 1 + mu_len + 1);
                memcpy(combined, count_str, count_len);
                combined[count_len] = '/';
                memcpy(combined + count_len + 1, mu_str, mu_len + 1);
                free(count_str);
                free(mu_str);
                pico8_list_add(lb, pico8_value_string("exports:"));
                pico8_list_add(lb, pico8_value_string(combined));
            }
            pico8_list_add(lb, pico8_value_ref((void *)jo_kw_tuple(kw)));
            bs[n++].lq = pico8_value_ref(lb);
        }
    }

    /* From the second row on: the first row's `lq` is a single icon, not a
     * list. */
    for (i = 1; i < n; i++) {
        static const pico8_jb_item_t gap_items[] = {{.is_false = 1}, {.n = 1}};
        static const pico8_jb_row_t gap = {.count = 2, .items = gap_items};

        pico8_list_add(PICO8_REF_AS_LIST(bs[i].lq.as.ref), pico8_value_ref((void *)&gap));
    }

    frame = jo_pz_frame(self);
    pico8_pz(&frame, bs, n);
}

pico8_down_t pico8_jo_gv(const pico8_jo_t *self, const pico8_struct_header_t *s) {
    const char *me_val;
    pico8_down_t out = {0, NULL};

    if (self == NULL) {
        return out;
    }
    me_val = self->ke.me;
    if (me_val == NULL || strcmp(me_val, "ly") != 0) {
        return out;
    }
    /* Only a held `jc` carries `qu`: a held `bg`, `mz` or planet reads it absent. */
    if (s != NULL && s->cls == pico8_jc_struct_class() && ((const pico8_jc_t *)s)->qu == &self->ke.header) {
        pico8_jo_gh(self);
        return out;
    }
    out.stop = 1;
    return out;
}

/* C has no closures: the three values the Lua `my` callback captures ride
 * on the row as `_jo_fe_*` fields, which `pz()`'s own `js(b)` copies
 * verbatim onto the button this bridge is later dispatched on. */
static void pico8_jo_fe_my_dispatch(pico8_js_t *self) {
    pico8_jo_t *pt;
    pico8_hp_t *b;
    pico8_fixed_t od = pico8_fixed_from_int(0);

    if (self == NULL) {
        return;
    }
    pt = self->_jo_fe_pt;
    b = self->_jo_fe_b;
    if (self->has__jo_fe_od) {
        od = self->_jo_fe_od;
    }

    assert(pt != NULL && "jo:fe's my callback lost its captured planet");
    if (pico8_hy_lu(pico8_fm_hy(), od)) {
        pico8_jo_hu(pt, b);
    }
}

/* One button of `jo:fe`: its keys and the rows they point at, alive until
 * `pz` returns. */
typedef struct {
    pico8_lq_keys_t lq;
    pico8_value_t lq_items[PICO8_JO_ROW_CAP];
    pico8_lq_keys_t h;
    pico8_value_t h_item;
} pico8_jo_fe_row_t;

static void pico8_jo_fe_keys(pico8_jo_t *self, pico8_hp_t *b, pico8_js_keys_t *keys, pico8_jo_fe_row_t *row) {
    pico8_fixed_t od = pico8_hp_od(b);

    memset(keys, 0, sizeof *keys);
    if (b->lq != NULL) {
        row->lq = pico8_lq_keys_of_row(b->lq, row->lq_items, PICO8_JO_ROW_CAP);
        keys->lq_row = &row->lq;
        keys->lq = pico8_value_ref(NULL);
    } else {
        keys->lq = pico8_value_ref(pico8_hp_lq(b));
    }

    keys->has_h_ = 1;
    keys->h_ = pico8_value_bool(0);
    if (od > pico8_fixed_from_int(0)) {
        row->h_item = pico8_value_string(jo_dollar(pico8_value_tostr(pico8_value_fixed(od))));
        memset(&row->h, 0, sizeof row->h);
        row->h.items = &row->h_item;
        row->h.count = 1;
        row->h.has_c = 1;
        row->h.c = pico8_fixed_from_int(13);
        keys->h_ = pico8_value_ref(NULL);
        keys->h_row = &row->h;
    }

    keys->_jo_fe_pt = self;
    keys->_jo_fe_b = b;
    keys->has__jo_fe_od = 1;
    keys->_jo_fe_od = od;
    keys->has_my = 1;
    keys->my = pico8_jo_fe_my_dispatch;
    keys->has_jb = 1;
    keys->jb = pico8_value_ref(NULL);
    keys->jb_row = pico8_hp_jb(b);
}

void pico8_jo_fe(pico8_jo_t *self) {
    pico8_bj_kept_t bj_list;
    pico8_js_keys_t *bs;
    pico8_jo_fe_row_t *rows;
    pico8_pz_frame_t frame;
    int n = 0;
    int i;

    if (self == NULL) {
        return;
    }

    bj_list = pico8_bj((const pico8_struct_header_t *)self);
    bs = calloc(bj_list.count > 0 ? bj_list.count : 1, sizeof *bs);
    rows = calloc(bj_list.count > 0 ? bj_list.count : 1, sizeof *rows);
    assert(bs != NULL && rows != NULL);
    for (i = 0; i < bj_list.count; i++) {
        pico8_jo_fe_keys(self, bj_list.items[i], &bs[n], &rows[n]);
        n++;
    }

    frame = jo_pz_frame(self);
    pico8_pz(&frame, bs, n);
    free(bs);
    free(rows);
}

void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec_t p) {
    pico8_vec_t adjusted;
    const char *me_val;
    int px;
    int py;
    int spr_index = 43;
    const pico8_qh_t *qh;
    const pico8_nl_t *nl;

    if (self == NULL || !pico8_gd((const pico8_struct_header_t *)self)) {
        return;
    }

    /* Also decays `self.ma`. Everything below draws at `adjusted`, never
     * at the `p` that was passed in. */
    adjusted = pico8_ma(p, (pico8_struct_header_t *)self);

    me_val = self->ke.me;
    if (me_val != NULL && strcmp(me_val, "ll") == 0) {
        pico8_fixed_t t = self->ke.t;
        if (pico8_fixed_rnd(pico8_fixed_from_int(30)) < pico8_fixed_sub(t, pico8_fixed_from_int(30))) {
            return;
        }
    }

    px = pico8_fixed_flr_int(adjusted.x);
    py = pico8_fixed_flr_int(adjusted.y);

    pico8_circfill(gfx, mem, px, py, 7, 0);

    /* An absent `qw` reads as 0, i.e. the default palette block. */
    nl = pico8_nl_of(&self->ke.header);
    pico8_dv(mem, nl != NULL ? nl->qw : pico8_fixed_from_int(0));
    pico8_palt(mem, 3, 0);
    pico8_palt(mem, 0, 1);

    /* `A and B or C`: `nk.s` is never absent, so only a nil `nk` falls
     * through to `self.s`, then to the literal 43. */
    qh = pico8_qh_of(&self->ke.header);
    if (self->nk != NULL) {
        spr_index = self->nk->s;
    } else if (qh != NULL) {
        spr_index = pico8_fixed_flr_int(qh->s);
    }
    pico8_spr(gfx, mem, spr_index, px - 4, py - 4, 2, 2);
    pico8_dv_default(mem);

    if (pico8_jo_truthy(pico8_jo_gg(self))) {
        int mi = pico8_fixed_flr_int(self->has_mi ? self->mi : pico8_fixed_from_int(0));
        int color = 0;
        pico8_int_seq_at(pico8_jo_ka()->items, pico8_jo_ka()->count, mi, &color);
        pico8_circ(gfx, mem, px, py, 6, color);
        pico8_spr_default(gfx, mem, mi + 1, px - 3, py - 6);
    }

    {
        if (self->lq != NULL) {
            pico8_vec_t offset =
                pico8_vec_add(adjusted, pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(7)));
            assert(self->lq->header.cls->ol != NULL && "lq must have an `ol`");
            self->lq->header.cls->ol(self->lq, offset, PICO8_FIXED_LIT(0x00008000, 0.5));
        }
    }

    {
        if (self->fo != NULL) {
            pico8_fixed_t dw = pico8_fixed_from_int(0);
            pico8_fixed_t radius_f;
            int radius;

            if (self->fo->cls->probe_of != NULL) {
                dw = self->fo->cls->probe_of(self->fo)->dw;
            }
            radius_f = pico8_fixed_mul(pico8_fixed_sqrt(dw), pico8_fixed_from_int(128));
            radius = pico8_fixed_flr_int(radius_f);

            /* The pattern literal's fractional `.1` is PICO-8's
             * transparency flag: its set bits leave the screen untouched,
             * so this is `_transparent`, not plain `pico8_fillp`. */
            pico8_fillp_transparent(gfx, 0xAAAAu);
            pico8_circfill(gfx, mem, px, py, radius, 2);
            pico8_fillp_reset(gfx);
            self->fo = NULL;
        }
    }
}
