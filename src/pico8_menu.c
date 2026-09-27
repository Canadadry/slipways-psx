#include "pico8_menu.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_struct.h"
#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_input.h"
#include "pico8_layout.h"
#include "pico8_list.h"
#include "pico8_msgbox.h"
#include "pico8_pi.h"
#include "pico8_probe.h"
#include "pico8_score_panel.h"
#include "pico8_score_panel_display.h"
#include "pico8_tech_gate.h"
#include "pico8_type_tag.h"
#include "pico8_ui_button.h"
#include "pico8_value.h"
#include "pico8_ref_audit.h"

static pico8_ob_table_t *g_ix = NULL;
static pico8_ob_table_t *g_ha = NULL;

typedef struct {
    pico8_struct_header_t header;
    pico8_value_t hi;
    pico8_value_t gq;
    pico8_value_t ca;
} pico8_ha_pz_t;

static int pico8_ha_pz_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_ha_pz_t *o = self;
    pico8_value_t value;

    if (strcmp(name, "hi") == 0) {
        value = o->hi;
    } else if (strcmp(name, "gq") == 0) {
        value = o->gq;
    } else if (strcmp(name, "ca") == 0) {
        value = o->ca;
    } else {
        return 0;
    }
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return 0;
    }
    *out_value = value;
    return 1;
}

static const pico8_struct_class_t g_ha_pz_struct_class = {
    {0},
    pico8_ha_pz_field,
    NULL,
};

/* Never allocated nor freed: `pico8_menu_init()` points its `hi`/`gq`/`ca`
 * at `g_ha`'s own values, `pico8_menu_shutdown()` resets them to nil. */
static pico8_ha_pz_t g_ha_pz = {
    {PICO8_TYPE_TAG_STRUCT, &g_ha_pz_struct_class},
    {PICO8_VALUE_REF, {.ref = NULL}},
    {PICO8_VALUE_REF, {.ref = NULL}},
    {PICO8_VALUE_REF, {.ref = NULL}},
};

static int cr_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static char *cr_concat2(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char *out = malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

/* `"_cr_t"` is not a cart field: it is where pico8_cr_step stashes the row's
 * captured tech entry, read back here off the button `js(b)` copied it to. */
static pico8_value_t pico8_cr_row_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const void *inst = self;
    pico8_value_t t_val;
    (void)argc;
    (void)arg0;
    (void)arg1;
    if (inst == NULL) {
        return pico8_value_ref(NULL);
    }
    if (pico8_entity_get_field(inst, "_cr_t", &t_val) && t_val.kind == PICO8_VALUE_REF && t_val.as.ref != NULL) {
        pico8_hy_kt(pico8_fm_hy(), (const pico8_ob_table_t *)t_val.as.ref);
    }
    return pico8_value_ref(NULL);
}

/* TROU-7 */
/* An unaffordable tech gets "my" = false, not an absent field: `t.e_` is a
 * comparison result in `cb()`, so Lua's `and` yields the boolean here. */
