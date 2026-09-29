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
#include "pico8_game_data.h"
#include "pico8_hud.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_map_tables.h"
#include "pico8_msgbox.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_probe.h"
#include "pico8_score_panel_display.h"
#include "pico8_struct.h"
#include "pico8_type_tag.h"
#include "pico8_vec.h"

static int g_pj_up = 0;
static pico8_bz_row_t g_bz[PICO8_BZ_COUNT];
static pico8_pj_t *g_ju = NULL;

static pico8_fixed_t fixed_min(pico8_fixed_t a, pico8_fixed_t b) {
    return (a < b) ? a : b;
}

static pico8_fixed_t fm_field_fixed(const pico8_struct_header_t *fm, const char *name) {
    pico8_value_t v;
    if (fm != NULL && pico8_entity_get_field(fm, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static pico8_value_t fm_field_value(const pico8_struct_header_t *fm, const char *name) {
    pico8_value_t v;
    if (fm != NULL && pico8_entity_get_field(fm, name, &v)) {
        return v;
    }
    return pico8_value_ref(NULL);
}

static const pico8_qe_map_t *fm_field_qe_map(const pico8_struct_header_t *fm, const char *name) {
    pico8_value_t v;
    if (fm != NULL && pico8_entity_get_field(fm, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (const pico8_qe_map_t *)v.as.ref;
    }
    return NULL;
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

static int pico8_pj_cr_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_pj_cr_t *cr = self;

    if (strcmp(name, "hi") == 0) {
        if (cr->hi == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)cr->hi);
    } else if (strcmp(name, "gq") == 0) {
        if (cr->gq == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)cr->gq);
    } else if (strcmp(name, "ca") == 0) {
        *out_value = pico8_value_fixed(cr->ca);
    } else if (strcmp(name, "ep") == 0) {
        if (cr->ep == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(cr->ep);
    } else {
        return 0;
    }
    return 1;
}

static const pico8_struct_class_t g_pj_cr_struct_class = {
    .field = pico8_pj_cr_field,
};

static pico8_pj_cr_t g_pj_cr = {
    {PICO8_TYPE_TAG_STRUCT, &g_pj_cr_struct_class},
    &pico8_pj_cr_start.hi,
    &pico8_pj_cr_start.gq,
    0,
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
    return pico8_pj_start.i_;
}

const pico8_jb_row_t *pico8_pj_gs(void) {
    return pico8_pj_start.gs;
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

static int pico8_eg_gg_pred(pico8_value_t e, void *ctx) {
    pico8_value_t gg;
    (void)ctx;
    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    assert(pico8_type_tag_of(e.as.ref) == PICO8_TYPE_TAG_STRUCT && "cg(\"jo\") only ever holds entities");
    gg = pico8_jo_gg((const pico8_jo_t *)e.as.ref);
    if (gg.kind == PICO8_VALUE_BOOL && gg.as.boolean == 0) {
        return 0;
    }
    if (gg.kind == PICO8_VALUE_REF && gg.as.ref == NULL) {
        return 0;
    }
    return 1;
}

void pico8_eg(void) {
    pico8_mem_t *mem = pico8_dispatch_mem();
    pico8_list_t filtered;
    pico8_fixed_t ps;
    int i;

    filtered = pico8_jp(pico8_cg("jo"), pico8_eg_gg_pred, NULL);
    ps = pico8_fixed_div(pico8_fixed_from_int(filtered.count), pico8_fixed_from_int(2));
    pico8_list_free(&filtered);

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

pico8_jb_item_t pico8_pj_item_of(pico8_value_t v) {
    switch (v.kind) {
    case PICO8_VALUE_STRING:
        return (pico8_jb_item_t){.s = v.as.string};
    case PICO8_VALUE_FIXED:
        return (pico8_jb_item_t){.is_fixed = 1, .fixed = v.as.fixed};
    case PICO8_VALUE_BOOL:
        assert(!v.as.boolean && "pj writes a string, a number, false or nil into an item");
        return (pico8_jb_item_t){.is_false = 1};
    default:
        assert(v.kind == PICO8_VALUE_REF && v.as.ref == NULL &&
               "pj writes a string, a number, false or nil into an item");
        return (pico8_jb_item_t){.is_ref = 1};
    }
}

void pico8_pj_hb(void) {
    const pico8_struct_header_t *fm = pico8_fm();
    pico8_fixed_t fm_ge = fm_field_fixed(fm, "ge");
    pico8_fixed_t fm_nz = fm_field_fixed(fm, "nz");
    pico8_value_t fm_jk = fm_field_value(fm, "jk");
    pico8_fixed_t fm_happiness = fm_field_fixed(fm, "happiness");
    pico8_fixed_t fm_lo = fm_field_fixed(fm, "lo");
    pico8_value_t fm_yr = fm_field_value(fm, "yr");
    const pico8_qe_map_t *fm_po = fm_field_qe_map(fm, "po");
    pico8_pj_ou_t *ou_state;
    pico8_value_t ou[7];
    char *cf;
    char *nz_str;
    char *happiness_str;
    char *plus_num;
    int discard_color = 0;
    int i;

    cf = pico8_ne(fm_ge, 0, NULL, &discard_color);

    nz_str = pico8_value_tostr(pico8_value_fixed(fm_nz));
    ou[0] = pico8_value_string(pj_concat2(nz_str, "$"));
    free(nz_str);

    ou[1] = pico8_value_string(cf);

    ou[2] = fm_jk;

    {
        int k_key;
        pico8_fixed_t po_val = pico8_fixed_from_int(0);
        if (pico8_r_lookup("k", &k_key)) {
            pico8_qe_map_get(fm_po, pico8_fixed_from_int(k_key), &po_val);
        }
        /* `..` binds looser than `+`: the Lua reads `"+"..(fm.po[r.k]+1)`. */
        plus_num =
            pico8_value_tostr(pico8_value_fixed(pico8_fixed_add(po_val, pico8_fixed_from_int(1))));
        ou[3] = pico8_value_string(pj_concat2("+", plus_num));
        free(plus_num);
    }

    happiness_str = pico8_value_tostr(pico8_value_fixed(fm_happiness));
    ou[4] = pico8_value_string(pj_concat2(happiness_str, "%"));
    free(happiness_str);

    {
        int lo = pico8_fixed_flr_int(fm_lo);
        ou[5] = (lo >= 1 && lo <= PICO8_EF_COUNT) ? pico8_value_string(pico8_ef_table[lo - 1]) : pico8_value_ref(NULL);
    }

    ou[6] = fm_yr;

    ou_state = pico8_pj_ou();
    if (ou_state != NULL) {
        for (i = 0; i < 7; i++) {
            /* TROU-7 */
            ou_state->sub_items[i][0] = pico8_pj_item_of(ou[i]);
        }
    }

    pico8_eg();
}

static void pico8_pj_hb_dispatch(void *self) {
    (void)self;
    pico8_pj_hb();
}

void pico8_pj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_pj_t *self) {
    const pico8_jb_row_t *i_ = pico8_pj_i_();
    pico8_pj_ou_t *ou_state = pico8_pj_ou();
    pico8_list_t combined;
    int i;

    pico8_list_init(&combined);
    for (i = 0; i < i_->count; i++) {
        pico8_list_add(&combined, pico8_value_ref((void *)i_->items[i].row));
    }
    for (i = 0; ou_state != NULL && i < ou_state->row.count; i++) {
        pico8_list_add(&combined, pico8_value_ref((void *)ou_state->row.items[i].row));
    }

    pico8_bl_default(gfx, mem, &combined);

    pico8_list_free(&combined);

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

static void pico8_pj_hk_dispatch(void *self, pico8_fixed_t yr) {
    (void)self;
    pico8_pj_hk(yr);
}

static void pico8_pj_ph_dispatch(void *self, pico8_fixed_t t) {
    (void)t;
    pico8_pj_ph((pico8_pj_t *)self);
}

pico8_value_t pico8_pj_ed(const pico8_pj_t *self) {
    if (self == NULL) {
        return pico8_value_ref(NULL);
    }
    if (self->ed == NULL) {
        return pico8_value_fixed(pico8_pj_start.ed);
    }
    return pico8_value_ref(self->ed);
}

/* `ou`, `ea`, `cr` read off a panel answer the shared state, as the class
 * chain did. */
static int pico8_pj_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_pj_t *pj = self;
    pico8_value_t value;

    if (strcmp(name, "ou") == 0) {
        value = pico8_value_ref(pico8_pj_ou());
    } else if (strcmp(name, "ea") == 0) {
        value = pico8_value_ref(pico8_pj_ea());
    } else if (strcmp(name, "cr") == 0) {
        value = pico8_value_ref(pico8_pj_cr());
    } else if (strcmp(name, "fa") == 0) {
        value = pico8_value_fixed(pj->fa);
    } else if (strcmp(name, "gs") == 0) {
        if (pj->gs == NULL) {
            return 0;
        }
        value = pico8_value_ref((void *)pj->gs);
    } else if (strcmp(name, "i_") == 0) {
        if (pj->i_ == NULL) {
            return 0;
        }
        value = pico8_value_ref((void *)pj->i_);
    } else if (strcmp(name, "ed") == 0) {
        value = pico8_pj_ed(pj);
    } else if (strcmp(name, "c_") == 0) {
        if (!pj->has_c_) {
            return 0;
        }
        value = pico8_value_bool(pj->c_);
    } else if (strcmp(name, "gz") == 0) {
        if (!pj->has_gz) {
            return 0;
        }
        value = pico8_value_fixed(pj->gz);
    } else {
        return pico8_ke_field(self, name, out_value);
    }
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return 0;
    }
    *out_value = value;
    return 1;
}

static const pico8_struct_class_t g_pj_struct_class = {
    .kj = pico8_pj_kj_dispatch,
    .hk = pico8_pj_hk_dispatch,
    .hb = pico8_pj_hb_dispatch,
    .states = {[PICO8_STATE_PH] = pico8_pj_ph_dispatch},
    .field = pico8_pj_field,
};

static void *pico8_pj_struct_new(void) {
    pico8_pj_t *self;

    assert(g_pj_up && "pico8_score_panel_init() must run first");
    self = malloc(sizeof *self);
    pico8_ke_struct_init(&self->ke, &g_pj_struct_class);
    self->fa = pico8_pj_start.fa;
    self->gs = pico8_pj_start.gs;
    self->i_ = pico8_pj_start.i_;
    self->ed = NULL;
    self->c_ = 0;
    self->has_c_ = 0;
    self->gz = 0;
    self->has_gz = 0;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("pj");
#endif
    pico8_ke_struct_pq(&self->ke);
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
    pj_row_copy(&g_pj_ou.row, g_pj_ou.items, 7, pico8_pj_start.ou);
    for (i = 0; i < g_pj_ou.row.count; i++) {
        pj_row_copy(&g_pj_ou.subs[i], g_pj_ou.sub_items[i], 4, pico8_pj_start.ou->items[i].row);
        g_pj_ou.items[i].row = &g_pj_ou.subs[i];
    }
    pj_row_copy(&g_pj_ea.row, g_pj_ea.items, 3, pico8_pj_start.ea);
    pj_row_copy(&g_pj_ea.second, g_pj_ea.second_items, 5, pico8_pj_start.ea->items[1].row);
    g_pj_ea.items[1].row = &g_pj_ea.second;
    g_pj_ea.lh = NULL;
    g_pj_cr.ca = pico8_pj_start.cr->ca;
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
