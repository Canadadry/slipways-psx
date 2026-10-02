#include "game/economy_score.h"

#include <assert.h>
#include <string.h>

#include "game/economy_formulas.h"
#include "engine/entity_registry.h"
#include "game/main_economy.h"
#include "game/game_data.h"
#include "game/map_tables.h"
#include "game/planet.h"
#include "p8/str.h"
#include "game/trade.h"
#include "engine/frame_zone.h"

static p8_fixed_t fixed_min(p8_fixed_t a, p8_fixed_t b) {
    return (a < b) ? a : b;
}

typedef void (*cg_visit_fn)(entity_t *e, void *ctx);

static void for_each_cg(const char *tag, cg_visit_fn fn, void *ctx) {
    const entity_list_t *list = entities_by_tag(tag);
    entity_list_iter_t it;
    entity_t *e;
    if (list == NULL) {
        return;
    }
    it = entity_list_all(list);
    while (entity_list_iter_next(&it, &e)) {
        if (e != NULL) {
            fn(e, ctx);
        }
    }
}

static int call_gg_truthy(entity_t *pt) {
    if (pt == NULL) {
        return 0;
    }
    assert(planet_is_family(pt) && "gg: pt is a planet of cg.jo");
    return planet_is_colonized((const planet_t *)pt);
}

p8_fixed_t pico8_da(entity_t *pt) {
    if (pt != NULL && (pt->class->lineage & LINEAGE(LINEAGE_STRUCTURE)) && ((const planet_t *)pt)->archetype->is_structure) {
        const entity_list_t *qh_list = entities_by_tag("structure");
        return p8_fixed_from_int(qh_list != NULL ? qh_list->count : 0);
    }
    if (call_gg_truthy(pt)) {
        const planet_t *jo = (const planet_t *)pt;
        int es;
        if (jo->has_development_tier &&
            int_seq_at(upkeep_by_tier_table, UPKEEP_BY_TIER_COUNT, p8_fixed_flr_int(jo->development_tier), &es)) {
            return p8_fixed_from_int(es);
        }
    }
    return p8_fixed_from_int(0);
}

static int cz_predicate(entity_t *e, void *ctx) {
    (void)ctx;
    if (e == NULL) {
        return 0;
    }
    return call_gg_truthy(e);
}

p8_fixed_t pico8_cz(void) {
    const entity_list_t *jo_list = entities_by_tag("planet");
    entity_list_t filtered = entity_list_filter(frame_zone(), jo_list, cz_predicate, NULL);
    int mu = filtered.count;
    p8_fixed_t result;
    result = p8_fixed_mul(P8_FIXED_LIT(0x00002e14, 0.18), p8_fixed_from_int(mu * mu));
    return p8_fixed_flr(result);
}

p8_fixed_t pico8_db(const entity_t *pt) {
    p8_fixed_t total = p8_fixed_from_int(0);
    const planet_t *jo;
    const entity_list_t *by;
    const pico8_js_t *dk = pico8_dk();

    if (pt == NULL) {
        return total;
    }
    assert(planet_is_family(pt) && "db: pt is a planet of cg.jo");
    jo = (const planet_t *)pt;
    by = jo->outgoing_routes;

    if (by != NULL) {
        int i;
        for (i = 0; i < by->count; i++) {
            entity_t *e = by->items[i];
            entity_t *oz_val;
            if (e == NULL) {
                continue;
            }
            if (!trade_last_planet((const trade_route_t *)e, &oz_val) || oz_val == NULL) {
                continue;
            }
            {
                const planet_t *oz;
                assert(planet_is_family(oz_val) && "db: a trade leads to a planet");
                oz = (const planet_t *)oz_val;
                if (oz->archetype->is_planet) {
                    p8_fixed_t trade_mult = p8_fixed_from_int(0);
                    p8_fixed_t hs = jo->archetype->trade_multiplier;
                    p8_fixed_t ct_pt = p8_fixed_from_int(0);
                    p8_fixed_t ct_oz = p8_fixed_from_int(0);
                    p8_fixed_t bonus;

                    if (dk != NULL && dk->has_trade) {
                        trade_mult = dk->trade;
                    }
                    if (jo->has_development_tier) {
                        game_data_fixed_seq_at(partner_bonus_by_tier_table, PARTNER_BONUS_BY_TIER_COUNT, p8_fixed_flr_int(jo->development_tier), &ct_pt);
                    }
                    if (oz->has_development_tier) {
                        game_data_fixed_seq_at(partner_bonus_by_tier_table, PARTNER_BONUS_BY_TIER_COUNT, p8_fixed_flr_int(oz->development_tier),
                                           &ct_oz);
                    }
                    bonus = p8_fixed_add(p8_fixed_from_int(1), p8_fixed_add(ct_pt, ct_oz));
                    total = p8_fixed_add(total, p8_fixed_mul(p8_fixed_mul(trade_mult, hs), bonus));
                }
            }
        }
    }

    {
        const production_t *kw = jo->production;
        if (kw != NULL) {
            int w;
            if (kw->has_resource && resource_code_lookup("w", &w) && kw->resource == p8_fixed_from_int(w)) {
                p8_fixed_t mu = kw->amount;
                p8_fixed_t trade_mult = p8_fixed_from_int(0);
                if (dk != NULL && dk->has_trade) {
                    trade_mult = dk->trade;
                }
                total = p8_fixed_add(
                    total, p8_fixed_mul(p8_fixed_mul(mu, trade_mult), P8_FIXED_LIT(0x00018000, 1.5)));
            }
        }
    }

    return total;
}

