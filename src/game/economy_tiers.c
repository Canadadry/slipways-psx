#include "game/economy_tiers.h"

#include <assert.h>
#include <string.h>

#include "engine/entity_registry.h"
#include "game/game_data.h"
#include "game/planet.h"
#include "game/planet_trade.h"
#include "game/research.h"
#include "game/slipway.h"
#include "game/trade.h"
#include "engine/game_zone.h"

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

static void rebuild_incoming_routes_reset_visit(entity_t *pt, void *ctx) {
    planet_t *jo;
    entity_list_t *cc;
    (void)ctx;
    if (pt == NULL) {
        return;
    }
    assert(planet_is_family(pt) && "rebuild_incoming_routes_reset_visit: the planet tag holds planets only");
    jo = (planet_t *)pt;
    if (jo->incoming_routes != NULL) {
        entity_list_clear(jo->incoming_routes);
        return;
    }
    cc = (entity_list_t *)allocator_alloc(game_zone(), sizeof *cc);
    entity_list_init(cc, game_zone());
    jo->incoming_routes = cc;
}

static void rebuild_incoming_routes_route_visit(entity_t *entity, void *ctx) {
    planet_t *from_planet;
    entity_list_t *outgoing;
    entity_list_iter_t iter;
    entity_t *element;
    (void)ctx;
    if (entity == NULL) {
        return;
    }

    assert(planet_is_family(entity) && "rebuild_incoming_routes_route_visit: the planet tag holds planets only");
    from_planet = (planet_t *)entity;
    outgoing = planet_rebuild_trades(from_planet);
    from_planet->outgoing_routes = outgoing;

    if (outgoing == NULL) {
        return;
    }

    iter = entity_list_all(outgoing);
    while (entity_list_iter_next(&iter, &element)) {
        const trade_route_t *route;
        const entity_list_t *slipways;
        const entity_list_t *planets;
        entity_t *to_val;
        int index;

        if (element == NULL) {
            continue;
        }
        assert(element->class == trade_class() && "rebuild_incoming_routes_route_visit: outgoing_routes holds trades only");
        route = (const trade_route_t *)element;
        slipways = route->slipways;
        planets = route->planets;

        if (slipways != NULL && planets != NULL) {
            for (index = 0; index < slipways->count; index++) {
                const entity_t *w_val = slipways->items[index];
                const slipway_t *w;
                const entity_t *planet_val;
                const planet_t *planet;

                if (w_val == NULL) {
                    continue;
                }
                if (index + 1 >= planets->count) {
                    continue;
                }
                planet_val = planets->items[index + 1];
                if (planet_val == NULL) {
                    continue;
                }
                assert(planet_is_family(planet_val) && "rebuild_incoming_routes_route_visit: a trade's planets list holds planets only");
                planet = (const planet_t *)planet_val;
                if (planet->planet_id == NULL) {
                    continue;
                }
                assert(w_val->class == slipway_class() && "rebuild_incoming_routes_route_visit: a trade's slipways list holds slipways only");
                w = (const slipway_t *)w_val;
                /* A slipway with no `hg` set is skipped; Lua would error
                 * indexing nil here. */
                if (w->served_planets == NULL) {
                    continue;
                }
                slipway_served_planets_add(w->served_planets, planet->planet_id);
            }
        }

        if (trade_last_planet(route, &to_val) && to_val != NULL) {
            planet_t *dest;

            assert(planet_is_family(to_val) && "rebuild_incoming_routes_route_visit: a trade leads to a planet");
            dest = (planet_t *)to_val;
            if (dest->incoming_routes != NULL) {
                entity_list_add(dest->incoming_routes, element);
            }
        }
    }
}

void economy_tiers_rebuild_incoming_routes(void) {
    for_each_tagged("planet", rebuild_incoming_routes_reset_visit, NULL);
    for_each_tagged("planet", rebuild_incoming_routes_route_visit, NULL);
}

