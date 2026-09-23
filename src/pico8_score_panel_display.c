#include "pico8_score_panel_display.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_economy_score.h"
#include "pico8_entity.h"
#include "pico8_layout.h"
#include "pico8_list.h"
#include "pico8_msgbox.h"
#include "pico8_probe.h"
#include "pico8_score_panel.h"
#include "pico8_ui_button.h"

static pico8_ob_table_t *g_de = NULL;

/* Lua truthiness: only `false` and this port's absent filler `ref(NULL)`
 * read as falsy -- 0 does not. */
static int pj_display_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static pico8_value_t fm_field_value(const pico8_instance_t *fm, const char *name) {
    pico8_value_t v;
    if (fm != NULL && pico8_instance_get_field(fm, name, &v)) {
        return v;
    }
    return pico8_value_ref(NULL);
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

/* Reads `pj`'s OWN class-level table -- the one every instance shares, so
 * writing through it mutates `pj.<name>` for all of them. NULL when the
 * class is absent or the field is not a table. */
static pico8_ob_table_t *pj_class_field_ref_table(const char *name) {
    pico8_value_t v;
    const pico8_class_t *cls = pico8_pj_class();
    if (cls != NULL && cls->defaults != NULL && pico8_ob_table_get_name(cls->defaults, name, &v) &&
        v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
               "pj's class-level field must hold a plain table");
        return (pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

/* The captured `t` of `pj:hk`'s `iw.lh` closure. One static is enough:
 * `pj.ea.lh` is replaced wholesale by every call that sets it, so only one
 * such closure is ever live. */
static int g_pj_hk_lh_t = 0;

static pico8_value_t pico8_pj_hk_lh_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    g_pj_hk_lh_t -= 1;
    return pico8_value_bool(g_pj_hk_lh_t > 0);
}

void pico8_pj_hk(pico8_fixed_t yr) {
    pico8_ob_table_t *iw = pj_class_field_ref_table("ea");
    const pico8_ob_table_t *gs = pj_class_field_ref_table("gs");
    pico8_value_t pp = pico8_value_ref(NULL);
    pico8_value_t iw2_val;
    char *yr_str;
    char *year_label;
    int idx;

    if (iw == NULL) {
        return;
    }

    idx = 3426 - pico8_fixed_flr_int(yr);
    if (gs != NULL) {
        pico8_ob_table_get_index(gs, idx, &pp);
    }

    yr_str = pico8_value_tostr(pico8_value_fixed(yr));
    year_label = pj_concat2("year ", yr_str);
    free(yr_str);

    if (pico8_ob_table_get_index(iw, 2, &iw2_val) && iw2_val.kind == PICO8_VALUE_REF && iw2_val.as.ref != NULL) {
        assert(pico8_type_tag_of(iw2_val.as.ref) == PICO8_TYPE_TAG_OB_TABLE && "pj.ea[2] must hold a plain table");
        pico8_ob_table_set_index((pico8_ob_table_t *)iw2_val.as.ref, 1, pico8_value_string(year_label));
    }

    if (pj_display_truthy(pp)) {
        pico8_ob_table_set_index(iw, 3, pp);
        g_pj_hk_lh_t = 90;
        pico8_ob_table_set_name(iw, "lh", pico8_value_function(pico8_pj_hk_lh_dispatch));
    } else {
        pico8_ob_table_remove_index(iw, 3);
    }

    {
        pico8_instance_t *made = pico8_mh_new(iw);
        (void)made;
    }
}

/* Returns a new list the caller owns and must release with
 * pico8_list_free(). */
static pico8_list_t pico8_score_panel_display_positional_values(const pico8_ob_table_t *table) {
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

/* Mirror image of the helper above: one positional entry per list element,
 * so the caller can then set `rh`/`lh` on it as named fields. Returns a new
 * table the caller owns and must release with pico8_ob_table_free(). */
static pico8_ob_table_t *pico8_score_panel_display_list_to_ob_table(const pico8_list_t *list) {
    pico8_ob_table_t *table = pico8_ob_table_new();
    pico8_list_iter_t it;
    pico8_value_t v;
    int i = 1;

    if (list == NULL) {
        return table;
    }
    it = pico8_list_all(list);
    while (pico8_list_iter_next(&it, &v)) {
        pico8_ob_table_set_index(table, i, v);
        i++;
    }
    return table;
}

static pico8_value_t pico8_pj_de_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *ju = pico8_ju();
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    if (ju != NULL) {
        pico8_value_t c_val;
        int current = pico8_instance_get_field(ju, "c_", &c_val) && pj_display_truthy(c_val);
        pico8_ob_table_set_name(ju->fields, "c_", pico8_value_bool(!current));
    }
    return pico8_value_ref(NULL);
}

/* `self` inside `pj:ph` is always `ju` -- `pj()` is constructed exactly once
 * in the cart -- so reading `ju.c_` here IS reading the captured upvalue. */
static pico8_value_t pico8_pj_ph_lh_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *ju = pico8_ju();
    pico8_value_t c_val;
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    if (ju != NULL && pico8_instance_get_field(ju, "c_", &c_val)) {
        return pico8_value_bool(pj_display_truthy(c_val));
    }
    return pico8_value_bool(0);
}

typedef struct {
    const pico8_ob_table_t *s;
} pico8_pj_ph_ctx_t;

static int pico8_pj_ph_map(pico8_value_t e, void *ctx_ptr, pico8_value_t *out_value) {
    const pico8_pj_ph_ctx_t *ctx = (const pico8_pj_ph_ctx_t *)ctx_ptr;
    pico8_ob_table_t *st;
    pico8_value_t score_val;
    int score_i;
    pico8_value_t s_val;
    char *s_str;
    pico8_value_t ra_val = pico8_value_ref(NULL);
    pico8_instance_t *lq_inst;
    pico8_ob_table_t *wrapped;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        *out_value = e;
        return 1;
    }
    assert(pico8_type_tag_of(e.as.ref) == PICO8_TYPE_TAG_OB_TABLE && "bz rows must hold plain tables");
    st = (pico8_ob_table_t *)e.as.ref;

    if (!pico8_ob_table_get_name(st, "score", &score_val) || score_val.kind != PICO8_VALUE_FIXED) {
        *out_value = e;
        return 1;
    }

    score_i = pico8_fixed_flr_int(score_val.as.fixed);

    s_val = pico8_value_ref(NULL);
    if (ctx->s != NULL) {
        pico8_ob_table_get_index(ctx->s, score_i, &s_val);
    }
    /* `s[0]` is genuinely absent for score 0, and this port has no nil to
     * format: `pico8_value_tostr` renders the absent filler as "", which
     * PICO-8's own tostr would not. */
    s_str = pico8_value_tostr(s_val);
    /* TROU-7 */
    pico8_ob_table_set_index(st, 3, pico8_value_string(s_str));

    if (score_i == 0) {
        const pico8_instance_t *fm = pico8_fm();
        pico8_value_t hl_val = fm_field_value(fm, "hl");
        const char *hl_str = (hl_val.kind == PICO8_VALUE_STRING) ? hl_val.as.string : "current score";
        const pico8_ob_table_t *dk = pico8_dk();
        const char *px = "";
        pico8_value_t px_val;
        char *piece1;
        char *piece2;
        char *label;

        if (dk != NULL && pico8_ob_table_get_name(dk, "px", &px_val) && px_val.kind == PICO8_VALUE_STRING) {
            px = px_val.as.string;
        }
        piece1 = pj_concat2(hl_str, " (");
        piece2 = pj_concat2(piece1, px);
        free(piece1);
        label = pj_concat2(piece2, ")");
        free(piece2);
        pico8_ob_table_set_index(st, 1, pico8_value_string(label));
    }

    if (score_i == 6) {
        pico8_value_t s6_val = pico8_value_fixed(pico8_fixed_from_int(0));
        int i;
        if (ctx->s != NULL) {
            pico8_ob_table_get_index(ctx->s, 6, &s6_val);
        }
        assert(s6_val.kind == PICO8_VALUE_FIXED && "ky() returns a numeric tier at s[6]");
        for (i = 1; i <= 5; i++) {
            int icon = (i <= (int)pico8_fixed_to_double(s6_val.as.fixed)) ? 56 : 57;
            pico8_ob_table_set_index(st, i, pico8_value_fixed(pico8_fixed_from_int(icon)));
        }
    }

    pico8_ob_table_get_name(st, "ra", &ra_val);
    /* TROU-7 */
    lq_inst = pico8_lq_new(st);
    wrapped = pico8_ob_table_new();
    pico8_ob_table_set_index(wrapped, 1, pico8_value_ref(lq_inst));
    pico8_ob_table_set_index(wrapped, 2, ra_val);
    pico8_ob_table_set_index(wrapped, 3, pico8_value_fixed(PICO8_FIXED_LIT(0x00008000, 0.5)));
    pico8_ob_table_set_name(wrapped, "fn", pico8_value_string("ld"));
    *out_value = pico8_value_ref(wrapped);
    return 1;
}

