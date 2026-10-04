#ifndef GAME_TRADE_H
#define GAME_TRADE_H

#include "engine/entity_list.h"
#include "p8/fixed.h"
#include "engine/entity_class.h"
#include "engine/allocator.h"

typedef struct {
    entity_t header;
    entity_list_t *planets;
    entity_list_t *slipways;
    p8_fixed_t resource;
    int has_resource;
    int resource_is_bool;
} trade_route_t;

void trade_init(void);
void trade_shutdown(void);

int trade_ready(void);
const entity_class_t *trade_class(void);

typedef struct {
    enum { RESOURCE_NIL, RESOURCE_FALSE, RESOURCE_FIXED } kind;
    p8_fixed_t number;
} resource_t;

int resource_equal(resource_t a, resource_t b);

typedef struct {
    entity_list_t *planets;
    entity_list_t *slipways;
    p8_fixed_t resource;
    int has_resource;
    int resource_is_bool;
} trade_keys_t;

trade_route_t *trade_new(const allocator_t *zone, const trade_keys_t *keys);

trade_route_t *trade_copy(const allocator_t *zone, const trade_route_t *self);

/* GAP-24 */
resource_t trade_resource(const trade_route_t *self);

int trade_last_planet(const trade_route_t *self, entity_t **out);

/* GAP-24 */
trade_route_t *trade_extend(const allocator_t *zone, const trade_route_t *self,
                              entity_t *np, entity_t *jc);

int trade_equal(const trade_route_t *l, const trade_route_t *r);

#endif
