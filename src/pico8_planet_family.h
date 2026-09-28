#ifndef PICO8_PLANET_FAMILY_H
#define PICO8_PLANET_FAMILY_H

#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_planet.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module -- this file and its `_trade` sibling -- a NULL
 * instance, gfx or mem argument is a safe no-op, and an absent or
 * wrong-kind field reads as its zero/false default rather than crashing. */

/* TROU-8 */
/* Calls `pico8_pi_init()` first: the specs contain `v(1,0)` and
 * `b(0,0,-1,-1)`, so the 'v'/'b' constructors must already be registered
 * when the spec strings are parsed. */
void pico8_planet_family_init(void);

/* Frees the six specs, never any instance. Idempotent, and safe before
 * the first `pico8_planet_family_init()`. */
void pico8_planet_family_shutdown(void);

/* A laboratory: a bare planet, then qh's own defaults and `et`, which its
 * own `hq` (`bv`) writes. A default of qh whose name is a member of `jo` or
 * `ke` (`me`, `nu`, `hf`, `mi`) lives in that member, never in a second
 * copy here. `nu`, `px` and `ba` hold the very tables of qh's spec,
 * borrowed. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_jo_t jo;
    pico8_fixed_t hp;
    pico8_ob_table_t *px;
    const char *dq;
    pico8_fixed_t qh;
    int qh_is_bool; /* 1: `qh` holds a boolean (0/1), 0: a fixed */
    pico8_fixed_t od;
    pico8_fixed_t fp;
    pico8_fixed_t dw;
    pico8_fixed_t s;
    pico8_ob_table_t *ba;
    pico8_struct_header_t *et; /* borrowed, never freed */
    int has_et;                /* 0: nil; 1: present, and a NULL `et` is `false` */
} pico8_qh_t;

/* `qh` as a value: a boolean when `qh_is_bool`, a fixed otherwise; REF NULL
 * when `self` is NULL. */
pico8_value_t pico8_qh_qh(const pico8_qh_t *self);

/* `et` as a value: REF NULL when `self` is NULL or `et` is absent, `false`
 * when present and NULL, a REF to the target otherwise. */
pico8_value_t pico8_qh_et(const pico8_qh_t *self);

/* A trading hub: a laboratory, then nl's own defaults `rr`, `nl` and `qw`
 * (`qw` also written by its own `ly`). A default of nl whose name is a member of `qh`
 * or `jo` (`nu`, `qh`, `hp`, `s`, `px`, `od`, `ba`) lives in that member.
 * `nu`, `px` and `ba` hold the very tables of nl's spec, borrowed. Nothing
 * frees the struct: TROU-7. */
typedef struct {
    pico8_qh_t qh;
    pico8_value_t rr;
    pico8_value_t nl;
    pico8_value_t qw;
} pico8_nl_t;

/* A processor: a laboratory and nothing of its own. Each default of os
 * lives in the member of that name, qh's (`hp`, `px`, `qh`, `s`, `od`) or
 * jo's (`nu`, `hs`). `nu` and `px` hold the very tables of os's spec, `ba`
 * the one of qh's, borrowed. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_qh_t qh;
} pico8_os_t;

/* An ascension: a laboratory, then hj's own emitter defaults and `qy`,
 * which its own `pq` (`f_`) writes. A default of hj whose name is a member
 * of `qh` or `jo` (`nu`, `hp`, `qh`, `px`, `od`, `s`) lives in that member.
 * `nu`, `px` and `k_` hold the very tables of hj's spec, `ba` the one of
 * qh's, borrowed; `qy` is the ascension's own. Nothing frees the struct:
 * TROU-7. */
typedef struct {
    pico8_qh_t qh;
    pico8_value_t rr;
    pico8_value_t py;
    pico8_value_t ki;
    pico8_value_t pn;
    pico8_value_t pk;
    pico8_value_t fc;
    pico8_value_t k_;
    pico8_value_t du;
    pico8_value_t qo;
    pico8_value_t ro;
    pico8_value_t qg;
    pico8_value_t q_;
    pico8_value_t qy;
} pico8_hj_t;

/* A protostar: an ascension, then pm's own defaults `ng` and `fz`, which no
 * ancestor has a member for. A default of pm whose name is a member of
 * `hj`, `qh`, `jo` or `ke` (`me`, `nu`, `hp`, `px`, `s` and the 11 emitter
 * defaults) lives in that member. `nu`, `px`, `k_` and `ng` hold the very
 * tables of pm's spec, `ba` the one of qh's, borrowed; `qy` is the
 * protostar's own. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_hj_t hj;
    pico8_value_t ng;
    pico8_value_t fz;
} pico8_pm_t;

/* A particle burst: a bare planet, then ga's own defaults and `qy`, which
 * its own `pq` (`f_`) writes. A default of ga whose name is a member of
 * `jo` (`nu`, `hf`, `by`, `kq`, `fa`) lives in that member. `nu`, `kq`,
 * `by` and `k_` hold the very tables of ga's spec, borrowed: `by` is the
 * one list every burst shares, which `ib` extends in place. A burst spawned
 * by `pm:pq` holds that call's `k_` instead. Nothing frees the struct:
 * TROU-7. */
typedef struct {
    pico8_jo_t jo;
    pico8_value_t fz;
    pico8_value_t fc;
    pico8_value_t qo;
    pico8_value_t ro;
    pico8_value_t qg;
    pico8_value_t q_;
    pico8_value_t k_;
    pico8_value_t iu;
    pico8_value_t py;
    pico8_value_t ms;
    pico8_value_t qy;
} pico8_ga_t;