void pico8_pj_ph(pico8_instance_t *self) {
    const pico8_instance_t *fm = pico8_fm();
    pico8_value_t hl_val = fm_field_value(fm, "hl");
    pico8_value_t gz_val;
    int has_gz = (self != NULL) && pico8_instance_get_field(self, "gz", &gz_val);

    if (pj_display_truthy(hl_val) && !has_gz) {
        pico8_ob_table_t *my_init_overrides;
        pico8_ob_table_t *spec1;
        pico8_ob_table_t *spec2;
        pico8_instance_t *made1;
        pico8_instance_t *made2;

        (void)pico8_fq_new(NULL);

        my_init_overrides = pico8_ob_table_new();
        pico8_ob_table_set_name(my_init_overrides, "my", pico8_init_field_value());
        spec1 = pico8_ob("\nfa=15,\nra=v(51,100),\nlq=o(\" again? \"),\n", my_init_overrides);
        pico8_ob_table_free(my_init_overrides);
        made1 = pico8_js_new(spec1);
        (void)made1;
        pico8_ob_table_free(spec1);

        spec2 = pico8_ob("\nfa=15,\nra=v(39,100),\nlq=o(o(10,7)),\n", g_de);
        made2 = pico8_js_new(spec2);
        (void)made2;
        pico8_ob_table_free(spec2);

        if (self != NULL) {
            pico8_ob_table_set_name(self->fields, "c_", pico8_value_bool(1));
            pico8_ob_table_set_name(self->fields, "gz", pico8_value_fixed(pico8_fixed_from_int(0)));
            pico8_ob_table_set_name(self->fields, "fa", pico8_value_fixed(pico8_fixed_from_int(13)));
        }
    }

    {
        pico8_value_t c_val;
        int c_truthy = (self != NULL) && pico8_instance_get_field(self, "c_", &c_val) && pj_display_truthy(c_val);
        pico8_value_t ed_val;
        int ed_equals_kc = 0;

        /* `self.ed`'s class-level default is the number 1, never a REF, and a
         * non-REF `ed` is never `kc`: the block has to fire on the first
         * eligible tick. Comparing only a REF-valued `ed` silently disables it. */
        if (self != NULL && pico8_instance_get_field(self, "ed", &ed_val) && ed_val.kind == PICO8_VALUE_REF) {
            ed_equals_kc = (pico8_kc() == (pico8_instance_t *)ed_val.as.ref);
        }

        if (c_truthy && !ed_equals_kc) {
            pico8_ob_table_t *s = pico8_ky();
            pico8_list_t bz_list = pico8_score_panel_display_positional_values(pico8_bz());
            pico8_pj_ph_ctx_t ctx;
            pico8_list_t iw_list;
            pico8_ob_table_t *iw_table;
            pico8_instance_t *made;

            ctx.s = s;
            iw_list = pico8_nv(&bz_list, pico8_pj_ph_map, &ctx);
            pico8_list_free(&bz_list);

            iw_table = pico8_score_panel_display_list_to_ob_table(&iw_list);
            pico8_list_free(&iw_list);

            pico8_ob_table_set_name(iw_table, "rh", pico8_value_fixed(pico8_fixed_from_int(3)));
            pico8_ob_table_set_name(iw_table, "lh", pico8_value_function(pico8_pj_ph_lh_dispatch));

            made = pico8_mh_new(iw_table);
            pico8_ob_table_free(iw_table);
            pico8_ob_table_free(s);

            if (self != NULL) {
                pico8_ob_table_set_name(self->fields, "ed", pico8_value_ref(made));
            }
        }
    }
}

void pico8_score_panel_display_init(void) {
    pico8_score_panel_display_shutdown();
    g_de = pico8_ob_table_new();
    pico8_ob_table_set_name(g_de, "my", pico8_value_function(pico8_pj_de_my_dispatch));
    g_pj_hk_lh_t = 0;
}

void pico8_score_panel_display_shutdown(void) {
    if (g_de != NULL) {
        pico8_ob_table_free(g_de);
        g_de = NULL;
    }
}

const pico8_ob_table_t *pico8_de(void) {
    return g_de;
}
