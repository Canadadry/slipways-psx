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
static const pico8_int_seq_t g_mx_ht_spent = {0, NULL};

static int g_family_trade_up = 0;

down_t pico8_nr_gv(const pico8_jo_t *self, const entity_t *s) {
    int qu_not_self;
    int oe_count = 0;
    down_t out = {0, NULL};

    if (self == NULL) {
        return out;
    }

    /* Only a held `jc` carries `qu`: a held `bg`, `mz` or planet reads it absent. */
    qu_not_self = !(s != NULL && s->class == pico8_jc_struct_class() &&
                    ((const pico8_jc_t *)s)->qu == &self->header);

    oe_count = self->oe != NULL ? self->oe->count : 0;

    out.stop = qu_not_self && oe_count < 3;
    return out;
}

static down_t pico8_nr_gv_dispatch(entity_t *self, const entity_t *s) {
    return pico8_nr_gv((const pico8_jo_t *)self, s);
}

static const entity_class_t g_nr_ldown_empty_struct_class = {0};

typedef struct {
    entity_t header;
} pico8_nr_ldown_empty_t;

/* The `{}` `nr:ldown` hands to `nr:gv`: never allocated, never freed,
 * never written. */
static const pico8_nr_ldown_empty_t g_nr_ldown_empty = {{.class = &g_nr_ldown_empty_struct_class}};

down_t pico8_nr_ldown(pico8_jo_t *self) {
    down_t gv_result = {0, NULL};
    pico8_jc_keys_t keys = {0};

    if (self == NULL) {
        return gv_result;
    }

    gv_result = pico8_nr_gv(self, (const entity_t *)&g_nr_ldown_empty);

    if (!gv_result.stop) {
        return gv_result;
    }

    assert(pico8_jc_ready() && "pico8_jc_init() must run before nr:ldown builds a jc");
    keys.qu = (entity_t *)self;
    gv_result.held = (entity_t *)pico8_jc_new(&keys);
    gv_result.stop = gv_result.held != NULL;
    return gv_result;
}

static down_t pico8_nr_ldown_dispatch(entity_t *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    (void)qa;
    (void)ra;
    return pico8_nr_ldown((pico8_jo_t *)self);
}

void pico8_nr_hb(pico8_jo_t *self) {
    int oe_count = 0;
    pico8_fixed_t s_val = pico8_fixed_from_int(0);

    if (self == NULL) {
        return;
    }

    oe_count = self->oe != NULL ? self->oe->count : 0;

    if (self->def->hz != NULL) {
        int entry;
        if (pico8_int_seq_at(self->def->hz->items, self->def->hz->count, oe_count + 1, &entry)) {
            s_val = pico8_fixed_from_int(entry);
        }
    }

    self->s = s_val;
}

static void pico8_nr_hb_dispatch(entity_t *self) {
    pico8_nr_hb((pico8_jo_t *)self);
}

void pico8_nr_cn(pico8_jo_t *self, pico8_jo_trade_proposal_t *lw) {
    pico8_trade_t *t;
    const pico8_ents_t *oe = NULL;
    pico8_ents_iter_t it;
    entity_t *w;

    if (self == NULL || lw == NULL) {
        return;
    }
    t = lw->t;

    oe = self->oe;

    it = pico8_ents_all(oe);
    while (pico8_ents_iter_next(&it, &w)) {
        const pico8_ents_t *t_rl = NULL;

        if (w == NULL) {
            continue;
        }

        if (lw->mu <= pico8_fixed_from_int(0)) {
            continue;
        }

        if (t != NULL && t->rl != NULL) {
            t_rl = t->rl;
        }
        if (pico8_ents_has(t_rl, w)) {
            continue;
        }

        {
            pico8_jo_trade_proposal_t gf = *lw;
            pico8_trade_t *extended = pico8_trade_kg(pico8_zone_frame(), t, &self->header, w);
            entity_t *other;

            gf.t = extended;
            assert(w->class == pico8_jc_struct_class() && "nr.oe holds slipways only: jo:kf adds to it");
            other = pico8_jc_eb((const pico8_jc_t *)w, (const entity_t *)self);
            /* `&gf` is a stack address passed to a dispatched `cn`: safe
             * only while no `cn` implementation stores its proposal. */
            if (other != NULL && other->class->offer_trade != NULL) {
                other->class->offer_trade(other, &gf);
            }
            lw->mu = gf.mu;
        }
    }
}

