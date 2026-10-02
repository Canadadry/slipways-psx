#ifndef GAME_PLANET_FAMILY_H
#define GAME_PLANET_FAMILY_H

#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/particle.h"
#include "game/planet.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* Throughout this module -- this file and its `_trade` sibling -- a NULL
 * instance, gfx or mem argument is a safe no-op, and an absent or
 * wrong-kind field reads as its zero/false default rather than crashing. */

/* TROU-8 */
/* Calls `pico8_pi_init()` first. */
void planet_family_init(void);

/* Frees no instance; ga's shared `by` is forgotten without being read, its
 * array going with the loading sub-zone. Idempotent, and safe before the
 * first `pico8_planet_family_init()`. */
void planet_family_shutdown(void);

/* Every `_ready` below is 1 from `pico8_planet_family_init()` to
 * `pico8_planet_family_shutdown()`, 0 otherwise. A `_kind` accessor is never
 * NULL. */

/* A laboratory: a planet whose `s` and probe are copied from its kind's
 * archetype, the probe's with `et`, which its own `hq` (`bv`) writes. */
int structure_ready(void);
const entity_kind_t *structure_kind(void);
const entity_class_t *structure_class(void);

/* Initializes an allocated pico8_jo_t under `cls` and `def`, its kind's
 * archetype: jo's part, then `s` and the probe copied from `def`. Reads no
 * key, runs neither `pq` nor `lt`, records no coverage. Requires
 * `pico8_planet_family_init()`. */
void structure_init_instance(planet_t *self, const entity_class_t *cls, const planet_archetype_t *archetype);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new laboratory registers itself into `hv`/`cv`/`cg` through `lt`, and it
 * lives in the game generation. Requires `pico8_planet_family_init()`. */
planet_t *structure_new(const planet_keys_t *keys);

/* A trading hub: a laboratory whose `qw` is copied from its archetype and
 * written by its own `ly`. */
int trading_hub_ready(void);
const entity_kind_t *trading_hub_kind(void);
const entity_class_t *trading_hub_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new trading hub registers itself into `hv`/`cv`/`cg` through `lt`, and
 * it lives in the game generation. Requires `pico8_planet_family_init()`. */
planet_t *trading_hub_new(const planet_keys_t *keys);

/* A processor: a laboratory and nothing of its own; its defaults are its
 * archetype's. */
int processor_ready(void);
const entity_kind_t *processor_kind(void);
const entity_class_t *processor_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new processor registers itself into `hv`/`cv`/`cg` through `lt`, and it
 * lives in the game generation. Requires `pico8_planet_family_init()`. */
planet_t *processor_new(const planet_keys_t *keys);

/* An ascension: a laboratory whose emitter is copied from its kind's
 * archetype, with `qy`, which its own `pq` (`f_`) writes. */
int ascension_ready(void);
const entity_kind_t *ascension_kind(void);
const entity_class_t *ascension_class(void);

/* `keys` (NULL means none) is read, not kept: `id` only, `p` and `ra`
 * NULL. The new ascension registers itself into `hv`/`cv`/`cg` through `lt`, and it
 * lives in the game generation. Requires `pico8_planet_family_init()`. */
planet_t *ascension_new(const planet_keys_t *keys);

/* Initializes an allocated pico8_jo_t under `cls` and `def`, its kind's
 * archetype: qh's part, then the emitter copied from `def`, `qy` nil. Reads
 * no key, runs neither `pq` nor `lt`, records no coverage. Requires
 * `pico8_planet_family_init()`. */
void ascension_init_instance(planet_t *self, const entity_class_t *cls, const planet_archetype_t *archetype);

/* The dispatchers hj's descriptor holds, for a daughter's `static const`
 * descriptor to name. */
void structure_update_settled_method(entity_t *self, p8_fixed_t t);
void ascension_draw_method(entity_t *self, const vec_t *ra);
/* pico8_jo_rendre_dispatch, then the planet's `qy`. */
void ascension_release_method(entity_t *self);

/* `pm` and `ga` are the only classes here whose spec sets `fz`: the two
 * kinds `jo:hu`'s `kv[hp.cw]` can resolve. */

/* A protostar: an ascension and nothing of its own; its defaults are its
 * archetype's. */
int protostar_ready(void);
const entity_kind_t *protostar_kind(void);
const entity_class_t *protostar_class(void);

/* `keys` (NULL means none) is read, not kept. The new protostar first
 * spawns its `ga` at `ra + ng`, then registers itself into `hv`/`cv`/`cg`
 * through `lt`, after that `ga`; it lives in the game generation. Requires
 * `pico8_planet_family_init()`. */
planet_t *protostar_new(const planet_keys_t *keys);

/* A particle burst: a planet whose `iu`, `ms` and emitter are copied from
 * its archetype, with `qy`, which its own `pq` (`f_`) writes; the emitter's
 * `pn`, `du` are left unwritten, read by `g_:mm` only. `by` is the one list
 * every burst shares, which `ib` extends in place and each init empties. A
 * burst spawned by `pm:pq` takes `iu`, `ms`, `py`, `k_`, `qo`, `ro`, `qg`,
 * `q_` from its keys instead. */
int burst_ready(void);
const entity_kind_t *burst_kind(void);
const entity_class_t *burst_class(void);

/* A burst's construction keys: `jo`'s (jo:hu), whose `p`, `ra`, `id` the
 * burst borrows, then the group pm:pq passes (`pq` NULL: ga's own defaults
 * stand; its `k_` NULL: `k_` is nil). `ra_copie`, when not NULL, is copied
 * into the burst's own place and becomes its `ra`; `jo.ra` must then be
 * NULL. */
typedef struct {
    planet_keys_t planet;
    const vec_t *pos_copy;
    const burst_construct_keys_t *construct;
} burst_keys_t;

/* `keys` (NULL means none) is not kept. The new burst lives, with its `oe`
 * and its copied `ra`, in one place of the `ga` sort, and registers itself
 * into `hv`/`cv`, in no `cg`, through `lt`; its `rendre` gives that place
 * back once it leaves through the exit queue. Requires
 * `pico8_planet_family_init()`. */
planet_t *burst_new(const burst_keys_t *keys);

void trading_hub_update_settled(planet_t *self, p8_fixed_t t);

void ascension_draw(p8_graphics_t *gfx, p8_memory_t *mem, planet_t *self, vec_t p);

void protostar_construct(planet_t *self);

void burst_construct(planet_t *self);

/* A particle (`has_a` 1) from pico8_particule_prendre, which `ez` stores
 * into `self.qy`. */
particle_t *burst_spawn_particle(entity_t *self);

void burst_draw(p8_graphics_t *graphics, p8_memory_t *memory, planet_t *self, vec_t screen_pos);

#endif
