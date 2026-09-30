#include "pico8_score_panel.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy_formulas.h"
#include "pico8_entity.h"
#include "pico8_fm.h"
#include "pico8_game_data.h"
#include "pico8_hud.h"
#include "pico8_ke.h"
#include "pico8_map_tables.h"
#include "pico8_msgbox.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_str.h"
#include "pico8_struct.h"
#include "pico8_vec.h"

static int g_pj_up = 0;
static pico8_bz_row_t g_bz[PICO8_BZ_COUNT];
static pico8_pj_t *g_ju = NULL;

static pico8_fixed_t fixed_min(pico8_fixed_t a, pico8_fixed_t b) {
    return (a < b) ? a : b;
}

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

/* pj's class state `ou` and `ea`, reset by each init to their generated
 * rows and rewritten in place by `hb`/`hk`. */
static pico8_pj_ou_t g_pj_ou;
static pico8_pj_ea_t g_pj_ea;

/* Copies `src` into `dst` and its item storage (`cap` items), `fn` and `c`
 * included; nested rows stay the generated ones. */
static void pj_row_copy(pico8_jb_row_t *dst, pico8_jb_item_t *items, int cap, const pico8_jb_row_t *src) {
    int i;

    assert(src->count <= cap && "a copied row of pj holds no more items than its generated row");
    for (i = 0; i < src->count; i++) {
        items[i] = src->items[i];
    }
    *dst = *src;
    dst->items = items;
}

static const pico8_struct_class_t g_pj_cr_struct_class = {0};

static pico8_pj_cr_t g_pj_cr = {
    {.cls = &g_pj_cr_struct_class},
    &(const pico8_vec_t){0x0, 0xb0000},
    &(const pico8_vec_t){0x0, 0xb0000},
    0xa0000,
    NULL,
};

pico8_pj_ou_t *pico8_pj_ou(void) {
    return g_pj_up ? &g_pj_ou : NULL;
}

pico8_pj_ea_t *pico8_pj_ea(void) {
    return g_pj_up ? &g_pj_ea : NULL;
}

pico8_pj_cr_t *pico8_pj_cr(void) {
    return g_pj_up ? &g_pj_cr : NULL;
}

const pico8_struct_class_t *pico8_pj_cr_struct_class(void) {
    return &g_pj_cr_struct_class;
}

const pico8_jb_row_t *pico8_pj_i_(void) {
    return pico8_pj_def.i_;
}

const pico8_jb_row_t *pico8_pj_gs(void) {
    return pico8_pj_def.gs;
}

static void pico8_pj_init_placeholder(pico8_js_t *self) {
    (void)self;
}

static pico8_js_fn g_init_dispatch = pico8_pj_init_placeholder;

pico8_js_fn pico8_init_field_value(void) {
    return g_init_dispatch;
}

void pico8_score_panel_set_init(pico8_js_fn fn) {
    g_init_dispatch = (fn != NULL) ? fn : pico8_pj_init_placeholder;
}

static int pico8_eg_gg_pred(pico8_struct_header_t *e, void *ctx) {
    (void)ctx;
    if (e == NULL) {
        return 0;
    }
    return pico8_jo_gg((const pico8_jo_t *)e);
}

void pico8_eg(void) {
    pico8_mem_t *mem = pico8_dispatch_mem();
    pico8_ents_t filtered;
    pico8_fixed_t ps;
    int i;

    filtered = pico8_jp_ents(pico8_cg("jo"), pico8_eg_gg_pred, NULL);
    ps = pico8_fixed_div(pico8_fixed_from_int(filtered.count), pico8_fixed_from_int(2));
    pico8_ents_free(&filtered);

    if (mem == NULL) {
        return;
    }

    for (i = 1; i <= PICO8_FB_COUNT; i++) {
        int ny = pico8_fb_table[i - 1];
        int pd = 0x3101 + 4 * ny;
        unsigned char cur = pico8_peek(mem, pd);
        int newval = (cur % 128) + ((ps < pico8_fixed_from_int(i)) ? 128 : 0);
        pico8_poke(mem, pd, (unsigned char)newval);
    }
}