static int pico8_cr_step(pico8_value_t e, void *ctx, pico8_value_t *out_value) {
    pico8_ob_table_t *t;
    pico8_ob_table_t *row;
    pico8_ob_table_t *lq_table;
    pico8_ob_table_t *h_table;
    pico8_ob_table_t *pair_table;
    pico8_value_t nk_val = pico8_value_ref(NULL);
    pico8_value_t px_val = pico8_value_ref(NULL);
    pico8_value_t oy_val;
    pico8_value_t e_val;
    pico8_fixed_t gi_result;
    char *gi_str;
    (void)ctx;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    assert(pico8_type_tag_of(e.as.ref) == PICO8_TYPE_TAG_OB_TABLE && "cb() must yield plain nb entries");
    t = (pico8_ob_table_t *)e.as.ref;

    row = pico8_ob_table_new();

    pico8_ob_table_get_name(t, "nk", &nk_val);
    pico8_ob_table_get_name(t, "px", &px_val);
    lq_table = pico8_ob_table_new();
    pico8_ob_table_set_index(lq_table, 1, nk_val);
    pico8_ob_table_set_index(lq_table, 2, px_val);
    pico8_ob_table_set_name(lq_table, "c", pico8_value_fixed(pico8_fixed_from_int(6)));
    pico8_ob_table_set_name(row, "lq", pico8_value_ref(lq_table));

    pair_table = pico8_ob_table_new();
    pico8_ob_table_set_index(pair_table, 1, pico8_value_fixed(pico8_fixed_from_int(24)));
    pico8_ob_table_set_index(pair_table, 2, pico8_value_fixed(pico8_fixed_from_int(5)));

    gi_result = pico8_gi(t);
    gi_str = pico8_value_tostr(pico8_value_fixed(gi_result));

    h_table = pico8_ob_table_new();
    pico8_ob_table_set_index(h_table, 1, pico8_value_ref(pair_table));
    pico8_ob_table_set_index(h_table, 2, pico8_value_string(gi_str));
    pico8_ob_table_set_name(row, "h_", pico8_value_ref(h_table));

    if (pico8_ob_table_get_name(t, "oy", &oy_val)) {
        pico8_ob_table_set_name(row, "jb", oy_val);
    }

    if (pico8_ob_table_get_name(t, "e_", &e_val) && cr_truthy(e_val)) {
        pico8_ob_table_set_name(row, "_cr_t", pico8_value_ref(t));
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_cr_row_my_dispatch));
    } else {
        pico8_ob_table_set_name(row, "my", pico8_value_bool(0));
    }

    *out_value = pico8_value_ref(row);
    return 1;
}

/* TROU-7 */
/* Returns a fresh table the caller owns. Storing the list REF unconverted
 * instead is what `js:pq`'s later `jb(self.jb)` would read at the wrong
 * element stride. */
static pico8_ob_table_t *pico8_menu_list_to_ob_table(const pico8_list_t *list) {
    return pico8_ob_table_from_list(list);
}

