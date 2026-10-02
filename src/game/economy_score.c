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

typedef void (*entities_by_tag_visit_fn_t)(entity_t *e, void *ctx);

static void for_each_tagged(const char *tag, entities_by_tag_visit_fn_t fn, void *ctx) {
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

static int is_colonized_truthy(entity_t *pt) {
    if (pt == NULL) {
        return 0;
    }
    assert(planet_is_family(pt) && "is_colonized_truthy: the entity is a planet");
    return planet_is_colonized((const planet_t *)pt);
}

p8_fixed_t planet_upkeep(entity_t *pt) {
    if (pt != NULL && (pt->class->lineage & LINEAGE(LINEAGE_STRUCTURE)) && ((const planet_t *)pt)->archetype->is_structure) {
        const entity_list_t *structure_list = entities_by_tag("structure");
        return p8_fixed_from_int(structure_list != NULL ? structure_list->count : 0);
    }
    if (is_colonized_truthy(pt)) {
        const planet_t *jo = (const planet_t *)pt;
        int es;
        if (jo->has_development_tier &&
            int_seq_at(upkeep_by_tier_table, UPKEEP_BY_TIER_COUNT, p8_fixed_flr_int(jo->development_tier), &es)) {
            return p8_fixed_from_int(es);
        }
    }
    return p8_fixed_from_int(0);
}

static int colony_upkeep_predicate(entity_t *e, void *ctx) {
    (void)ctx;
    if (e == NULL) {
        return 0;
    }
    return is_colonized_truthy(e);
}

p8_fixed_t economy_score_colony_upkeep(void) {
    const entity_list_t *planet_list = entities_by_tag("planet");
    entity_list_t filtered = entity_list_filter(frame_zone(), planet_list, colony_upkeep_predicate, NULL);
    int mu = filtered.count;
    p8_fixed_t result;
    result = p8_fixed_mul(P8_FIXED_LIT(0x00002e14, 0.18), p8_fixed_from_int(mu * mu));
    return p8_fixed_flr(result);
}

p8_fixed_t economy_score_trade_income(const entity_t *entity) {
    p8_fixed_t total = p8_fixed_from_int(0);
    const planet_t *planet;
    const entity_list_t *outgoing;
    const button_t *difficulty = economy_score_difficulty();

    if (entity == NULL) {
        return total;
    }
    assert(planet_is_family(entity) && "economy_score_trade_income: the entity is a planet");
    planet = (const planet_t *)entity;
    outgoing = planet->outgoing_routes;

    if (outgoing != NULL) {
        int i;
        for (i = 0; i < outgoing->count; i++) {
            entity_t *e = outgoing->items[i];
            entity_t *partner;
            if (e == NULL) {
                continue;
            }
            if (!trade_last_planet((const trade_route_t *)e, &partner) || partner == NULL) {
                continue;
            }
            {
                const planet_t *oz;
                assert(planet_is_family(partner) && "economy_score_trade_income: a trade leads to a planet");
                oz = (const planet_t *)partner;
                if (oz->archetype->is_planet) {
                    p8_fixed_t trade_mult = p8_fixed_from_int(0);
                    p8_fixed_t hs = planet->archetype->trade_multiplier;
                    p8_fixed_t planet_bonus = p8_fixed_from_int(0);
                    p8_fixed_t partner_bonus = p8_fixed_from_int(0);
                    p8_fixed_t bonus;

                    if (difficulty != NULL && difficulty->has_trade) {
                        trade_mult = difficulty->trade;
                    }
                    if (planet->has_development_tier) {
                        game_data_fixed_seq_at(partner_bonus_by_tier_table, PARTNER_BONUS_BY_TIER_COUNT, p8_fixed_flr_int(planet->development_tier), &planet_bonus);
                    }
                    if (oz->has_development_tier) {
                        game_data_fixed_seq_at(partner_bonus_by_tier_table, PARTNER_BONUS_BY_TIER_COUNT, p8_fixed_flr_int(oz->development_tier),
                                           &partner_bonus);
                    }
                    bonus = p8_fixed_add(p8_fixed_from_int(1), p8_fixed_add(planet_bonus, partner_bonus));
                    total = p8_fixed_add(total, p8_fixed_mul(p8_fixed_mul(trade_mult, hs), bonus));
                }
            }
        }
    }

    {
        const production_t *kw = planet->production;
        if (kw != NULL) {
            int w;
            if (kw->has_resource && resource_code_lookup("w", &w) && kw->resource == p8_fixed_from_int(w)) {
                p8_fixed_t mu = kw->amount;
                p8_fixed_t trade_mult = p8_fixed_from_int(0);
                if (difficulty != NULL && difficulty->has_trade) {
                    trade_mult = difficulty->trade;
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
} sum_by_development_tier_ctx_t;

static void sum_by_development_tier_visit(entity_t *pt, void *opaque_ctx) {
    sum_by_development_tier_ctx_t *ctx = (sum_by_development_tier_ctx_t *)opaque_ctx;
    const planet_t *jo;
    int v;
    if (!is_colonized_truthy(pt)) {
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

final_score_t economy_score_final(void) {
    sum_by_development_tier_ctx_t development_ctx;
    p8_fixed_t population = p8_fixed_from_int(0);
    p8_fixed_t base;
    p8_fixed_t technology;
    p8_fixed_t total;
    const economy_t *fm = main_economy_typed();
    p8_fixed_t happiness = fm != NULL && fm->has_happiness ? fm->happiness : p8_fixed_from_int(0);
    const pointer_list_t *invented = fm != NULL ? fm->invented : NULL;
    int invented_count = (invented != NULL) ? invented->count : 0;
    const resource_map_t *main_economy_production = fm != NULL ? fm->production : NULL;
    char *happiness_str;
    final_score_t result;

    development_ctx.total = p8_fixed_from_int(0);
    development_ctx.seq = development_score_by_tier_table;
    development_ctx.count = DEVELOPMENT_SCORE_BY_TIER_COUNT;
    for_each_tagged("planet", sum_by_development_tier_visit, &development_ctx);

    if (main_economy_production != NULL) {
        int p;
        p8_fixed_t population_val;
        if (resource_code_lookup("p", &p) && resource_map_get(main_economy_production, p8_fixed_from_int(p), &population_val)) {
            population = p8_fixed_mul(population_val, p8_fixed_from_int(40));
        }
    }

    base = p8_fixed_mul(p8_fixed_from_int(invented_count), P8_FIXED_LIT(0x00013333, 1.2));
    technology = p8_fixed_mul(p8_fixed_flr(p8_fixed_mul(base, base)), p8_fixed_from_int(10));

    total = p8_fixed_flr(p8_fixed_mul(
        p8_fixed_mul(p8_fixed_add(p8_fixed_add(development_ctx.total, population), technology), P8_FIXED_LIT(0x0000028f, 0.01)),
        happiness));

    happiness_str = p8_tostr_fixed(frame_zone(), happiness);

    result.development = development_ctx.total;
    result.population = population;
    result.technology = technology;
    result.happiness = concat_percent(happiness_str);
    result.total = total;
    result.tier = fixed_min(p8_fixed_flr(p8_fixed_div(total, p8_fixed_from_int(2000))),
                            p8_fixed_from_int(5));
    return result;
}

p8_fixed_t economy_score_happiness(void) {
    const economy_t *fm = main_economy_typed();
    const resource_map_t *main_economy_production = fm != NULL ? fm->production : NULL;
    p8_fixed_t r_h = p8_fixed_from_int(0);
    p8_fixed_t r_w = p8_fixed_from_int(0);
    p8_fixed_t h_val;
    sum_by_development_tier_ctx_t ctx;

    if (main_economy_production != NULL) {
        int h_key;
        int w_key;
        if (resource_code_lookup("h", &h_key)) {
            resource_map_get(main_economy_production, p8_fixed_from_int(h_key), &r_h);
        }
        if (resource_code_lookup("w", &w_key)) {
            resource_map_get(main_economy_production, p8_fixed_from_int(w_key), &r_w);
        }
    }
    h_val =
        p8_fixed_add(p8_fixed_from_int(100), p8_fixed_mul(p8_fixed_from_int(2), p8_fixed_add(r_h, r_w)));

    ctx.total = p8_fixed_from_int(0);
    ctx.seq = happiness_by_tier_table;
    ctx.count = HAPPINESS_BY_TIER_COUNT;
    for_each_tagged("planet", sum_by_development_tier_visit, &ctx);

    return p8_fixed_add(h_val, ctx.total);
}

static const button_t *g_difficulty = NULL;

const button_t *economy_score_difficulty(void) {
    return g_difficulty;
}

void economy_score_set_difficulty(const button_t *dk) {
    g_difficulty = dk;
}

void economy_score_difficulty_quits(const entity_t *e) {
    static button_t copy;

    if (g_difficulty == NULL || &g_difficulty->header != e) {
        return;
    }
    memset(&copy, 0, sizeof copy);
    copy.name = g_difficulty->name;
    copy.has_trade = g_difficulty->has_trade;
    copy.trade = g_difficulty->trade;
    copy.has_type_bias = g_difficulty->has_type_bias;
    copy.type_bias = g_difficulty->type_bias;
    g_difficulty = &copy;
}

p8_fixed_t economy_score_net_income(entity_t *p) {
    p8_fixed_t kr = p8_fixed_flr(p8_fixed_sub(economy_score_trade_income(p), planet_upkeep(p)));
    planet_t *jo;

    assert(planet_is_family(p) && "economy_score_net_income: the planet tag holds planets only");
    jo = (planet_t *)p;
    jo->net_income = kr;
    jo->has_net_income = 1;
    return kr;
}
