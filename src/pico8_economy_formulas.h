#ifndef PICO8_ECONOMY_FORMULAS_H
#define PICO8_ECONOMY_FORMULAS_H

#include "pico8_fixed.h"
#include "pico8_map_tables.h"
#include "pico8_vec.h"

/* Throughout this module an absent or wrong-kind field reads as its
 * zero/false default rather than crashing, and a NULL instance is a safe
 * no-op. */

/* `r`'s 13 resources and at most 3 more. */
enum { PICO8_QE_MAP_CAP = PICO8_R_COUNT + 3 };

/* A table keyed by resource (`qe`, floored to an integer as Lua indexes
 * it), entries in insertion order. */
typedef struct {
    int count;
    struct {
        int qe;
        pico8_fixed_t v;
    } entries[PICO8_QE_MAP_CAP];
} pico8_qe_map_t;

/* Returns 1 and writes *out when `qe` is a key, 0 and leaves *out untouched
 * otherwise. */
int pico8_qe_map_get(const pico8_qe_map_t *map, pico8_fixed_t qe, pico8_fixed_t *out);
void pico8_qe_map_set(pico8_qe_map_t *map, pico8_fixed_t qe, pico8_fixed_t v);

/* Each empties `out` first. `cm`: every `qe` VALUE of `r` (not `r`'s keys)
 * mapped to `jm`; `bx`: `cm(0)` plus each producing planet's `kw.mu`;
 * `ck`: `cm(1)` less 0.15 per `qh` for the resource of its last `jy`. */
void pico8_cm(pico8_qe_map_t *out, pico8_fixed_t jm);
void pico8_bx(pico8_qe_map_t *out);
void pico8_ck(pico8_qe_map_t *out);

/* `#(to.p-pb.p)` is the SQUARED distance, not the Euclidean one. */
pico8_fixed_t pico8_dj(pico8_vec_t pb_p, pico8_vec_t to_p);

/* An absent `cg.jc` counts as zero slipways. */
pico8_fixed_t pico8_bk(void);

#endif
