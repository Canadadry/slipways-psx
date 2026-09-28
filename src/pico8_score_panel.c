#include "pico8_score_panel.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_game_data.h"
#include "pico8_hud.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_msgbox.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_probe.h"
#include "pico8_score_panel_display.h"
#include "pico8_stash.h"
#include "pico8_struct.h"
#include "pico8_type_tag.h"
#include "pico8_vec.h"

static pico8_ob_table_t *g_pj_spec = NULL;
static pico8_ob_table_t *g_ef = NULL;
static pico8_ob_table_t *g_bz = NULL;
static pico8_pj_t *g_ju = NULL;

/* Returns a new list the caller owns and must release with
 * pico8_list_free(). */
static pico8_list_t pico8_score_panel_positional_values(const pico8_ob_table_t *table) {
    pico8_list_t list;
    int i;

    pico8_list_init(&list);
    if (table == NULL) {
        return list;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
            pico8_list_add(&list, table->entries[i].value);
        }
    }
    return list;
}

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

static const pico8_ob_table_t *fm_field_ref_table(const pico8_struct_header_t *fm, const char *name) {
    pico8_value_t v;
    if (fm != NULL && pico8_entity_get_field(fm, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (const pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

static int get_by_value(const pico8_ob_table_t *table, pico8_value_t key, pico8_value_t *out) {
    if (table == NULL) {
        return 0;
    }
    if (key.kind == PICO8_VALUE_FIXED) {
        return pico8_ob_table_get_index(table, pico8_fixed_flr_int(key.as.fixed), out);
    } else if (key.kind == PICO8_VALUE_STRING && key.as.string != NULL) {
        return pico8_ob_table_get_name(table, key.as.string, out);
    }
    return 0;
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

enum { PJ_EA, PJ_GS, PJ_CR, PJ_I_, PJ_OU };

/* pj's class state, read once at init from its spec (the spec's REFs,
 * never re-parsed), except `cr`, which is `g_pj_cr`. */
static struct {
    const char *name;
    pico8_value_t value;
} g_pj_defaults[] = {
    [PJ_EA] = {"ea", {0}}, [PJ_GS] = {"gs", {0}}, [PJ_CR] = {"cr", {0}}, [PJ_I_] = {"i_", {0}}, [PJ_OU] = {"ou", {0}},
};

static int pico8_pj_cr_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_pj_cr_t *cr = self;

    if (strcmp(name, "hi") == 0) {
        if (cr->hi == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(cr->hi);
    } else if (strcmp(name, "gq") == 0) {
        if (cr->gq == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(cr->gq);
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
    {0},
    pico8_pj_cr_field,
    NULL,
};

/* Two vectors, as the spec's two `v(0,11)`. */
static pico8_vec_t g_pj_cr_hi = {0, 11 << PICO8_FIXED_SHIFT};
static pico8_vec_t g_pj_cr_gq = {0, 11 << PICO8_FIXED_SHIFT};

static pico8_pj_cr_t g_pj_cr = {
    {PICO8_TYPE_TAG_STRUCT, &g_pj_cr_struct_class},
    &g_pj_cr_hi,
    &g_pj_cr_gq,
    10 << PICO8_FIXED_SHIFT,
    NULL,
};

static void pico8_pj_defaults_reset(void) {
    size_t i;
    for (i = 0; i < sizeof g_pj_defaults / sizeof g_pj_defaults[0]; i++) {
        g_pj_defaults[i].value = pico8_value_ref(NULL);
    }
}

static pico8_ob_table_t *pj_default_table(int which) {
    return (pico8_ob_table_t *)g_pj_defaults[which].value.as.ref;
}

pico8_ob_table_t *pico8_pj_ou(void) {
    return pj_default_table(PJ_OU);
}

pico8_ob_table_t *pico8_pj_ea(void) {
    return pj_default_table(PJ_EA);
}

pico8_pj_cr_t *pico8_pj_cr(void) {
    return g_pj_defaults[PJ_CR].value.as.ref;
}

const pico8_struct_class_t *pico8_pj_cr_struct_class(void) {
    return &g_pj_cr_struct_class;
}

const pico8_ob_table_t *pico8_pj_i_(void) {
    return pj_default_table(PJ_I_);
}

const pico8_ob_table_t *pico8_pj_gs(void) {
    return pj_default_table(PJ_GS);
}

static pico8_value_t pico8_pj_init_placeholder(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(NULL);
}

static pico8_value_function_t g_init_dispatch = pico8_pj_init_placeholder;

pico8_value_t pico8_init_field_value(void) {
    return pico8_value_function(g_init_dispatch);
}

void pico8_score_panel_set_init(pico8_value_function_t fn) {
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
    const pico8_ob_table_t *fb;
    int i;

    filtered = pico8_jp(pico8_cg("jo"), pico8_eg_gg_pred, NULL);
    ps = pico8_fixed_div(pico8_fixed_from_int(filtered.count), pico8_fixed_from_int(2));
    pico8_list_free(&filtered);

    if (mem == NULL) {
        return;
    }

    fb = pico8_fb();
    if (fb == NULL) {
        return;
    }

    for (i = 1; i <= fb->count; i++) {
        pico8_value_t ny_val;
        if (pico8_ob_table_get_index(fb, i, &ny_val) && ny_val.kind == PICO8_VALUE_FIXED) {
            int ny = pico8_fixed_flr_int(ny_val.as.fixed);
            int pd = 0x3101 + 4 * ny;
            unsigned char cur = pico8_peek(mem, pd);
            int newval = (cur % 128) + ((ps < pico8_fixed_from_int(i)) ? 128 : 0);
            pico8_poke(mem, pd, (unsigned char)newval);
        }
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
    const pico8_ob_table_t *fm_po = fm_field_ref_table(fm, "po");
    const pico8_ob_table_t *r = pico8_r();
    pico8_ob_table_t *ou_table;
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
        pico8_value_t k_key;
        pico8_value_t po_val = pico8_value_fixed(pico8_fixed_from_int(0));
        if (r != NULL && pico8_ob_table_get_name(r, "k", &k_key)) {
            pico8_value_t looked;
            if (get_by_value(fm_po, k_key, &looked) && looked.kind == PICO8_VALUE_FIXED) {
                po_val = looked;
            }
        }
        /* `..` binds looser than `+`: the Lua reads `"+"..(fm.po[r.k]+1)`. */
        plus_num =
            pico8_value_tostr(pico8_value_fixed(pico8_fixed_add(po_val.as.fixed, pico8_fixed_from_int(1))));
        ou[3] = pico8_value_string(pj_concat2("+", plus_num));
        free(plus_num);
    }

    happiness_str = pico8_value_tostr(pico8_value_fixed(fm_happiness));
    ou[4] = pico8_value_string(pj_concat2(happiness_str, "%"));
    free(happiness_str);

    {
        pico8_value_t ef_val = pico8_value_ref(NULL);
        if (g_ef != NULL) {
            pico8_ob_table_get_index(g_ef, pico8_fixed_flr_int(fm_lo), &ef_val);
        }
        ou[5] = ef_val;
    }

    ou[6] = fm_yr;

    ou_table = pico8_pj_ou();
    if (ou_table != NULL) {
        for (i = 0; i < 7; i++) {
            pico8_value_t sub_val;
            if (pico8_ob_table_get_index(ou_table, i + 1, &sub_val) && sub_val.kind == PICO8_VALUE_REF &&
                sub_val.as.ref != NULL) {
                assert(pico8_type_tag_of(sub_val.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
                       "pj.ou rows must hold plain tables");
                /* TROU-7 */
                pico8_ob_table_set_index((pico8_ob_table_t *)sub_val.as.ref, 1, ou[i]);
            }
        }
    }

    pico8_eg();
}

static pico8_value_t pico8_pj_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_pj_hb();
    return pico8_value_ref(NULL);
}

void pico8_pj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_pj_t *self) {
    pico8_list_t i_list = pico8_score_panel_positional_values(pico8_pj_i_());
    pico8_list_t ou_list = pico8_score_panel_positional_values(pico8_pj_ou());
    pico8_list_t combined = pico8_la(&i_list, &ou_list);

    pico8_bl_default(gfx, mem, &combined);

    pico8_list_free(&i_list);
    pico8_list_free(&ou_list);
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

static pico8_value_t pico8_pj_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg1;
    (void)arg0;
    pico8_pj_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_pj_t *)self);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_pj_hk_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t yr = pico8_fixed_from_int(0);
    (void)self;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        yr = arg0.as.fixed;
    }
    pico8_pj_hk(yr);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_pj_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_pj_ph((pico8_pj_t *)self);
    return pico8_value_ref(NULL);
}

pico8_value_t pico8_pj_ed(const pico8_pj_t *self) {
    if (self == NULL) {
        return pico8_value_ref(NULL);
    }
    if (self->ed == NULL) {
        return pico8_value_fixed(pico8_fixed_from_int(1));
    }
    return pico8_value_ref(self->ed);
}

/* `ou`, `ea`, `cr` read off a panel answer the shared state, as the class
 * chain did. */
static int pico8_pj_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_pj_t *pj = self;
    pico8_value_t value;

    if (strcmp(name, "ou") == 0) {
        value = g_pj_defaults[PJ_OU].value;
    } else if (strcmp(name, "ea") == 0) {
        value = g_pj_defaults[PJ_EA].value;
    } else if (strcmp(name, "cr") == 0) {
        value = g_pj_defaults[PJ_CR].value;
    } else if (strcmp(name, "fa") == 0) {
        value = pico8_value_fixed(pj->fa);
    } else if (strcmp(name, "gs") == 0) {
        if (pj->gs == NULL) {
            return 0;
        }
        value = pico8_value_ref(pj->gs);
    } else if (strcmp(name, "i_") == 0) {
        if (pj->i_ == NULL) {
            return 0;
        }
        value = pico8_value_ref(pj->i_);
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
    {
        [PICO8_METHOD_HB] = pico8_pj_hb_dispatch,
        [PICO8_METHOD_KJ] = pico8_pj_kj_dispatch,
        [PICO8_METHOD_HK] = pico8_pj_hk_dispatch,
        [PICO8_METHOD_PH] = pico8_pj_ph_dispatch,
    },
    pico8_pj_field,
    NULL,
};

static void *pico8_pj_struct_new(const pico8_ob_table_t *overrides) {
    pico8_pj_t *self;

    assert(g_pj_spec != NULL && "pico8_score_panel_init() must run first");
    self = malloc(sizeof *self);
    pico8_ke_struct_init(&self->ke, &g_pj_struct_class);
    self->fa = pico8_fixed_from_int(10);
    self->gs = pj_default_table(PJ_GS);
    self->i_ = pj_default_table(PJ_I_);
    self->ed = NULL;
    self->c_ = 0;
    self->has_c_ = 0;
    self->gz = 0;
    self->has_gz = 0;
    assert((overrides == NULL || overrides->count == 0) &&
           "pj: the only road is pico8_pj_new(NULL) (cart.c); a construction key has no member "
           "-- stop, do not add one");
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("pj");
#endif
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_score_panel_init(void) {
    size_t i;

    pico8_score_panel_shutdown();

    pico8_pi_init();

    g_ef = pico8_ob(
        "\"JAN\",\"FEB\",\"MAR\",\"APR\",\"MAY\",\"JUN\",\"JUL\",\"AUG\",\"SEP\",\"OCT\",\"NOV\",\"DEC\",\"UND\","
        "\"DUO\",\"TER\",",
        NULL);

    g_bz = pico8_ob("\n"
                     "o(26,86,0,fn=\"br\"),\n"
                     "o(51,59,1,fn=\"br\"),\n"
                     "o(76,77,1,fn=\"br\"),\n"
                     "o(24,25,1,fn=\"br\"),\n"
                     "o(33,41,1,fn=\"br\"),\n"
                     "o(\"\",score=0,ra=v(64,21),c=5),\n"
                     "o(55,\"planets:   \",\n"
                     "score=1,ra=v(64,34)),\n"
                     "o(19,\"population:\",\n"
                     "score=2,ra=v(64,43)),\n"
                     "o(24,\"technology:\",\n"
                     "score=3,ra=v(64,52)),\n"
                     "o(o(27,5),o(f,0),f,o(25,5),\"happiness\",\n"
                     "score=4,ra=v(64,61)),\n"
                     "o(\"total:\",\"\",\n"
                     "score=5,ra=v(64,74)),\n"
                     "o(score=6,ra=v(64,82)),\n",
                     NULL);

    g_pj_spec = pico8_ob(
        "\nfa=10,\nea=o(\no(42,43,1,fn=\"br\"),\no(\"\",64,40,6,0.5,fn=\"qr\"),\n),\n"
        "gs=o(\no(\"last year!\",64,48,9,0.5,fn=\"qr\"),\no(\"2 years remain\",64,48,2,0.5,fn=\"qr\"),\nf,f,\n"
        "o(\"5 years remain\",64,48,2,0.5,fn=\"qr\"),\n),\n"
        "cr=o(\nhi=v(0,11),gq=v(0,11),ca=10,\n),\n"
        "i_=o(\no(126,127,1,fn=\"br\"),\no(0,1,1,fn=\"br\"),\no(8,118,34,fn=\"rf\"),\no(0,-3,26,fn=\"rf\"),"
        "\no(81,-3,117,fn=\"rf\"),\n),\n"
        "ou=o(\no(f,13,121,9,fn=\"qr\"),\no(f,37,121,2,fn=\"qr\"),\no(f,13,2,9,fn=\"qr\"),\no(f,29,2,2,fn=\"qr\"),"
        "\no(f,59,2,2,fn=\"qr\"),\no(f,85,2,13,fn=\"qr\"),\no(f,99,2,13,fn=\"qr\"),\n),\n"
        "ed=1,\n",
        NULL);
    for (i = 0; i < sizeof g_pj_defaults / sizeof g_pj_defaults[0]; i++) {
        pico8_value_t *v = &g_pj_defaults[i].value;
        int found = pico8_pj_default(g_pj_defaults[i].name, v);
        assert(found && "pj: every default of the struct is a key of pj's spec");
        assert((i == PJ_CR ||
                (v->kind == PICO8_VALUE_REF && v->as.ref != NULL &&
                 pico8_type_tag_of(v->as.ref) == PICO8_TYPE_TAG_OB_TABLE)) &&
               "pj: its 4 table accessors cast these REFs to plain tables, and hb/hk write into ou/ea");
        (void)found;
    }
    g_pj_cr.ep = NULL;
    g_pj_defaults[PJ_CR].value = pico8_value_ref(&g_pj_cr);

    pico8_score_panel_set_init(NULL);
    g_ju = NULL;

    pico8_score_panel_display_init();
}

void pico8_score_panel_shutdown(void) {
    pico8_score_panel_display_shutdown();
    pico8_pj_defaults_reset();
    pico8_ob_table_free(g_pj_spec);
    g_pj_spec = NULL;
    if (g_ef != NULL) {
        pico8_ob_table_free(g_ef);
        g_ef = NULL;
    }
    if (g_bz != NULL) {
        pico8_ob_table_free(g_bz);
        g_bz = NULL;
    }
    pico8_score_panel_set_init(NULL);
    g_ju = NULL;
}

int pico8_pj_ready(void) {
    return g_pj_spec != NULL;
}

int pico8_pj_default(const char *name, pico8_value_t *out_value) {
    return g_pj_spec != NULL &&
           (pico8_ob_table_get_name(g_pj_spec, name, out_value) || pico8_ke_default(name, out_value));
}

const pico8_struct_class_t *pico8_pj_struct_class(void) {
    return &g_pj_struct_class;
}

pico8_pj_t *pico8_pj_new(const pico8_ob_table_t *overrides) {
    return pico8_pj_struct_new(overrides);
}

const pico8_ob_table_t *pico8_ef(void) {
    return g_ef;
}

const pico8_ob_table_t *pico8_bz(void) {
    return g_bz;
}

pico8_pj_t *pico8_ju(void) {
    return g_ju;
}

void pico8_pj_set_ju(pico8_pj_t *instance) {
    g_ju = instance;
}
