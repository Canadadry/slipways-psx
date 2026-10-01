#ifndef PICO8_SLIPWAY_H
#define PICO8_SLIPWAY_H

#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_ke.h"
#include "pico8_probe_comp.h"
#include "pico8_ptrs.h"
#include "pico8_struct.h"

/* Throughout this module a NULL struct argument is a safe no-op (Lua nil
 * for the value-returning functions), and a nil or wrong-kind member, or
 * an absent or wrong-kind field of another entity, reads as absent rather
 * than crashing. */

enum { PICO8_JC_HG_CAP = 90 };

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

/* `ts` is this slipway's own, made by `pico8_jc_pq`, which also empties
 * `hg_own` and points `hg` at it (NULL before). `od` and `et` (NULL is nil)
 * live in the probe; `probe` points at `probe_own`: never copy the struct.
 * Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_struct_header_t header;
    const pico8_jc_def_t *def;
    pico8_fixed_t fa;
    pico8_fixed_t nh;
    pico8_struct_header_t *qu; /* borrowed, never freed */
    pico8_ptrs_t *ts;
    pico8_jc_hg_t *hg;
    pico8_jc_hg_t hg_own;
    pico8_struct_header_t *re; /* borrowed, never freed */
    pico8_fixed_t mt;
    int has_mt;                /* 0: `mt` is nil, and is then never read */
    pico8_struct_header_t *fo; /* borrowed, never freed */
    int p_;
    pico8_probe_t *probe;
    pico8_probe_t probe_own;
} pico8_jc_t;

/* The one static `{qs="-too far-"}` struct `jc:ce` returns: `fo` is the last
 * slipway that hung itself on it, borrowed. */
typedef struct {
    pico8_struct_header_t header;
    const char *qs;
    pico8_jc_t *fo;
} pico8_jc_too_far_t;

/* The `{ra,p}` `jc:hq` hangs on `re` over nothing: a fresh one per call,
 * never freed, nor its `p`; `ra` is the caller's `mp` itself, not a copy. */
typedef struct {
    pico8_struct_header_t header;
    const pico8_vec_t *ra;
    const pico8_vec_t *p;
} pico8_jc_mouse_point_t;

void pico8_jc_init(void);

/* Never frees any `jc`. Idempotent, and safe before the first
 * `pico8_jc_init()`. */
void pico8_jc_shutdown(void);

/* 1 from pico8_jc_init() to pico8_jc_shutdown(), 0 otherwise. */
int pico8_jc_ready(void);

/* `p`/`ra` of an end of a slipway (a planet, or the mouse point `jc:hq`
 * hangs on `re` over nothing), read through its link: NULL, an end without
 * the link, or an absent vector reads as the zero vector. */
pico8_vec_t pico8_jc_end_p(const pico8_struct_header_t *end);
pico8_vec_t pico8_jc_end_ra(const pico8_struct_header_t *end);

/* The cart's key `qu` of a slipway: NULL is nil, else a planet a slipway
 * starts from, which the type does not enforce (an assert does). */
typedef struct {
    pico8_struct_header_t *qu;
} pico8_jc_keys_t;

/* `keys` is read, not kept (NULL: none). The struct registers itself into
 * `hv`/`cv`/`cg` through `lt`, and nothing frees it. */
pico8_jc_t *pico8_jc_new(const pico8_jc_keys_t *keys);

/* TROU-7 */
/* Allocates the `ts` and `hg` containers every other method here assumes
 * are already present. */
void pico8_jc_pq(pico8_jc_t *self);

/* TROU-7 */
/* Rebinds `self.re` every frame: the hovered entity when it answers to
 * `gv`, otherwise a fresh `{ra,p}` struct, never freed. Also rebinds `od`,
 * `mt`, `et` and `fa`, and points the `fo` of whatever `ce()` found -- a
 * planet, another `jc` struct or the static `{qs}` struct -- back at this
 * slipway. */
void pico8_jc_hq(pico8_jc_t *self, pico8_vec_t *mp);

/* A NULL `re`, or a `d_()` that refuses, only raises `self.p_`; both
 * endpoints are registered on success. */
void pico8_jc_ey(pico8_jc_t *self, pico8_struct_header_t *re);

/* Returns the existing slipway to reuse, the first planet or slipway in
 * the way, the one static `{qs="-too far-"}` struct, or NULL. */
pico8_struct_header_t *pico8_jc_ce(pico8_jc_t *self);

const pico8_struct_class_t *pico8_jc_too_far_struct_class(void);

const pico8_struct_class_t *pico8_jc_mouse_point_struct_class(void);

/* The other end of the slipway seen from `jo_instance`; NULL for nil. */
pico8_struct_header_t *pico8_jc_eb(const pico8_jc_t *self, const pico8_struct_header_t *jo_instance);

void pico8_jc_ev(pico8_jc_t *self);

void pico8_jc_kj_dispatch(pico8_struct_header_t *self, const pico8_vec_t *ra);

#endif
