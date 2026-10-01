#include "pico8_economy_score.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_economy_formulas.h"
#include "pico8_entity.h"
#include "pico8_fm.h"
#include "pico8_game_data.h"
#include "pico8_map_tables.h"
#include "pico8_planet.h"
#include "pico8_str.h"
#include "pico8_trade.h"
#include "pico8_zone_frame.h"

static pico8_fixed_t fixed_min(pico8_fixed_t a, pico8_fixed_t b) {
    return (a < b) ? a : b;
}

typedef void (*cg_visit_fn)(pico8_struct_header_t *e, void *ctx);

static void for_each_cg(const char *tag, cg_visit_fn fn, void *ctx) {
    const pico8_ents_t *list = pico8_cg(tag);
    pico8_ents_iter_t it;
    pico8_struct_header_t *e;
    if (list == NULL) {
        return;
    }
    it = pico8_ents_all(list);
    while (pico8_ents_iter_next(&it, &e)) {
        if (e != NULL) {
            fn(e, ctx);
        }
    }
}

static int call_gg_truthy(pico8_struct_header_t *pt) {
    if (pt == NULL) {
        return 0;
    }
    assert(pico8_jo_is_family(pt) && "gg: pt is a planet of cg.jo");
    return pico8_jo_gg((const pico8_jo_t *)pt);
}

pico8_fixed_t pico8_da(pico8_struct_header_t *pt) {
    if (pt != NULL && (pt->cls->lineage & PICO8_LINEAGE(PICO8_LINEAGE_QH)) && ((const pico8_jo_t *)pt)->def->qh) {
        const pico8_ents_t *qh_list = pico8_cg("qh");
        return pico8_fixed_from_int(qh_list != NULL ? qh_list->count : 0);
    }
    if (call_gg_truthy(pt)) {
        const pico8_jo_t *jo = (const pico8_jo_t *)pt;
        int es;
        if (jo->has_mi &&
            pico8_int_seq_at(pico8_es_table, PICO8_ES_COUNT, pico8_fixed_flr_int(jo->mi), &es)) {
            return pico8_fixed_from_int(es);
        }
    }
    return pico8_fixed_from_int(0);
}

static int cz_predicate(pico8_struct_header_t *e, void *ctx) {
    (void)ctx;
    if (e == NULL) {
        return 0;
    }
    return call_gg_truthy(e);
}

pico8_fixed_t pico8_cz(void) {
    const pico8_ents_t *jo_list = pico8_cg("jo");
    pico8_ents_t filtered = pico8_jp_ents(pico8_zone_frame(), jo_list, cz_predicate, NULL);
    int mu = filtered.count;
    pico8_fixed_t result;
    result = pico8_fixed_mul(PICO8_FIXED_LIT(0x00002e14, 0.18), pico8_fixed_from_int(mu * mu));
    return pico8_fixed_flr(result);
}

