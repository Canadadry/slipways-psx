#ifndef PICO8_PROBE_COMP_H
#define PICO8_PROBE_COMP_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* What `bv`, `dl` and `d_` read and write of their host, the probe's own
 * fields, stored in the host (`probe_own`) and never copied; the rest is
 * read on `def`, the host's archetype's probe (`pico8_probe_def_none` on
 * research_icon's `pb`). `et` is borrowed; NULL is nil or false, which no
 * reader tells apart. */
typedef struct pico8_probe {
    const pico8_probe_def_t *def;
    entity_t *et;
    p8_fixed_t od;
    int has_od; /* 0: `od` is nil, and is then never read */
    vec_t bv_ra;
    vec_t bv_p;
} pico8_probe_t;

/* The host's `ra` and `p` point at its probe's `bv_ra` (NULL for a nil `mp`)
 * and `bv_p`, rewritten in place: a copy of `mp`, which stays the caller's.
 * `et` ends in one of three states -- a real target, a
 * static struct with no readable field but `fo`, or literal `false` -- of
 * which only `false` is falsy; collapsing the throwaway to nil would change
 * `d_`. */
void pico8_bv(entity_t *self, vec_t *mp, vec_t *rp);

void pico8_bv_dispatch(entity_t *self, vec_t *qa, vec_t *ra);

/* Points the `fo` of `et` back at `self`, which it borrows: `et` is a planet
 * of jo's family, a `jc`, the `jc_too_far` struct or `bv`'s throwaway target
 * (asserted, as is the class of `self` each accepts). */
void pico8_probe_et_hang_fo(entity_t *et, entity_t *self);

/* The `{}` `bv` puts in `et` over the void: one static struct, never
 * allocated nor freed; only `fo` is written, by pico8_probe_et_hang_fo(),
 * and the next `bv` overwrites it. */
typedef struct {
    entity_t header;
    entity_t *fo;
} pico8_bv_et_t;

const entity_class_t *pico8_bv_et_struct_class(void);

/* The tooltip of a probe host: `self` must carry a probe (asserted), and is
 * read, never retained. */
void pico8_dl(const p8_graphics_t *gfx, p8_memory_t *mem, entity_t *self);

/* The same, for a caller that is not a host (the research icon): `me` NULL
 * is nil, `pb` is read during the call only. */
void pico8_dl_show(const p8_graphics_t *gfx, p8_memory_t *mem, const char *me, const pico8_probe_t *pb);

void pico8_dl_dispatch(entity_t *self, const vec_t *ra);

/* Returns 1 on the success branch (Lua true), 0 on the other (nil); `jc:ey`
 * and `mz:ey` consume it. */
int pico8_d_(entity_t *self);

void pico8_d__dispatch(entity_t *self, entity_t *kv);

#endif
