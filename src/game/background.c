#include "game/background.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "p8/fixed.h"
#include "game/gen.h"
#include "engine/entity.h"
#include "game/probe.h"
#include "game/scroll.h"
#include "engine/vec.h"
#include "game/wormhole.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

static int g_bg_up = 0;

static int pico8_bg_to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

static void pico8_bg_kj_dispatch(entity_t *self, const vec_t *ra) {
    (void)ra;
    pico8_bg_kj(dispatch_context_gfx(), dispatch_context_mem(), (const pico8_bg_t *)self);
}

static down_t pico8_bg_ldown_dispatch(entity_t *self, vec_t *qa, vec_t *ra) {
    (void)qa;
    (void)ra;
    return pico8_bg_ldown((pico8_bg_t *)self);
}

static down_t pico8_bg_rdown_dispatch(entity_t *self, vec_t *qa, vec_t *ra) {
    (void)qa;
    return pico8_bg_rdown((pico8_bg_t *)self, ra);
}

static void pico8_bg_hq_dispatch(entity_t *self, vec_t *qa, vec_t *ra) {
    (void)qa;
    pico8_bg_hq((pico8_bg_t *)self, ra);
}

static const entity_class_t g_bg_struct_class = {
    .draw = pico8_bg_kj_dispatch,
    .hit_test = entity_registry_hit_test_always,
    .left_press = pico8_bg_ldown_dispatch,
    .right_press = pico8_bg_rdown_dispatch,
    .drag = pico8_bg_hq_dispatch,
};

/* A nil `rp` reads as the zero vector, as an unset `mq` does. */
static void pico8_bg_set_mq(pico8_bg_t *self, const vec_t *rp) {
    self->has_mq = rp != NULL;
    self->mq = rp != NULL ? *rp : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

static pico8_bg_t *pico8_bg_struct_new(void) {
    pico8_bg_t *self = (pico8_bg_t *)allocator_alloc(game_zone(), sizeof *self);

    entity_init_instance(&self->header, &g_bg_struct_class, background_archetype.state, background_archetype.state_time);
    self->def = &background_archetype;
    self->header.layer = &self->def->layer;
    pico8_bg_set_mq(self, NULL);
    self->header.is_hovered = &self->m_;
    self->m_ = 0;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("background");
#endif
    pico8_bg_pq(dispatch_context_mem(), self);
    entity_construct(&self->header);
    return self;
}

void pico8_bg_init(void) {
    pico8_bg_shutdown();
    g_bg_up = 1;
}

void pico8_bg_shutdown(void) {
    g_bg_up = 0;
}

int pico8_bg_ready(void) {
    return g_bg_up;
}

const entity_class_t *pico8_bg_struct_class(void) {
    return &g_bg_struct_class;
}

pico8_bg_t *pico8_bg_new(void) {
    return pico8_bg_struct_new();
}

/* `vs[x] or 1` is only ever taken on row y=0; every later row reads back the
 * previous row's own `vh`. `oh` is local to the call. */
void pico8_bg_pq(p8_memory_t *mem, pico8_bg_t *self) {
    p8_fixed_t vs[32];
    int vs_set[32];
    p8_fixed_t vh = p8_fixed_from_int(0);
    random_walk_t *oh;
    const int_seq_t *pl = self != NULL && self->def->tile_choices != NULL ? self->def->tile_choices : &background_tile_choices_seq;
    int x, y;

    if (mem == NULL) {
        return;
    }

    for (x = 0; x < 32; x++) {
        vs_set[x] = 0;
    }

    oh = random_walk_new(frame_zone(), P8_FIXED_LIT(0x00001999, 0.1), p8_fixed_from_int(0));

    for (y = 0; y <= 31; y++) {
        for (x = 0; x <= 31; x++) {
            p8_fixed_t prev = vs_set[x] ? vs[x] : p8_fixed_from_int(1);
            p8_fixed_t step =
                p8_fixed_sub(p8_fixed_mul(random_walk_next(oh), p8_fixed_from_int(8)), p8_fixed_from_int(4));
            p8_fixed_t roll;
            int tile;

            vh = p8_fixed_add(step, p8_fixed_mul(p8_fixed_add(vh, prev), P8_FIXED_LIT(0x00008000, 0.5)));
            vs[x] = vh;
            vs_set[x] = 1;

            roll = p8_fixed_rnd_default();
            if (roll < P8_FIXED_LIT(0x00000106, 0.004)) {
                tile = 122 + pico8_bg_to_int(p8_fixed_rnd(p8_fixed_from_int(5)));
            } else {
                p8_fixed_t idx_f = p8_fixed_flr(p8_fixed_add(vh, p8_fixed_rnd(p8_fixed_from_int(4))));
                int idx = pico8_bg_to_int(idx_f);

                tile = 0;
                int_seq_at(pl->items, pl->count, idx, &tile);
            }

            p8_mset(mem, x, y, tile);
        }
    }
}

void pico8_bg_kj(p8_graphics_t *gfx, p8_memory_t *mem, const pico8_bg_t *self) {
    int xy;

    (void)self;

    if (gfx == NULL || mem == NULL) {
        return;
    }

    p8_map(gfx, mem, 0, 0, 0, 0, 32, 32);
    p8_fillp(gfx, 0xA5A5u);
    for (xy = 3; xy <= 262; xy += 32) {
        p8_rectfill(gfx, mem, xy, 0, xy, 262, 1);
        p8_rectfill(gfx, mem, 0, xy, 262, xy, 1);
    }
    p8_fillp_reset(gfx);
    p8_rect(gfx, mem, 0, 0, 262, 262, 5);
}

down_t pico8_bg_ldown(pico8_bg_t *self) {
    const entity_list_t *g_tag;
    int count;
    entity_t *made;
    down_t out;

    (void)self;

    g_tag = entities_by_tag("probe");
    count = (g_tag != NULL) ? g_tag->count : 0;
    if (count > 0) {
        assert(pico8_mz_ready() && "pico8_wormhole_init() must run before bg:ldown builds an mz");
        made = (entity_t *)pico8_mz_new();
    } else {
        made = (entity_t *)pico8_g__new(NULL);
    }
    out.stop = made != NULL;
    out.held = made;
    return out;
}

down_t pico8_bg_rdown(pico8_bg_t *self, vec_t *rp) {
    down_t out = {0, NULL};

    if (self != NULL) {
        pico8_bg_set_mq(self, rp);
        out.stop = 1;
        out.held = &self->header;
    }
    return out;
}

/* `self.mq` is read here and overwritten by the last statement of the
 * function: that read-then-replace is the drag delta itself. */
void pico8_bg_hq(pico8_bg_t *self, vec_t *rp) {
    vec_t zero = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    vec_t mq;
    vec_t rp_vec;
    pico8_qb_t *mo;

    if (self == NULL) {
        return;
    }

    mq = self->has_mq ? self->mq : zero;
    rp_vec = rp != NULL ? *rp : zero;
    if (main_camera() != NULL) {
        mo = pico8_mo_qb();
        assert(mo != NULL && "mo: the placeholder has no writer; a write before jl() stops the run");
        pico8_qb_set_v(mo, vec_sub(mq, rp_vec));
    }

    pico8_bg_set_mq(self, rp);
}
