#include "game/planet_trade.h"

#include <assert.h>
#include <stddef.h>

#include "engine/collect.h"
#include "game/map_tables.h"
#include "game/planet.h"
#include "game/trade.h"
#include "game/slipway.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

/* Membership by trade equality (same endpoints), not pico8_ents_has's pointer
 * identity: every candidate offered here is a freshly built route, which
 * identity would never match. NULL-safe on `list`. */
static int contains_trade(const entity_list_t *list, const trade_route_t *v) {
    entity_list_iter_t it;
    entity_t *e;

    if (list == NULL) {
        return 0;
    }
    it = entity_list_all(list);
    while (entity_list_iter_next(&it, &e)) {
        if (trade_equal((const trade_route_t *)e, v)) {
            return 1;
        }
    }
    return 0;
}

static void planet_rebuild_trades_attach_slipway_closure(trade_offer_t *self, trade_route_t *a) {
    if (self == NULL || self->accepted_routes == NULL) {
        return;
    }
    if (!contains_trade(self->accepted_routes, a)) {
        assert(self->accepted_routes->zone == game_zone() &&
               "planet_rebuild_trades: a kept route is copied into the game zone; a list of another zone would outlive it");
        entity_list_add(self->accepted_routes, &trade_copy(game_zone(), a)->header);
        self->amount -= p8_fixed_from_int(1);
    }
}

void planet_attach_slipway(planet_t *self, entity_t *sw) {
    entity_list_t *oe = NULL;
    entity_t *other = NULL;

    if (self == NULL) {
        return;
    }

    oe = self->slipways;
    if (oe != NULL) {
        entity_list_add(oe, sw);
    }

    if (sw != NULL) {
        assert(sw->class == slipway_class() && "planet_attach_slipway: the slipway is one slipway_drop registers");
        other = slipway_other_end((const slipway_t *)sw, (const entity_t *)self);
    }
    if (other != NULL) {
        assert(planet_is_family(other) && "planet_attach_slipway: the other end of a registered slipway is a planet");
        if (((const planet_t *)other)->fold_anchor != 0) {
            self->fold_anchor = p8_fixed_from_int(1);
        }
    }

    planet_restart_shake(self);
}

void planet_offer_trade(planet_t *self, trade_offer_t *trade_offer) {
    int resource;
    resource_t r_e = {.kind = RESOURCE_NIL};
    resource_t r_q = {.kind = RESOURCE_NIL};
    int have_r_e;
    int have_r_q;
    trade_route_t *route;
    resource_t t_resource = {.kind = RESOURCE_NIL};
    pointer_list_t *needs = NULL;
    pointer_list_iter_t iter;
    void *entry;

    if (self == NULL || trade_offer == NULL) {
        return;
    }
    route = trade_offer->route;
    if (route != NULL) {
        t_resource = trade_resource(route);
    }

    have_r_e = resource_code_lookup("e", &resource);
    if (have_r_e) {
        r_e = (resource_t){.kind = RESOURCE_FIXED, .number = p8_fixed_from_int(resource)};
    }
    have_r_q = resource_code_lookup("?", &resource);
    if (have_r_q) {
        r_q = (resource_t){.kind = RESOURCE_FIXED, .number = p8_fixed_from_int(resource)};
    }

    needs = self->needs;

    if (have_r_e && resource_equal(t_resource, r_e) && !self->has_production_bonus) {
        iter = pointer_list_all(needs);
        while (pointer_list_iter_next(&iter, &entry)) {
            if (entry != NULL) {
                const need_t *i = entry;
                p8_fixed_t gc = i->has_satisfied ? i->satisfied : p8_fixed_from_int(0);
                int resource_is_r_q;

                resource_is_r_q = have_r_q && resource_equal(need_resource(i), r_q);
                if (gc == p8_fixed_from_int(0) && !resource_is_r_q) {
                    pointer_list_del(needs, entry);
                    break;
                }
            }
        }

        {
            p8_fixed_t e_resource = r_e.number;
            need_t *fresh = need_new(game_zone(), &e_resource);
            pointer_list_t prefix;
            pointer_list_t merged;
            pointer_list_t *merged_own;

            pointer_list_init(&prefix, frame_zone());
            pointer_list_add(&prefix, fresh);
            merged = collect_concat_pointer_list(game_zone(), &prefix, needs);

            merged_own = (pointer_list_t *)allocator_alloc(game_zone(), sizeof *merged_own);
            *merged_own = merged;
            self->needs = merged_own;
            needs = merged_own;
        }
        self->production_bonus = p8_fixed_from_int(1);
        self->has_production_bonus = 1;
    }

    iter = pointer_list_all(needs);
    while (pointer_list_iter_next(&iter, &entry)) {
        need_t *i;
        resource_t resource_val;

        if (entry == NULL) {
            continue;
        }
        i = entry;
        resource_val = need_resource(i);

        if (have_r_q && resource_equal(resource_val, r_q)) {
            int accepted = 0;

            if ((self->header.class->lineage & LINEAGE(LINEAGE_STRUCTURE)) && self->archetype->accepted_resources != NULL) {
                accepted = t_resource.kind == RESOURCE_FIXED && int_seq_has_fixed(self->archetype->accepted_resources, t_resource.number);
            }
            if (accepted) {
                assert(t_resource.kind == RESOURCE_FIXED && "a need's resource is a resource code, never a boolean");
                i->row.has_resource = 1;
                i->row.resource = t_resource.number;
                resource_val = t_resource;
            }
        }

        if (resource_equal(t_resource, resource_val) && !(have_r_q && resource_equal(resource_val, r_q))) {
            trade_route_t *extended = trade_extend(frame_zone(), route, &self->header, NULL);
            if (trade_offer->accept != NULL) {
                trade_offer->accept(trade_offer, extended);
            }
            return;
        }
    }
}

