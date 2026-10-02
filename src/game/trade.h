#ifndef GAME_TRADE_H
#define GAME_TRADE_H

#include "engine/entity_list.h"
#include "p8/fixed.h"
#include "engine/entity_class.h"
#include "engine/allocator.h"

/* `planets`/`slipways` are the lists the keys held, borrowed, not copied. */
typedef struct {
    entity_t header;
    entity_list_t *planets;
    entity_list_t *slipways;
    p8_fixed_t resource;
    int has_resource;     /* 0: no resource, and `resource` is then never read */
    int resource_is_bool; /* 1: `resource` holds a boolean (0/1), 0: a fixed; read only when `has_resource` */
} trade_route_t;

/* A trade route is a root class with no archetype and no construct step,
 * so it never registers itself (entity_register): no registry lists it.
 *
 * Route equality is trade_equal below. The generic membership helper
 * (entity_list_has) compares two routes by pointer identity, not by
 * trade_equal; membership that must honour route equality goes through
 * `contains_trade` (src/game/planet_trade.c). */
void trade_init(void);
void trade_shutdown(void);

/* 1 from trade_init() to trade_shutdown(), 0 otherwise. */
int trade_ready(void);
const entity_class_t *trade_class(void);

/* A route's or a need's resource: `number` is read only for RESOURCE_FIXED. */
typedef struct {
    enum { RESOURCE_NIL, RESOURCE_FALSE, RESOURCE_FIXED } kind;
    p8_fixed_t number;
} resource_t;

/* Equality of two resources: absent equals absent, false equals false,
 * absent is not false; numbers by value. */
int resource_equal(resource_t a, resource_t b);

/* The keys of trade_new: `planets`/`slipways` kept as given, NULL
 * included; `has_resource`, `resource_is_bool`, `resource` as in
 * trade_route_t, all 0 for no resource. */
typedef struct {
    entity_list_t *planets;
    entity_list_t *slipways;
    p8_fixed_t resource;
    int has_resource;
    int resource_is_bool;
} trade_keys_t;

/* The struct is made in `zone`; `keys` NULL means none: every member NULL
 * or 0. */
trade_route_t *trade_new(const allocator_t *zone, const trade_keys_t *keys);

/* A copy made wholly in `zone`: the struct and both lists, item storage
 * included, are new, so it shares no storage with `self` and outlives it. A
 * NULL list stays NULL; NULL for a NULL self. */
trade_route_t *trade_copy(const allocator_t *zone, const trade_route_t *self);

/* The route's resource as resource_equal compares it: RESOURCE_NIL for no
 * resource or a NULL self, RESOURCE_FALSE only as trade_extend writes it
 * (TROU-24); a true boolean stops the run. */
resource_t trade_resource(const trade_route_t *self);

/* Returns 1 and writes *out (the last planet) for a non-empty planets list;
 * returns 0 with *out untouched for a NULL self, or a NULL or empty list. */
int trade_last_planet(const trade_route_t *self, entity_t **out);

/* Returns a brand-new route made in `zone`, never mutating self; NULL for a
 * NULL self.
 * A NULL planets or slipways list on self reads as an empty list, no
 * resource as false (TROU-24). A NULL node or slipway contributes nothing
 * to the lists. */
trade_route_t *trade_extend(const allocator_t *zone, const trade_route_t *self,
                              entity_t *np, entity_t *jc);

/* Two trades are equal iff they start (first planet) and end (last planet)
 * at the same planet, compared by pointer identity, whatever lies between.
 * Returns 0 for a NULL route or a NULL or empty planets list on either
 * side. */
int trade_equal(const trade_route_t *l, const trade_route_t *r);

#endif
