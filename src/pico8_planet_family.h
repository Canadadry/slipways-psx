#ifndef PICO8_PLANET_FAMILY_H
#define PICO8_PLANET_FAMILY_H

#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_particle.h"
#include "pico8_planet.h"
#include "pico8_probe.h"
#include "pico8_struct.h"
#include "pico8_vec.h"

/* Throughout this module -- this file and its `_trade` sibling -- a NULL
 * instance, gfx or mem argument is a safe no-op, and an absent or
 * wrong-kind field reads as its zero/false default rather than crashing. */

/* TROU-8 */
/* Calls `pico8_pi_init()` first. */
void pico8_planet_family_init(void);

/* Frees nothing, no instance nor ga's shared `by`. Idempotent, and safe
 * before the first `pico8_planet_family_init()`. */
void pico8_planet_family_shutdown(void);

/* A laboratory: a bare planet, then qh's own defaults, `dq`, `od`, `fp`
 * and `dw` in the probe with `et`, which its own `hq` (`bv`) writes. A
 * default of qh whose name is a member of `jo` or `ke` (`me`, `hf`,
 * `mi`) lives in that member, never in a second copy here. `px` and
 * `ba` point at qh's generated data (pico8_game_data.h). `probe` points at
 * `probe_own`: never copy the struct. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_jo_t jo;
    pico8_fixed_t hp;
    const pico8_jb_row_t *px;
    int qh;
    pico8_fixed_t s;
    const pico8_int_seq_t *ba;
    pico8_probe_t *probe;
    pico8_probe_t probe_own;
} pico8_qh_t;

/* The `probe_of` of qh's descriptor, for its daughters' to name. */
pico8_probe_t *pico8_qh_probe_of(void *self);

/* A trading hub: a laboratory, then nl's own defaults `rr`, `nl` and `qw`
 * (`qw` also written by its own `ly`). A default of nl whose name is a member of `qh`
 * or `jo` (`qh`, `hp`, `s`, `px`, `od`, `ba`) lives in that member.
 * `px`, `ba` and `rr` point at nl's generated data. Nothing frees
 * the struct: TROU-7. */
typedef struct {
    pico8_qh_t qh;
    const char *rr;
    pico8_fixed_t nl;
    pico8_fixed_t qw;
} pico8_nl_t;

/* A processor: a laboratory and nothing of its own. Each default of os
 * lives in the member of that name, qh's (`hp`, `px`, `qh`, `s`, `od`) or
 * jo's (`hs`). `px` points at os's generated data, `ba` at
 * qh's. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_qh_t qh;
} pico8_os_t;

/* An ascension: a laboratory, then `rr` and hj's own emitter defaults,
 * in the emitter with `qy`, which its own `pq` (`f_`) writes. A default of
 * hj whose name is a member of `qh` or `jo` (`hp`, `qh`, `px`, `od`,
 * `s`) lives in that member. `px`, the emitter's `k_` and `rr` point
 * at hj's generated data, `ba` at qh's. `emitter` points at `emitter_own`:
 * never copy the struct. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_qh_t qh;
    const char *rr;
    pico8_emitter_t *emitter;
    pico8_emitter_t emitter_own;
} pico8_hj_t;

/* The `emitter_of` of hj's descriptor, for `mx`'s to name (`pm`'s too). */
pico8_emitter_t *pico8_hj_emitter_of(void *self);

/* A protostar: an ascension, then pm's own defaults `ng` and `fz`, which no
 * ancestor has a member for. A default of pm whose name is a member of
 * `hj`, `qh`, `jo` or `ke` (`me`, `hp`, `px`, `s` and the 11 emitter
 * defaults) lives in that member. `px`, `k_`, `ng` and `fz` point
 * at pm's generated data, `ba` at qh's; `qy` is the protostar's own.
 * Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_hj_t hj;
    const pico8_vec_t *ng;
    const char *fz;
} pico8_pm_t;

/* A particle burst: a bare planet, then ga's own defaults, those of the
 * emitter in it with `qy`, which its own `pq` (`f_`) writes. A default of
 * ga whose name is a member of `jo` (`hf`, `by`, `kq`, `fa`) lives in
 * that member. The emitter's `k_`, `kq` and `fz` point at ga's
 * generated data; `by` is the one list every burst shares, which `ib`
 * extends in place and each init empties. A burst spawned by `pm:pq` points
 * `k_` at pico8_pm_ga_k__seq instead. `emitter` points at `emitter_own`:
 * never copy the struct. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_jo_t jo;
    const char *fz;
    pico8_fixed_t iu;
    pico8_fixed_t ms;
    pico8_emitter_t *emitter;
    pico8_emitter_t emitter_own;
} pico8_ga_t;

/* Every `_ready` below is 1 from `pico8_planet_family_init()` to
 * `pico8_planet_family_shutdown()`, 0 otherwise. A `_kind` accessor is never
 * NULL. */
