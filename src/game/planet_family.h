#ifndef GAME_PLANET_FAMILY_H
#define GAME_PLANET_FAMILY_H

#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/particle.h"
#include "game/planet.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* A NULL instance returns at once from trading_hub_update_settled,
 * ascension_draw, protostar_construct, burst_construct and burst_draw; the
 * other functions dereference it. */

/* TROU-8 */
/* Requires planet_init() (asserted); calls planet_family_shutdown(), then
 * draw_functions_init(). */
void planet_family_init(void);

/* Frees no instance; the outgoing routes list every burst shares gives its
 * array back to the loading sub-zone (entity_list_free). Idempotent, and
 * safe before the first planet_family_init(). */
void planet_family_shutdown(void);

/* Every _ready below is 1 from planet_family_init() to
 * planet_family_shutdown(), 0 otherwise. A _kind accessor is never NULL. */

/* A laboratory: a planet whose sprite and targeting are copied from its
 * kind's archetype, the targeting with a target, which its own drag method
 * (targeting_track) writes. */
int structure_ready(void);
const entity_kind_t *structure_kind(void);
const entity_class_t *structure_class(void);

/* Initializes an allocated planet_t under the given class and archetype:
 * the planet's part, then the sprite and the targeting copied from the
 * archetype. Reads no key, runs neither planet_construct nor
 * entity_construct, records no coverage. Requires planet_init(), which
 * planet_init_instance asserts. */
void structure_init_instance(planet_t *self, const entity_class_t *cls, const planet_archetype_t *archetype);

/* `keys` (NULL means none) is not kept: planet_id only, borrowed; map_pos
 * and pos NULL (asserted). The new laboratory registers itself (entity_register),
 * and lives in the construction zone. Requires planet_family_init(). */
planet_t *structure_new(const planet_keys_t *keys);

/* A trading hub: a laboratory whose palette block is copied from its
 * archetype and written by its own settled update. */
int trading_hub_ready(void);
const entity_kind_t *trading_hub_kind(void);
const entity_class_t *trading_hub_class(void);

/* `keys` (NULL means none) is not kept: planet_id only, borrowed; map_pos
 * and pos NULL (asserted). The new trading hub registers itself (entity_register),
 * and lives in the construction zone. Requires planet_family_init(). */
planet_t *trading_hub_new(const planet_keys_t *keys);

/* A processor: a laboratory and nothing of its own; its defaults are its
 * archetype's. */
int processor_ready(void);
const entity_kind_t *processor_kind(void);
const entity_class_t *processor_class(void);

/* `keys` (NULL means none) is not kept: planet_id only, borrowed; map_pos
 * and pos NULL (asserted). The new processor registers itself (entity_register),
 * and lives in the construction zone. Requires planet_family_init(). */
planet_t *processor_new(const planet_keys_t *keys);

/* An ascension: a laboratory whose emitter is copied from its kind's
 * archetype, with a particle list, which its construction (emitter_init)
 * writes. */
int ascension_ready(void);
const entity_kind_t *ascension_kind(void);
const entity_class_t *ascension_class(void);

/* `keys` (NULL means none) is not kept: planet_id only, borrowed; map_pos
 * and pos NULL (asserted). The new ascension registers itself (entity_register),
 * and lives in the construction zone. Requires planet_family_init(). */
planet_t *ascension_new(const planet_keys_t *keys);

/* Initializes an allocated planet_t under the given class and archetype:
 * the laboratory's part (structure_init_instance), then the emitter copied
 * from the archetype, its particle list NULL. Reads no key, runs neither
 * emitter_init, planet_construct nor entity_construct, records no
 * coverage. Requires planet_init(), which planet_init_instance asserts. */
void ascension_init_instance(planet_t *self, const entity_class_t *cls, const planet_archetype_t *archetype);

/* The methods the ascension class holds, for a derived class's static
 * const class to name. */
void structure_update_settled_method(entity_t *self, p8_fixed_t t);
void ascension_draw_method(entity_t *self, const vec_t *screen_pos);
/* planet_release_row, then the particles (emitter_release_particles),
 * then planet_grab_arena_released. */
void ascension_release_method(entity_t *self);

/* Protostar and burst are the only classes here with a class name in the
 * table of planet_kind_named: the two kinds planet_apply_project can turn
 * a planet into, by its research project's class name. */

/* A protostar: an ascension and nothing of its own; its defaults are its
 * archetype's. */
int protostar_ready(void);
const entity_kind_t *protostar_kind(void);
const entity_class_t *protostar_class(void);

/* `keys` (NULL means none) is not kept; its map_pos, pos and planet_id
 * are borrowed. The new protostar first spawns its burst at its position
 * plus the archetype's spawn_offset, then registers itself
 * (entity_register), after that burst; it lives in the construction zone.
 * Requires planet_family_init(). */
planet_t *protostar_new(const planet_keys_t *keys);

/* A particle burst: a planet whose acceleration factor, speed and emitter
 * are copied from its archetype, with a particle list, which its
 * construction (emitter_init) writes; the emitter's angle slices and base
 * radius are left unwritten, read by probe_spawn_particle only. Its
 * outgoing routes are the one list every burst shares, which
 * planet_rebuild_trades extends in place and each init empties. A burst
 * spawned by a protostar's construction takes the acceleration factor,
 * speed, speed factor, trail colors and the four matrix entries from its
 * keys instead. */
int burst_ready(void);
const entity_kind_t *burst_kind(void);
const entity_class_t *burst_class(void);

/* A burst's construction keys: a planet's keys (planet_apply_project),
 * whose map_pos, pos and planet_id the burst borrows, then the group a
 * protostar's construction passes (construct NULL: the archetype's own
 * values stand; its trail_colors NULL: the burst has none). pos_copy, when
 * not NULL, is copied into the burst's own slot and becomes its pos;
 * planet.pos must then be NULL (asserted). */
typedef struct {
    planet_keys_t planet;
    const vec_t *pos_copy;
    const burst_construct_keys_t *construct;
} burst_keys_t;

/* `keys` (NULL means none) is not kept; its map_pos, pos and planet_id
 * are borrowed. The new burst lives, with its slipways list and its copied
 * position, in one slot of the burst kind (particle_burst_slot_take), and
 * registers itself (entity_register) under no tag; its release method
 * gives that slot back once it leaves through the exit queue. Requires
 * planet_family_init(). */
planet_t *burst_new(const burst_keys_t *keys);

void trading_hub_update_settled(planet_t *self, p8_fixed_t t);

void ascension_draw(p8_graphics_t *gfx, p8_memory_t *mem, planet_t *self, vec_t p);

void protostar_construct(planet_t *self);

void burst_construct(planet_t *self);

/* A particle with an acceleration, from particle_take, which emitter_spawn
 * stores into the burst's particle list. */
particle_t *burst_spawn_particle(entity_t *self);

void burst_draw(p8_graphics_t *graphics, p8_memory_t *memory, planet_t *self, vec_t screen_pos);

#endif
