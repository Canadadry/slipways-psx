#include "game/planet.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/collect.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "game/gen.h"
#include "engine/entity.h"
#include "game/message_box.h"
#include "game/draw_functions.h"
#include "game/research.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

static int g_jo_up = 0;

static weighted_picker_t *g_jo_oc = NULL;
static random_walk_t *g_jo_ir = NULL;

/* Shared by every bare planet: written in place by `bb` and `mx:kf` until
 * `lz` gives the planet its own. */
static production_t g_jo_kw_shared;

/* Nom des arenes de prise : son adresse les reconnait (pico8_jo_prise_arene_rendue). */
static const char g_jo_nom_prise[] = "prise";
#define PRISES_LIBRES_MAX 8

/* Non NULL seulement entre pico8_jo_prise_ouvrir et _fermer. */
static const allocator_t *g_jo_prise = NULL;
static const allocator_t *g_jo_prises_libres[PRISES_LIBRES_MAX];
static int g_jo_prises_libres_n = 0;

int pico8_jo_hd_dispatch(entity_t *self, const vec_t *qa, const vec_t *ra) {
    return entity_geom_contains_cursor(self, qa, ra);
}

void pico8_jo_hk_dispatch(entity_t *self, p8_fixed_t yr) {
    (void)yr;
    pico8_jo_hk((const pico8_jo_t *)self);
}

int pico8_jo_is_family(const entity_t *e) {
    return (e->class->lineage & LINEAGE(LINEAGE_PLANET)) != 0;
}

void pico8_jo_ll_dispatch(entity_t *self, p8_fixed_t t) {
    pico8_jo_ll((pico8_jo_t *)self, t);
}

static const entity_class_t g_jo_struct_class = {
    .draw = pico8_jo_kj_dispatch,
    .hit_test = pico8_jo_hd_dispatch,
    .new_year = pico8_jo_hk_dispatch,
    .refresh = pico8_jo_hb_dispatch,
    .release = pico8_jo_rendre_dispatch,
    .left_press = pico8_jo_ldown_dispatch,
    .accept_drop = pico8_jo_gv_dispatch,
    .attach_slipway = pico8_jo_kf_dispatch,
    .offer_trade = pico8_jo_cn_dispatch,
    .states = {[ENTITY_STATE_VANISHING] = pico8_jo_ll_dispatch},
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET),
};

void pico8_jo_struct_init(pico8_jo_t *self, const entity_class_t *cls, const planet_archetype_t *def) {
    assert(g_jo_up && "pico8_jo_init() must run first");
    entity_init_instance(&self->header, cls, def->state, def->state_time);
    self->def = def;
    self->header.layer = &self->def->layer;
    /* Read-only: the planet modules write through their members. */
    self->header.pos = (const vec_t *const *)&self->ra;
    self->header.map_pos = (const vec_t *const *)&self->p;
    self->header.hit_box = &self->def->hit_box;
    self->header.shake = &self->ma;
    self->header.fold_anchor = &self->fx;
    self->header.is_dead = &self->p_;
    self->ma = self->def->shake;
    self->kw = &g_jo_kw_shared;
    self->id = NULL;
    self->p = NULL;
    self->ra = NULL;
    self->oe = NULL;
    self->hn = NULL;
    self->ep = NULL;
    self->jy = NULL;
    self->nk = NULL;
    self->fx = 0;
    self->lq = NULL;
    self->lq_place = NULL;
    self->fo = NULL;
    self->jh = 0;
    self->has_jh = 0;
    self->p_ = 0;
    self->kr = 0;
    self->has_kr = 0;
    self->cc = NULL;
    self->by = NULL;
    self->mi = self->def->development_tier;
    self->has_mi = self->def->has_development_tier;
    self->header.is_hovered = &self->m_;
    self->m_ = 0;
    self->kw_own = (production_t){0};
    self->probe = NULL;
    self->emitter = NULL;
}