int pico8_qh_ready(void);
const pico8_entity_kind_t *pico8_qh_kind(void);
const pico8_struct_class_t *pico8_qh_struct_class(void);

/* Initializes an allocated pico8_qh_t under `cls`: jo's part, qh's members
 * nil, qh's defaults. Reads no key, runs neither `pq` nor `lt`, records no
 * coverage. Requires `pico8_planet_family_init()`. */
void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new laboratory registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
 * ever frees it. Requires `pico8_planet_family_init()`. */
pico8_qh_t *pico8_qh_new(const pico8_jo_keys_t *keys);

int pico8_nl_ready(void);
const pico8_entity_kind_t *pico8_nl_kind(void);
const pico8_struct_class_t *pico8_nl_struct_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new trading hub registers itself into `hv`/`cv`/`cg` through `lt`, and
 * nothing ever frees it. Requires `pico8_planet_family_init()`. */
pico8_nl_t *pico8_nl_new(const pico8_jo_keys_t *keys);

/* `e` itself when its class is qh's or has qh's up its `parent` chain; NULL
 * otherwise, and for a NULL `e`. */
const pico8_qh_t *pico8_qh_of(const pico8_struct_header_t *e);

/* `e` itself when its class is nl's or has nl's up its `parent` chain; NULL
 * otherwise, and for a NULL `e`. */
const pico8_nl_t *pico8_nl_of(const pico8_struct_header_t *e);

int pico8_os_ready(void);
const pico8_entity_kind_t *pico8_os_kind(void);
const pico8_struct_class_t *pico8_os_struct_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new processor registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
 * ever frees it. Requires `pico8_planet_family_init()`. */
pico8_os_t *pico8_os_new(const pico8_jo_keys_t *keys);

int pico8_hj_ready(void);
const pico8_entity_kind_t *pico8_hj_kind(void);
const pico8_struct_class_t *pico8_hj_struct_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new ascension registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
 * ever frees it. Requires `pico8_planet_family_init()`. */
pico8_hj_t *pico8_hj_new(const pico8_jo_keys_t *keys);

/* Initializes an allocated pico8_hj_t under `cls`: qh's part, hj's members
 * nil, hj's defaults. Reads no key, runs neither `pq` nor `lt`, records no
 * coverage. Requires `pico8_planet_family_init()`. */
void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls);

/* The dispatchers hj's descriptor holds, for a daughter's `static const`
 * descriptor to name. */
void pico8_qh_ly_dispatch(void *self, pico8_fixed_t t);
void pico8_hj_kj_dispatch(void *self, const pico8_vec_t *ra);

/* `pm` and `ga` are the only classes here whose spec sets `fz`: the two
 * kinds `jo:hu`'s `kv[hp.cw]` can resolve. */
int pico8_pm_ready(void);
const pico8_entity_kind_t *pico8_pm_kind(void);
const pico8_struct_class_t *pico8_pm_struct_class(void);

/* `keys` (NULL means none) is read, not kept. The new protostar first
 * spawns its `ga` at `ra + ng`, then registers itself into `hv`/`cv`/`cg`
 * through `lt`, after that `ga`; nothing ever frees it. Requires
 * `pico8_planet_family_init()`. */
pico8_pm_t *pico8_pm_new(const pico8_jo_keys_t *keys);

int pico8_ga_ready(void);
const pico8_entity_kind_t *pico8_ga_kind(void);
const pico8_struct_class_t *pico8_ga_struct_class(void);

/* A burst's construction keys: `jo`'s (jo:hu), then the group pm:pq passes
 * (`pq` NULL: ga's own defaults stand; its `k_` NULL: `k_` is nil). */
typedef struct {
    pico8_jo_keys_t jo;
    const pico8_ga_pq_keys_t *pq;
} pico8_ga_keys_t;

/* `keys` (NULL means none) is read, not kept. The new burst registers
 * itself into `hv`/`cv`, in no `cg`, through `lt`, and nothing ever frees
 * it. Requires `pico8_planet_family_init()`. */
pico8_ga_t *pico8_ga_new(const pico8_ga_keys_t *keys);

void pico8_nl_ly(pico8_nl_t *self, pico8_fixed_t t);

void pico8_hj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_hj_t *self, pico8_vec_t p);

/* TROU-7 */
void pico8_pm_pq(pico8_pm_t *self);

void pico8_ga_pq(pico8_ga_t *self);

/* TROU-7 */
/* Returns a freshly allocated `pico8_qy_t` (`has_a` 1), one allocation,
 * which `ez` stores into `self.qy`. */
pico8_qy_t *pico8_ga_mm(void *self);

void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ga_t *self, pico8_vec_t p);

#endif
