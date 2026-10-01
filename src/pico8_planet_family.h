#ifndef PICO8_PLANET_FAMILY_H
#define PICO8_PLANET_FAMILY_H

#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_particle.h"
#include "pico8_planet.h"
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

/* Every `_ready` below is 1 from `pico8_planet_family_init()` to
 * `pico8_planet_family_shutdown()`, 0 otherwise. A `_kind` accessor is never
 * NULL. */

/* A laboratory: a planet whose `s` and probe are copied from its kind's
 * archetype, the probe's with `et`, which its own `hq` (`bv`) writes. */
int pico8_qh_ready(void);
const pico8_entity_kind_t *pico8_qh_kind(void);
const pico8_struct_class_t *pico8_qh_struct_class(void);

/* Initializes an allocated pico8_jo_t under `cls` and `def`, its kind's
 * archetype: jo's part, then `s` and the probe copied from `def`. Reads no
 * key, runs neither `pq` nor `lt`, records no coverage. Requires
 * `pico8_planet_family_init()`. */
void pico8_qh_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new laboratory registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
 * ever frees it. Requires `pico8_planet_family_init()`. */
pico8_jo_t *pico8_qh_new(const pico8_jo_keys_t *keys);

/* A trading hub: a laboratory whose `qw` is copied from its archetype and
 * written by its own `ly`. */
int pico8_nl_ready(void);
const pico8_entity_kind_t *pico8_nl_kind(void);
const pico8_struct_class_t *pico8_nl_struct_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new trading hub registers itself into `hv`/`cv`/`cg` through `lt`, and
 * nothing ever frees it. Requires `pico8_planet_family_init()`. */
pico8_jo_t *pico8_nl_new(const pico8_jo_keys_t *keys);

/* A processor: a laboratory and nothing of its own; its defaults are its
 * archetype's. */
int pico8_os_ready(void);
const pico8_entity_kind_t *pico8_os_kind(void);
const pico8_struct_class_t *pico8_os_struct_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new processor registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
 * ever frees it. Requires `pico8_planet_family_init()`. */
pico8_jo_t *pico8_os_new(const pico8_jo_keys_t *keys);

/* An ascension: a laboratory whose emitter is copied from its kind's
 * archetype, with `qy`, which its own `pq` (`f_`) writes. */
int pico8_hj_ready(void);
const pico8_entity_kind_t *pico8_hj_kind(void);
const pico8_struct_class_t *pico8_hj_struct_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new ascension registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
 * ever frees it. Requires `pico8_planet_family_init()`. */
pico8_jo_t *pico8_hj_new(const pico8_jo_keys_t *keys);

/* Initializes an allocated pico8_jo_t under `cls` and `def`, its kind's
 * archetype: qh's part, then the emitter copied from `def`, `qy` nil. Reads
 * no key, runs neither `pq` nor `lt`, records no coverage. Requires
 * `pico8_planet_family_init()`. */
void pico8_hj_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def);

/* The dispatchers hj's descriptor holds, for a daughter's `static const`
 * descriptor to name. */
void pico8_qh_ly_dispatch(pico8_struct_header_t *self, pico8_fixed_t t);
void pico8_hj_kj_dispatch(pico8_struct_header_t *self, const pico8_vec_t *ra);
/* pico8_jo_rendre_dispatch, then the planet's `qy`. */
void pico8_hj_rendre_dispatch(pico8_struct_header_t *self);

/* `pm` and `ga` are the only classes here whose spec sets `fz`: the two
 * kinds `jo:hu`'s `kv[hp.cw]` can resolve. */

/* A protostar: an ascension and nothing of its own; its defaults are its
 * archetype's. */
int pico8_pm_ready(void);
const pico8_entity_kind_t *pico8_pm_kind(void);
const pico8_struct_class_t *pico8_pm_struct_class(void);

/* `keys` (NULL means none) is read, not kept. The new protostar first
 * spawns its `ga` at `ra + ng`, then registers itself into `hv`/`cv`/`cg`
 * through `lt`, after that `ga`; nothing ever frees it. Requires
 * `pico8_planet_family_init()`. */
pico8_jo_t *pico8_pm_new(const pico8_jo_keys_t *keys);

/* A particle burst: a planet whose `iu`, `ms` and emitter are copied from
 * its archetype, with `qy`, which its own `pq` (`f_`) writes; the emitter's
 * `pn`, `du` are left unwritten, read by `g_:mm` only. `by` is the one list
 * every burst shares, which `ib` extends in place and each init empties. A
 * burst spawned by `pm:pq` takes `iu`, `ms`, `py`, `k_`, `qo`, `ro`, `qg`,
 * `q_` from its keys instead. */
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
pico8_jo_t *pico8_ga_new(const pico8_ga_keys_t *keys);

void pico8_nl_ly(pico8_jo_t *self, pico8_fixed_t t);

void pico8_hj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec_t p);

/* TROU-7 */
void pico8_pm_pq(pico8_jo_t *self);

void pico8_ga_pq(pico8_jo_t *self);

/* A particle (`has_a` 1) from pico8_particule_prendre, which `ez` stores
 * into `self.qy`. */
pico8_qy_t *pico8_ga_mm(pico8_struct_header_t *self);

void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec_t p);

#endif
