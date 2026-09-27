#include "pico8_economy_score.h"

#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_entity.h"
#include "pico8_game_data.h"
#include "pico8_probe.h"
#include "pico8_stash.h"
#include "pico8_trade.h"
#include "pico8_ref_audit.h"

static int economy_truthy(pico8_value_t v) {
    return !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

static int economy_value_equal_content(pico8_value_t a, pico8_value_t b) {
    if (a.kind == PICO8_VALUE_STRING && b.kind == PICO8_VALUE_STRING) {
        return strcmp(a.as.string, b.as.string) == 0;
    }
    return pico8_value_equal(a, b);
}

static pico8_fixed_t fixed_min(pico8_fixed_t a, pico8_fixed_t b) {
    return (a < b) ? a : b;
}

static pico8_fixed_t field_fixed(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static int field_truthy(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst == NULL || !pico8_entity_get_field(inst, name, &v)) {
        return 0;
    }
    return economy_truthy(v);
}

static const pico8_ob_table_t *field_ref_table(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (const pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

static const pico8_list_t *field_list(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return PICO8_REF_AS_LIST_CONST(v.as.ref);
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

typedef void (*cg_visit_fn)(pico8_struct_header_t *e, void *ctx);

static void for_each_cg(const char *tag, cg_visit_fn fn, void *ctx) {
    const pico8_list_t *list = pico8_cg(tag);
    pico8_list_iter_t it;
    pico8_value_t e;
    if (list == NULL) {
        return;
    }
    it = pico8_list_all(list);
    while (pico8_list_iter_next(&it, &e)) {
        if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
            fn((pico8_struct_header_t *)e.as.ref, ctx);
        }
    }
}

static int call_gg_truthy(pico8_struct_header_t *pt) {
    pico8_value_t out;
    if (pt == NULL) {
        return 0;
    }
    if (!pico8_mv(pt, "gg", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &out)) {
        return 0;
    }
    return economy_truthy(out);
}

pico8_fixed_t pico8_da(pico8_struct_header_t *pt) {
    if (field_truthy(pt, "qh")) {
        const pico8_list_t *qh_list = pico8_cg("qh");
        return pico8_fixed_from_int(qh_list != NULL ? qh_list->count : 0);
    }
    if (call_gg_truthy(pt)) {
        const pico8_ob_table_t *es = pico8_es();
        pico8_value_t mi_val;
        pico8_value_t es_val;
        if (es != NULL && pico8_entity_get_field(pt, "mi", &mi_val) && mi_val.kind == PICO8_VALUE_FIXED &&
            pico8_ob_table_get_index(es, pico8_fixed_flr_int(mi_val.as.fixed), &es_val) &&
            es_val.kind == PICO8_VALUE_FIXED) {
            return es_val.as.fixed;
        }
    }
    return pico8_fixed_from_int(0);
}

static int cz_predicate(pico8_value_t e, void *ctx) {
    (void)ctx;
    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    return call_gg_truthy((pico8_struct_header_t *)e.as.ref);
}

pico8_fixed_t pico8_cz(void) {
    const pico8_list_t *jo_list = pico8_cg("jo");
    pico8_list_t filtered = pico8_jp(jo_list, cz_predicate, NULL);
    int mu = filtered.count;
    pico8_fixed_t result;
    pico8_list_free(&filtered);
    result = pico8_fixed_mul(PICO8_FIXED_LIT(0x00002e14, 0.18), pico8_fixed_from_int(mu * mu));
    return pico8_fixed_flr(result);
}

pico8_fixed_t pico8_db(const pico8_struct_header_t *pt) {
    pico8_fixed_t total = pico8_fixed_from_int(0);
    const pico8_list_t *by = field_list(pt, "by");
    const pico8_ob_table_t *ct = pico8_ct();
    const pico8_ob_table_t *r = pico8_r();
    const pico8_js_t *dk = pico8_dk();
    pico8_value_t pt_mi_val;
    pico8_fixed_t pt_mi = pico8_fixed_from_int(0);
    int has_pt_mi = pico8_entity_get_field(pt, "mi", &pt_mi_val) && pt_mi_val.kind == PICO8_VALUE_FIXED;
    if (has_pt_mi) {
        pt_mi = pt_mi_val.as.fixed;
    }

    if (by != NULL) {
        int i;
        for (i = 0; i < by->count; i++) {
            pico8_value_t e_val = by->items[i];
            pico8_struct_header_t *e;
            pico8_value_t oz_val;
            if (e_val.kind != PICO8_VALUE_REF || e_val.as.ref == NULL) {
                continue;
            }
            e = (pico8_struct_header_t *)e_val.as.ref;
            if (!pico8_trade_to((const pico8_trade_t *)e, &oz_val) || oz_val.kind != PICO8_VALUE_REF ||
                oz_val.as.ref == NULL) {
                continue;
            }
            {
                pico8_struct_header_t *oz = (pico8_struct_header_t *)oz_val.as.ref;
                if (field_truthy(oz, "hf")) {
                    pico8_fixed_t trade_mult = pico8_fixed_from_int(0);
                    pico8_fixed_t hs = field_fixed(pt, "hs");
                    pico8_fixed_t ct_pt = pico8_fixed_from_int(0);
                    pico8_fixed_t ct_oz = pico8_fixed_from_int(0);
                    pico8_fixed_t bonus;
                    pico8_value_t trade_val;
                    pico8_value_t oz_mi_val;

                    if (dk != NULL && pico8_entity_get_field(dk, "trade", &trade_val) &&
                        trade_val.kind == PICO8_VALUE_FIXED) {
                        trade_mult = trade_val.as.fixed;
                    }
                    if (ct != NULL && has_pt_mi) {
                        pico8_value_t v;
                        if (pico8_ob_table_get_index(ct, pico8_fixed_flr_int(pt_mi), &v) &&
                            v.kind == PICO8_VALUE_FIXED) {
                            ct_pt = v.as.fixed;
                        }
                    }
                    if (ct != NULL && pico8_entity_get_field(oz, "mi", &oz_mi_val) &&
                        oz_mi_val.kind == PICO8_VALUE_FIXED) {
                        pico8_value_t v;
                        if (pico8_ob_table_get_index(ct, pico8_fixed_flr_int(oz_mi_val.as.fixed), &v) &&
                            v.kind == PICO8_VALUE_FIXED) {
                            ct_oz = v.as.fixed;
                        }
                    }
                    bonus = pico8_fixed_add(pico8_fixed_from_int(1), pico8_fixed_add(ct_pt, ct_oz));
                    total = pico8_fixed_add(total, pico8_fixed_mul(pico8_fixed_mul(trade_mult, hs), bonus));
                }
            }
        }
    }

    {
        const pico8_ob_table_t *kw = field_ref_table(pt, "kw");
        if (kw != NULL && r != NULL) {
            pico8_value_t qe_val;
            pico8_value_t w_val;
            /* TROU-3 */
            if (pico8_ob_table_get_name(kw, "qe", &qe_val) && pico8_ob_table_get_name(r, "w", &w_val) &&
                economy_value_equal_content(qe_val, w_val)) {
                pico8_fixed_t mu = pico8_fixed_from_int(0);
                pico8_value_t mu_val;
                pico8_fixed_t trade_mult = pico8_fixed_from_int(0);
                pico8_value_t trade_val;
                if (pico8_ob_table_get_name(kw, "mu", &mu_val) && mu_val.kind == PICO8_VALUE_FIXED) {
                    mu = mu_val.as.fixed;
                }
                if (dk != NULL && pico8_entity_get_field(dk, "trade", &trade_val) &&
                    trade_val.kind == PICO8_VALUE_FIXED) {
                    trade_mult = trade_val.as.fixed;
                }
                total = pico8_fixed_add(
                    total, pico8_fixed_mul(pico8_fixed_mul(mu, trade_mult), PICO8_FIXED_LIT(0x00018000, 1.5)));
            }
        }
    }

    return total;
}

typedef struct {
    pico8_fixed_t total;
    const pico8_ob_table_t *table;
} sum_by_mi_ctx_t;

static void sum_by_mi_visit(pico8_struct_header_t *pt, void *ctx_) {
    sum_by_mi_ctx_t *ctx = (sum_by_mi_ctx_t *)ctx_;
    pico8_value_t mi_val;
    pico8_value_t v;
    if (!call_gg_truthy(pt) || ctx->table == NULL) {
        return;
    }
    if (pico8_entity_get_field(pt, "mi", &mi_val) && mi_val.kind == PICO8_VALUE_FIXED &&
        pico8_ob_table_get_index(ctx->table, pico8_fixed_flr_int(mi_val.as.fixed), &v) &&
        v.kind == PICO8_VALUE_FIXED) {
        ctx->total = pico8_fixed_add(ctx->total, v.as.fixed);
    }
}

static char *concat_percent(const char *s) {
    size_t len = strlen(s);
    char *out = malloc(len + 2);
    memcpy(out, s, len);
    out[len] = '%';
    out[len + 1] = '\0';
    return out;
}

pico8_ob_table_t *pico8_ky(void) {
    sum_by_mi_ctx_t di_ctx;
    pico8_fixed_t gl = pico8_fixed_from_int(0);
    pico8_fixed_t base;
    pico8_fixed_t el;
    pico8_fixed_t total;
    pico8_fixed_t happiness = field_fixed(pico8_fm(), "happiness");
    const pico8_list_t *invented = field_list(pico8_fm(), "invented");
    int invented_count = (invented != NULL) ? invented->count : 0;
    const pico8_ob_table_t *r = pico8_r();
    const pico8_ob_table_t *fm_po = field_ref_table(pico8_fm(), "po");
    char *happiness_str;
    pico8_ob_table_t *result;

    di_ctx.total = pico8_fixed_from_int(0);
    di_ctx.table = pico8_b_();
    for_each_cg("jo", sum_by_mi_visit, &di_ctx);

    if (fm_po != NULL && r != NULL) {
        pico8_value_t p_val;
        pico8_value_t gl_val;
        if (pico8_ob_table_get_name(r, "p", &p_val) && get_by_value(fm_po, p_val, &gl_val) &&
            gl_val.kind == PICO8_VALUE_FIXED) {
            gl = pico8_fixed_mul(gl_val.as.fixed, pico8_fixed_from_int(40));
        }
    }

    base = pico8_fixed_mul(pico8_fixed_from_int(invented_count), PICO8_FIXED_LIT(0x00013333, 1.2));
    el = pico8_fixed_mul(pico8_fixed_flr(pico8_fixed_mul(base, base)), pico8_fixed_from_int(10));

    total = pico8_fixed_flr(pico8_fixed_mul(
        pico8_fixed_mul(pico8_fixed_add(pico8_fixed_add(di_ctx.total, gl), el), PICO8_FIXED_LIT(0x0000028f, 0.01)),
        happiness));

    happiness_str = pico8_value_tostr(pico8_value_fixed(happiness));

    result = pico8_ob_table_new();
    pico8_ob_table_set_index(result, 1, pico8_value_fixed(di_ctx.total));
    pico8_ob_table_set_index(result, 2, pico8_value_fixed(gl));
    pico8_ob_table_set_index(result, 3, pico8_value_fixed(el));
    /* TROU-7 */
    pico8_ob_table_set_index(result, 4, pico8_value_string(concat_percent(happiness_str)));
    free(happiness_str);
    pico8_ob_table_set_index(result, 5, pico8_value_fixed(total));
    pico8_ob_table_set_index(
        result, 6, pico8_value_fixed(fixed_min(pico8_fixed_flr(pico8_fixed_div(total, pico8_fixed_from_int(2000))),
                                                pico8_fixed_from_int(5))));
    return result;
}

pico8_fixed_t pico8_bf(void) {
    const pico8_ob_table_t *fm_po = field_ref_table(pico8_fm(), "po");
    const pico8_ob_table_t *r = pico8_r();
    const pico8_ob_table_t *co = pico8_co();
    pico8_fixed_t r_h = pico8_fixed_from_int(0);
    pico8_fixed_t r_w = pico8_fixed_from_int(0);
    pico8_fixed_t h_val;
    sum_by_mi_ctx_t ctx;

    if (fm_po != NULL && r != NULL) {
        pico8_value_t h_key;
        pico8_value_t w_key;
        pico8_value_t hv;
        pico8_value_t wv;
        if (pico8_ob_table_get_name(r, "h", &h_key) && get_by_value(fm_po, h_key, &hv) &&
            hv.kind == PICO8_VALUE_FIXED) {
            r_h = hv.as.fixed;
        }
        if (pico8_ob_table_get_name(r, "w", &w_key) && get_by_value(fm_po, w_key, &wv) &&
            wv.kind == PICO8_VALUE_FIXED) {
            r_w = wv.as.fixed;
        }
    }
    h_val =
        pico8_fixed_add(pico8_fixed_from_int(100), pico8_fixed_mul(pico8_fixed_from_int(2), pico8_fixed_add(r_h, r_w)));

    ctx.total = pico8_fixed_from_int(0);
    ctx.table = co;
    for_each_cg("jo", sum_by_mi_visit, &ctx);

    return pico8_fixed_add(h_val, ctx.total);
}

static const pico8_js_t *g_dk = NULL;

const pico8_js_t *pico8_dk(void) {
    return g_dk;
}

void pico8_economy_set_dk(const pico8_js_t *dk) {
    g_dk = dk;
}