void pico8_pj_hb(void) {
    const pico8_hy_t *fm = pico8_fm_hy();
    const pico8_fm_placeholder_t *provisional = pico8_fm_placeholder();
    pico8_fixed_t fm_ge = fm != NULL ? fm->ge : pico8_fixed_from_int(0);
    pico8_fixed_t fm_nz = pico8_fixed_from_int(0);
    pico8_jb_item_t fm_jk = {.is_ref = 1};
    pico8_fixed_t fm_happiness = fm != NULL && fm->has_happiness ? fm->happiness : pico8_fixed_from_int(0);
    pico8_fixed_t fm_lo = fm != NULL ? fm->lo : pico8_fixed_from_int(0);
    pico8_jb_item_t fm_yr = fm != NULL ? (pico8_jb_item_t){.is_fixed = 1, .fixed = fm->yr}
                                       : (pico8_jb_item_t){.is_ref = 1};
    const pico8_qe_map_t *fm_po = fm != NULL ? fm->po : NULL;
    pico8_pj_ou_t *ou_state;
    pico8_jb_item_t ou[7];
    char *cf;
    char *nz_str;
    char *happiness_str;
    char *plus_num;
    int discard_color = 0;
    int i;

    if (fm != NULL) {
        fm_nz = fm->nz;
        fm_jk = (pico8_jb_item_t){.is_fixed = 1, .fixed = fm->jk};
    } else if (provisional != NULL) {
        if (provisional->has_nz) {
            fm_nz = provisional->nz;
        }
        if (provisional->has_jk) {
            fm_jk = (pico8_jb_item_t){.is_fixed = 1, .fixed = provisional->jk};
        }
    }

    cf = pico8_ne(fm_ge, 0, NULL, &discard_color);

    nz_str = pico8_tostr_fixed(fm_nz);
    ou[0] = (pico8_jb_item_t){.s = pj_concat2(nz_str, "$")};
    free(nz_str);

    ou[1] = (pico8_jb_item_t){.s = cf};

    ou[2] = fm_jk;

    {
        int k_key;
        pico8_fixed_t po_val = pico8_fixed_from_int(0);
        if (pico8_r_lookup("k", &k_key)) {
            pico8_qe_map_get(fm_po, pico8_fixed_from_int(k_key), &po_val);
        }
        /* `..` binds looser than `+`: the Lua reads `"+"..(fm.po[r.k]+1)`. */
        plus_num =
            pico8_tostr_fixed(pico8_fixed_add(po_val, pico8_fixed_from_int(1)));
        ou[3] = (pico8_jb_item_t){.s = pj_concat2("+", plus_num)};
        free(plus_num);
    }

    happiness_str = pico8_tostr_fixed(fm_happiness);
    ou[4] = (pico8_jb_item_t){.s = pj_concat2(happiness_str, "%")};
    free(happiness_str);

    {
        int lo = pico8_fixed_flr_int(fm_lo);
        ou[5] = (lo >= 1 && lo <= PICO8_EF_COUNT) ? (pico8_jb_item_t){.s = pico8_ef_table[lo - 1]}
                                                  : (pico8_jb_item_t){.is_ref = 1};
    }

    ou[6] = fm_yr;

    ou_state = pico8_pj_ou();
    if (ou_state != NULL) {
        for (i = 0; i < 7; i++) {
            /* TROU-7 */
            ou_state->sub_items[i][0] = ou[i];
        }
    }

    pico8_eg();
}

static void pico8_pj_hb_dispatch(void *self) {
    (void)self;
    pico8_pj_hb();
}

/* `la(pj.i_, pj.ou)`: 5 then 7 articles in the cart. */
enum { PICO8_PJ_KJ_ARTICLES_CAP = 5 + 7 };

