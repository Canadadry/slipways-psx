#include "game/planet_family_trade.h"

#include <assert.h>
#include <string.h>

#include "engine/entity_class.h"
#include "game/particle.h"
#include "game/draw_functions.h"
#include "game/planet.h"
#include "game/planet_family.h"
#include "game/planet_trade.h"
#include "game/probe.h"
#include "game/slipway.h"
#include "game/trade.h"
#include "engine/frame_zone.h"

/* What `mx:kf` leaves in `ht` once it accepts a resource: `o()`. */
static const int_seq_t g_synthesizer_synthesizable_resources_spent = {0, NULL};

static int g_family_trade_up = 0;

down_t slipgate_accept_drop(const planet_t *self, const entity_t *s) {
    int start_not_self;
    int slipways_count = 0;
    down_t out = {0, NULL};

    if (self == NULL) {
        return out;
    }

    /* Only a held `jc` carries `qu`: a held `bg`, `mz` or planet reads it absent. */
    start_not_self = !(s != NULL && s->class == slipway_class() &&
                    ((const slipway_t *)s)->start == &self->header);

    slipways_count = self->slipways != NULL ? self->slipways->count : 0;

    out.stop = start_not_self && slipways_count < 3;
    return out;
}

static down_t slipgate_accept_drop_method(entity_t *self, const entity_t *s) {
    return slipgate_accept_drop((const planet_t *)self, s);
}

static const entity_class_t g_slipgate_left_press_empty_class = {0};

typedef struct {
    entity_t header;
} slipgate_left_press_empty_t;

/* The `{}` `nr:ldown` hands to `nr:gv`: never allocated, never freed,
 * never written. */
static const slipgate_left_press_empty_t g_slipgate_left_press_empty = {{.class = &g_slipgate_left_press_empty_class}};

down_t slipgate_left_press(planet_t *self) {
    down_t accept_drop_result = {0, NULL};
    slipway_keys_t keys = {0};

    if (self == NULL) {
        return accept_drop_result;
    }

    accept_drop_result = slipgate_accept_drop(self, (const entity_t *)&g_slipgate_left_press_empty);

    if (!accept_drop_result.stop) {
        return accept_drop_result;
    }

    assert(slipway_ready() && "pico8_jc_init() must run before nr:ldown builds a jc");
    keys.start = (entity_t *)self;
    accept_drop_result.held = (entity_t *)slipway_new(&keys);
    accept_drop_result.stop = accept_drop_result.held != NULL;
    return accept_drop_result;
}

static down_t slipgate_left_press_method(entity_t *self, vec_t *qa, vec_t *ra) {
    (void)qa;
    (void)ra;
    return slipgate_left_press((planet_t *)self);
}

void slipgate_refresh(planet_t *self) {
    int slipways_count = 0;
    p8_fixed_t s_val = p8_fixed_from_int(0);

    if (self == NULL) {
        return;
    }

    slipways_count = self->slipways != NULL ? self->slipways->count : 0;

    if (self->archetype->link_sprites != NULL) {
        int entry;
        if (int_seq_at(self->archetype->link_sprites->items, self->archetype->link_sprites->count, slipways_count + 1, &entry)) {
            s_val = p8_fixed_from_int(entry);
        }
    }

    self->sprite = s_val;
}

static void slipgate_refresh_method(entity_t *self) {
    slipgate_refresh((planet_t *)self);
}

void slipgate_offer_trade(planet_t *self, trade_offer_t *trade_offer) {
    trade_route_t *route;
    const entity_list_t *slipways = NULL;
    entity_list_iter_t iter;
    entity_t *slipway;

    if (self == NULL || trade_offer == NULL) {
        return;
    }
    route = trade_offer->route;

    slipways = self->slipways;

    iter = entity_list_all(slipways);
    while (entity_list_iter_next(&iter, &slipway)) {
        const entity_list_t *t_slipways = NULL;

        if (slipway == NULL) {
            continue;
        }

        if (trade_offer->amount <= p8_fixed_from_int(0)) {
            continue;
        }

        if (route != NULL && route->slipways != NULL) {
            t_slipways = route->slipways;
        }
        if (entity_list_has(t_slipways, slipway)) {
            continue;
        }

        {
            trade_offer_t gf = *trade_offer;
            trade_route_t *extended = trade_extend(frame_zone(), route, &self->header, slipway);
            entity_t *other;

            gf.route = extended;
            assert(slipway->class == slipway_class() && "nr.oe holds slipways only: jo:kf adds to it");
            other = slipway_other_end((const slipway_t *)slipway, (const entity_t *)self);
            /* `&gf` is a stack address passed to a dispatched `cn`: safe
             * only while no `cn` implementation stores its proposal. */
            if (other != NULL && other->class->offer_trade != NULL) {
                other->class->offer_trade(other, &gf);
            }
            trade_offer->amount = gf.amount;
        }
    }
}

static void slipgate_offer_trade_method(entity_t *self, struct trade_offer *lw) {
    slipgate_offer_trade((planet_t *)self, lw);
}

static const entity_class_t g_slipgate_class = {
    .draw = planet_draw_method,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = slipgate_refresh_method,
    .release = planet_release_method,
    .left_press = slipgate_left_press_method,
    .accept_drop = slipgate_accept_drop_method,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = slipgate_offer_trade_method,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_SLIPGATE) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

/* Initializes an allocated pico8_jo_t under `cls` and `def`, its kind's
 * archetype: qh's part, nr having no member of its own. Reads no key, runs
 * neither `pq` nor `lt`, records no coverage. Requires
 * `pico8_planet_family_trade_init()`. */