void pico8_cr(void) {
    pico8_struct_header_t *fm = pico8_fm();
    pico8_fixed_t tl = pico8_fixed_from_int(0);
    int gr = 0;
    pico8_fixed_t qi = pico8_fixed_from_int(0);
    pico8_value_t tl_val;
    pico8_value_t invented_val;
    const pico8_ob_table_t *ek;
    pico8_list_t cb_list;
    pico8_list_t bs;
    pico8_pj_cr_t *cr;

    if (fm != NULL && pico8_entity_get_field(fm, "ew", &tl_val) && tl_val.kind == PICO8_VALUE_FIXED) {
        tl = tl_val.as.fixed;
    }
    if (fm != NULL && pico8_entity_get_field(fm, "invented", &invented_val) &&
        invented_val.kind == PICO8_VALUE_REF && invented_val.as.ref != NULL) {
        gr = PICO8_REF_AS_LIST_CONST(invented_val.as.ref)->count;
    }

    ek = pico8_ek();
    if (ek != NULL) {
        pico8_value_t qi_val;
        int idx = pico8_fixed_flr_int(tl) - 1;
        if (pico8_ob_table_get_index(ek, idx, &qi_val) && qi_val.kind == PICO8_VALUE_FIXED) {
            qi = qi_val.as.fixed;
        }
    }

    /* `true` is the ignore-cost flag: this screen lists every tech of the era,
     * affordable or not. `ix[2].hb`'s bare `cb()` wants only the affordable
     * ones. The two sites do not merge. */
    cb_list = pico8_cb(pico8_value_bool(1));
    bs = pico8_nv(&cb_list, pico8_cr_step, NULL);
    pico8_list_free(&cb_list);

    if (gr > 0) {
        pico8_ob_table_t *extra = pico8_ob_table_new();
        pico8_ob_table_t *lq2 = pico8_ob_table_new();
        char *gr_str = pico8_value_tostr(pico8_value_fixed(pico8_fixed_from_int(gr)));
        char *label = cr_concat2(gr_str, " invented");
        free(gr_str);
        pico8_ob_table_set_index(lq2, 1, pico8_value_fixed(pico8_fixed_from_int(30)));
        pico8_ob_table_set_index(lq2, 2, pico8_value_string(label));
        pico8_ob_table_set_name(lq2, "c", pico8_value_fixed(pico8_fixed_from_int(3)));
        pico8_ob_table_set_name(extra, "lq", pico8_value_ref(lq2));
        {
            const pico8_list_t *invented_list =
                (invented_val.kind == PICO8_VALUE_REF) ? PICO8_REF_AS_LIST_CONST(invented_val.as.ref) : NULL;
            pico8_ob_table_set_name(extra, "jb", pico8_value_ref(pico8_menu_list_to_ob_table(invented_list)));
        }
        pico8_list_add(&bs, pico8_value_ref(extra));
    }

    cr = pico8_pj_cr();
    if (cr != NULL) {
        pico8_fixed_t ek_tl = pico8_fixed_from_int(0);
        pico8_value_t ek_tl_val;
        char *tl_str;
        char *diff1_str;
        char *diff2_str;
        char *tmp1;
        char *tmp2;
        char *tmp3;
        char *ep;

        if (ek != NULL && pico8_ob_table_get_index(ek, pico8_fixed_flr_int(tl), &ek_tl_val) &&
            ek_tl_val.kind == PICO8_VALUE_FIXED) {
            ek_tl = ek_tl_val.as.fixed;
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

        cr->ep = pico8_value_string(ep);
        pico8_pz((const pico8_struct_header_t *)cr, &bs);
    }

    pico8_list_free(&bs);
}

static pico8_ob_table_t *pico8_menu_index_table(pico8_ob_table_t *table, int idx) {
    pico8_value_t v;
    if (table != NULL && pico8_ob_table_get_index(table, idx, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

/* Returns a new, heap-backed list the caller owns and must release with
 * pico8_list_free(). */
static pico8_list_t pico8_menu_positional_values(const pico8_ob_table_t *table) {
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

static pico8_value_t pico8_ix1_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_hy_t *fm = pico8_fm_hy();
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    if (fm != NULL) {
        fm->lo = pico8_value_fixed(pico8_fixed_from_int(1));
    }
    pico8_hy_hk(fm);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_ix2_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_cr();
    return pico8_value_ref(NULL);
}

/* TROU-7 */
/* The bare `cb()` counts only affordable techs. pico8_cr()'s `cb(true)` is the
 * other, non-interchangeable site: passing the flag here would light the icon
 * whenever any tech of the era is left. */
static pico8_value_t pico8_ix2_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    void *inst = self;
    pico8_list_t cb_list;
    int cb_count;
    pico8_ob_table_t *overrides;
    pico8_lq_t *built;
    (void)argc;
    (void)arg0;
    (void)arg1;

    if (inst == NULL) {
        return pico8_value_ref(NULL);
    }

    cb_list = pico8_cb(pico8_value_ref(NULL));
    cb_count = cb_list.count;
    pico8_list_free(&cb_list);

    overrides = pico8_ob_table_new();
    pico8_ob_table_set_index(overrides, 1, pico8_value_fixed(pico8_fixed_from_int(cb_count > 0 ? 24 : 37)));
    built = pico8_lq_new(overrides);
    pico8_ob_table_free(overrides);

    pico8_entity_set_field(inst, "lq", pico8_value_ref(built));

    return pico8_value_ref(NULL);
}

/* `de` is borrowed from pico8_score_panel_display.c: the same table `pj:ph()`
 * uses, not a copy. */
static pico8_value_t pico8_ix3_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_pj_t *ju = pico8_ju();
    pico8_fq_t *built;
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    if (ju != NULL) {
        ju->c_ = pico8_value_bool(1);
    }
    built = pico8_fq_new(pico8_de());
    (void)built;
    return pico8_value_ref(NULL);
}

/* TROU-10 */
static pico8_value_t pico8_ha2_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    int stat_val;
    pico8_fixed_t s;
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    stat_val = pico8_stat(pico8_dispatch_input(), 20);
    s = pico8_fixed_sgn(pico8_fixed_from_int(stat_val));
    pico8_music(-pico8_fixed_flr_int(s));
    return pico8_value_ref(NULL);
}

/* TROU-10 */
/* `ih` is the real global of pico8_audio.c, which pico8_rn() reads; a private
 * copy here would leave that mute check on the old value. */
static pico8_value_t pico8_ha3_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_set_ih(!pico8_ih());
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_ha4_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(NULL);
}

/* `pz(ha,ha)`: `ha`'s rows go in as `bs`, and `p` is `g_ha_pz`, which
 * borrows `ha`'s `hi`/`gq`/`ca`. */