void pico8_pj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_pj_t *self) {
    const pico8_jb_row_t *i_ = pico8_pj_i_();
    pico8_pj_ou_t *ou_state = pico8_pj_ou();
    const pico8_jb_row_t *combined[PICO8_PJ_KJ_ARTICLES_CAP];
    int n = 0;
    int i;

    assert(i_->count + (ou_state != NULL ? ou_state->row.count : 0) <= PICO8_PJ_KJ_ARTICLES_CAP &&
           "la(pj.i_, pj.ou) is 5 + 7 articles in the cart");
    for (i = 0; i < i_->count; i++) {
        combined[n++] = i_->items[i].row;
    }
    for (i = 0; ou_state != NULL && i < ou_state->row.count; i++) {
        combined[n++] = ou_state->row.items[i].row;
    }

    pico8_bl_default(gfx, mem, combined, n);

    if (self != NULL && self->has_gz) {
        pico8_fixed_t gz = pico8_fixed_add(self->gz, PICO8_FIXED_LIT(0x00003333, 0.2));
        pico8_fixed_t capped = fixed_min(gz, PICO8_FIXED_LIT(0x00033333, 3.2));
        pico8_fixed_t h = pico8_fixed_mul(capped, capped);

        self->gz = gz;
        pico8_rk(gfx, mem, pico8_fixed_from_int(0), h, 0);
        /* The cart's only colour-less `rk`: the bottom bar inherits the pen
         * colour the line above leaves behind. Passing a colour here would
         * change what is drawn. */
        pico8_rk_default(gfx, mem, pico8_fixed_sub(pico8_fixed_from_int(127), h), pico8_fixed_from_int(127));
    }
}

static void pico8_pj_kj_dispatch(void *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_pj_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_pj_t *)self);
}

static const pico8_struct_class_t g_pj_struct_class = {
    .kj = pico8_pj_kj_dispatch,
    .hk = pico8_pj_hk_dispatch,
    .hb = pico8_pj_hb_dispatch,
    .states = {[PICO8_STATE_PH] = pico8_pj_ph_dispatch},
};

static pico8_pj_t *pico8_pj_struct_new(void) {
    pico8_pj_t *self;

    assert(g_pj_up && "pico8_score_panel_init() must run first");
    self = malloc(sizeof *self);
    pico8_ke_struct_init(&self->header, &g_pj_struct_class, pico8_pj_def.me, pico8_pj_def.t);
    self->def = &pico8_pj_def;
    self->fa = self->def->fa;
    self->header.fa = &self->fa;
    self->ed = NULL;
    self->c_ = 0;
    self->has_c_ = 0;
    self->gz = 0;
    self->has_gz = 0;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("pj");
#endif
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_score_panel_init(void) {
    int i;

    pico8_score_panel_shutdown();

    pico8_pi_init();

    for (i = 0; i < PICO8_BZ_COUNT; i++) {
        const pico8_bz_entry_t *e = &pico8_bz_table[i];
        pico8_bz_row_t *b = &g_bz[i];
        int k;

        pj_row_copy(&b->row, b->items, 5, &e->row);
        b->row.fn = e->fn;
        for (k = 0; k < 5; k++) {
            b->has[k] = k < e->row.count;
        }
        b->ra = pico8_vec_v(pico8_fixed_from_int(e->ra_x), pico8_fixed_from_int(e->ra_y));
        b->e = e;
    }

    g_pj_up = 1;
    pj_row_copy(&g_pj_ou.row, g_pj_ou.items, 7, &pico8_pj_ou_row);
    for (i = 0; i < g_pj_ou.row.count; i++) {
        pj_row_copy(&g_pj_ou.subs[i], g_pj_ou.sub_items[i], 4, pico8_pj_ou_row.items[i].row);
        g_pj_ou.items[i].row = &g_pj_ou.subs[i];
    }
    pj_row_copy(&g_pj_ea.row, g_pj_ea.items, 3, &pico8_pj_ea_row);
    pj_row_copy(&g_pj_ea.second, g_pj_ea.second_items, 5, pico8_pj_ea_row.items[1].row);
    g_pj_ea.items[1].row = &g_pj_ea.second;
    g_pj_ea.lh = NULL;
    g_pj_cr.ep = NULL;

    pico8_score_panel_set_init(NULL);
    g_ju = NULL;

    pico8_score_panel_display_init();
}

void pico8_score_panel_shutdown(void) {
    pico8_score_panel_display_shutdown();
    g_pj_up = 0;
    pico8_score_panel_set_init(NULL);
    g_ju = NULL;
}

int pico8_pj_ready(void) {
    return g_pj_up;
}

const pico8_struct_class_t *pico8_pj_struct_class(void) {
    return &g_pj_struct_class;
}

pico8_pj_t *pico8_pj_new(void) {
    return pico8_pj_struct_new();
}

pico8_bz_row_t *pico8_pj_bz(void) {
    return g_pj_up ? g_bz : NULL;
}

pico8_pj_t *pico8_ju(void) {
    return g_ju;
}

void pico8_pj_set_ju(pico8_pj_t *instance) {
    g_ju = instance;
}
