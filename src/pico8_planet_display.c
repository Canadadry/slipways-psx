#include "pico8_planet_display.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_hud.h"
#include "pico8_msgbox.h"
#include "pico8_planet.h"
#include "pico8_probe.h"
#include "pico8_research.h"
#include "pico8_research_display.h"
#include "pico8_stash.h"
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

/* Field readers: an absent field, a wrong kind, or a NULL instance all read
 * as the zero/false/NULL default. */
static int field_truthy(const pico8_instance_t *self, const char *name) {
    pico8_value_t v;
    if (self == NULL || !pico8_instance_get_field(self, name, &v)) {
        return 0;
    }
    return pico8_jo_truthy(v);
}

static pico8_fixed_t field_fixed(const pico8_instance_t *self, const char *name) {
    pico8_value_t v;
    if (self != NULL && pico8_instance_get_field(self, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static const pico8_list_t *field_list(const pico8_instance_t *self, const char *name) {
    pico8_value_t v;
    if (self != NULL && pico8_instance_get_field(self, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return PICO8_REF_AS_LIST_CONST(v.as.ref);
    }
    return NULL;
}

static const pico8_ob_table_t *field_ref_table(const pico8_instance_t *self, const char *name) {
    pico8_value_t v;
    if (self != NULL && pico8_instance_get_field(self, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE && "field read as an ob_table is not one");
        return (const pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

/* The returned list borrows the table's values: the caller frees the list,
 * never the table. */
static pico8_list_t pico8_jo_positional_values(const pico8_ob_table_t *table) {
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

void pico8_jo_hb(pico8_instance_t *self) {
    pico8_value_t jy_val;
    pico8_value_t kw_val;
    pico8_value_t by_val;
    const pico8_list_t *jy_list = NULL;
    const pico8_list_t *by_list = NULL;
    pico8_value_t qe = pico8_value_ref(NULL);
    pico8_fixed_t mu = pico8_fixed_from_int(0);
    pico8_instance_t *lq_result;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "jy", &jy_val) && jy_val.kind == PICO8_VALUE_REF && jy_val.as.ref != NULL) {
        jy_list = PICO8_REF_AS_LIST_CONST(jy_val.as.ref);
    }
    if (pico8_instance_get_field(self, "kw", &kw_val) && kw_val.kind == PICO8_VALUE_REF && kw_val.as.ref != NULL) {
        const pico8_ob_table_t *kw = (const pico8_ob_table_t *)kw_val.as.ref;
        pico8_value_t mu_val;
        if (!pico8_ob_table_get_name(kw, "qe", &qe)) {
            qe = pico8_value_ref(NULL);
        }
        if (pico8_ob_table_get_name(kw, "mu", &mu_val) && mu_val.kind == PICO8_VALUE_FIXED) {
            mu = mu_val.as.fixed;
        }
    }
    if (pico8_instance_get_field(self, "by", &by_val) && by_val.kind == PICO8_VALUE_REF && by_val.as.ref != NULL) {
        by_list = PICO8_REF_AS_LIST_CONST(by_val.as.ref);
    }

    lq_result = pico8_jo_ex(jy_list, qe, mu, pico8_fixed_from_int(by_list != NULL ? by_list->count : 0),
                             pico8_fixed_from_int(0));
    pico8_ob_table_set_name(self->fields, "lq", pico8_value_ref(lq_result));
}

typedef struct {
    pico8_list_t *ji;
} pico8_jo_gh_ctx_t;

/* Filters on `gc == 0`, and also appends the element to the `ji` list in
 * `ctx` once per `gc`: the Lua predicate has that side effect. */
static int pico8_jo_gh_jp_step(pico8_value_t e, void *ctx_ptr) {
    pico8_jo_gh_ctx_t *ctx = (pico8_jo_gh_ctx_t *)ctx_ptr;
    pico8_fixed_t gc = pico8_fixed_from_int(0);
    int count;
    int j;

    if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
        pico8_value_t gc_val;
        if (pico8_ob_table_get_name((const pico8_ob_table_t *)e.as.ref, "gc", &gc_val) &&
            gc_val.kind == PICO8_VALUE_FIXED) {
            gc = gc_val.as.fixed;
        }
    }
    count = pico8_fixed_flr_int(gc);
    for (j = 0; j < count; j++) {
        pico8_list_add(ctx->ji, e);
    }
    return gc == pico8_fixed_from_int(0);
}

/* One `bs` row in the `{lq=...,h_=...}` shape `pz()` expects. The row owns
 * nothing: `lq_list` must outlive it. */
static pico8_ob_table_t *pico8_jo_gh_row(pico8_list_t *lq_list) {
    pico8_ob_table_t *row = pico8_ob_table_new();
    pico8_ob_table_set_name(row, "lq", pico8_value_ref(lq_list));
    return row;
}

void pico8_jo_gh(const pico8_instance_t *self) {
    pico8_list_t bs;
    pico8_ob_table_t *bs1;
    pico8_value_t px_val;
    pico8_value_t icon_val = pico8_value_ref(NULL);
    pico8_value_t kr_val;
    pico8_value_t h_field;
    pico8_list_t *ji;
    pico8_jo_gh_ctx_t ctx;
    const pico8_list_t *jy;
    pico8_list_t jx;
    int i;

    if (self == NULL) {
        return;
    }

    pico8_list_init(&bs);

    bs1 = pico8_ob_table_new();
    if (pico8_instance_get_field(self, "px", &px_val) && pico8_jo_truthy(px_val)) {
        icon_val = px_val;
    } else {
        pico8_value_t mi_val;
        if (pico8_instance_get_field(self, "mi", &mi_val) && mi_val.kind == PICO8_VALUE_FIXED &&
            pico8_jo_class() != NULL) {
            pico8_value_t dz_val;
            if (pico8_ob_table_get_name(pico8_jo_class()->defaults, "dz", &dz_val) && dz_val.kind == PICO8_VALUE_REF &&
                dz_val.as.ref != NULL) {
                pico8_ob_table_get_index((const pico8_ob_table_t *)dz_val.as.ref,
                                          pico8_fixed_flr_int(mi_val.as.fixed), &icon_val);
            }
        }
    }
    pico8_ob_table_set_name(bs1, "lq", icon_val);

    if (pico8_instance_get_field(self, "kr", &kr_val) && pico8_jo_truthy(kr_val)) {
        pico8_ob_table_t *h_tuple = pico8_ob_table_new();
        int discard_color = 0;
        pico8_fixed_t kr_fixed = (kr_val.kind == PICO8_VALUE_FIXED) ? kr_val.as.fixed : pico8_fixed_from_int(0);
        char *text = pico8_ne(kr_fixed, 0, NULL, &discard_color);
        size_t text_len = strlen(text);
        char *with_suffix = malloc(text_len + 2);
        memcpy(with_suffix, text, text_len);
        with_suffix[text_len] = '$';
        with_suffix[text_len + 1] = '\0';
        free(text);
        pico8_ob_table_set_index(h_tuple, 1, pico8_value_string(with_suffix));
        pico8_ob_table_set_name(h_tuple, "bg",
                                 (kr_val.kind == PICO8_VALUE_FIXED && kr_val.as.fixed >= pico8_fixed_from_int(0))
                                     ? pico8_value_fixed(pico8_fixed_from_int(1))
                                     : pico8_value_bool(0));
        h_field = pico8_value_ref(h_tuple);
    } else {
        h_field = pico8_value_ref(NULL);
    }
    pico8_ob_table_set_name(bs1, "h_", h_field);
    pico8_list_add(&bs, pico8_value_ref(bs1));

    ji = malloc(sizeof *ji);
    pico8_list_init(ji);
    pico8_list_add(ji, pico8_value_string("receives:"));
    ctx.ji = ji;

    jy = field_list(self, "jy");
    jx = pico8_jp(jy, pico8_jo_gh_jp_step, &ctx);

    if (jx.count > 0) {
        pico8_list_t prefix;
        pico8_list_t *merged = malloc(sizeof *merged);
        pico8_list_init(&prefix);
        pico8_list_add(&prefix, pico8_value_string("needs:"));
        *merged = pico8_la(&prefix, &jx);
        pico8_list_free(&prefix);
        pico8_list_add(&bs, pico8_value_ref(pico8_jo_gh_row(merged)));
    } else {
        pico8_fixed_t mi_fixed = field_fixed(self, "mi");
        const pico8_ob_table_t *kw = field_ref_table(self, "kw");
        pico8_value_t kw_qe = pico8_value_ref(NULL);
        pico8_value_t r_w;
        int qe_is_r_w;

        if (kw != NULL) {
            pico8_ob_table_get_name(kw, "qe", &kw_qe);
        }
        /* TROU-3 */
        qe_is_r_w = pico8_ob_table_get_name(pico8_r(), "w", &r_w) && pico8_value_equal(kw_qe, r_w);

        if (mi_fixed < pico8_fixed_from_int(4) && jy != NULL && jy->count > 0 && !qe_is_r_w) {
            pico8_list_t prefix;
            pico8_list_t *merged = malloc(sizeof *merged);
            pico8_list_init(&prefix);
            pico8_list_add(&prefix, pico8_value_string("wants more:"));
            if (field_truthy(self, "qh")) {
                pico8_list_t last_only;
                pico8_list_init(&last_only);
                pico8_list_add(&last_only, jy->items[jy->count - 1]);
                *merged = pico8_la(&prefix, &last_only);
                pico8_list_free(&last_only);
            } else {
                *merged = pico8_la(&prefix, jy);
            }
            pico8_list_free(&prefix);
            pico8_list_add(&bs, pico8_value_ref(pico8_jo_gh_row(merged)));
        }
    }
    pico8_list_free(&jx);

    /* A `ji` still holding only its header means the predicate appended
     * nothing; Lua's `add(t,nil)` is a no-op, so there is no row at all,
     * not a row holding false. */
    if (ji->count >= 2) {
        pico8_list_add(&bs, pico8_value_ref(pico8_jo_gh_row(ji)));
    } else {
        pico8_list_free(ji);
        free(ji);
    }

    {
        const pico8_ob_table_t *kw = field_ref_table(self, "kw");
        pico8_fixed_t mu = pico8_fixed_from_int(0);
        pico8_value_t qe_val = pico8_value_ref(NULL);

        if (kw != NULL) {
            pico8_value_t mu_val;
            if (pico8_ob_table_get_name(kw, "mu", &mu_val) && mu_val.kind == PICO8_VALUE_FIXED) {
                mu = mu_val.as.fixed;
            }
            pico8_ob_table_get_name(kw, "qe", &qe_val);
        }

        if (mu > pico8_fixed_from_int(0)) {
            pico8_list_t *lb = malloc(sizeof *lb);
            int makes = 0;

            pico8_list_init(lb);

            if (pico8_jo_class() != NULL) {
                pico8_value_t cq_val;
                if (pico8_ob_table_get_name(pico8_jo_class()->defaults, "cq", &cq_val) &&
                    cq_val.kind == PICO8_VALUE_REF && cq_val.as.ref != NULL) {
                    pico8_list_t cq_list = pico8_jo_positional_values((const pico8_ob_table_t *)cq_val.as.ref);
                    makes = pico8_gu(&cq_list, qe_val);
                    pico8_list_free(&cq_list);
                }
            }

            if (makes) {
                char *mu_str = pico8_value_tostr(pico8_value_fixed(mu));
                pico8_list_add(lb, pico8_value_string("makes:"));
                pico8_list_add(lb, pico8_value_string(mu_str));
            } else {
                const pico8_list_t *by = field_list(self, "by");
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
            pico8_list_add(lb, pico8_value_ref((void *)kw));
            pico8_list_add(&bs, pico8_value_ref(pico8_jo_gh_row(lb)));
        }
    }

    /* From the second row on: the first row's `lq` is a single icon value,
     * not a list. */
    for (i = 1; i < bs.count; i++) {
        pico8_value_t row_val = bs.items[i];
        if (row_val.kind == PICO8_VALUE_REF && row_val.as.ref != NULL) {
            pico8_ob_table_t *row = (pico8_ob_table_t *)row_val.as.ref;
            pico8_value_t lq_val;
            if (pico8_ob_table_get_name(row, "lq", &lq_val) && lq_val.kind == PICO8_VALUE_REF &&
                lq_val.as.ref != NULL) {
                pico8_ob_table_t *tuple = pico8_ob_table_new();
                pico8_ob_table_set_index(tuple, 1, pico8_value_bool(0));
                pico8_ob_table_set_index(tuple, 2, pico8_value_fixed(pico8_fixed_from_int(1)));
                pico8_list_add(PICO8_REF_AS_LIST(lq_val.as.ref), pico8_value_ref(tuple));
            }
        }
    }

    pico8_pz(self, &bs);
    pico8_list_free(&bs);
}

pico8_value_t pico8_jo_gv(const pico8_instance_t *self, const pico8_instance_t *s) {
    pico8_value_t me_val;
    pico8_value_t qu_val;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }
    if (!pico8_instance_get_field(self, "me", &me_val) || me_val.kind != PICO8_VALUE_STRING ||
        strcmp(me_val.as.string, "ly") != 0) {
        return pico8_value_ref(NULL);
    }
    if (s != NULL && pico8_entity_get_field(s, "qu", &qu_val) && qu_val.kind == PICO8_VALUE_REF &&
        qu_val.as.ref == (const void *)self) {
        pico8_jo_gh(self);
        return pico8_value_ref(NULL);
    }
    return pico8_value_bool(1);
}

typedef struct {
    pico8_instance_t *self;
} pico8_jo_fe_ctx_t;

/* C has no closures: the three values the Lua `my` callback captures ride
 * on the row as `_jo_fe_*` fields, which `pz()`'s own `js(b)` copies
 * verbatim onto the button instance this bridge is later dispatched on. */
static pico8_value_t pico8_jo_fe_my_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const pico8_instance_t *inst = (const pico8_instance_t *)self;
    pico8_value_t pt_val;
    pico8_value_t b_val;
    pico8_value_t od_val;
    pico8_instance_t *pt = NULL;
    pico8_instance_t *b = NULL;
    pico8_fixed_t od = pico8_fixed_from_int(0);

    (void)argc;
    (void)arg0;
    (void)arg1;

    if (inst == NULL) {
        return pico8_value_ref(NULL);
    }
    if (pico8_instance_get_field(inst, "_jo_fe_pt", &pt_val) && pt_val.kind == PICO8_VALUE_REF &&
        pt_val.as.ref != NULL) {
        pt = (pico8_instance_t *)pt_val.as.ref;
    }
    if (pico8_instance_get_field(inst, "_jo_fe_b", &b_val) && b_val.kind == PICO8_VALUE_REF && b_val.as.ref != NULL) {
        b = (pico8_instance_t *)b_val.as.ref;
    }
    if (pico8_instance_get_field(inst, "_jo_fe_od", &od_val) && od_val.kind == PICO8_VALUE_FIXED) {
        od = od_val.as.fixed;
    }

    assert(pt != NULL && "jo:fe's my callback lost its captured planet");
    if (pico8_hy_lu(pico8_fm(), od)) {
        pico8_jo_hu(pt, b);
    }
    return pico8_value_ref(NULL);
}

static int pico8_jo_fe_step(pico8_value_t e, void *ctx_ptr, pico8_value_t *out_value) {
    const pico8_jo_fe_ctx_t *ctx = (const pico8_jo_fe_ctx_t *)ctx_ptr;
    pico8_instance_t *b;
    pico8_fixed_t od;
    pico8_ob_table_t *row;
    pico8_value_t lq_val;
    pico8_value_t h_val;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    b = (pico8_instance_t *)e.as.ref;
    od = pico8_hp_od(b);

    row = pico8_ob_table_new();

    if (pico8_mv(b, "lq", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &lq_val)) {
        pico8_ob_table_set_name(row, "lq", lq_val);
    } else {
        assert(0 && "hp must expose \"lq\" as an mv()-dispatchable field");
    }

    if (od > pico8_fixed_from_int(0)) {
        pico8_ob_table_t *h_tuple = pico8_ob_table_new();
        char *od_str = pico8_value_tostr(pico8_value_fixed(od));
        size_t od_len = strlen(od_str);
        char *with_suffix = malloc(od_len + 2);
        memcpy(with_suffix, od_str, od_len);
        with_suffix[od_len] = '$';
        with_suffix[od_len + 1] = '\0';
        free(od_str);
        pico8_ob_table_set_index(h_tuple, 1, pico8_value_string(with_suffix));
        pico8_ob_table_set_name(h_tuple, "c", pico8_value_fixed(pico8_fixed_from_int(13)));
        h_val = pico8_value_ref(h_tuple);
    } else {
        h_val = pico8_value_bool(0);
    }
    pico8_ob_table_set_name(row, "h_", h_val);

    pico8_ob_table_set_name(row, "_jo_fe_pt", pico8_value_ref((void *)ctx->self));
    pico8_ob_table_set_name(row, "_jo_fe_b", pico8_value_ref(b));
    pico8_ob_table_set_name(row, "_jo_fe_od", pico8_value_fixed(od));
    pico8_ob_table_set_name(row, "my", pico8_value_function(pico8_jo_fe_my_dispatch));

    pico8_ob_table_set_name(row, "jb", pico8_value_ref(pico8_hp_jb(b)));

    *out_value = pico8_value_ref(row);
    return 1;
}

void pico8_jo_fe(pico8_instance_t *self) {
    pico8_list_t bj_list;
    pico8_list_t rows;
    pico8_jo_fe_ctx_t ctx;

    if (self == NULL) {
        return;
    }

    bj_list = pico8_bj(self);
    ctx.self = self;
    rows = pico8_nv(&bj_list, pico8_jo_fe_step, &ctx);
    pico8_list_free(&bj_list);

    pico8_pz(self, &rows);
    pico8_list_free(&rows);
}

void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p) {
    pico8_vec_t adjusted;
    pico8_value_t me_val;
    int px;
    int py;
    int spr_index = 43;
    pico8_value_t nk_val;
    pico8_value_t s_val;

    if (self == NULL || !pico8_gd(self)) {
        return;
    }

    /* Also decays `self.ma`. Everything below draws at `adjusted`, never
     * at the `p` that was passed in. */
    adjusted = pico8_ma(p, self);

    if (pico8_instance_get_field(self, "me", &me_val) && me_val.kind == PICO8_VALUE_STRING &&
        strcmp(me_val.as.string, "ll") == 0) {
        pico8_fixed_t t = field_fixed(self, "t");
        if (pico8_fixed_rnd(pico8_fixed_from_int(30)) < pico8_fixed_sub(t, pico8_fixed_from_int(30))) {
            return;
        }
    }

    px = pico8_fixed_flr_int(adjusted.x);
    py = pico8_fixed_flr_int(adjusted.y);

    pico8_circfill(gfx, mem, px, py, 7, 0);

    /* An absent `qw` reads as 0, i.e. the default palette block. */
    pico8_dv(mem, field_fixed(self, "qw"));
    pico8_palt(mem, 3, 0);
    pico8_palt(mem, 0, 1);

    /* `A and B or C`: a truthy `nk` whose own `s` is absent still falls
     * through to `self.s`, then to the literal 43. */
    if (pico8_instance_get_field(self, "nk", &nk_val) && pico8_jo_truthy(nk_val) && nk_val.kind == PICO8_VALUE_REF &&
        nk_val.as.ref != NULL &&
        pico8_ob_table_get_name((const pico8_ob_table_t *)nk_val.as.ref, "s", &s_val) &&
        s_val.kind == PICO8_VALUE_FIXED) {
        spr_index = pico8_fixed_flr_int(s_val.as.fixed);
    } else if (pico8_instance_get_field(self, "s", &s_val) && s_val.kind == PICO8_VALUE_FIXED) {
        spr_index = pico8_fixed_flr_int(s_val.as.fixed);
    }
    pico8_spr(gfx, mem, spr_index, px - 4, py - 4, 2, 2);
    pico8_dv_default(mem);

    if (pico8_jo_truthy(pico8_jo_gg(self))) {
        int mi = pico8_fixed_flr_int(field_fixed(self, "mi"));
        int color = 0;
        pico8_value_t ka_val;
        pico8_value_t ka_entry;

        if (pico8_jo_class() != NULL && pico8_ob_table_get_name(pico8_jo_class()->defaults, "ka", &ka_val) &&
            ka_val.kind == PICO8_VALUE_REF && ka_val.as.ref != NULL &&
            pico8_ob_table_get_index((const pico8_ob_table_t *)ka_val.as.ref, mi, &ka_entry) &&
            ka_entry.kind == PICO8_VALUE_FIXED) {
            color = pico8_fixed_flr_int(ka_entry.as.fixed);
        }
        pico8_circ(gfx, mem, px, py, 6, color);
        pico8_spr_default(gfx, mem, mi + 1, px - 3, py - 6);
    }

    /* `offset` is a stack vector handed over by address: safe only because
     * `mv()` resolves the call before returning. */
    {
        pico8_value_t lq_val;
        if (pico8_instance_get_field(self, "lq", &lq_val) && lq_val.kind == PICO8_VALUE_REF &&
            lq_val.as.ref != NULL) {
            pico8_vec_t offset =
                pico8_vec_add(adjusted, pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(7)));
            pico8_value_t offset_val = pico8_value_ref(&offset);
            pico8_value_t a_val = pico8_value_fixed(PICO8_FIXED_LIT(0x00008000, 0.5));
            pico8_value_t discard;
            int dispatched;
            assert(pico8_type_tag_of(lq_val.as.ref) == PICO8_TYPE_TAG_INSTANCE &&
                   "jo.lq must be an lq instance");
            dispatched = pico8_mv((pico8_instance_t *)lq_val.as.ref, "ol", 2, offset_val, a_val, &discard);
            assert(dispatched && "lq must expose \"ol\" as an mv()-dispatchable field");
            (void)dispatched;
        }
    }

    {
        pico8_value_t fo_val;
        if (pico8_instance_get_field(self, "fo", &fo_val) && pico8_jo_truthy(fo_val) &&
            fo_val.kind == PICO8_VALUE_REF && fo_val.as.ref != NULL) {
            pico8_fixed_t dw = pico8_fixed_from_int(0);
            pico8_value_t dw_val;
            pico8_fixed_t radius_f;
            int radius;

            if (pico8_instance_get_field((const pico8_instance_t *)fo_val.as.ref, "dw", &dw_val) &&
                dw_val.kind == PICO8_VALUE_FIXED) {
                dw = dw_val.as.fixed;
            }
            radius_f = pico8_fixed_mul(pico8_fixed_sqrt(dw), pico8_fixed_from_int(128));
            radius = pico8_fixed_flr_int(radius_f);

            /* The pattern literal's fractional `.1` is PICO-8's
             * transparency flag: its set bits leave the screen untouched,
             * so this is `_transparent`, not plain `pico8_fillp`. */
            pico8_fillp_transparent(gfx, 0xAAAAu);
            pico8_circfill(gfx, mem, px, py, radius, 2);
            pico8_fillp_reset(gfx);
            pico8_ob_table_set_name(self->fields, "fo", pico8_value_ref(NULL));
        }
    }
}
