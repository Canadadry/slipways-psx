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

static pico8_gn_state_t *g_jo_oc = NULL;
static pico8_jf_state_t *g_jo_ir = NULL;

/* Shared by every bare planet: written in place by `bb` and `mx:kf` until
 * `lz` gives the planet its own. */
static pico8_kw_t g_jo_kw_shared;

/* Nom des arenes de prise : son adresse les reconnait (pico8_jo_prise_arene_rendue). */
static const char g_jo_nom_prise[] = "prise";
#define PRISES_LIBRES_MAX 8

/* Non NULL seulement entre pico8_jo_prise_ouvrir et _fermer. */
static const pico8_zone_choisie_t *g_jo_prise = NULL;
static const pico8_zone_choisie_t *g_jo_prises_libres[PRISES_LIBRES_MAX];
static int g_jo_prises_libres_n = 0;

int pico8_jo_hd_dispatch(entity_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    return pico8_gk(self, qa, ra);
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

void pico8_jo_struct_init(pico8_jo_t *self, const entity_class_t *cls, const pico8_jo_def_t *def) {
    assert(g_jo_up && "pico8_jo_init() must run first");
    pico8_ke_struct_init(&self->header, cls, def->me, def->t);
    self->def = def;
    self->header.layer = &self->def->fa;
    /* Read-only: the planet modules write through their members. */
    self->header.pos = (const pico8_vec_t *const *)&self->ra;
    self->header.map_pos = (const pico8_vec_t *const *)&self->p;
    self->header.hit_box = &self->def->kq;
    self->header.shake = &self->ma;
    self->header.fold_anchor = &self->fx;
    self->header.is_dead = &self->p_;
    self->ma = self->def->ma;
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
    self->mi = self->def->mi;
    self->has_mi = self->def->has_mi;
    self->header.is_hovered = &self->m_;
    self->m_ = 0;
    self->kw_own = (pico8_kw_t){0};
    self->probe = NULL;
    self->emitter = NULL;
}

static pico8_jo_t *pico8_jo_struct_new(const planet_keys_t *keys) {
    pico8_jo_t *self = (pico8_jo_t *)pico8_zone_alloc(pico8_jo_zone_construction(), sizeof *self);

    pico8_jo_struct_init(self, &g_jo_struct_class, &pico8_jo_def);
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
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_jo_init(void) {
    pico8_jo_shutdown();
    pico8_pi_init();
    g_jo_up = 1;
    g_jo_kw_shared = (pico8_kw_t){0};
}

void pico8_jo_shutdown(void) {
    g_jo_up = 0;
    g_jo_oc = NULL;
    g_jo_ir = NULL;
}

void pico8_jo_set_oc(pico8_gn_state_t *oc) {
    g_jo_oc = oc;
}

void pico8_jo_set_ir(pico8_jf_state_t *ir) {
    g_jo_ir = ir;
}

int pico8_jo_ready(void) {
    return g_jo_up;
}

const pico8_jb_row_t *pico8_jo_dz(void) {
    return pico8_jo_dz_rows;
}

const pico8_int_seq_t *pico8_jo_cq(void) {
    return &pico8_jo_cq_seq;
}

const pico8_int_seq_t *pico8_jo_ka(void) {
    return &pico8_jo_ka_seq;
}

const pico8_jb_row_t *pico8_jo_em(void) {
    return &pico8_jo_em_row;
}

pico8_kw_t *pico8_jo_shared_kw(void) {
    return &g_jo_kw_shared;
}

const entity_class_t *pico8_jo_struct_class(void) {
    return &g_jo_struct_class;
}

pico8_jo_t *pico8_jo_new(const planet_keys_t *keys) {
    return pico8_jo_struct_new(keys);
}

const pico8_zone_choisie_t *pico8_jo_zone_construction(void) {
    return g_jo_prise != NULL ? g_jo_prise : pico8_zone_partie();
}

void pico8_jo_prise_ouvrir(void) {
    assert(g_jo_prise == NULL && "une prise d'icone est deja ouverte");
    if (g_jo_prises_libres_n > 0) {
        g_jo_prise = g_jo_prises_libres[--g_jo_prises_libres_n];
        pico8_zone_arene_vider(g_jo_prise);
    } else {
        g_jo_prise = pico8_zone_arene_prendre(pico8_zone_partie(), g_jo_nom_prise, PICO8_JO_PRISE_OCTETS);
    }
}

void pico8_jo_prise_fermer(void) {
    g_jo_prise = NULL;
}

void pico8_jo_prise_rendue(const pico8_jo_t *self) {
    pico8_jo_prise_zone_rendue(self->oe != NULL ? self->oe->zone : NULL, self->header.state);
}

void pico8_jo_prise_zone_rendue(const pico8_zone_choisie_t *zone, const char *me) {
    if (me == NULL || strcmp(me, "placing") != 0) {
        return;
    }
    pico8_jo_prise_arene_rendue(zone);
}

void pico8_jo_prise_arene_rendue(const pico8_zone_choisie_t *zone) {
    if (zone == NULL || zone->bump == NULL || zone->bump->name != g_jo_nom_prise) {
        return;
    }
    assert(g_jo_prises_libres_n < PRISES_LIBRES_MAX &&
           "prises libres pleines : plus de prises refusees en vol que prevu");
    g_jo_prises_libres[g_jo_prises_libres_n++] = zone;
}

/* TROU-26 */
void pico8_jo_oublier_fo(const entity_t *e) {
    const pico8_ents_t *np = pico8_cg("node");

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

void pico8_jo_pq_dans(pico8_jo_t *self, const pico8_zone_choisie_t *zone) {
    pico8_ents_t *oe;

    if (self == NULL) {
        return;
    }

    oe = (pico8_ents_t *)pico8_zone_alloc(zone, sizeof *oe);
    pico8_ents_init(oe, zone);
    self->oe = oe;

    if (self->header.class->lineage & LINEAGE(LINEAGE_STRUCTURE)) {
        pico8_hp_t *target = pico8_gb_at(p8_fixed_flr_int(self->def->hp));

        if (target != NULL) {
            pico8_jo_lz(self, target);
        }
    }
}

/* `mw(i)`: a copy of the want, never shared with `hp.i`. */
static int pico8_jo_lz_mw_step(void *e, void *ctx, void **out) {
    if (e != NULL) {
        pico8_jy_t *copy = (pico8_jy_t *)pico8_zone_alloc((const pico8_zone_choisie_t *)ctx, sizeof *copy);
        const pico8_jy_t *i = e;

        *copy = *i;
        *out = copy;
        return 1;
    }
    return 0;
}

void pico8_jo_lz(pico8_jo_t *self, pico8_hp_t *hp_instance) {
    const pico8_zone_choisie_t *zone = pico8_jo_zone_construction();
    pico8_ptrs_t jy;
    pico8_ptrs_t *jy_own;

    pico8_hp_assert(hp_instance);
    if (self == NULL) {
        return;
    }

    self->hn = hp_instance;
    self->ep = NULL;

    pico8_ptrs_init(&jy, zone);
    if (hp_instance != NULL && hp_instance->i != NULL) {
        jy = pico8_nv_ptrs(zone, hp_instance->i, pico8_jo_lz_mw_step, (void *)zone);
    }
    jy_own = (pico8_ptrs_t *)pico8_zone_alloc(zone, sizeof *jy_own);
    *jy_own = jy;
    self->jy = jy_own;

    self->kw_own.has_qe = hp_instance != NULL && hp_instance->has_o;
    self->kw_own.qe = self->kw_own.has_qe ? hp_instance->o : 0;
    self->kw_own.mu = p8_fixed_from_int(0);
    self->kw = &self->kw_own;
}

int pico8_jo_gg(const pico8_jo_t *self) {
    if (self == NULL) {
        return 0;
    }
    if (!self->def->hf) {
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
    const pico8_df_entry_t *nk = NULL;
    const char *ks_arg;

    if (self == NULL) {
        return;
    }

    if (g_jo_ir != NULL) {
        p8_fixed_t roll = pico8_jf_next(g_jo_ir);
        int use_ll = 0;

        /* `jo.ir()<0.45 and df.ll or jo.oc()`: a falsy `df.ll` falls through
         * to `jo.oc()` even on a winning roll -- both conditions, never the
         * roll alone. */
        if (roll < P8_FIXED_LIT(0x00007333, 0.45)) {
            use_ll = pico8_df_lookup("vanishing") != NULL;
        }
        if (use_ll) {
            nk = pico8_df_lookup("vanishing");
        } else if (g_jo_oc != NULL) {
            nk = pico8_gn_next(g_jo_oc);
        }
    }
    self->nk = nk;
    self->ep = nk != NULL ? nk->d : NULL;

    if (nk != NULL && nk->me != NULL) {
        ks_arg = nk->me;
    } else {
        ks_arg = "known";
    }
    pico8_ke_ks((entity_t *)self, ks_arg);
    pico8_jo_kd(self);
}

void pico8_jo_hk(const pico8_jo_t *self) {
    if (self == NULL) {
        return;
    }
    if (!self->has_kr || self->kr == p8_fixed_from_int(0)) {
        return;
    }
    if (!pico8_gd((const entity_t *)self)) {
        return;
    }
    {
        pico8_iv_keys_t keys = {0};
        int color = 0;
        char *text;

        keys.ra = self->ra;
        text = pico8_ne(pico8_zone_frame(), self->kr, 10, "$", &color);
        /* `ne()` is the last expression of the Lua constructor, so both of its
         * return values land as positional entries 1 and 2, not named fields. */
        keys.at1 = text;
        keys.at2 = p8_fixed_from_int(color);
        pico8_iv_new(&keys);
    }
}

void pico8_na(entity_t *self, p8_fixed_t t) {
    pico8_vec_t *ra_vec;

    assert((self == NULL || pico8_jo_is_family(self)) && "na: self is a planet (qh:ly, nl:ly) -- stop");
    ra_vec = self != NULL ? ((pico8_jo_t *)self)->ra : NULL;
    if (ra_vec != NULL) {
        p8_fixed_t delta = p8_fixed_mul(p8_fixed_sin(p8_fixed_mul(t, P8_FIXED_LIT(0x00000147, 0.005))),
                                               P8_FIXED_LIT(0x00000f5c, 0.06));
        ra_vec->y = p8_fixed_add(ra_vec->y, delta);
    }
}
