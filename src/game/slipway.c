#include "game/slipway.h"

#include <assert.h>
#include <string.h>

#include "p8/audio.h"
#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "game/economy_formulas.h"
#include "engine/entity_registry.h"
#include "game/main_economy.h"
#include "engine/geom.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "game/planet.h"
#include "game/targeting.h"
#include "game/tech_gate.h"
#include "game/ui.h"
#include "engine/vec.h"
#include "engine/frame_zone.h"
#include "game/particle_zone.h"

static int g_jc_up = 0;

static const entity_class_t g_jc_too_far_struct_class = {0};

/* The `{qs="-too far-"}` `jc:ce` returns beyond range: never allocated,
 * never freed; `qs` is fixed, only `fo` is written, and the next `hq`
 * overwrites it. */
static pico8_jc_too_far_t g_jc_too_far = {{.class = &g_jc_too_far_struct_class}, "-too far-", NULL};

static const entity_class_t g_jc_mouse_point_struct_class = {0};

pico8_vec_t pico8_jc_end_p(const entity_t *end) {
    const pico8_vec_t *p = end != NULL && end->map_pos != NULL ? *end->map_pos : NULL;

    return p != NULL ? *p : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

pico8_vec_t pico8_jc_end_ra(const entity_t *end) {
    const pico8_vec_t *ra = end != NULL && end->pos != NULL ? *end->pos : NULL;

    return ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static int jc_mt_of(const entity_t *e, pico8_fixed_t *out) {
    if (e == NULL || e->infraspace == NULL) {
        return 0;
    }
    *out = *e->infraspace;
    return 1;
}

void pico8_jc_hg_add(pico8_jc_hg_t *hg, const char *id) {
    assert(id != NULL &&
           "jc.hg: a nil id does not fit the member's type -- stop, do not widen the member");
    if (pico8_jc_hg_has(hg, id)) {
        return;
    }
    assert(hg->count < PICO8_JC_HG_CAP && "jc.hg: more planets than bw(90) makes -- stop, do not widen the member");
    hg->ids[hg->count] = id;
    hg->count++;
}

int pico8_jc_hg_has(const pico8_jc_hg_t *hg, const char *id) {
    int i;

    if (hg == NULL || id == NULL) {
        return 0;
    }
    for (i = 0; i < hg->count; i++) {
        if (strcmp(hg->ids[i], id) == 0) {
            return 1;
        }
    }
    return 0;
}

void pico8_jc_pq(pico8_jc_t *self) {
    const pico8_zone_choisie_t *zone;
    pico8_ptrs_t *ts;

    if (self == NULL) {
        return;
    }

    pico8_rn(pico8_fixed_from_int(50));

    zone = pico8_jo_zone_construction();
    ts = (pico8_ptrs_t *)pico8_zone_alloc(zone, sizeof *ts);
    pico8_ptrs_init_reserve(ts, zone, PICO8_JC_TS_RESERVE);
    self->ts = ts;

    self->hg_own.count = 0;
    self->hg = &self->hg_own;
}

void pico8_jc_hq(pico8_jc_t *self, pico8_vec_t *mp) {
    pico8_ui_t *ok;
    entity_t *hovered;

    if (self == NULL) {
        return;
    }

    ok = pico8_ok();
    hovered = ok != NULL ? ok->o_ : NULL;

    if (hovered != NULL && hovered->class->accept_drop != NULL) {
        self->re = hovered;
    } else {
        self->point_ra = mp != NULL ? *mp : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
        self->point.ra = mp != NULL ? &self->point_ra : NULL;
        self->point_p = pico8_vec_div(self->point_ra, pico8_fixed_from_int(128));
        self->re = &self->point.header;
    }

    self->probe->od = pico8_dj(pico8_jc_end_p(self->qu), pico8_jc_end_p(self->re));
    self->probe->has_od = 1;

    /* A fixed is always true in Lua: `qu.mt or re.mt` takes the first present. */
    self->mt = pico8_fixed_from_int(0);
    self->has_mt = jc_mt_of(self->qu, &self->mt) || jc_mt_of(self->re, &self->mt);
    self->header.infraspace = self->has_mt ? &self->mt : NULL;

    self->probe->et = pico8_jc_ce(self);
    if (self->probe->et != NULL) {
        pico8_probe_et_hang_fo(self->probe->et, &self->header);
    }

    self->fa = self->has_mt ? pico8_fixed_from_int(2) : pico8_fixed_from_int(4);
}

void pico8_jc_ey(pico8_jc_t *self, entity_t *re) {
    int should_fail;

    if (self == NULL) {
        return;
    }

    /* `d_()` charges the player and advances the clock, so it must stay
     * behind the `re == NULL` short-circuit. */
    should_fail = (re == NULL) || !pico8_d_((entity_t *)self);
    if (should_fail) {
        /* pico8_stat has no case for 19, so it reads 0 and this sfx never
         * fires; ported anyway. It dereferences `input` without checking it,
         * so the NULL guard below is load-bearing. */
        const pico8_input_t *input = pico8_dispatch_input();
        if (input != NULL && pico8_stat(input, 19) == 50) {
            pico8_rn(pico8_fixed_from_int(-1));
        }
        self->p_ = 1;
        return;
    }

    self->nh = pico8_fixed_from_int(1);
    self->re = re;

    {
        entity_t *qu = self->qu;
        int qu_registered = qu != NULL && qu->class->attach_slipway != NULL;
        int re_registered;
        if (qu_registered) {
            qu->class->attach_slipway(qu, &self->header);
        }
        re_registered = re != NULL && re->class->attach_slipway != NULL;
        if (re_registered) {
            re->class->attach_slipway(re, &self->header);
        }
        assert(qu_registered && re_registered &&
               "jc:ey: both endpoints must expose \"kf\", or the slipway never joins their `oe`");
        (void)qu_registered;
        (void)re_registered;
        pico8_hy_nx(pico8_fm_hy(), pico8_fixed_from_int(1));
    }
}

typedef struct {
    pico8_vec_t sp;
    pico8_vec_t dp;
} pico8_jc_ce_np_ctx_t;

/* `be()` excludes the segment's own endpoints (their projection fraction is
 * exactly 0 or 1), so `qu` and `re` never match themselves here. */
static int pico8_jc_ce_np_predicate(entity_t *e, void *ctx_ptr) {
    const pico8_jc_ce_np_ctx_t *ctx = (const pico8_jc_ce_np_ctx_t *)ctx_ptr;

    if (e == NULL) {
        return 0;
    }
    return pico8_geom_be(ctx->sp, ctx->dp, pico8_jc_end_p(e), PICO8_FIXED_LIT(0x000000d1, 0.0032));
}

typedef struct {
    pico8_vec_t sp;
    pico8_vec_t dp;
    int self_has_mt;
    pico8_fixed_t self_mt;
} pico8_jc_ce_jc_ctx_t;

/* `self` is already registered in `cg("jc")` when this runs; `cl()` of a
 * segment against itself is always false, so no self-exclusion is needed. */
static int pico8_jc_ce_jc_predicate(entity_t *e, void *ctx_ptr) {
    const pico8_jc_ce_jc_ctx_t *ctx = (const pico8_jc_ce_jc_ctx_t *)ctx_ptr;
    const pico8_jc_t *sw;

    if (e == NULL) {
        return 0;
    }
    assert(e->class == pico8_jc_struct_class() && "cg.jc holds slipways only");
    sw = (const pico8_jc_t *)e;
    if (sw->has_mt != ctx->self_has_mt || (sw->has_mt && sw->mt != ctx->self_mt)) {
        return 0;
    }
    return pico8_geom_cl(ctx->sp, ctx->dp, pico8_jc_end_p(sw->qu), pico8_jc_end_p(sw->re));
}

entity_t *pico8_jc_ce(pico8_jc_t *self) {
    entity_t *qu;
    entity_t *re;
    pico8_vec_t sp;
    pico8_vec_t dp;
    pico8_fixed_t threshold;

    if (self == NULL) {
        return NULL;
    }

    qu = self->qu;
    re = self->re;
    sp = pico8_jc_end_p(qu);
    dp = pico8_jc_end_p(re);

    threshold = pico8_ov("space folding", PICO8_FIXED_LIT(0x00008000, 0.5), PICO8_FIXED_LIT(0x0001147a, 1.08));

    if (pico8_vec_len_sq(pico8_vec_sub(sp, dp)) > threshold) {
        return &g_jc_too_far.header;
    }

    {
        const pico8_ents_t *oe = NULL;

        if (qu != NULL) {
            assert(pico8_jo_is_family(qu) && "jc.qu is a planet: the constructor asserts it");
            oe = ((const pico8_jo_t *)qu)->oe;
        }
        if (oe != NULL) {
            pico8_ents_iter_t it = pico8_ents_all(oe);
            entity_t *w;
            while (pico8_ents_iter_next(&it, &w)) {
                if (w == NULL) {
                    continue;
                }
                assert(w->class == pico8_jc_struct_class() &&
                       "qu.oe must hold jc structs: only jo:kf adds to it, and only slipways");
                if (pico8_jc_eb((const pico8_jc_t *)w, qu) == re) {
                    return w;
                }
            }
        }
    }

    {
        pico8_jc_ce_np_ctx_t np_ctx;
        pico8_jc_ce_jc_ctx_t jc_ctx;
        pico8_ents_t part1;
        pico8_ents_t part2;
        pico8_ents_t ds;
        entity_t *result = NULL;

        np_ctx.sp = sp;
        np_ctx.dp = dp;
        jc_ctx.sp = sp;
        jc_ctx.dp = dp;
        jc_ctx.self_has_mt = self->has_mt;
        jc_ctx.self_mt = self->mt;

        part1 = pico8_jp_ents(pico8_zone_frame(), pico8_cg("node"), pico8_jc_ce_np_predicate, &np_ctx);
        part2 = pico8_jp_ents(pico8_zone_frame(), pico8_cg("slipway"), pico8_jc_ce_jc_predicate, &jc_ctx);
        ds = pico8_ents_cat(pico8_zone_frame(), &part1, &part2);

        if (ds.count > 0) {
            result = ds.items[0];
        }
        return result;
    }
}

entity_t *pico8_jc_eb(const pico8_jc_t *self, const entity_t *jo_instance) {
    if (self == NULL) {
        return NULL;
    }

    /* `A and B or C`: a falsy `self.re` falls through to `self.qu`, so this
     * is not a plain ternary. */
    if (jo_instance == self->qu && self->re != NULL) {
        return self->re;
    }
    return self->qu;
}

void pico8_jc_ev(pico8_jc_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->qu != self->re) {
        pico8_dl(pico8_dispatch_gfx(), pico8_dispatch_mem(), (entity_t *)self);
    }
}

static void pico8_jc_hq_dispatch(entity_t *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    (void)ra;
    pico8_jc_hq((pico8_jc_t *)self, qa);
}

static void pico8_jc_ey_dispatch(entity_t *self, entity_t *kv) {
    pico8_jc_ey((pico8_jc_t *)self, kv);
}

static void pico8_jc_ev_dispatch(entity_t *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_jc_ev((pico8_jc_t *)self);
}

static void pico8_jc_rendre_dispatch(entity_t *self) {
    pico8_jc_t *jc = (pico8_jc_t *)self;
    const pico8_zone_choisie_t *zone = jc->ts->zone;

    for (int i = 0; i < jc->ts->count; i++)
        pico8_zone_release(pico8_zone_particules(PICO8_PARTICULES_TS), jc->ts->items[i]);
    jc->ts->count = 0;
    pico8_jo_prise_zone_rendue(zone, jc->header.state);
}

static int jc_is_endpoint_class(const entity_class_t *cls) {
    return cls->states[ENTITY_STATE_SETTLED] != NULL || cls == pico8_jo_struct_class();
}

static const entity_class_t g_jc_struct_class = {
    .draw = pico8_jc_kj_dispatch,
    .draw_overlay = pico8_jc_ev_dispatch,
    .drag = pico8_jc_hq_dispatch,
    .drop = pico8_jc_ey_dispatch,
    .release = pico8_jc_rendre_dispatch,
    .tags = TAG(TAG_SLIPWAY),
};

static pico8_jc_t *pico8_jc_struct_new(const pico8_jc_keys_t *keys) {
    pico8_jc_t *self;

    pico8_jo_prise_ouvrir();
    self = (pico8_jc_t *)pico8_zone_alloc(pico8_jo_zone_construction(), sizeof *self);

    pico8_ke_struct_init(&self->header, &g_jc_struct_class, pico8_jc_def.me, pico8_jc_def.t);
    self->def = &pico8_jc_def;
    self->fa = self->def->fa;
    self->header.layer = &self->fa;
    self->header.targeting = &self->probe_own;
    self->probe = &self->probe_own;
    self->probe->def = &self->def->probe;
    self->nh = self->def->nh;
    self->qu = NULL;
    self->ts = NULL;
    self->hg = NULL;
    self->re = NULL;
    self->probe->od = self->def->probe.od;
    self->probe->has_od = self->def->probe.has_od;
    self->mt = 0;
    self->has_mt = 0;
    self->header.infraspace = NULL;
    self->probe->et = NULL;
    self->fo = NULL;
    self->header.is_dead = &self->p_;
    self->p_ = 0;
    entity_init_header(&self->point.header, &g_jc_mouse_point_struct_class);
    self->point.header.pos = &self->point.ra;
    self->point.header.map_pos = &self->point.p;
    self->point.ra = NULL;
    self->point.p = &self->point_p;
    self->point_ra = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    self->point_p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    if (keys != NULL && keys->qu != NULL) {
        assert(jc_is_endpoint_class(keys->qu->class) &&
               "jc.qu: a key neither nil nor a planet a slipway starts from (jo, qh, nl, os, hj, pm, nr, fr, mx) "
               "does not fit the member's type -- stop, do not widen the member");
        self->qu = keys->qu;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("slipway");
#endif
    pico8_jc_pq(self);
    pico8_jo_prise_fermer();
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_jc_init(void) {
    pico8_jc_shutdown();
    g_jc_up = 1;
}

void pico8_jc_shutdown(void) {
    g_jc_up = 0;
}

int pico8_jc_ready(void) {
    return g_jc_up;
}

const entity_class_t *pico8_jc_struct_class(void) {
    return &g_jc_struct_class;
}

pico8_jc_t *pico8_jc_new(const pico8_jc_keys_t *keys) {
    return pico8_jc_struct_new(keys);
}

const entity_class_t *pico8_jc_too_far_struct_class(void) {
    return &g_jc_too_far_struct_class;
}

const entity_class_t *pico8_jc_mouse_point_struct_class(void) {
    return &g_jc_mouse_point_struct_class;
}