static pico8_jo_t *pico8_jo_struct_new(const planet_keys_t *keys) {
    pico8_jo_t *self = (pico8_jo_t *)allocator_alloc(pico8_jo_zone_construction(), sizeof *self);

    pico8_jo_struct_init(self, &g_jo_struct_class, &planet_archetype);
    if (keys != NULL) {
        if (keys->map_pos != NULL) {
            self->p = keys->map_pos;
        }
        if (keys->pos != NULL) {
            self->ra = keys->pos;
        }
        if (keys->planet_id != NULL) {
            self->id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("planet");
#endif
    pico8_jo_pq(self);
    entity_construct(&self->header);
    return self;
}

void pico8_jo_init(void) {
    pico8_jo_shutdown();
    draw_functions_init();
    g_jo_up = 1;
    g_jo_kw_shared = (production_t){0};
}

void pico8_jo_shutdown(void) {
    g_jo_up = 0;
    g_jo_oc = NULL;
    g_jo_ir = NULL;
}

void pico8_jo_set_oc(weighted_picker_t *oc) {
    g_jo_oc = oc;
}

void pico8_jo_set_ir(random_walk_t *ir) {
    g_jo_ir = ir;
}

int pico8_jo_ready(void) {
    return g_jo_up;
}

const data_row_t *pico8_jo_dz(void) {
    return planet_mood_rows_rows;
}

const int_seq_t *pico8_jo_cq(void) {
    return &planet_made_resources_seq;
}

const int_seq_t *pico8_jo_ka(void) {
    return &planet_ring_colors_seq;
}

const data_row_t *pico8_jo_em(void) {
    return &planet_spacer_row;
}

production_t *pico8_jo_shared_kw(void) {
    return &g_jo_kw_shared;
}

const entity_class_t *pico8_jo_struct_class(void) {
    return &g_jo_struct_class;
}

pico8_jo_t *pico8_jo_new(const planet_keys_t *keys) {
    return pico8_jo_struct_new(keys);
}

const allocator_t *pico8_jo_zone_construction(void) {
    return g_jo_prise != NULL ? g_jo_prise : game_zone();
}

void pico8_jo_prise_ouvrir(void) {
    assert(g_jo_prise == NULL && "une prise d'icone est deja ouverte");
    if (g_jo_prises_libres_n > 0) {
        g_jo_prise = g_jo_prises_libres[--g_jo_prises_libres_n];
        arena_flush(g_jo_prise);
    } else {
        g_jo_prise = arena_take(game_zone(), g_jo_nom_prise, PICO8_JO_PRISE_OCTETS);
    }
}

void pico8_jo_prise_fermer(void) {
    g_jo_prise = NULL;
}

void pico8_jo_prise_rendue(const pico8_jo_t *self) {
    pico8_jo_prise_zone_rendue(self->oe != NULL ? self->oe->zone : NULL, self->header.state);
}

void pico8_jo_prise_zone_rendue(const allocator_t *zone, const char *me) {
    if (me == NULL || strcmp(me, "placing") != 0) {
        return;
    }
    pico8_jo_prise_arene_rendue(zone);
}

void pico8_jo_prise_arene_rendue(const allocator_t *zone) {
    if (zone == NULL || zone->bump == NULL || zone->bump->name != g_jo_nom_prise) {
        return;
    }
    assert(g_jo_prises_libres_n < PRISES_LIBRES_MAX &&
           "prises libres pleines : plus de prises refusees en vol que prevu");
    g_jo_prises_libres[g_jo_prises_libres_n++] = zone;
}

/* TROU-26 */
void pico8_jo_oublier_fo(const entity_t *e) {
    const entity_list_t *np = entities_by_tag("node");

    for (int i = 0; np != NULL && i < np->count; i++) {
        pico8_jo_t *p = (pico8_jo_t *)np->items[i];

        assert(pico8_jo_is_family(&p->header) && "cg.np ne tient que des planetes, qui ont un fo");
        if (p->fo == e) {
            p->fo = NULL;
        }
    }
}

void pico8_jo_oublier_prises(void) {
    g_jo_prises_libres_n = 0;
}

void pico8_jo_pq(pico8_jo_t *self) {
    pico8_jo_pq_dans(self, pico8_jo_zone_construction());
}

void pico8_jo_pq_dans(pico8_jo_t *self, const allocator_t *zone) {
    entity_list_t *oe;

    if (self == NULL) {
        return;
    }

    oe = (entity_list_t *)allocator_alloc(zone, sizeof *oe);
    entity_list_init(oe, zone);
    self->oe = oe;

    if (self->header.class->lineage & LINEAGE(LINEAGE_STRUCTURE)) {
        pico8_hp_t *target = pico8_gb_at(p8_fixed_flr_int(self->def->project_index));

        if (target != NULL) {
            pico8_jo_lz(self, target);
        }
    }
}

/* `mw(i)`: a copy of the want, never shared with `hp.i`. */
static int pico8_jo_lz_mw_step(void *e, void *ctx, void **out) {
    if (e != NULL) {
        pico8_jy_t *copy = (pico8_jy_t *)allocator_alloc((const allocator_t *)ctx, sizeof *copy);
        const pico8_jy_t *i = e;

        *copy = *i;
        *out = copy;
        return 1;
    }
    return 0;
}

void pico8_jo_lz(pico8_jo_t *self, pico8_hp_t *hp_instance) {
    const allocator_t *zone = pico8_jo_zone_construction();
    pointer_list_t jy;
    pointer_list_t *jy_own;

    pico8_hp_assert(hp_instance);
    if (self == NULL) {
        return;
    }

    self->hn = hp_instance;
    self->ep = NULL;

    pointer_list_init(&jy, zone);
    if (hp_instance != NULL && hp_instance->i != NULL) {
        jy = collect_map_pointer_list(zone, hp_instance->i, pico8_jo_lz_mw_step, (void *)zone);
    }
    jy_own = (pointer_list_t *)allocator_alloc(zone, sizeof *jy_own);
    *jy_own = jy;
    self->jy = jy_own;

    self->kw_own.has_resource = hp_instance != NULL && hp_instance->has_o;
    self->kw_own.resource = self->kw_own.has_resource ? hp_instance->o : 0;
    self->kw_own.amount = p8_fixed_from_int(0);
    self->kw = &self->kw_own;
}

int pico8_jo_gg(const pico8_jo_t *self) {
    if (self == NULL) {
        return 0;
    }
    if (!self->def->is_planet) {
        return 0;
    }
    return self->header.state != NULL && strcmp(self->header.state, "settled") == 0;
}

void pico8_jo_kd(pico8_jo_t *self) {
    if (self == NULL) {
        return;
    }
    self->ma = P8_FIXED_LIT(0x00014000, 1.25);
}

void pico8_jo_ll(pico8_jo_t *self, p8_fixed_t t) {
    if (self == NULL) {
        return;
    }
    self->p_ = t > p8_fixed_from_int(60);
}

void pico8_jo_hh(pico8_jo_t *self) {
    const planet_types_entry_t *nk = NULL;
    const char *ks_arg;

    if (self == NULL) {
        return;
    }

    if (g_jo_ir != NULL) {
        p8_fixed_t roll = random_walk_next(g_jo_ir);
        int use_ll = 0;

        /* `jo.ir()<0.45 and df.ll or jo.oc()`: a falsy `df.ll` falls through
         * to `jo.oc()` even on a winning roll -- both conditions, never the
         * roll alone. */
        if (roll < P8_FIXED_LIT(0x00007333, 0.45)) {
            use_ll = planet_types_lookup("vanishing") != NULL;
        }
        if (use_ll) {
            nk = planet_types_lookup("vanishing");
        } else if (g_jo_oc != NULL) {
            nk = weighted_picker_next(g_jo_oc);
        }
    }
    self->nk = nk;
    self->ep = nk != NULL ? nk->description : NULL;

    if (nk != NULL && nk->state != NULL) {
        ks_arg = nk->state;
    } else {
        ks_arg = "known";
    }
    entity_set_state((entity_t *)self, ks_arg);
    pico8_jo_kd(self);
}

void pico8_jo_hk(const pico8_jo_t *self) {
    if (self == NULL) {
        return;
    }
    if (!self->has_kr || self->kr == p8_fixed_from_int(0)) {
        return;
    }
    if (!entity_geom_is_on_screen((const entity_t *)self)) {
        return;
    }
    {
        pico8_iv_keys_t keys = {0};
        int color = 0;
        char *text;

        keys.ra = self->ra;
        text = collect_format_signed(frame_zone(), self->kr, 10, "$", &color);
        /* `ne()` is the last expression of the Lua constructor, so both of its
         * return values land as positional entries 1 and 2, not named fields. */
        keys.at1 = text;
        keys.at2 = p8_fixed_from_int(color);
        pico8_iv_new(&keys);
    }
}

void pico8_na(entity_t *self, p8_fixed_t t) {
    vec_t *ra_vec;

    assert((self == NULL || pico8_jo_is_family(self)) && "na: self is a planet (qh:ly, nl:ly) -- stop");
    ra_vec = self != NULL ? ((pico8_jo_t *)self)->ra : NULL;
    if (ra_vec != NULL) {
        p8_fixed_t delta = p8_fixed_mul(p8_fixed_sin(p8_fixed_mul(t, P8_FIXED_LIT(0x00000147, 0.005))),
                                               P8_FIXED_LIT(0x00000f5c, 0.06));
        ra_vec->y = p8_fixed_add(ra_vec->y, delta);
    }
}
