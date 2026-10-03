#include "game/trade.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/frame_zone.h"

static int g_trade_up = 0;

resource_t trade_resource(const trade_route_t *self) {
    if (self == NULL || !self->has_resource) {
        return (resource_t){.kind = RESOURCE_NIL};
    }
    assert(!(self->resource_is_bool && self->resource != 0) && "no route of src/ holds true: resource_t has no kind for it");
    return self->resource_is_bool ? (resource_t){.kind = RESOURCE_FALSE}
                            : (resource_t){.kind = RESOURCE_FIXED, .number = self->resource};
}

int resource_equal(resource_t a, resource_t b) {
    return a.kind == b.kind && (a.kind != RESOURCE_FIXED || a.number == b.number);
}

static const entity_class_t trade_class_def = {0};

static trade_route_t *trade_create(const allocator_t *zone, const trade_keys_t *keys) {
    trade_route_t *self = (trade_route_t *)allocator_alloc(zone, sizeof *self);

#ifdef CLASS_COVERAGE_BUILD
    class_coverage_record("trade");
#endif
    entity_init_header(&self->header, &trade_class_def);
    self->planets = NULL;
    self->slipways = NULL;
    self->resource = 0;
    self->has_resource = 0;
    self->resource_is_bool = 0;
    if (keys != NULL) {
        assert((!keys->resource_is_bool || keys->resource == 0 || keys->resource == 1) &&
               "trade_create: a boolean key holds 0 or 1; trade_resource would read any other as a fixed");
        assert((keys->has_resource || (!keys->resource_is_bool && keys->resource == 0)) &&
               "trade_create: an absent key carries neither a kind nor a value; the member reads 0 when absent");
        self->planets = keys->planets;
        self->slipways = keys->slipways;
        self->resource = keys->resource;
        self->has_resource = keys->has_resource;
        self->resource_is_bool = keys->resource_is_bool;
    }
    return self;
}

static void assert_trade(const trade_route_t *self) {
    assert((self == NULL ||
            self->header.class == &trade_class_def) &&
           "trade: every outgoing or incoming route and every offer's route is a route; anything else would be read "
           "through the wrong layout");
}

void trade_init(void) {
    trade_shutdown();
    g_trade_up = 1;
}

void trade_shutdown(void) {
    g_trade_up = 0;
}

int trade_ready(void) {
    return g_trade_up;
}

const entity_class_t *trade_class(void) {
    return &trade_class_def;
}

trade_route_t *trade_new(const allocator_t *zone, const trade_keys_t *keys) {
    return trade_create(zone, keys);
}

static entity_list_t *entity_list_copy(const allocator_t *zone, const entity_list_t *src) {
    entity_list_t empty;
    entity_list_t *copy;

    if (src == NULL) {
        return NULL;
    }
    entity_list_init(&empty, zone);
    copy = (entity_list_t *)allocator_alloc(zone, sizeof *copy);
    *copy = entity_list_concat(zone, src, &empty);
    return copy;
}

trade_route_t *trade_copy(const allocator_t *zone, const trade_route_t *self) {
    trade_keys_t keys = {0};

    assert_trade(self);
    if (self == NULL) {
        return NULL;
    }
    keys.planets = entity_list_copy(zone, self->planets);
    keys.slipways = entity_list_copy(zone, self->slipways);
    keys.resource = self->resource;
    keys.has_resource = self->has_resource;
    keys.resource_is_bool = self->resource_is_bool;
    return trade_new(zone, &keys);
}

int trade_last_planet(const trade_route_t *self, entity_t **out) {
    const entity_list_t *lp;

    assert_trade(self);
    if (self == NULL || self->planets == NULL) {
        return 0;
    }
    lp = self->planets;
    if (lp->count == 0) {
        return 0;
    }
    *out = lp->items[lp->count - 1];
    return 1;
}

/* The one-element node and slipway lists live in the frame zone:
 * entity_list_concat copies their element into the lists it returns. A NULL
 * node or slipway leaves its list empty, so it contributes nothing. */
trade_route_t *trade_extend(const allocator_t *zone, const trade_route_t *self,
                              entity_t *node, entity_t *slipway) {
    trade_keys_t keys = {0};
    entity_list_t empty_planets;
    entity_list_t empty_slipways;
    const entity_list_t *lp;
    const entity_list_t *rl;
    entity_list_t node_list;
    entity_list_t slipway_list;
    entity_list_t *new_planets;
    entity_list_t *new_slipways;

    assert_trade(self);
    if (self == NULL) {
        return NULL;
    }

    entity_list_init(&empty_planets, frame_zone());
    entity_list_init(&empty_slipways, frame_zone());
    lp = self->planets != NULL ? self->planets : &empty_planets;
    rl = self->slipways != NULL ? self->slipways : &empty_slipways;
    /* TROU-24 */
    keys.has_resource = 1;
    keys.resource_is_bool = self->has_resource ? self->resource_is_bool : 1;
    keys.resource = self->has_resource ? self->resource : 0;

    entity_list_init(&node_list, frame_zone());
    if (node != NULL) {
        entity_list_add(&node_list, node);
    }
    entity_list_init(&slipway_list, frame_zone());
    if (slipway != NULL) {
        entity_list_add(&slipway_list, slipway);
    }

    new_planets = (entity_list_t *)allocator_alloc(zone, sizeof *new_planets);
    *new_planets = entity_list_concat(zone, lp, &node_list);
    new_slipways = (entity_list_t *)allocator_alloc(zone, sizeof *new_slipways);
    *new_slipways = entity_list_concat(zone, rl, &slipway_list);

    keys.planets = new_planets;
    keys.slipways = new_slipways;
    return trade_new(zone, &keys);
}

int trade_equal(const trade_route_t *l, const trade_route_t *r) {
    const entity_list_t *l_planets, *r_planets;
    const entity_t *l_first, *r_first;
    entity_t *l_last, *r_last;

    assert_trade(l);
    assert_trade(r);
    if (l == NULL || r == NULL) {
        return 0;
    }
    if (l->planets == NULL || r->planets == NULL) {
        return 0;
    }
    l_planets = l->planets;
    r_planets = r->planets;
    if (l_planets->count == 0 || r_planets->count == 0) {
        return 0;
    }
    l_first = l_planets->items[0];
    r_first = r_planets->items[0];

    if (l_first != r_first) {
        return 0;
    }
    if (!trade_last_planet(l, &l_last) || !trade_last_planet(r, &r_last)) {
        return 0;
    }
    return l_last == r_last;
}
