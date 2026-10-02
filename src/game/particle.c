#include "game/particle.h"

#include <assert.h>
#include <stddef.h>

#include "engine/dispatch_context.h"
#include "engine/entity_geom.h"
#include "p8/fixed.h"
#include "game/game_data.h"
#include "engine/vec.h"
#include "game/particle_zone.h"

typedef char particle_fits_its_slot_t[sizeof(particle_t) <= PARTICLE_SLOT ? 1 : -1];

/* The host's layer, and its position into the out vector (zero when absent). */
static p8_fixed_t emitter_host_layer_pos(const entity_t *self, vec_t *ra) {
    assert(self->layer != NULL && self->pos != NULL &&
           "emitter_update: a host that carries no layer or no pos, none of whose draw runs it -- stop");
    *ra = *self->pos != NULL ? **self->pos : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    return *self->layer;
}

static int particle_to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

particle_t *particle_take(void) {
    return (particle_t *)allocator_alloc(particle_zone(PARTICLE_SLOT_PARTICLE), sizeof(particle_t));
}

const allocator_t *particle_burst_slot_take(void) {
    return particle_zone_slot(PARTICLE_SLOT_BURST);
}

void emitter_init(entity_t *self) {
    emitter_init_in(self, PARTICLE_SLOT_LIST);
}

void emitter_init_in(entity_t *self, particle_slot_kind_t slot_kind) {
    const allocator_t *slot;
    emitter_t *emitter;
    pointer_list_t *qy;

    if (self == NULL) {
        return;
    }
    assert(self->emitter != NULL && "emitter_init_in: a class that carries no particle emitter has no particle list -- stop");
    assert((slot_kind == PARTICLE_SLOT_LIST || slot_kind == PARTICLE_SLOT_HELP_LIST) && "emitter_init_in: a particle list is a list or a help screen list");

    emitter = self->emitter;
    emitter_release_particles(emitter);
    slot = particle_zone_slot(slot_kind);
    qy = (pointer_list_t *)allocator_alloc(slot, sizeof *qy);
    pointer_list_init_reserve(qy, slot,
                            slot_kind == PARTICLE_SLOT_HELP_LIST ? PARTICLES_HELP_SCREEN_CAPACITY : PARTICLES_CAPACITY);
    emitter->particles = qy;
    emitter->particles_slot_kind = slot_kind;
}

void emitter_release_particles(emitter_t *emitter) {
    pointer_list_t *qy = emitter->particles;
    const allocator_t *particle = particle_zone(PARTICLE_SLOT_PARTICLE);
    int i;

    if (qy == NULL) {
        return;
    }
    for (i = 0; i < qy->count; i++) {
        allocator_release(particle, qy->items[i]);
    }
    /* The list lives in its own arena: the list dies with it. */
    arena_release(qy->zone);
    emitter->particles = NULL;
}

void emitter_spawn(entity_t *self, p8_fixed_t n) {
    emitter_t *emitter;
    pointer_list_t *qy;

    if (self == NULL) {
        return;
    }
    emitter = self->emitter;
    qy = emitter != NULL ? emitter->particles : NULL;

    while (p8_fixed_rnd_default() < n) {
        if (qy != NULL && self->class->spawn_particle != NULL) {
            pointer_list_add(qy, self->class->spawn_particle(self));
            particle_zone_record_elements(emitter->particles_slot_kind, qy->count);
        }
        n = p8_fixed_sub(n, p8_fixed_from_int(1));
    }
}

void emitter_update(const p8_graphics_t *graphics, p8_memory_t *memory, entity_t *self) {
    const emitter_t *emitter;
    p8_fixed_t layer;
    vec_t screen_pos;
    p8_fixed_t center_x;
    p8_fixed_t center_y;
    p8_fixed_t matrix_xx;
    p8_fixed_t matrix_yx;
    p8_fixed_t matrix_xy;
    p8_fixed_t matrix_yy;
    const int_seq_t *colors;
    pointer_list_t *particles;
    int index;

    if (self == NULL) {
        return;
    }
    assert(self->emitter != NULL && "emitter_update: a class that carries no particle emitter is no receiver -- stop");

    emitter = self->emitter;
    layer = emitter_host_layer_pos(self, &screen_pos);
    if (layer < p8_fixed_from_int(10) && !entity_geom_is_on_screen(self)) {
        return;
    }

    center_x = screen_pos.x;
    center_y = screen_pos.y;
    matrix_xx = emitter->matrix_xx;
    matrix_yx = emitter->matrix_yx;
    matrix_xy = emitter->matrix_xy;
    matrix_yy = emitter->matrix_yy;
    colors = emitter->trail_colors;

    particles = emitter->particles;
    if (particles != NULL) {
        for (index = 0; index < particles->count; index++) {
            particle_t *particle = particles->items[index];
            vec_t pv;

            if (particle == NULL) {
                continue;
            }
            pv = particle->velocity;

            particle->pos = vec_add(particle->pos, pv);

            particle->velocity.x = p8_fixed_add(p8_fixed_mul(matrix_xx, pv.x), p8_fixed_mul(matrix_xy, pv.y));
            /* The second row reads the x just written, not the snapshot's:
             * passing pv.x here instead would look like a fix and would
             * move every particle whose matrix_yx is not 0. */
            particle->velocity.y = p8_fixed_add(p8_fixed_mul(matrix_yx, particle->velocity.x), p8_fixed_mul(matrix_yy, pv.y));

            if (particle->has_acceleration) {
                particle->velocity = vec_add(particle->velocity, particle->acceleration);
            }

            particle->life = p8_fixed_sub(particle->life, P8_FIXED_LIT(0x000023d7, 0.14));

            if (particle->life <= p8_fixed_from_int(1)) {
                /* TROU-14 */
                /* Never compensate the index: the particle that slides
                 * into the freed slot waits for the next frame. */
                pointer_list_del(particles, particle);
                allocator_release(particle_zone(PARTICLE_SLOT_PARTICLE), (unsigned char *)particle);
            } else {
                int px = particle_to_int(p8_fixed_add(center_x, particle->pos.x));
                int py = particle_to_int(p8_fixed_add(center_y, particle->pos.y));
                int col = 0;
                int idx = particle_to_int(p8_fixed_flr(particle->life));

                if (colors != NULL) {
                    int_seq_at(colors->items, colors->count, idx, &col);
                }

                p8_pset(graphics, memory, px, py, col);
            }
        }
    }

    emitter_spawn(self, emitter->archetype->spawn_rate);
}

void emitter_update_method(entity_t *self, const vec_t *ra) {
    (void)ra;
    emitter_update(dispatch_context_gfx(), dispatch_context_mem(), self);
}