static void slipgate_init_instance(planet_t *self, const entity_class_t *cls, const planet_archetype_t *archetype) {
    pico8_qh_struct_init(self, cls, archetype);
}

static planet_t *slipgate_create(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    slipgate_init_instance(self, &g_slipgate_class, &slipgate_archetype);
    if (keys != NULL) {
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
        if (keys->map_pos != NULL) {
            self->map_pos = keys->map_pos;
        }
        if (keys->pos != NULL) {
            self->pos = keys->pos;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("slipgate");
#endif
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

static const entity_class_t g_infraspace_gate_class = {
    .draw = planet_draw_method,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = slipgate_refresh_method,
    .release = planet_release_method,
    .left_press = slipgate_left_press_method,
    .accept_drop = slipgate_accept_drop_method,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = slipgate_offer_trade_method,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_SLIPGATE) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

static planet_t *infraspace_gate_create(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    slipgate_init_instance(self, &g_infraspace_gate_class, &infraspace_gate_archetype);
    self->header.infraspace = &self->archetype->infraspace;
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "fr: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("infraspace_gate");
#endif
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

void synthesizer_attach_slipway(planet_t *self, entity_t *slipway) {
    entity_t *other_end = NULL;

    if (self != NULL && slipway != NULL) {
        assert(slipway->class == slipway_class() && "mx:kf: sw is the slipway jc:ey registers");
        other_end = slipway_other_end((const slipway_t *)slipway, (const entity_t *)self);
    }

    if (other_end != NULL) {
        const pointer_list_t *jy;
        pointer_list_iter_t it;
        void *i_val;

        assert(planet_is_family(other_end) && "mx:kf: the other end of a registered slipway is a planet");
        jy = ((const planet_t *)other_end)->needs;

        it = pointer_list_all(jy);
        while (pointer_list_iter_next(&it, &i_val)) {
            const pico8_jy_t *i;
            p8_fixed_t gc;

            if (i_val == NULL) {
                continue;
            }
            i = i_val;

            gc = i->has_gc ? i->gc : p8_fixed_from_int(0);
            if (gc != p8_fixed_from_int(0)) {
                continue;
            }

            if (i->row.has_resource && self->synthesizable_resources != NULL && int_seq_has_fixed(self->synthesizable_resources, i->row.resource)) {
                production_t *kw = self->production;
                if (kw != NULL) {
                    kw->has_resource = 1;
                    kw->resource = i->row.resource;
                }
                self->synthesizable_resources = &g_synthesizer_synthesizable_resources_spent;
            }
        }
    }

    planet_attach_slipway(self, slipway);
}

static void synthesizer_attach_slipway_method(entity_t *self, entity_t *sw) {
    synthesizer_attach_slipway((planet_t *)self, sw);
}

static const entity_class_t g_synthesizer_class = {
    .draw = pico8_hj_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = pico8_hj_rendre_dispatch,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = synthesizer_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .spawn_particle = pico8_g__mm,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

static planet_t *synthesizer_create(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_hj_struct_init(self, &g_synthesizer_class, &synthesizer_archetype);
    self->synthesizable_resources = self->archetype->synthesizable_resources;
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "mx: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("synthesizer");
#endif
    pico8_f_((entity_t *)self);
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

void planet_family_trade_init(void) {
    assert(pico8_qh_ready() && pico8_hj_ready() &&
           "pico8_planet_family_init() must run first");
    planet_family_trade_shutdown();
    draw_functions_init();
    g_family_trade_up = 1;
}

void planet_family_trade_shutdown(void) {
    g_family_trade_up = 0;
}

int slipgate_ready(void) {
    return g_family_trade_up;
}

static entity_t *slipgate_kind_new(const planet_keys_t *keys) {
    return &slipgate_create(keys)->header;
}

static const entity_kind_t g_slipgate_kind = {slipgate_kind_new, &slipgate_kind_record};

const entity_kind_t *slipgate_kind(void) {
    return &g_slipgate_kind;
}

const entity_class_t *slipgate_class(void) {
    return &g_slipgate_class;
}

planet_t *slipgate_new(const planet_keys_t *keys) {
    return slipgate_create(keys);
}

int infraspace_gate_ready(void) {
    return g_family_trade_up;
}

static entity_t *infraspace_gate_kind_new(const planet_keys_t *keys) {
    return &infraspace_gate_create(keys)->header;
}

static const entity_kind_t g_infraspace_gate_kind = {infraspace_gate_kind_new, &infraspace_gate_kind_record};

const entity_kind_t *infraspace_gate_kind(void) {
    return &g_infraspace_gate_kind;
}

const entity_class_t *infraspace_gate_class(void) {
    return &g_infraspace_gate_class;
}

planet_t *infraspace_gate_new(const planet_keys_t *keys) {
    return infraspace_gate_create(keys);
}

int synthesizer_ready(void) {
    return g_family_trade_up;
}

static entity_t *synthesizer_kind_new(const planet_keys_t *keys) {
    return &synthesizer_create(keys)->header;
}

static const entity_kind_t g_synthesizer_kind = {synthesizer_kind_new, &synthesizer_kind_record};

const entity_kind_t *synthesizer_kind(void) {
    return &g_synthesizer_kind;
}

const entity_class_t *synthesizer_class(void) {
    return &g_synthesizer_class;
}

planet_t *synthesizer_new(const planet_keys_t *keys) {
    return synthesizer_create(keys);
}
