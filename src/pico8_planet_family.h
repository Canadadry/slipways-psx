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

/* A laboratory: a bare planet, then qh's members copied from its kind's
 * archetype, the probe's with `et`, which its own `hq` (`bv`) writes.
 * `probe` points at `probe_own`: never copy the struct. Nothing frees the
 * struct: TROU-7. */
typedef struct {
    pico8_jo_t jo;
    pico8_fixed_t s;
    pico8_probe_t *probe;
    pico8_probe_t probe_own;
} pico8_qh_t;

/* The `probe_of` of qh's descriptor, for its daughters' to name. */
pico8_probe_t *pico8_qh_probe_of(void *self);

/* A trading hub: a laboratory, then `qw`, copied from its archetype and
 * written by its own `ly`. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_qh_t qh;
    pico8_fixed_t qw;
} pico8_nl_t;

/* A processor: a laboratory and nothing of its own; its defaults are its
 * archetype's. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_qh_t qh;
} pico8_os_t;

/* An ascension: a laboratory, then an emitter copied from its kind's
 * archetype, with `qy`, which its own `pq` (`f_`) writes. `emitter` points
 * at `emitter_own`: never copy the struct. Nothing frees the struct:
 * TROU-7. */
typedef struct {
    pico8_qh_t qh;
    pico8_emitter_t *emitter;
    pico8_emitter_t emitter_own;
} pico8_hj_t;

/* The `emitter_of` of hj's descriptor, for `mx`'s to name (`pm`'s too). */
pico8_emitter_t *pico8_hj_emitter_of(void *self);

/* A protostar: an ascension and nothing of its own; its defaults are its
 * archetype's. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_hj_t hj;
} pico8_pm_t;

/* A particle burst: a bare planet, then `iu`, `ms` and an emitter copied
 * from its archetype, with `qy`, which its own `pq` (`f_`) writes; the
 * emitter's `pn`, `du`, `ki`, `pk` are left unwritten, read by `g_:mm`
 * only. `jo.by` is the one list every burst shares, which `ib` extends in
 * place and each init empties. A burst spawned by `pm:pq` takes `iu`, `ms`,
 * `py`, `k_`, `qo`, `ro`, `qg`, `q_` from its keys instead. `emitter` points
 * at `emitter_own`: never copy the struct. Nothing frees the struct:
 * TROU-7. */
typedef struct {
    pico8_jo_t jo;
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

/* Initializes an allocated pico8_qh_t under `cls` and `def`, its kind's
 * archetype: jo's part, then `s` and the probe copied from `def`. Reads no
 * key, runs neither `pq` nor `lt`, records no coverage. Requires
 * `pico8_planet_family_init()`. */
void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);

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

/* Initializes an allocated pico8_hj_t under `cls` and `def`, its kind's
 * archetype: qh's part, then the emitter copied from `def`, `qy` nil. Reads
 * no key, runs neither `pq` nor `lt`, records no coverage. Requires
 * `pico8_planet_family_init()`. */
void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);

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