static pico8_value_t pico8_ix4_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_ob_table_t *row;
    pico8_list_t bs;
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;

    row = pico8_menu_index_table(g_ha, 1);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_init_field_value());
    }
    row = pico8_menu_index_table(g_ha, 2);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ha2_my_dispatch));
    }
    row = pico8_menu_index_table(g_ha, 3);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ha3_my_dispatch));
    }
    row = pico8_menu_index_table(g_ha, 4);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ha4_my_dispatch));
    }

    if (g_ha != NULL) {
        bs = pico8_menu_positional_values(g_ha);
        pico8_pz((const pico8_struct_header_t *)&g_ha_pz, &bs);
        pico8_list_free(&bs);
    }

    return pico8_value_ref(NULL);
}

const pico8_ob_table_t *pico8_ix(void) {
    return g_ix;
}

const pico8_ob_table_t *pico8_ha(void) {
    return g_ha;
}

const void *pico8_ha_pz(void) {
    return g_ha != NULL ? &g_ha_pz : NULL;
}

const pico8_struct_class_t *pico8_ha_pz_struct_class(void) {
    return &g_ha_pz_struct_class;
}

void pico8_menu_init(void) {
    pico8_ob_table_t *row;

    pico8_menu_shutdown();

    pico8_pi_init();

    g_ix = pico8_ob("\n"
                     "o(\n"
                     "ra=v(118,0),\n"
                     "lq=o(o(15,7)),\n"
                     "),o(\n"
                     "ra=v(0,0),\n"
                     "lq=o(o(37,7)),\n"
                     "),o(\n"
                     "ra=v(47,0),\n"
                     "lq=o(o(25,7)),\n"
                     "),o(\n"
                     "ra=v(0,117),\n"
                     "lq=o(o(8,7)),\n"
                     "),\n",
                     NULL);

    g_ha = pico8_ob("\n"
                     "o(lq=o(31,\"restart     \")),\n"
                     "o(lq=o(42,\"toggle music\")),\n"
                     "o(lq=o(40,\"toggle sfx  \")),\n"
                     "o(lq=o(26,\"how to play?\"),fg=5,),\n"
                     "hi=v(0,107),gq=v(0,-11),\n"
                     "ca=14,\n",
                     NULL);
    {
        int found = pico8_ob_table_get_name(g_ha, "hi", &g_ha_pz.hi) &&
                    pico8_ob_table_get_name(g_ha, "gq", &g_ha_pz.gq) &&
                    pico8_ob_table_get_name(g_ha, "ca", &g_ha_pz.ca);
        assert(found && g_ha_pz.hi.kind == PICO8_VALUE_REF && g_ha_pz.hi.as.ref != NULL &&
               g_ha_pz.gq.kind == PICO8_VALUE_REF && g_ha_pz.gq.as.ref != NULL &&
               g_ha_pz.ca.kind == PICO8_VALUE_FIXED &&
               "menu: pz reads hi/gq as vectors and ca as a fixed off ha's own spec");
        (void)found;
    }

    row = pico8_menu_index_table(g_ix, 1);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ix1_my_dispatch));
    }
    row = pico8_menu_index_table(g_ix, 2);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ix2_my_dispatch));
        pico8_ob_table_set_name(row, "hb", pico8_value_function(pico8_ix2_hb_dispatch));
    }
    row = pico8_menu_index_table(g_ix, 3);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ix3_my_dispatch));
    }
    row = pico8_menu_index_table(g_ix, 4);
    if (row != NULL) {
        pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_ix4_my_dispatch));
    }
    PICO8_MEMBER_CENSUS_SEE(&g_ha_pz, pico8_ha_pz_t);
}

void pico8_menu_shutdown(void) {
    if (g_ix != NULL) {
        pico8_ob_table_free(g_ix);
        g_ix = NULL;
    }
    g_ha_pz.hi = pico8_value_ref(NULL);
    g_ha_pz.gq = pico8_value_ref(NULL);
    g_ha_pz.ca = pico8_value_ref(NULL);
    if (g_ha != NULL) {
        pico8_ob_table_free(g_ha);
        g_ha = NULL;
    }
    /* `ih` is deliberately not reset here: it belongs to pico8_audio.c, and
     * rebuilding this module's tables must not move it. */
}