/* Every `_ready` below is 1 from `pico8_planet_family_init()` to
 * `pico8_planet_family_shutdown()`, 0 otherwise. Every `_default` reads that
 * kind's spec, then its parent's reader (qh, ga: jo; nl, os, hj: qh; pm:
 * hj); 0 when absent, or when not ready. A REF is borrowed from the spec,
 * valid until the shutdown. A `_kind` accessor is never NULL, and its reader
 * is that `_default`. */
int pico8_qh_ready(void);
int pico8_qh_default(const char *name, pico8_value_t *out_value);
const pico8_entity_kind_t *pico8_qh_kind(void);
const pico8_struct_class_t *pico8_qh_struct_class(void);

/* Initializes an allocated pico8_qh_t under `cls`: jo's part, qh's members
 * nil, qh's defaults. Reads no key, runs neither `pq` nor `lt`, records no
 * coverage. Requires `pico8_planet_family_init()`. */
void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls);

/* `overrides` (NULL means none) is read, not kept: `id` only. The new
 * laboratory registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
 * ever frees it. Requires `pico8_planet_family_init()`. */
pico8_qh_t *pico8_qh_new(const pico8_ob_table_t *overrides);

int pico8_nl_ready(void);
int pico8_nl_default(const char *name, pico8_value_t *out_value);
const pico8_entity_kind_t *pico8_nl_kind(void);
const pico8_struct_class_t *pico8_nl_struct_class(void);

/* `overrides` (NULL means none) is read, not kept: `id` only. The new
 * trading hub registers itself into `hv`/`cv`/`cg` through `lt`, and
 * nothing ever frees it. Requires `pico8_planet_family_init()`. */
pico8_nl_t *pico8_nl_new(const pico8_ob_table_t *overrides);

int pico8_os_ready(void);
int pico8_os_default(const char *name, pico8_value_t *out_value);
const pico8_entity_kind_t *pico8_os_kind(void);
const pico8_struct_class_t *pico8_os_struct_class(void);

/* `overrides` (NULL means none) is read, not kept: `id` only. The new
 * processor registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
 * ever frees it. Requires `pico8_planet_family_init()`. */
pico8_os_t *pico8_os_new(const pico8_ob_table_t *overrides);

int pico8_hj_ready(void);
int pico8_hj_default(const char *name, pico8_value_t *out_value);
const pico8_entity_kind_t *pico8_hj_kind(void);
const pico8_struct_class_t *pico8_hj_struct_class(void);

/* `overrides` (NULL means none) is read, not kept: `id` only. The new
 * ascension registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
 * ever frees it. Requires `pico8_planet_family_init()`. */
pico8_hj_t *pico8_hj_new(const pico8_ob_table_t *overrides);

/* Initializes an allocated pico8_hj_t under `cls`: qh's part, hj's members
 * nil, hj's defaults. Reads no key, runs neither `pq` nor `lt`, records no
 * coverage. Requires `pico8_planet_family_init()`. */
void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls);

/* The dispatchers hj's descriptor holds, for a daughter's `static const`
 * descriptor to name. */
pico8_value_t pico8_qh_ly_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);
pico8_value_t pico8_hj_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

/* `pm` and `ga` are the only classes here whose spec sets `fz`: the two
 * kinds `jo:hu`'s `kv[hp.cw]` can resolve. */
int pico8_pm_ready(void);
int pico8_pm_default(const char *name, pico8_value_t *out_value);
const pico8_entity_kind_t *pico8_pm_kind(void);
const pico8_struct_class_t *pico8_pm_struct_class(void);

/* `overrides` (NULL means none) is read, not kept: `p`, `ra`, `id`, each
 * optional. The new protostar first spawns its `ga` at `ra + ng`, then
 * registers itself into `hv`/`cv`/`cg` through `lt`, after that `ga`;
 * nothing ever frees it. Requires `pico8_planet_family_init()`. */
pico8_pm_t *pico8_pm_new(const pico8_ob_table_t *overrides);

int pico8_ga_ready(void);
int pico8_ga_default(const char *name, pico8_value_t *out_value);
const pico8_entity_kind_t *pico8_ga_kind(void);
const pico8_struct_class_t *pico8_ga_struct_class(void);

/* `overrides` (NULL means none) is read, not kept: `p`, `ra`, `id` (jo:hu)
 * and `qo`, `ro`, `qg`, `q_`, `k_`, `iu`, `py`, `ms` (pm:pq), each optional.
 * The new burst registers itself into `hv`/`cv`, in no `cg`, through `lt`,
 * and nothing ever frees it. Requires `pico8_planet_family_init()`. */
pico8_ga_t *pico8_ga_new(const pico8_ob_table_t *overrides);

void pico8_nl_ly(pico8_nl_t *self, pico8_fixed_t t);

void pico8_hj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_hj_t *self, pico8_vec_t p);

/* TROU-7 */
void pico8_pm_pq(pico8_pm_t *self);

void pico8_ga_pq(pico8_ga_t *self);

/* TROU-7 */
/* Returns a fresh particle record: a REF to a heap table holding heap
 * vectors, which `ez` stores into `self.qy`. */
pico8_value_t pico8_ga_mm(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ga_t *self, pico8_vec_t p);

#endif