static void update_tiers_visit(entity_t *entity, void *ctx) {
    int *changed = (int *)ctx;
    planet_t *planet;
    p8_fixed_t tier;
    int has_development_tier;
    p8_fixed_t development_tier = p8_fixed_from_int(0);

    if (entity == NULL) {
        return;
    }
    tier = economy_tiers_compute_tier(entity);
    assert(planet_is_family(entity) && "update_tiers_visit: the planet tag holds planets only");
    planet = (planet_t *)entity;
    has_development_tier = planet->has_development_tier;
    if (has_development_tier) {
        development_tier = planet->development_tier;
    }

    if (!has_development_tier || tier > development_tier) {
        if (has_development_tier && development_tier == p8_fixed_from_int(1)) {
            research_project_t *hn = planet->project;
            if (hn != NULL && hn->has_byproduct) {
                if (planet->needs != NULL) {
                    need_t *entry = need_new(game_zone(), &hn->byproduct);
                    pointer_list_add(planet->needs, entry);
                }
            }
        }
        planet->development_tier = tier;
        planet->has_development_tier = 1;
        development_tier = tier;
        *changed = 1;
    }

    if (development_tier != p8_fixed_from_int(0)) {
        research_project_t *hn = planet->project;
        if (hn != NULL) {
            const production_tables_entry_t *po = research_project_production(hn);
            if (po != NULL) {
                int idx = p8_fixed_flr_int(development_tier);
                int clamped_idx = (idx < po->count) ? idx : po->count;
                p8_fixed_t base = p8_fixed_from_int(0);
                p8_fixed_t jh = p8_fixed_from_int(0);

                if (clamped_idx >= 1) {
                    base = p8_fixed_from_int(po->values[clamped_idx - 1]);
                }
                if (planet->has_production_bonus) {
                    jh = planet->production_bonus;
                }
                if (planet->production != NULL) {
                    planet->production->amount = p8_fixed_add(base, jh);
                }
            }
        }
    }
}

int economy_tiers_update_tiers(void) {
    int jg = 0;
    for_each_tagged("planet", update_tiers_visit, &jg);
    return jg;
}

p8_fixed_t economy_tiers_compute_tier(const entity_t *entity) {
    const planet_t *planet;
    const pointer_list_t *needs;
    const entity_list_t *incoming_routes;
    p8_fixed_t tier = p8_fixed_from_int(1);
    int index;

    if (entity == NULL) {
        return p8_fixed_from_int(0);
    }
    assert(planet_is_family(entity) && "economy_tiers_compute_tier: the entity is a planet");
    planet = (const planet_t *)entity;
    if (planet->header.state == NULL || strcmp(planet->header.state, "settled") != 0) {
        return p8_fixed_from_int(0);
    }

    needs = planet->needs;
    incoming_routes = planet->incoming_routes;
    if (needs != NULL) {
        for (index = 0; index < needs->count; index++) {
            void *i_val = needs->items[index];
            need_t *want;
            resource_t resource_val;
            int gc = 0;
            int j;

            if (i_val == NULL) {
                continue;
            }
            want = i_val;
            resource_val = need_resource(want);
            if (incoming_routes != NULL) {
                for (j = 0; j < incoming_routes->count; j++) {
                    const entity_t *incoming = incoming_routes->items[j];
                    const trade_route_t *im;
                    if (incoming == NULL) {
                        continue;
                    }
                    assert(incoming->class == trade_class() && "economy_tiers_compute_tier: incoming_routes holds trades only");
                    im = (const trade_route_t *)incoming;
                    /* TROU-24 */
                    if (im->has_resource && resource_equal(trade_resource(im), resource_val)) {
                        gc++;
                    }
                }
            }
            want->has_satisfied = 1;
            want->satisfied = p8_fixed_from_int(gc);
            tier = p8_fixed_mul(tier, p8_fixed_from_int(gc));
        }
    }

    if (tier == p8_fixed_from_int(0)) {
        return p8_fixed_from_int(1);
    }

    if (entity->class->lineage & LINEAGE(LINEAGE_TRADING_HUB)) {
        int count = 0;
        if (needs != NULL) {
            for (index = 0; index < needs->count; index++) {
                void *i_val = needs->items[index];
                const need_t *want;
                if (i_val == NULL) {
                    continue;
                }
                want = i_val;
                if (want->has_satisfied && want->satisfied > p8_fixed_from_int(0)) {
                    count++;
                }
            }
        }
        return fixed_min(p8_fixed_from_int(count), p8_fixed_from_int(3));
    }

    if (!planet->archetype->is_planet) {
        if (needs != NULL && needs->count > 0) {
            void *last_val = needs->items[needs->count - 1];
            if (last_val != NULL) {
                const need_t *last = last_val;
                if (last->has_satisfied) {
                    return fixed_min(p8_fixed_add(last->satisfied, p8_fixed_from_int(1)), p8_fixed_from_int(4));
                }
            }
        }
        return p8_fixed_from_int(2);
    }

    {
        const entity_list_t *by = planet->outgoing_routes;
        int by_count = (by != NULL) ? by->count : 0;
        int incoming_routes_count = (incoming_routes != NULL) ? incoming_routes->count : 0;
        p8_fixed_t mid_val =
            p8_fixed_mid(p8_fixed_from_int(by_count), p8_fixed_from_int(incoming_routes_count), p8_fixed_from_int(1));
        return fixed_min(p8_fixed_from_int(4), p8_fixed_add(p8_fixed_from_int(1), mid_val));
    }
}
