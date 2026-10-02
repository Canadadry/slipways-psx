#ifndef GAME_TRADE_H
#define GAME_TRADE_H

#include "engine/entity_list.h"
#include "p8/fixed.h"
#include "engine/entity_class.h"
#include "engine/allocator.h"

/* `lp`/`rl` are the containers the keys held, borrowed, not copied. */
typedef struct {
    entity_t header;
    entity_list_t *planets;
    entity_list_t *slipways;
    p8_fixed_t resource;
    int has_resource;     /* 0: `qe` is nil, and `qe` is then never read */
    int resource_is_bool; /* 1: `qe` holds a boolean (0/1), 0: a fixed; read only when `has_qe` */
} trade_route_t;

/* `trade` is a root class with an empty spec and no `pq`, not
 * ke-descended, so it never self-registers into hv/cv/cg.
 *
 * This port has no operator overloading, so `trade.ns.__eq` is the plain
 * named pico8_trade_eq below. The generic membership helper (pico8_ents_has)
 * therefore compares two trades by pointer identity, not by pico8_trade_eq;
 * membership that must honour trade equality goes
 * through `gu_trade` (src/game/planet_trade.c). */
void trade_init(void);
void trade_shutdown(void);

/* 1 from pico8_trade_init() to pico8_trade_shutdown(), 0 otherwise. */
int trade_ready(void);
const entity_class_t *trade_class(void);

/* A route's or a want's `qe`: `n` is read only for FIXED. */
typedef struct {
    enum { RESOURCE_NIL, RESOURCE_FALSE, RESOURCE_FIXED } kind;
    p8_fixed_t number;
} resource_t;

/* Lua's `==` on two `qe`: nil equals nil, false equals false, nil is not
 * false; numbers by value. */
int resource_equal(resource_t a, resource_t b);

/* The keys of `pico8_trade_new`: `lp`/`rl` NULL are nil, kept as given;
 * `has_qe`, `qe_is_bool`, `qe` as in `pico8_trade_t`, all 0 for a nil `qe`. */
typedef struct {
    entity_list_t *planets;
    entity_list_t *slipways;
    p8_fixed_t resource;
    int has_resource;
    int resource_is_bool;
} trade_keys_t;

/* The struct is made in `zone`; `keys` NULL means none: every member nil. */
trade_route_t *trade_new(const allocator_t *zone, const trade_keys_t *keys);

/* A copy made wholly in `zone`: struct, `lp`, `rl` and their items are new,
 * so it shares nothing with `self` and outlives it. A nil `lp`/`rl` stays
 * nil; NULL for a NULL self. */
trade_route_t *trade_copy(const allocator_t *zone, const trade_route_t *self);

/* `qe` as the cart compares it: nil for a nil `qe` or a NULL self, false
 * only as `kg` writes it (TROU-24); a true `qe` stops the run. */
resource_t trade_resource(const trade_route_t *self);

/* Returns 1 and writes *out (the last of "lp") for a non-empty "lp"; returns
 * 0 with *out untouched for a NULL self, or a nil or empty "lp". */
int trade_last_planet(const trade_route_t *self, entity_t **out);

/* Returns a brand-new route made in `zone`, never mutating self; NULL for a
 * NULL self.
 * A nil "lp"/"rl" on self reads as an empty list, a nil "qe" as false
 * (TROU-24). A NULL `np`/`jc` is nil: it contributes nothing to "lp"/"rl". */
trade_route_t *trade_extend(const allocator_t *zone, const trade_route_t *self,
                              entity_t *np, entity_t *jc);

/* Two trades are equal iff they start (`lp[1]`) and end (`:to()`) at the
 * same planet, compared by pointer identity, whatever lies between. Returns 0
 * for a NULL l/r or a nil or empty "lp" on either side. */
int trade_equal(const trade_route_t *l, const trade_route_t *r);

#endif