static void pico8_nr_cn_dispatch(entity_t *self, struct pico8_jo_trade_proposal *lw) {
    pico8_nr_cn((pico8_jo_t *)self, lw);
}

static const entity_class_t g_nr_struct_class = {
    .draw = pico8_jo_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = pico8_jo_hd_dispatch,
    .new_year = pico8_jo_hk_dispatch,
    .refresh = pico8_nr_hb_dispatch,
    .release = pico8_jo_rendre_dispatch,
    .left_press = pico8_nr_ldown_dispatch,
    .accept_drop = pico8_nr_gv_dispatch,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = pico8_jo_kf_dispatch,
    .offer_trade = pico8_nr_cn_dispatch,
    .states =
        {
            [ENTITY_STATE_VANISHING] = pico8_jo_ll_dispatch,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_SLIPGATE) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

/* Initializes an allocated pico8_jo_t under `cls` and `def`, its kind's
 * archetype: qh's part, nr having no member of its own. Reads no key, runs
 * neither `pq` nor `lt`, records no coverage. Requires
 * `pico8_planet_family_trade_init()`. */
static void pico8_nr_struct_init(pico8_jo_t *self, const entity_class_t *cls, const pico8_jo_def_t *def) {
    pico8_qh_struct_init(self, cls, def);
}

static pico8_jo_t *pico8_nr_struct_new(const planet_keys_t *keys) {
    pico8_jo_t *self = (pico8_jo_t *)pico8_zone_alloc(pico8_jo_zone_construction(), sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_nr_struct_init(self, &g_nr_struct_class, &pico8_nr_def);
    if (keys != NULL) {
        if (keys->planet_id != NULL) {
            self->id = keys->planet_id;
        }
        if (keys->map_pos != NULL) {
            self->p = keys->map_pos;
        }
        if (keys->pos != NULL) {
            self->ra = keys->pos;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("nr");
#endif
    pico8_jo_pq(self);
    pico8_ke_struct_pq(&self->header);
    return self;
}

static const entity_class_t g_fr_struct_class = {
    .draw = pico8_jo_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = pico8_jo_hd_dispatch,
    .new_year = pico8_jo_hk_dispatch,
    .refresh = pico8_nr_hb_dispatch,
    .release = pico8_jo_rendre_dispatch,
    .left_press = pico8_nr_ldown_dispatch,
    .accept_drop = pico8_nr_gv_dispatch,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = pico8_jo_kf_dispatch,
    .offer_trade = pico8_nr_cn_dispatch,
    .states =
        {
            [ENTITY_STATE_VANISHING] = pico8_jo_ll_dispatch,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_SLIPGATE) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

static pico8_jo_t *pico8_fr_struct_new(const planet_keys_t *keys) {
    pico8_jo_t *self = (pico8_jo_t *)pico8_zone_alloc(pico8_jo_zone_construction(), sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_nr_struct_init(self, &g_fr_struct_class, &pico8_fr_def);
    self->header.infraspace = &self->def->mt;
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "fr: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("fr");
#endif
    pico8_jo_pq(self);
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_mx_kf(pico8_jo_t *self, entity_t *sw) {
    entity_t *cp = NULL;

    if (self != NULL && sw != NULL) {
        assert(sw->class == pico8_jc_struct_class() && "mx:kf: sw is the slipway jc:ey registers");
        cp = pico8_jc_eb((const pico8_jc_t *)sw, (const entity_t *)self);
    }

    if (cp != NULL) {
        const pico8_ptrs_t *jy;
        pico8_ptrs_iter_t it;
        void *i_val;

        assert(pico8_jo_is_family(cp) && "mx:kf: the other end of a registered slipway is a planet");
        jy = ((const pico8_jo_t *)cp)->jy;

        it = pico8_ptrs_all(jy);
        while (pico8_ptrs_iter_next(&it, &i_val)) {
            const pico8_jy_t *i;
            pico8_fixed_t gc;

            if (i_val == NULL) {
                continue;
            }
            i = i_val;

            gc = i->has_gc ? i->gc : pico8_fixed_from_int(0);
            if (gc != pico8_fixed_from_int(0)) {
                continue;
            }

            if (i->row.has_qe && self->ht != NULL && pico8_int_seq_has_fixed(self->ht, i->row.qe)) {
                pico8_kw_t *kw = self->kw;
                if (kw != NULL) {
                    kw->has_qe = 1;
                    kw->qe = i->row.qe;
                }
                self->ht = &g_mx_ht_spent;
            }
        }
    }

    pico8_jo_kf(self, sw);
}

static void pico8_mx_kf_dispatch(entity_t *self, entity_t *sw) {
    pico8_mx_kf((pico8_jo_t *)self, sw);
}

static const entity_class_t g_mx_struct_class = {
    .draw = pico8_hj_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = pico8_jo_hd_dispatch,
    .new_year = pico8_jo_hk_dispatch,
    .refresh = pico8_jo_hb_dispatch,
    .release = pico8_hj_rendre_dispatch,
    .left_press = pico8_jo_ldown_dispatch,
    .accept_drop = pico8_jo_gv_dispatch,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = pico8_mx_kf_dispatch,
    .offer_trade = pico8_jo_cn_dispatch,
    .spawn_particle = pico8_g__mm,
    .states =
        {
            [ENTITY_STATE_VANISHING] = pico8_jo_ll_dispatch,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

static pico8_jo_t *pico8_mx_struct_new(const planet_keys_t *keys) {
    pico8_jo_t *self = (pico8_jo_t *)pico8_zone_alloc(pico8_jo_zone_construction(), sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_hj_struct_init(self, &g_mx_struct_class, &pico8_mx_def);
    self->ht = self->def->ht;
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "mx: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("mx");
#endif
    pico8_f_((entity_t *)self);
    pico8_jo_pq(self);
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_planet_family_trade_init(void) {
    assert(pico8_qh_ready() && pico8_hj_ready() &&
           "pico8_planet_family_init() must run first");
    pico8_planet_family_trade_shutdown();
    pico8_pi_init();
    g_family_trade_up = 1;
}

void pico8_planet_family_trade_shutdown(void) {
    g_family_trade_up = 0;
}

int pico8_nr_ready(void) {
    return g_family_trade_up;
}

static entity_t *pico8_nr_kind_new(const planet_keys_t *keys) {
    return &pico8_nr_struct_new(keys)->header;
}

static const entity_kind_t g_nr_kind = {pico8_nr_kind_new, &pico8_nr_kind_rec};

const entity_kind_t *pico8_nr_kind(void) {
    return &g_nr_kind;
}

const entity_class_t *pico8_nr_struct_class(void) {
    return &g_nr_struct_class;
}

pico8_jo_t *pico8_nr_new(const planet_keys_t *keys) {
    return pico8_nr_struct_new(keys);
}

int pico8_fr_ready(void) {
    return g_family_trade_up;
}

static entity_t *pico8_fr_kind_new(const planet_keys_t *keys) {
    return &pico8_fr_struct_new(keys)->header;
}

static const entity_kind_t g_fr_kind = {pico8_fr_kind_new, &pico8_fr_kind_rec};

const entity_kind_t *pico8_fr_kind(void) {
    return &g_fr_kind;
}

const entity_class_t *pico8_fr_struct_class(void) {
    return &g_fr_struct_class;
}

pico8_jo_t *pico8_fr_new(const planet_keys_t *keys) {
    return pico8_fr_struct_new(keys);
}

int pico8_mx_ready(void) {
    return g_family_trade_up;
}

static entity_t *pico8_mx_kind_new(const planet_keys_t *keys) {
    return &pico8_mx_struct_new(keys)->header;
}

static const entity_kind_t g_mx_kind = {pico8_mx_kind_new, &pico8_mx_kind_rec};

const entity_kind_t *pico8_mx_kind(void) {
    return &g_mx_kind;
}

const entity_class_t *pico8_mx_struct_class(void) {
    return &g_mx_struct_class;
}

pico8_jo_t *pico8_mx_new(const planet_keys_t *keys) {
    return pico8_mx_struct_new(keys);
}
