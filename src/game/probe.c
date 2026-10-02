#include "game/probe.h"

#include <assert.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "engine/geom.h"
#include "p8/gfx.h"
#include "game/hud.h"
#include "p8/audio.h"
#include "engine/entity.h"
#include "game/particle.h"
#include "game/planet.h"
#include "game/slipway.h"
#include "game/ui.h"
#include "game/wormhole.h"
#include "p8/str.h"
#include "engine/frame_zone.h"

static int g_g__up = 0;
static entity_t *g_fm = NULL;

static const entity_class_t g_fm_placeholder_struct_class = {0};

static pico8_fm_placeholder_t g_fm_placeholder = {
    {.class = &g_fm_placeholder_struct_class}, 0, 0, 0, 0};

static int bv_et_is_holder_class(const entity_class_t *cls) {
    return cls->drag == pico8_bv_dispatch || cls == pico8_mz_struct_class();
}

static const entity_class_t g_bv_et_struct_class = {0};

/* The `{}` `bv` puts in `et` over the void, off the panel: never allocated,
 * never freed; only `fo` is written, and the next `bv` overwrites it. */
static pico8_bv_et_t g_bv_et = {{.class = &g_bv_et_struct_class}, NULL};

static int pico8_probe_to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

typedef struct {
    pico8_vec_t self_p;
} pico8_bv_ctx_t;

static int pico8_bv_jc_predicate(entity_t *e, void *ctx_ptr) {
    const pico8_bv_ctx_t *ctx = (const pico8_bv_ctx_t *)ctx_ptr;
    const pico8_jc_t *sw;
    pico8_vec_t qu_p;
    pico8_vec_t re_p;

    if (e == NULL) {
        return 0;
    }
    assert(e->class == pico8_jc_struct_class() && "cg.jc holds slipways only");
    sw = (const pico8_jc_t *)e;

    qu_p = pico8_jc_end_p(sw->qu);
    re_p = pico8_jc_end_p(sw->re);

    return pico8_geom_be(qu_p, re_p, ctx->self_p, P8_FIXED_LIT(0x000000d1, 0.0032));
}

/* `hw` is read off the `ui` class, not off `ok`; no class reads as "the
 * point is not inside the panel". */
static int pico8_probe_ui_hw_call(pico8_vec_t rp) {
    const pico8_qz_box_t *hw = pico8_ui_hw();

    if (hw == NULL) {
        return 0;
    }
    return pico8_qz_call(hw, rp);
}

/* `ra` and `p`, which a host of `bv` holds for itself: `g_`'s line, jo's
 * family (qh's line) or an `mz`. */
static void probe_host_set_ra_p(entity_t *self, pico8_vec_t *ra, pico8_vec_t *p) {
    pico8_g__t *g_;
    pico8_jo_t *jo;
    pico8_mz_t *mz;

    if (self->class->lineage & LINEAGE(LINEAGE_PROBE)) {
        g_ = (pico8_g__t *)self;
        g_->ra = ra;
        g_->p = p;
        return;
    }
    if (pico8_jo_is_family(self)) {
        jo = (pico8_jo_t *)self;
        jo->ra = ra;
        jo->p = p;
        return;
    }
    assert(self->class == pico8_mz_struct_class() &&
           "bv: a host neither of g_'s line, of jo's family nor an mz, none of whose hq runs bv -- stop");
    mz = (pico8_mz_t *)self;
    mz->ra = ra;
    mz->p = p;
}

/* What `bv` aims at: `cg.np` (jo, and qh's tree: exactly the descriptors
 * holding `ly`), a `jc`, or the throwaway target. */
static int g__is_target_class(const entity_class_t *cls) {
    return cls->states[ENTITY_STATE_SETTLED] != NULL || cls == pico8_jo_struct_class() ||
           cls == pico8_jc_struct_class() || cls == pico8_bv_et_struct_class();
}

