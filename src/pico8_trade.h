#ifndef PICO8_TRADE_H
#define PICO8_TRADE_H

#include "pico8_ents.h"
#include "pico8_fixed.h"
#include "pico8_struct.h"
#include "pico8_zone_choisie.h"

/* `lp`/`rl` are the containers the keys held, borrowed, not copied. */
typedef struct {
    pico8_struct_header_t header;
    pico8_ents_t *lp;
    pico8_ents_t *rl;
    pico8_fixed_t qe;
    int has_qe;     /* 0: `qe` is nil, and `qe` is then never read */
    int qe_is_bool; /* 1: `qe` holds a boolean (0/1), 0: a fixed; read only when `has_qe` */
} pico8_trade_t;

/* `trade` is a root class with an empty spec and no `pq`, not
 * ke-descended, so it never self-registers into hv/cv/cg.
 *
 * This port has no operator overloading, so `trade.ns.__eq` is the plain
 * named pico8_trade_eq below. The generic membership helper (pico8_ents_has)
 * therefore compares two trades by pointer identity, not by pico8_trade_eq;
 * membership that must honour trade equality goes
 * through `gu_trade` (src/pico8_planet_trade.c). */
void pico8_trade_init(void);
void pico8_trade_shutdown(void);

/* 1 from pico8_trade_init() to pico8_trade_shutdown(), 0 otherwise. */
int pico8_trade_ready(void);
const pico8_struct_class_t *pico8_trade_struct_class(void);

/* A route's or a want's `qe`: `n` is read only for FIXED. */
typedef struct {
    enum { PICO8_QE_NIL, PICO8_QE_FALSE, PICO8_QE_FIXED } kind;
    pico8_fixed_t n;
} pico8_qe_t;

/* Lua's `==` on two `qe`: nil equals nil, false equals false, nil is not
 * false; numbers by value. */
int pico8_qe_equal(pico8_qe_t a, pico8_qe_t b);

/* The keys of `pico8_trade_new`: `lp`/`rl` NULL are nil, kept as given;
 * `has_qe`, `qe_is_bool`, `qe` as in `pico8_trade_t`, all 0 for a nil `qe`. */
typedef struct {
    pico8_ents_t *lp;
    pico8_ents_t *rl;
    pico8_fixed_t qe;
    int has_qe;
    int qe_is_bool;
} pico8_trade_keys_t;

/* The struct is made in `zone`; `keys` NULL means none: every member nil. */
pico8_trade_t *pico8_trade_new(const pico8_zone_choisie_t *zone, const pico8_trade_keys_t *keys);

/* A copy made wholly in `zone`: struct, `lp`, `rl` and their items are new,
 * so it shares nothing with `self` and outlives it. A nil `lp`/`rl` stays
 * nil; NULL for a NULL self. */
pico8_trade_t *pico8_trade_copy(const pico8_zone_choisie_t *zone, const pico8_trade_t *self);

/* `qe` as the cart compares it: nil for a nil `qe` or a NULL self, false
 * only as `kg` writes it (TROU-24); a true `qe` stops the run. */
pico8_qe_t pico8_trade_qe(const pico8_trade_t *self);

/* Returns 1 and writes *out (the last of "lp") for a non-empty "lp"; returns
 * 0 with *out untouched for a NULL self, or a nil or empty "lp". */
int pico8_trade_to(const pico8_trade_t *self, pico8_struct_header_t **out);

/* Returns a brand-new route made in `zone`, never mutating self; NULL for a
 * NULL self.
 * A nil "lp"/"rl" on self reads as an empty list, a nil "qe" as false
 * (TROU-24). A NULL `np`/`jc` is nil: it contributes nothing to "lp"/"rl". */
pico8_trade_t *pico8_trade_kg(const pico8_zone_choisie_t *zone, const pico8_trade_t *self,
                              pico8_struct_header_t *np, pico8_struct_header_t *jc);

/* Two trades are equal iff they start (`lp[1]`) and end (`:to()`) at the
 * same planet, compared by pointer identity, whatever lies between. Returns 0
 * for a NULL l/r or a nil or empty "lp" on either side. */
int pico8_trade_eq(const pico8_trade_t *l, const pico8_trade_t *r);

#endif
