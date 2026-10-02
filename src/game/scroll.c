#include "game/scroll.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "game/draw_functions.h"
#include "engine/vec.h"
#include "engine/game_zone.h"

static int g_qb_up = 0;

/* NULL reads as the zero vector. */
static vec_t pico8_qb_vec(const vec_t *v) {
    if (v != NULL) {
        return *v;
    }
    return vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

/* dg() dispatches the per-tick state method as `qc[qc.me](qc,qc.t)`, so
 * `t` is the frame counter, which qb:ph does not read. */
static void pico8_qb_ph_dispatch(entity_t *self, p8_fixed_t t) {
    (void)t;
    pico8_qb_ph(dispatch_context_input(), (pico8_qb_t *)self);
}

static void pico8_qb_md_dispatch(entity_t *self, p8_fixed_t fy) {
    pico8_qb_md(dispatch_context_gfx(), (pico8_qb_t *)self, fy);
}

static const entity_class_t g_qb_struct_class = {
    .states = {[ENTITY_STATE_NORMAL] = pico8_qb_ph_dispatch},
    .apply_camera = pico8_qb_md_dispatch,
};

static pico8_qb_t *pico8_qb_struct_new(void) {
    pico8_qb_t *self = (pico8_qb_t *)allocator_alloc(game_zone(), sizeof *self);

    entity_init_instance(&self->header, &g_qb_struct_class, camera_archetype.state, camera_archetype.state_time);
    self->def = &camera_archetype;
    self->header.pos = &self->ra;
    self->header.map_pos = &self->p;
    self->header.shake = &self->ma;
    self->v = &self->def->velocity;
    self->ma = self->def->shake;
    self->p = &self->def->pos_before_shake;
    self->ra = &self->def->pos;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("camera");
#endif
    entity_construct(&self->header);
    return self;
}

void pico8_qb_init(void) {
    pico8_qb_shutdown();
    draw_functions_init();
    assert(entity_ready() && "qb descends from ke");
    g_qb_up = 1;
}

void pico8_qb_shutdown(void) {
    g_qb_up = 0;
}

int pico8_qb_ready(void) {
    return g_qb_up;
}

const entity_class_t *pico8_qb_struct_class(void) {
    return &g_qb_struct_class;
}

pico8_qb_t *pico8_qb_new(void) {
    return pico8_qb_struct_new();
}

void pico8_qb_ph(const p8_input_t *input, pico8_qb_t *self) {
    vec_t ip = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    const vec_seq_t *pc = NULL;
    vec_t p;
    vec_t v;
    int b;

    if (self == NULL || input == NULL) {
        return;
    }

    pc = self->def->dpad_directions;

    for (b = 0; b <= 3; b++) {
        int btn_p1 = p8_btn(input, b);
        /* TROU-2 */
        int btn_p2 = 0;

        if (btn_p1 || btn_p2) {
            vec_t dir = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
            if (pc != NULL && b < pc->count) {
                dir = pc->items[b];
            }
            ip = vec_add(ip, vec_mul(dir, p8_fixed_from_int(3)));
        }
    }

    p = pico8_qb_vec(self->p);
    v = pico8_qb_vec(self->v);

    p = vec_add(p, v);
    v = vec_add(v, vec_mul(vec_sub(ip, v), P8_FIXED_LIT(0x00003333, 0.2)));

    p.x = p8_fixed_mid(p8_fixed_from_int(-64), p8_fixed_from_int(320), p.x);
    p.y = p8_fixed_mid(p8_fixed_from_int(-64), p8_fixed_from_int(320), p.y);

    self->p_own = p;
    self->p = &self->p_own;
    pico8_qb_set_v(self, v);

    self->ra_own = entity_geom_shake(p, (entity_t *)self);
    self->ra = &self->ra_own;
}

void pico8_qb_set_v(pico8_qb_t *self, vec_t v) {
    self->v_own = v;
    self->v = &self->v_own;
}

void pico8_qb_md(p8_graphics_t *gfx, pico8_qb_t *self, p8_fixed_t fy) {
    vec_t cp;

    if (self == NULL || gfx == NULL) {
        return;
    }

    cp = vec_mul(pico8_qb_vec(self->ra), fy);
    p8_camera(gfx, p8_fixed_flr_int(cp.x), p8_fixed_flr_int(cp.y));
}

pico8_qb_t *pico8_mo_qb(void) {
    entity_t *mo = main_camera();

    if (mo == NULL || mo->class == main_camera_placeholder_class()) {
        return NULL;
    }
    assert(mo->class == pico8_qb_struct_class() && "mo: only the placeholder or a qb is ever stored");
    return (pico8_qb_t *)mo;
}