typedef struct {
    p8_fixed_t total;
    const int *seq;
    int count;
} sum_by_mi_ctx_t;

static void sum_by_mi_visit(entity_t *pt, void *ctx_) {
    sum_by_mi_ctx_t *ctx = (sum_by_mi_ctx_t *)ctx_;
    const planet_t *jo;
    int v;
    if (!call_gg_truthy(pt)) {
        return;
    }
    jo = (const planet_t *)pt;
    if (jo->has_development_tier &&
        int_seq_at(ctx->seq, ctx->count, p8_fixed_flr_int(jo->development_tier), &v)) {
        ctx->total = p8_fixed_add(ctx->total, p8_fixed_from_int(v));
    }
}

static char *concat_percent(const char *s) {
    size_t len = strlen(s);
    char *out = (char *)allocator_alloc(frame_zone(), len + 2);
    memcpy(out, s, len);
    out[len] = '%';
    out[len + 1] = '\0';
    return out;
}

pico8_ky_t pico8_ky(void) {
    sum_by_mi_ctx_t di_ctx;
    p8_fixed_t gl = p8_fixed_from_int(0);
    p8_fixed_t base;
    p8_fixed_t el;
    p8_fixed_t total;
    const pico8_hy_t *fm = pico8_fm_hy();
    p8_fixed_t happiness = fm != NULL && fm->has_happiness ? fm->happiness : p8_fixed_from_int(0);
    const pointer_list_t *invented = fm != NULL ? fm->invented : NULL;
    int invented_count = (invented != NULL) ? invented->count : 0;
    const pico8_qe_map_t *fm_po = fm != NULL ? fm->po : NULL;
    char *happiness_str;
    pico8_ky_t result;

    di_ctx.total = p8_fixed_from_int(0);
    di_ctx.seq = development_score_by_tier_table;
    di_ctx.count = DEVELOPMENT_SCORE_BY_TIER_COUNT;
    for_each_cg("planet", sum_by_mi_visit, &di_ctx);

    if (fm_po != NULL) {
        int p;
        p8_fixed_t gl_val;
        if (resource_code_lookup("p", &p) && pico8_qe_map_get(fm_po, p8_fixed_from_int(p), &gl_val)) {
            gl = p8_fixed_mul(gl_val, p8_fixed_from_int(40));
        }
    }

    base = p8_fixed_mul(p8_fixed_from_int(invented_count), P8_FIXED_LIT(0x00013333, 1.2));
    el = p8_fixed_mul(p8_fixed_flr(p8_fixed_mul(base, base)), p8_fixed_from_int(10));

    total = p8_fixed_flr(p8_fixed_mul(
        p8_fixed_mul(p8_fixed_add(p8_fixed_add(di_ctx.total, gl), el), P8_FIXED_LIT(0x0000028f, 0.01)),
        happiness));

    happiness_str = p8_tostr_fixed(frame_zone(), happiness);

    result.di = di_ctx.total;
    result.gl = gl;
    result.el = el;
    result.happiness = concat_percent(happiness_str);
    result.total = total;
    result.tier = fixed_min(p8_fixed_flr(p8_fixed_div(total, p8_fixed_from_int(2000))),
                            p8_fixed_from_int(5));
    return result;
}

p8_fixed_t pico8_bf(void) {
    const pico8_hy_t *fm = pico8_fm_hy();
    const pico8_qe_map_t *fm_po = fm != NULL ? fm->po : NULL;
    p8_fixed_t r_h = p8_fixed_from_int(0);
    p8_fixed_t r_w = p8_fixed_from_int(0);
    p8_fixed_t h_val;
    sum_by_mi_ctx_t ctx;

    if (fm_po != NULL) {
        int h_key;
        int w_key;
        if (resource_code_lookup("h", &h_key)) {
            pico8_qe_map_get(fm_po, p8_fixed_from_int(h_key), &r_h);
        }
        if (resource_code_lookup("w", &w_key)) {
            pico8_qe_map_get(fm_po, p8_fixed_from_int(w_key), &r_w);
        }
    }
    h_val =
        p8_fixed_add(p8_fixed_from_int(100), p8_fixed_mul(p8_fixed_from_int(2), p8_fixed_add(r_h, r_w)));

    ctx.total = p8_fixed_from_int(0);
    ctx.seq = happiness_by_tier_table;
    ctx.count = HAPPINESS_BY_TIER_COUNT;
    for_each_cg("planet", sum_by_mi_visit, &ctx);

    return p8_fixed_add(h_val, ctx.total);
}

static const pico8_js_t *g_dk = NULL;

const pico8_js_t *pico8_dk(void) {
    return g_dk;
}

void pico8_economy_set_dk(const pico8_js_t *dk) {
    g_dk = dk;
}

void pico8_economy_dk_quitte(const entity_t *e) {
    static pico8_js_t copie;

    if (g_dk == NULL || &g_dk->header != e) {
        return;
    }
    memset(&copie, 0, sizeof copie);
    copie.px = g_dk->px;
    copie.has_trade = g_dk->has_trade;
    copie.trade = g_dk->trade;
    copie.has_en = g_dk->has_en;
    copie.en = g_dk->en;
    g_dk = &copie;
}

p8_fixed_t pico8_kr(entity_t *p) {
    p8_fixed_t kr = p8_fixed_flr(p8_fixed_sub(pico8_db(p), pico8_da(p)));
    planet_t *jo;

    assert(planet_is_family(p) && "hy:lj: cg.jo holds planets only");
    jo = (planet_t *)p;
    jo->net_income = kr;
    jo->has_net_income = 1;
    return kr;
}