void pico8_probe_et_hang_fo(entity_t *et, entity_t *self) {
    if (pico8_jo_is_family(et)) {
        assert((self->class->drag == pico8_bv_dispatch || self->class == pico8_jc_struct_class()) &&
               "jo.fo: a write neither nil nor an object whose hq runs bv (g_, fv, qh, nl, os, hj, pm, nr, fr, mx) "
               "or a jc does not fit the member's type -- stop, do not widen the member");
        ((pico8_jo_t *)et)->fo = self;
        return;
    }
    if (et->class == pico8_jc_struct_class()) {
        assert((self->class->drag == pico8_bv_dispatch || self->class == pico8_jc_struct_class() ||
                self->class == pico8_mz_struct_class()) &&
               "jc.fo: a write neither nil nor what hangs fo on a slipway (an object whose hq runs bv: g_, fv, qh, "
               "nl, os, hj, pm, nr, fr, mx; an mz; another jc) does not fit the member's type -- stop");
        ((pico8_jc_t *)et)->fo = self;
        return;
    }
    if (et->class == pico8_jc_too_far_struct_class()) {
        assert(self->class == pico8_jc_struct_class() &&
               "jc_too_far.fo: a write neither nil nor a jc does not fit the member's type "
               "-- stop, do not widen the member");
        ((pico8_jc_too_far_t *)et)->fo = (pico8_jc_t *)self;
        return;
    }
    assert(et == &g_bv_et.header && "bv: et is a planet, a jc or the static throwaway struct, nothing else");
    assert(bv_et_is_holder_class(self->class) &&
           "bv_et.fo: a write neither nil nor an object whose hq runs bv (g_, fv, mz, qh, nl, os, hj, pm, nr, fr, "
           "mx) does not fit the member's type -- stop, do not widen the member");
    g_bv_et.fo = self;
}

void pico8_bv(entity_t *self, pico8_vec_t *mp, pico8_vec_t *rp) {
    pico8_probe_t *probe;
    pico8_vec_t mp_vec;
    pico8_vec_t p_vec;
    p8_fixed_t nq;
    entity_t *c;
    entity_t *et;

    if (self == NULL) {
        return;
    }
    assert(self->targeting != NULL && "bv: a class that carries no probe is no receiver -- stop");

    probe = self->targeting;
    mp_vec = mp != NULL ? *mp : pico8_vec_v(p8_fixed_from_int(0), p8_fixed_from_int(0));
    p_vec = pico8_vec_div(mp_vec, p8_fixed_from_int(128));
    probe->bv_ra = mp_vec;
    probe->bv_p = p_vec;
    probe_host_set_ra_p(self, mp != NULL ? &probe->bv_ra : NULL, &probe->bv_p);

    c = NULL;
    nq = pico8_ij_ents(p_vec, pico8_cg("node"), &c);

    et = NULL;

    if (nq < probe->def->dw && c != NULL) {
        et = c;
    } else {
        pico8_bv_ctx_t ctx;
        pico8_ents_t filtered;
        int have_first = 0;
        entity_t *first = NULL;

        ctx.self_p = p_vec;
        filtered = pico8_jp_ents(pico8_zone_frame(), pico8_cg("slipway"), pico8_bv_jc_predicate, &ctx);
        if (filtered.count > 0) {
            first = filtered.items[0];
            have_first = 1;
        }

        if (have_first && first != NULL) {
            et = first;
        } else {
            pico8_vec_t rp_vec = rp != NULL ? *rp : pico8_vec_v(p8_fixed_from_int(0), p8_fixed_from_int(0));
            if (!pico8_probe_ui_hw_call(rp_vec)) {
                et = &g_bv_et.header;
            } else {
                et = NULL;
            }
        }
    }

    assert((et == NULL ||
            g__is_target_class(et->class)) &&
           "bv: et is false or what bv aims at (a planet jo, qh, nl, os, hj, pm, nr, fr, mx, a jc, or the static "
           "throwaway struct), never a bare table");
    probe->et = et;

    if (probe->et != NULL) {
        pico8_probe_et_hang_fo(probe->et, self);
    }
}

void pico8_bv_dispatch(entity_t *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    pico8_bv(self, qa, ra);
}

/* `qs` of `dl`'s target: `mz.jj` and `jc:ce`'s "too far" struct have one;
 * any other target (a planet, a `jc`, the throwaway) reads as absent, NULL. */
static const char *probe_et_qs(const entity_t *et) {
    if (et->class == pico8_mz_jj_struct_class()) {
        return ((const pico8_mz_jj_t *)et)->qs;
    }
    if (et->class == pico8_jc_too_far_struct_class()) {
        return ((const pico8_jc_too_far_t *)et)->qs;
    }
    return NULL;
}

void pico8_dl(const p8_graphics_t *gfx, p8_memory_t *mem, entity_t *self) {
    if (self == NULL) {
        return;
    }
    assert(self->targeting != NULL && "dl: a class that carries no probe is no receiver -- stop");

    pico8_dl_show(gfx, mem, self->state, self->targeting);
}

