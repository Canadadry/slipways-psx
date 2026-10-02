#ifndef PICO8_SLIPWAY_H
#define PICO8_SLIPWAY_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "engine/entity.h"
#include "game/targeting.h"
#include "engine/pointer_list.h"
#include "engine/entity_class.h"

/* Throughout this module a NULL struct argument is a safe no-op (Lua nil
 * for the value-returning functions), and a nil or wrong-kind member, or
 * an absent or wrong-kind field of another entity, reads as absent rather
 * than crashing. */

enum { PICO8_JC_HG_CAP = 90 };

/* `ts` elements reserved in the grab arena: past them `ts` grows in it, and
 * a full arena is a pico8_fatal. */
enum { PICO8_JC_TS_RESERVE = 64 };

/* The `id` of each planet a slipway serves, each once; the strings are
 * borrowed from the planets. */
typedef struct {
    int count;
    const char *ids[PICO8_JC_HG_CAP];
} pico8_jc_hg_t;

/* `hg[id]=true`: `id` must not be NULL, and at most PICO8_JC_HG_CAP are
 * ever added (asserted). */
void pico8_jc_hg_add(pico8_jc_hg_t *hg, const char *id);

/* `hg[id]`: 1 for an `id` already added, 0 otherwise, NULL included. */
int pico8_jc_hg_has(const pico8_jc_hg_t *hg, const char *id);

/* The `{ra,p}` `jc:hq` hangs on `re` over nothing: the slipway's own,
 * rewritten in place at each call; `ra` points at the slipway's `point_ra`,
 * a copy of the caller's `mp` (NULL for a nil `mp`), `p` at its `point_p`. */
typedef struct {
    entity_t header;
    const vec_t *ra;
    const vec_t *p;
} pico8_jc_mouse_point_t;

/* `ts` is this slipway's own, made by `pico8_jc_pq`, which also empties
 * `hg_own` and points `hg` at it (NULL before). `od` and `et` (NULL is nil)
 * live in the probe; `probe` points at `probe_own`, `point.ra` and `point.p`
 * at `point_ra` and `point_p`: never copy the struct. Built with `ts` in a
 * grab arena (`pico8_jo_prise_ouvrir`), which a refused slipway's `rendre`
 * puts back on the free grabs. */
typedef struct {
    entity_t header;
    const pico8_jc_def_t *def;
    p8_fixed_t fa;
    p8_fixed_t nh;
    entity_t *qu; /* borrowed, never freed */
    pointer_list_t *ts;
    pico8_jc_hg_t *hg;
    pico8_jc_hg_t hg_own;
    entity_t *re; /* borrowed, never freed */
    p8_fixed_t mt;
    int has_mt;                /* 0: `mt` is nil, and is then never read */
    entity_t *fo; /* borrowed, never freed */
    int p_;
    pico8_probe_t *probe;
    pico8_probe_t probe_own;
    pico8_jc_mouse_point_t point;
    vec_t point_ra;
    vec_t point_p;
} pico8_jc_t;

/* The one static `{qs="-too far-"}` struct `jc:ce` returns: `fo` is the last
 * slipway that hung itself on it, borrowed. */
typedef struct {
    entity_t header;
    const char *qs;
    pico8_jc_t *fo;
} pico8_jc_too_far_t;

void pico8_jc_init(void);

/* Never frees any `jc`. Idempotent, and safe before the first
 * `pico8_jc_init()`. */
void pico8_jc_shutdown(void);

/* 1 from pico8_jc_init() to pico8_jc_shutdown(), 0 otherwise. */
int pico8_jc_ready(void);

/* `p`/`ra` of an end of a slipway (a planet, or the mouse point `jc:hq`
 * hangs on `re` over nothing), read through its link: NULL, an end without
 * the link, or an absent vector reads as the zero vector. */
vec_t pico8_jc_end_p(const entity_t *end);
vec_t pico8_jc_end_ra(const entity_t *end);

/* The cart's key `qu` of a slipway: NULL is nil, else a planet a slipway
 * starts from, which the type does not enforce (an assert does). */
typedef struct {
    entity_t *qu;
} pico8_jc_keys_t;

/* `keys` is read, not kept (NULL: none). The struct registers itself into
 * `hv`/`cv`/`cg` through `lt`; it lives in its grab arena, kept until the
 * generation is given back once placed, recycled after `rendre` if refused. */
pico8_jc_t *pico8_jc_new(const pico8_jc_keys_t *keys);

/* Allocates the `ts` and `hg` containers every other method here assumes
 * are already present, `ts` in the construction zone with room for
 * PICO8_JC_TS_RESERVE. */
void pico8_jc_pq(pico8_jc_t *self);

/* Rebinds `self.re` every frame: the hovered entity when it answers to
 * `gv`, otherwise `self.point`, rewritten in place. Also rebinds `od`,
 * `mt`, `et` and `fa`, and points the `fo` of whatever `ce()` found -- a
 * planet, another `jc` struct or the static `{qs}` struct -- back at this
 * slipway. */
void pico8_jc_hq(pico8_jc_t *self, vec_t *mp);

/* A NULL `re`, or a `d_()` that refuses, only raises `self.p_`; both
 * endpoints are registered on success. */
void pico8_jc_ey(pico8_jc_t *self, entity_t *re);

/* Returns the existing slipway to reuse, the first planet or slipway in
 * the way, the one static `{qs="-too far-"}` struct, or NULL. */
entity_t *pico8_jc_ce(pico8_jc_t *self);

const entity_class_t *pico8_jc_too_far_struct_class(void);

const entity_class_t *pico8_jc_mouse_point_struct_class(void);

/* The other end of the slipway seen from `jo_instance`; NULL for nil. */
entity_t *pico8_jc_eb(const pico8_jc_t *self, const entity_t *jo_instance);

void pico8_jc_ev(pico8_jc_t *self);

void pico8_jc_kj_dispatch(entity_t *self, const vec_t *ra);

#endif