pico8_fixed_t pico8_db(const pico8_struct_header_t *pt) {
    pico8_fixed_t total = pico8_fixed_from_int(0);
    const pico8_jo_t *jo;
    const pico8_ents_t *by;
    const pico8_js_t *dk = pico8_dk();

    if (pt == NULL) {
        return total;
    }
    assert(pico8_jo_is_family(pt) && "db: pt is a planet of cg.jo");
    jo = (const pico8_jo_t *)pt;
    by = jo->by;

    if (by != NULL) {
        int i;
        for (i = 0; i < by->count; i++) {
            pico8_struct_header_t *e = by->items[i];
            pico8_struct_header_t *oz_val;
            if (e == NULL) {
                continue;
            }
            if (!pico8_trade_to((const pico8_trade_t *)e, &oz_val) || oz_val == NULL) {
                continue;
            }
            {
                const pico8_jo_t *oz;
                assert(pico8_jo_is_family(oz_val) && "db: a trade leads to a planet");
                oz = (const pico8_jo_t *)oz_val;
                if (oz->def->hf) {
                    pico8_fixed_t trade_mult = pico8_fixed_from_int(0);
                    pico8_fixed_t hs = jo->def->hs;
                    pico8_fixed_t ct_pt = pico8_fixed_from_int(0);
                    pico8_fixed_t ct_oz = pico8_fixed_from_int(0);
                    pico8_fixed_t bonus;

                    if (dk != NULL && dk->has_trade) {
                        trade_mult = dk->trade;
                    }
                    if (jo->has_mi) {
                        pico8_fixed_seq_at(pico8_ct_table, PICO8_CT_COUNT, pico8_fixed_flr_int(jo->mi), &ct_pt);
                    }
                    if (oz->has_mi) {
                        pico8_fixed_seq_at(pico8_ct_table, PICO8_CT_COUNT, pico8_fixed_flr_int(oz->mi),
                                           &ct_oz);
                    }
                    bonus = pico8_fixed_add(pico8_fixed_from_int(1), pico8_fixed_add(ct_pt, ct_oz));
                    total = pico8_fixed_add(total, pico8_fixed_mul(pico8_fixed_mul(trade_mult, hs), bonus));
                }
            }
        }
    }

    {
        const pico8_kw_t *kw = jo->kw;
        if (kw != NULL) {
            int w;
            if (kw->has_qe && pico8_r_lookup("w", &w) && kw->qe == pico8_fixed_from_int(w)) {
                pico8_fixed_t mu = kw->mu;
                pico8_fixed_t trade_mult = pico8_fixed_from_int(0);
                if (dk != NULL && dk->has_trade) {
                    trade_mult = dk->trade;
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
    const int *seq;
    int count;
} sum_by_mi_ctx_t;

static void sum_by_mi_visit(pico8_struct_header_t *pt, void *ctx_) {
    sum_by_mi_ctx_t *ctx = (sum_by_mi_ctx_t *)ctx_;
    const pico8_jo_t *jo;
    int v;
    if (!call_gg_truthy(pt)) {
        return;
    }
    jo = (const pico8_jo_t *)pt;
    if (jo->has_mi &&
        pico8_int_seq_at(ctx->seq, ctx->count, pico8_fixed_flr_int(jo->mi), &v)) {
        ctx->total = pico8_fixed_add(ctx->total, pico8_fixed_from_int(v));
    }
}

static char *concat_percent(const char *s) {
    size_t len = strlen(s);
    char *out = (char *)pico8_zone_alloc(pico8_zone_frame(), len + 2);
    memcpy(out, s, len);
    out[len] = '%';
    out[len + 1] = '\0';
    return out;
}

pico8_ky_t pico8_ky(void) {
    sum_by_mi_ctx_t di_ctx;
    pico8_fixed_t gl = pico8_fixed_from_int(0);
    pico8_fixed_t base;
    pico8_fixed_t el;
    pico8_fixed_t total;
    const pico8_hy_t *fm = pico8_fm_hy();
    pico8_fixed_t happiness = fm != NULL && fm->has_happiness ? fm->happiness : pico8_fixed_from_int(0);
    const pico8_ptrs_t *invented = fm != NULL ? fm->invented : NULL;
    int invented_count = (invented != NULL) ? invented->count : 0;
    const pico8_qe_map_t *fm_po = fm != NULL ? fm->po : NULL;
    char *happiness_str;
    pico8_ky_t result;

    di_ctx.total = pico8_fixed_from_int(0);
    di_ctx.seq = pico8_b__table;
    di_ctx.count = PICO8_B__COUNT;
    for_each_cg("jo", sum_by_mi_visit, &di_ctx);

    if (fm_po != NULL) {
        int p;
        pico8_fixed_t gl_val;
        if (pico8_r_lookup("p", &p) && pico8_qe_map_get(fm_po, pico8_fixed_from_int(p), &gl_val)) {
            gl = pico8_fixed_mul(gl_val, pico8_fixed_from_int(40));
        }
    }

    base = pico8_fixed_mul(pico8_fixed_from_int(invented_count), PICO8_FIXED_LIT(0x00013333, 1.2));
    el = pico8_fixed_mul(pico8_fixed_flr(pico8_fixed_mul(base, base)), pico8_fixed_from_int(10));

    total = pico8_fixed_flr(pico8_fixed_mul(
        pico8_fixed_mul(pico8_fixed_add(pico8_fixed_add(di_ctx.total, gl), el), PICO8_FIXED_LIT(0x0000028f, 0.01)),
        happiness));

    happiness_str = pico8_tostr_fixed(pico8_zone_frame(), happiness);

    result.di = di_ctx.total;
    result.gl = gl;
    result.el = el;
    result.happiness = concat_percent(happiness_str);
    result.total = total;
    result.tier = fixed_min(pico8_fixed_flr(pico8_fixed_div(total, pico8_fixed_from_int(2000))),
                            pico8_fixed_from_int(5));
    return result;
}

pico8_fixed_t pico8_bf(void) {
    const pico8_hy_t *fm = pico8_fm_hy();
    const pico8_qe_map_t *fm_po = fm != NULL ? fm->po : NULL;
    pico8_fixed_t r_h = pico8_fixed_from_int(0);
    pico8_fixed_t r_w = pico8_fixed_from_int(0);
    pico8_fixed_t h_val;
    sum_by_mi_ctx_t ctx;

    if (fm_po != NULL) {
        int h_key;
        int w_key;
        if (pico8_r_lookup("h", &h_key)) {
            pico8_qe_map_get(fm_po, pico8_fixed_from_int(h_key), &r_h);
        }
        if (pico8_r_lookup("w", &w_key)) {
            pico8_qe_map_get(fm_po, pico8_fixed_from_int(w_key), &r_w);
        }
    }
    h_val =
        pico8_fixed_add(pico8_fixed_from_int(100), pico8_fixed_mul(pico8_fixed_from_int(2), pico8_fixed_add(r_h, r_w)));

    ctx.total = pico8_fixed_from_int(0);
    ctx.seq = pico8_co_table;
    ctx.count = PICO8_CO_COUNT;
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

pico8_fixed_t pico8_kr(pico8_struct_header_t *p) {
    pico8_fixed_t kr = pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)));
    pico8_jo_t *jo;

    assert(pico8_jo_is_family(p) && "hy:lj: cg.jo holds planets only");
    jo = (pico8_jo_t *)p;
    jo->kr = kr;
    jo->has_kr = 1;
    return kr;
}