void pico8_dl_show(const p8_graphics_t *gfx, p8_memory_t *mem, const char *me, const pico8_probe_t *pb) {
    pico8_vec_t mp;
    const pico8_nf_t *je;
    const char *display = "";

    if (me == NULL || strcmp(me, "placing") != 0) {
        return;
    }

    je = pico8_je_nf();
    mp = je != NULL && je->ra != NULL ? *je->ra : pico8_vec_v(p8_fixed_from_int(0), p8_fixed_from_int(0));

    if (pb->et != NULL) {
        const char *qs = probe_et_qs(pb->et);
        if (qs != NULL) {
            display = qs;
        } else {
            if (gfx != NULL && mem != NULL) {
                p8_spr_default(gfx, mem, 31, pico8_probe_to_int(p8_fixed_sub(mp.x, p8_fixed_from_int(3))),
                                   pico8_probe_to_int(p8_fixed_sub(mp.y, p8_fixed_from_int(8))));
            }
        }
    } else if (pb->has_od) {
        display = p8_concat2(pico8_zone_frame(), p8_tostr_fixed(pico8_zone_frame(), pb->od), "$");
    }

    if (gfx != NULL && mem != NULL) {
        p8_fixed_t a = P8_FIXED_LIT(0x00008000, 0.5);
        pico8_io(gfx, mem, display, mp.x, p8_fixed_sub(mp.y, p8_fixed_from_int(8)), 8, &a);
    }
}

void pico8_dl_dispatch(entity_t *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_dl(pico8_dispatch_gfx(), pico8_dispatch_mem(), self);
}

/* Raises `p_`, which a host of `d_` holds for itself: `g_`'s line, jo's
 * family (qh's line), a `jc` or an `mz`. */
static void probe_host_raise_p_(entity_t *self) {
    assert(self->is_dead != NULL && "d_: a host that carries no p_, none of whose ey runs d_ -- stop");
    *self->is_dead = 1;
}

int pico8_d_(entity_t *self) {
    const pico8_probe_t *probe;
    int has_et;
    pico8_hy_t *fm;
    int lu_ok = 0;

    if (self == NULL) {
        return 0;
    }
    assert(self->targeting != NULL && "d_: a class that carries no probe is no receiver -- stop");

    probe = self->targeting;
    has_et = probe->et != NULL;

    fm = pico8_fm_hy();
    if (!has_et) {
        lu_ok = pico8_hy_lu(fm, probe->has_od ? probe->od : p8_fixed_from_int(0));
    }

    assert((!has_et || !lu_ok) && "d_: fm:lu must stay short-circuited when et is set");

    if (!has_et && lu_ok) {
        pico8_ke_ks(self, probe->def->dq != NULL ? probe->def->dq : "normal");

        pico8_hy_nx(pico8_fm_hy(), probe->def->fp);

        p8_play_sfx(probe->def->has_ik ? probe->def->ik : p8_fixed_from_int(54));

        return 1;
    }

    p8_play_sfx(p8_fixed_from_int(57));
    probe_host_raise_p_(self);
    return 0;
}

void pico8_d__dispatch(entity_t *self, entity_t *kv) {
    (void)kv;
    (void)pico8_d_(self);
}

pico8_qy_t *pico8_g__mm(entity_t *self) {
    const pico8_emitter_t *emitter = NULL;
    p8_fixed_t pn = 0, t = 0, du = 0, py = 0, ki = 0;
    int pk = 0;
    p8_fixed_t a, rnd_pn, rnd_half;
    pico8_vec_t p, v;
    pico8_qy_t *particle;

    if (self != NULL) {
        assert(self->emitter != NULL && "g_.mm: a class that carries no particle emitter has no mm -- stop");
        emitter = self->emitter;
        pn = emitter->pn;
        t = self->state_time;
        du = emitter->du;
        py = emitter->py;
        ki = emitter->def->ki;
        pk = emitter->def->pk != 0;
    }

    /* Two statements, not two arguments: C leaves argument order unspecified
     * (gcc drew rnd(0.5) first), Lua draws rnd(self.pn) first. */
    rnd_pn = p8_fixed_rnd(pn);
    rnd_half = p8_fixed_rnd(P8_FIXED_LIT(0x00008000, 0.5));
    a = p8_fixed_add(
        p8_fixed_div(p8_fixed_add(p8_fixed_flr(rnd_pn), rnd_half), pn),
        p8_fixed_mul(t, P8_FIXED_LIT(0x00000041, 0.001)));

    p = pico8_vec_qm(p8_fixed_add(du, p8_fixed_rnd_default()), a);
    v = pico8_vec_mul(p, py);

    if (pk) {
        v = pico8_vec_pa(v);
    }

    particle = pico8_particule_prendre();
    particle->p = p;
    particle->v = v;
    particle->has_a = 0;
    particle->a = pico8_vec_v(p8_fixed_from_int(0), p8_fixed_from_int(0));
    particle->l = ki;

    return particle;
}

