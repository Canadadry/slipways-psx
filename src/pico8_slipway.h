#ifndef PICO8_SLIPWAY_H
#define PICO8_SLIPWAY_H

#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_ke.h"
#include "pico8_struct.h"
#include "pico8_value.h"

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

/* `hg[id]=true`: `id` must be a string, and at most PICO8_JC_HG_CAP are
 * ever added (asserted). */
void pico8_jc_hg_add(pico8_jc_hg_t *hg, pico8_value_t id);

/* `hg[id]`: 1 for a string `id` already added, 0 otherwise. */
int pico8_jc_hg_has(const pico8_jc_hg_t *hg, pico8_value_t id);

/* `nu` points at `pico8_jc_nu_tags`, `k_` at `pico8_jc_k__seq`; `ts` is this
 * slipway's own, made by `pico8_jc_pq`, which also empties `hg_own` and
 * points `hg` at it (NULL before). Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_ke_t ke;
    const pico8_str_seq_t *nu;
    pico8_fixed_t fa;
    int fp;
    pico8_fixed_t ik;
    pico8_fixed_t dw;
    const pico8_int_seq_t *k_;
    pico8_fixed_t nh;
    pico8_struct_header_t *qu; /* borrowed, never freed */
    pico8_list_t *ts;
    pico8_jc_hg_t *hg;
    pico8_jc_hg_t hg_own;
    pico8_struct_header_t *re; /* borrowed, never freed */
    pico8_fixed_t od;
    int has_od; /* 0: `od` is nil, and is then never read */
    pico8_fixed_t mt;
    int has_mt;                /* 0: `mt` is nil, and is then never read */
    pico8_struct_header_t *et; /* borrowed, never freed */
    pico8_struct_header_t *fo; /* borrowed, never freed */
    int p_;
    int has_p_; /* 0: `p_` is nil, and is then never read */
} pico8_jc_t;

void pico8_jc_init(void);

/* Never frees any `jc`. Idempotent, and safe before the first
 * `pico8_jc_init()`. */
void pico8_jc_shutdown(void);

/* 1 from pico8_jc_init() to pico8_jc_shutdown(), 0 otherwise. */
int pico8_jc_ready(void);
const pico8_struct_class_t *pico8_jc_struct_class(void);

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
void pico8_jc_hq(pico8_jc_t *self, pico8_value_t mp);

/* A NULL `re`, or a `d_()` that refuses, only raises `self.p_`; both
 * endpoints are registered on success. */
void pico8_jc_ey(pico8_jc_t *self, pico8_struct_header_t *re);

/* Returns the existing slipway to reuse, the first planet or slipway in
 * the way, the one static `{qs="-too far-"}` struct, or nothing. */
pico8_value_t pico8_jc_ce(pico8_jc_t *self);

const pico8_struct_class_t *pico8_jc_too_far_struct_class(void);

const pico8_struct_class_t *pico8_jc_mouse_point_struct_class(void);

pico8_value_t pico8_jc_eb(const pico8_jc_t *self, const pico8_struct_header_t *jo_instance);

void pico8_jc_ev(pico8_jc_t *self);

#endif