entity_list_t *planet_rebuild_trades(planet_t *self) {
    entity_list_t *accepted_routes;
    trade_keys_t keys = {0};
    p8_fixed_t amount = p8_fixed_from_int(0);
    const entity_list_t *slipways = NULL;
    entity_list_iter_t iter;
    entity_t *w_val;

    if (self == NULL) {
        return NULL;
    }

    if (self->outgoing_routes != NULL) {
        accepted_routes = self->outgoing_routes;
    } else {
        accepted_routes = (entity_list_t *)allocator_alloc(game_zone(), sizeof *accepted_routes);
        entity_list_init(accepted_routes, game_zone());
    }

    if (self->production != NULL) {
        if (self->production->has_resource) {
            keys.has_resource = 1;
            keys.resource = self->production->resource;
        }
        amount = self->production->amount;
    }

    slipways = self->slipways;

    iter = entity_list_all(slipways);
    while (entity_list_iter_next(&iter, &w_val)) {
        entity_t *w;
        trade_offer_t lw;
        entity_list_t *planets_list;
        entity_list_t *slipways_list;
        entity_t *other;

        if (w_val == NULL) {
            continue;
        }
        w = w_val;

        planets_list = (entity_list_t *)allocator_alloc(frame_zone(), sizeof *planets_list);
        entity_list_init(planets_list, frame_zone());
        entity_list_add(planets_list, &self->header);
        slipways_list = (entity_list_t *)allocator_alloc(frame_zone(), sizeof *slipways_list);
        entity_list_init(slipways_list, frame_zone());
        entity_list_add(slipways_list, w);

        keys.planets = planets_list;
        keys.slipways = slipways_list;
        lw.route = trade_new(frame_zone(), &keys);
        /* Recomputed each iteration: an earlier proposal's `kf` may already
         * have grown `ib`. */
        lw.amount = amount - p8_fixed_from_int(accepted_routes->count);
        lw.accept = planet_rebuild_trades_attach_slipway_closure;
        lw.accepted_routes = accepted_routes;

        if (lw.amount == p8_fixed_from_int(0)) {
            break;
        }

        /* `lw` is on the stack: a callee that keeps the REF past this
         * iteration holds a dangling pointer. `nr:cn` copies it instead. */
        assert(w->class == slipway_class() && "a planet's slipways list holds slipways only: planet_attach_slipway adds to it");
        other = slipway_other_end((const slipway_t *)w, (const entity_t *)self);
        if (other != NULL && other->class->offer_trade != NULL) {
            other->class->offer_trade(other, &lw);
        }
    }

    return accepted_routes;
}

void planet_attach_slipway_method(entity_t *self, entity_t *sw) {
    planet_attach_slipway((planet_t *)self, sw);
}

void planet_offer_trade_method(entity_t *self, struct trade_offer *lw) {
    planet_offer_trade((planet_t *)self, lw);
}