void pico8_g__rendre_dispatch(entity_t *self) {
    const pico8_g__t *g_ = (const pico8_g__t *)self;

    pico8_qy_rendre(self->emitter);
    if (g_->prise != NULL && (self->state == NULL || strcmp(self->state, "settled") != 0)) {
        pico8_jo_prise_arene_rendue(g_->prise);
    }
}

static const entity_class_t g_g__struct_class = {
    .draw = pico8_ei_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .spawn_particle = pico8_g__mm,
    .release = pico8_g__rendre_dispatch,
    .tags = TAG(TAG_PROBE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PROBE),
};

void pico8_g__struct_init(pico8_g__t *self, const entity_class_t *cls, const pico8_g__def_t *def) {
    pico8_ke_struct_init(&self->header, cls, def->me, def->t);
    self->def = def;
    self->header.layer = &self->def->fa;
    self->header.pos = &self->ra;
    self->header.map_pos = &self->p;
    self->header.fold_anchor = &self->def->fx;
    self->header.is_dead = &self->p_;
    self->header.emitter = &self->emitter_own;
    self->header.targeting = &self->probe_own;
    self->emitter = &self->emitter_own;
    self->emitter->def = &self->def->emitter;
    self->emitter->qy = NULL;
    self->ra = self->def->has_ra ? &self->def->ra : NULL;
    self->p = NULL;
    self->probe = &self->probe_own;
    self->probe->def = &self->def->probe;
    self->probe->et = NULL;
    self->p_ = 0;
    self->prise = NULL;
    self->probe->od = self->def->probe.od;
    self->probe->has_od = self->def->probe.has_od;
    self->emitter->py = self->def->emitter.py;
    self->emitter->pn = self->def->emitter.pn;
    self->emitter->k_ = self->def->emitter.k_;
    self->emitter->qo = self->def->emitter.qo;
    self->emitter->ro = self->def->emitter.ro;
    self->emitter->qg = self->def->emitter.qg;
    self->emitter->q_ = self->def->emitter.q_;
    self->emitter->du = self->def->emitter.du;
}

static pico8_g__t *pico8_g__struct_new(const pico8_g__keys_t *keys) {
    pico8_g__t *self;

    assert(g_g__up && "pico8_probe_init() must run first");
    pico8_jo_prise_ouvrir();
    self = (pico8_g__t *)pico8_zone_alloc(pico8_jo_zone_construction(), sizeof *self);
    pico8_g__struct_init(self, &g_g__struct_class, &pico8_g__def);
    self->prise = pico8_jo_zone_construction();
    pico8_jo_prise_fermer();
    if (keys != NULL) {
        self->header.state_time = keys->t;
        if (keys->has_pn) {
            self->emitter->pn = keys->pn;
        }
        if (keys->has_du) {
            self->emitter->du = keys->du;
        }
        if (keys->has_py) {
            self->emitter->py = keys->py;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("probe");
#endif
    pico8_f_((entity_t *)self);
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_probe_init(void) {
    pico8_probe_shutdown();

    g_g__up = 1;

    g_fm_placeholder.jk = 0;
    g_fm_placeholder.has_jk = 0;
    g_fm_placeholder.nz = 0;
    g_fm_placeholder.has_nz = 0;
    g_fm = (entity_t *)&g_fm_placeholder;
}

void pico8_probe_shutdown(void) {
    g_g__up = 0;
    g_fm = NULL;
}

int pico8_g__ready(void) {
    return g_g__up;
}

const entity_class_t *pico8_g__struct_class(void) {
    return &g_g__struct_class;
}

pico8_g__t *pico8_g__new(const pico8_g__keys_t *keys) {
    return pico8_g__struct_new(keys);
}

const entity_class_t *pico8_fm_placeholder_struct_class(void) {
    return &g_fm_placeholder_struct_class;
}

entity_t *pico8_fm(void) {
    return g_fm;
}

pico8_hy_t *pico8_fm_hy(void) {
    if (g_fm == NULL || g_fm == &g_fm_placeholder.header) {
        return NULL;
    }
    assert(g_fm->class == pico8_hy_struct_class() && "fm: only the provisional or a hy is ever stored");
    return (pico8_hy_t *)g_fm;
}

const pico8_fm_placeholder_t *pico8_fm_placeholder(void) {
    return g_fm == &g_fm_placeholder.header ? &g_fm_placeholder : NULL;
}

void pico8_probe_set_fm(entity_t *instance) {
    g_fm = instance;
}

const entity_class_t *pico8_bv_et_struct_class(void) {
    return &g_bv_et_struct_class;
}
