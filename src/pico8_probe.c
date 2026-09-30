#include "pico8_probe.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_fixed.h"
#include "pico8_geom.h"
#include "pico8_gfx.h"
#include "pico8_hud.h"
#include "pico8_audio.h"
#include "pico8_collect.h"
#include "pico8_ke.h"
#include "pico8_particle.h"
#include "pico8_planet.h"
#include "pico8_slipway.h"
#include "pico8_ui.h"
#include "pico8_wormhole.h"
#include "pico8_str.h"

static int g_g__up = 0;
static pico8_struct_header_t *g_fm = NULL;

static const pico8_struct_class_t g_fm_placeholder_struct_class = {0};

static pico8_fm_placeholder_t g_fm_placeholder = {
    {.cls = &g_fm_placeholder_struct_class}, 0, 0, 0, 0};

static int bv_et_is_holder_class(const pico8_struct_class_t *cls) {
    return cls->hq == pico8_bv_dispatch || cls == pico8_mz_struct_class();
}

static const pico8_struct_class_t g_bv_et_struct_class = {0};

/* The `{}` `bv` puts in `et` over the void, off the panel: never allocated,
 * never freed; only `fo` is written, and the next `bv` overwrites it. */
static pico8_bv_et_t g_bv_et = {{.cls = &g_bv_et_struct_class}, NULL};

static int pico8_probe_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

typedef struct {
    pico8_vec_t self_p;
} pico8_bv_ctx_t;

static int pico8_bv_jc_predicate(void *e, void *ctx_ptr) {
    const pico8_bv_ctx_t *ctx = (const pico8_bv_ctx_t *)ctx_ptr;
    const pico8_jc_t *sw;
    pico8_vec_t qu_p;
    pico8_vec_t re_p;

    if (e == NULL) {
        return 0;
    }
    assert(((const pico8_struct_header_t *)e)->cls == pico8_jc_struct_class() && "cg.jc holds slipways only");
    sw = e;

    qu_p = pico8_jc_end_p(sw->qu);
    re_p = pico8_jc_end_p(sw->re);

    return pico8_geom_be(qu_p, re_p, ctx->self_p, PICO8_FIXED_LIT(0x000000d1, 0.0032));
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
static void probe_host_set_ra_p(pico8_struct_header_t *self, pico8_vec_t *ra, pico8_vec_t *p) {
    pico8_g__t *g_ = pico8_g__of(self);
    pico8_jo_t *jo;
    pico8_mz_t *mz;

    if (g_ != NULL) {
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
    assert(self->cls == pico8_mz_struct_class() &&
           "bv: a host neither of g_'s line, of jo's family nor an mz, none of whose hq runs bv -- stop");
    mz = (pico8_mz_t *)self;
    mz->ra = ra;
    mz->p = p;
}

/* What `bv` aims at: `cg.np` (jo, and qh's tree: exactly the descriptors
 * holding `ly`), a `jc`, or the throwaway target. */
static int g__is_target_class(const pico8_struct_class_t *cls) {
    return cls->states[PICO8_STATE_LY] != NULL || cls == pico8_jo_struct_class() ||
           cls == pico8_jc_struct_class() || cls == pico8_bv_et_struct_class();
}

void pico8_probe_et_hang_fo(pico8_struct_header_t *et, pico8_struct_header_t *self) {
    if (pico8_jo_is_family(et)) {
        assert((self->cls->hq == pico8_bv_dispatch || self->cls == pico8_jc_struct_class()) &&
               "jo.fo: a write neither nil nor an object whose hq runs bv (g_, fv, qh, nl, os, hj, pm, nr, fr, mx) "
               "or a jc does not fit the member's type -- stop, do not widen the member");
        ((pico8_jo_t *)et)->fo = self;
        return;
    }
    if (et->cls == pico8_jc_struct_class()) {
        assert((self->cls->hq == pico8_bv_dispatch || self->cls == pico8_jc_struct_class() ||
                self->cls == pico8_mz_struct_class()) &&
               "jc.fo: a write neither nil nor what hangs fo on a slipway (an object whose hq runs bv: g_, fv, qh, "
               "nl, os, hj, pm, nr, fr, mx; an mz; another jc) does not fit the member's type -- stop");
        ((pico8_jc_t *)et)->fo = self;
        return;
    }
    if (et->cls == pico8_jc_too_far_struct_class()) {
        assert(self->cls == pico8_jc_struct_class() &&
               "jc_too_far.fo: a write neither nil nor a jc does not fit the member's type "
               "-- stop, do not widen the member");
        ((pico8_jc_too_far_t *)et)->fo = (pico8_jc_t *)self;
        return;
    }
    assert(et == &g_bv_et.header && "bv: et is a planet, a jc or the static throwaway struct, nothing else");
    assert(bv_et_is_holder_class(self->cls) &&
           "bv_et.fo: a write neither nil nor an object whose hq runs bv (g_, fv, mz, qh, nl, os, hj, pm, nr, fr, "
           "mx) does not fit the member's type -- stop, do not widen the member");
    g_bv_et.fo = self;
}

void pico8_bv(pico8_struct_header_t *self, pico8_vec_t *mp, pico8_vec_t *rp) {
    pico8_probe_t *probe;
    pico8_vec_t mp_vec;
    pico8_vec_t p_vec;
    pico8_vec_t *p_heap;
    pico8_fixed_t nq;
    pico8_struct_header_t *c;
    pico8_struct_header_t *et;

    if (self == NULL) {
        return;
    }
    assert(self->cls->probe_of != NULL && "bv: a class that carries no probe is no receiver -- stop");

    probe = self->cls->probe_of(self);
    mp_vec = mp != NULL ? *mp : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    p_vec = pico8_vec_div(mp_vec, pico8_fixed_from_int(128));
    /* TROU-7 */
    p_heap = malloc(sizeof *p_heap);
    *p_heap = p_vec;
    probe_host_set_ra_p(self, mp, p_heap);

    c = NULL;
    nq = pico8_ij(p_vec, pico8_cg("np"), &c);

    et = NULL;

    if (nq < probe->dw && c != NULL) {
        et = c;
    } else {
        pico8_bv_ctx_t ctx;
        pico8_ptrs_t filtered;
        int have_first = 0;
        pico8_struct_header_t *first = NULL;

        ctx.self_p = p_vec;
        filtered = pico8_jp_ptrs(pico8_cg("jc"), pico8_bv_jc_predicate, &ctx);
        if (filtered.count > 0) {
            first = filtered.items[0];
            have_first = 1;
        }
        pico8_ptrs_free(&filtered);

        if (have_first && first != NULL) {
            et = first;
        } else {
            pico8_vec_t rp_vec = rp != NULL ? *rp : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
            if (!pico8_probe_ui_hw_call(rp_vec)) {
                et = &g_bv_et.header;
            } else {
                et = NULL;
            }
        }
    }

    assert((et == NULL ||
            g__is_target_class(et->cls)) &&
           "bv: et is false or what bv aims at (a planet jo, qh, nl, os, hj, pm, nr, fr, mx, a jc, or the static "
           "throwaway struct), never a bare table");
    probe->et = et;

    if (probe->et != NULL) {
        pico8_probe_et_hang_fo(probe->et, self);
    }
}

void pico8_bv_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    pico8_bv((pico8_struct_header_t *)self, qa, ra);
}

/* `qs` of `dl`'s target: `mz.jj` and `jc:ce`'s "too far" struct have one;
 * any other target (a planet, a `jc`, the throwaway) reads as absent, NULL. */
static const char *probe_et_qs(const pico8_struct_header_t *et) {
    if (et->cls == pico8_mz_jj_struct_class()) {
        return ((const pico8_mz_jj_t *)et)->qs;
    }
    if (et->cls == pico8_jc_too_far_struct_class()) {
        return ((const pico8_jc_too_far_t *)et)->qs;
    }
    return NULL;
}

void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self) {
    if (self == NULL) {
        return;
    }
    assert(self->cls->probe_of != NULL && "dl: a class that carries no probe is no receiver -- stop");

    pico8_dl_show(gfx, mem, self->me, self->cls->probe_of(self));
}

void pico8_dl_show(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *me, const pico8_probe_t *pb) {
    pico8_vec_t mp;
    const pico8_nf_t *je;
    const char *display = "";
    char *heap_display = NULL;

    if (me == NULL || strcmp(me, "kz") != 0) {
        return;
    }

    je = pico8_je_nf();
    mp = je != NULL && je->ra != NULL ? *je->ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));

    if (pb->et != NULL) {
        const char *qs = probe_et_qs(pb->et);
        if (qs != NULL) {
            display = qs;
        } else {
            if (gfx != NULL && mem != NULL) {
                pico8_spr_default(gfx, mem, 31, pico8_probe_to_int(pico8_fixed_sub(mp.x, pico8_fixed_from_int(3))),
                                   pico8_probe_to_int(pico8_fixed_sub(mp.y, pico8_fixed_from_int(8))));
            }
        }
    } else if (pb->has_od) {
        char *od_str = pico8_tostr_fixed(pb->od);
        size_t len = strlen(od_str);
        heap_display = malloc(len + 2);
        memcpy(heap_display, od_str, len);
        heap_display[len] = '$';
        heap_display[len + 1] = '\0';
        free(od_str);
        display = heap_display;
    }

    if (gfx != NULL && mem != NULL) {
        pico8_fixed_t a = PICO8_FIXED_LIT(0x00008000, 0.5);
        pico8_io(gfx, mem, display, mp.x, pico8_fixed_sub(mp.y, pico8_fixed_from_int(8)), 8, &a);
    }

    free(heap_display);
}

void pico8_dl_dispatch(void *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_dl(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_struct_header_t *)self);
}

/* Raises `p_`, which a host of `d_` holds for itself: `g_`'s line, jo's
 * family (qh's line), a `jc` or an `mz`. */
static void probe_host_raise_p_(pico8_struct_header_t *self) {
    pico8_g__t *g_ = pico8_g__of(self);
    pico8_jo_t *jo;
    pico8_jc_t *jc;
    pico8_mz_t *mz;

    if (g_ != NULL) {
        g_->p_ = 1;
        g_->has_p_ = 1;
        return;
    }
    if (pico8_jo_is_family(self)) {
        jo = (pico8_jo_t *)self;
        jo->p_ = 1;
        jo->has_p_ = 1;
        return;
    }
    if (self->cls == pico8_jc_struct_class()) {
        jc = (pico8_jc_t *)self;
        jc->p_ = 1;
        jc->has_p_ = 1;
        return;
    }
    assert(self->cls == pico8_mz_struct_class() &&
           "d_: a host neither of g_'s line, of jo's family, a jc nor an mz, none of whose ey runs d_ -- stop");
    mz = (pico8_mz_t *)self;
    mz->p_ = 1;
    mz->has_p_ = 1;
}

int pico8_d_(pico8_struct_header_t *self) {
    const pico8_probe_t *probe;
    int has_et;
    pico8_hy_t *fm;
    int lu_ok = 0;

    if (self == NULL) {
        return 0;
    }
    assert(self->cls->probe_of != NULL && "d_: a class that carries no probe is no receiver -- stop");

    probe = self->cls->probe_of(self);
    has_et = probe->et != NULL;

    fm = pico8_fm_hy();
    if (!has_et) {
        lu_ok = pico8_hy_lu(fm, probe->has_od ? probe->od : pico8_fixed_from_int(0));
    }

    assert((!has_et || !lu_ok) && "d_: fm:lu must stay short-circuited when et is set");

    if (!has_et && lu_ok) {
        pico8_ke_ks(self, probe->dq != NULL ? probe->dq : "ph");

        pico8_hy_nx(pico8_fm_hy(), probe->fp);

        pico8_rn(probe->has_ik ? probe->ik : pico8_fixed_from_int(54));

        return 1;
    }

    pico8_rn(pico8_fixed_from_int(57));
    probe_host_raise_p_(self);
    return 0;
}

void pico8_d__dispatch(void *self, pico8_struct_header_t *kv) {
    (void)kv;
    (void)pico8_d_((pico8_struct_header_t *)self);
}

pico8_qy_t *pico8_g__mm(void *self_ptr) {
    pico8_struct_header_t *self = (pico8_struct_header_t *)self_ptr;
    const pico8_emitter_t *emitter = NULL;
    pico8_fixed_t pn = 0, t = 0, du = 0, py = 0, ki = 0;
    int pk = 0;
    pico8_fixed_t a, rnd_pn, rnd_half;
    pico8_vec_t p, v;
    pico8_qy_t *particle;

    if (self != NULL) {
        assert(self->cls->emitter_of != NULL && "g_.mm: a class that carries no particle emitter has no mm -- stop");
        emitter = self->cls->emitter_of(self);
        pn = emitter->pn;
        t = self->t;
        du = emitter->du;
        py = emitter->py;
        ki = emitter->ki;
        pk = emitter->pk != 0;
    }

    /* Two statements, not two arguments: C leaves argument order unspecified
     * (gcc drew rnd(0.5) first), Lua draws rnd(self.pn) first. */
    rnd_pn = pico8_fixed_rnd(pn);
    rnd_half = pico8_fixed_rnd(PICO8_FIXED_LIT(0x00008000, 0.5));
    a = pico8_fixed_add(
        pico8_fixed_div(pico8_fixed_add(pico8_fixed_flr(rnd_pn), rnd_half), pn),
        pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00000041, 0.001)));

    p = pico8_vec_qm(pico8_fixed_add(du, pico8_fixed_rnd_default()), a);
    v = pico8_vec_mul(p, py);

    if (pk) {
        v = pico8_vec_pa(v);
    }

    /* TROU-7 */
    particle = malloc(sizeof *particle);
    particle->p = p;
    particle->v = v;
    particle->has_a = 0;
    particle->a = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    particle->l = ki;

    return particle;
}

const pico8_str_seq_t *pico8_g__get_nu(const void *self) {
    return ((const pico8_g__t *)self)->nu;
}

int pico8_g__get_fa(const void *self, pico8_fixed_t *out) {
    *out = ((const pico8_g__t *)self)->fa;
    return 1;
}

const pico8_vec_t *pico8_g__get_ra(const void *self) {
    return ((const pico8_g__t *)self)->ra;
}

const pico8_vec_t *pico8_g__get_p(const void *self) {
    return ((const pico8_g__t *)self)->p;
}

/* `fx` is a fixed here, and a fixed is always true in Lua, even 0. */
int pico8_g__get_fx(const void *self) {
    (void)self;
    return 1;
}

int pico8_g__get_p_(const void *self) {
    const pico8_g__t *g_ = self;

    return g_->has_p_ && g_->p_;
}

static const pico8_struct_class_t g_g__struct_class = {
    .kj = pico8_ei_dispatch,
    .ev = pico8_dl_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .mm = pico8_g__mm,
    .emitter_of = pico8_g__emitter_of,
    .probe_of = pico8_g__probe_of,
    .get_nu = pico8_g__get_nu,
    .get_fa = pico8_g__get_fa,
    .get_ra = pico8_g__get_ra,
    .get_p = pico8_g__get_p,
    .get_fx = pico8_g__get_fx,
    .get_p_ = pico8_g__get_p_,
};

void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls) {
    pico8_ke_struct_init(&self->header, cls);
    self->emitter = &self->emitter_own;
    self->emitter->qy = NULL;
    self->ra = NULL;
    self->p = NULL;
    self->probe = &self->probe_own;
    self->probe->et = NULL;
    self->p_ = 0;
    self->has_p_ = 0;
    self->nu = pico8_g__start.nu;
    self->fx = pico8_g__start.fx;
    self->probe->dq = pico8_g__start.dq;
    self->probe->od = pico8_g__start.od;
    self->probe->has_od = 1;
    self->probe->fp = pico8_g__start.fp;
    self->probe->ik = 0;
    self->probe->has_ik = 0;
    self->fa = pico8_g__start.fa;
    self->probe->dw = pico8_g__start.dw;
    self->emitter->py = pico8_g__start.py;
    self->emitter->ki = pico8_g__start.ki;
    self->emitter->pn = pico8_g__start.pn;
    self->emitter->pk = pico8_g__start.pk;
    self->emitter->fc = pico8_g__start.fc;
    self->emitter->k_ = pico8_g__start.k_;
    self->emitter->qo = pico8_g__start.qo;
    self->emitter->ro = pico8_g__start.ro;
    self->emitter->qg = pico8_g__start.qg;
    self->emitter->q_ = pico8_g__start.q_;
    self->emitter->du = pico8_g__start.du;
}

static void *pico8_g__struct_new(const pico8_g__keys_t *keys) {
    pico8_g__t *self;

    assert(g_g__up && "pico8_probe_init() must run first");
    self = malloc(sizeof *self);
    pico8_g__struct_init(self, &g_g__struct_class);
    if (keys != NULL) {
        self->header.t = keys->t;
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
    pico8_class_coverage_record("g_");
#endif
    pico8_f_((pico8_struct_header_t *)self);
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
    g_fm = (pico8_struct_header_t *)&g_fm_placeholder;
}

static void fm_free(pico8_struct_header_t *fm) {
    if (fm == NULL || fm == (pico8_struct_header_t *)&g_fm_placeholder) {
        return;
    }
    assert(((const pico8_struct_header_t *)fm)->cls == pico8_hy_struct_class() &&
           "fm: pico8_probe_set_fm only ever stores a hy besides the provisional");
    free(fm);
}

void pico8_probe_shutdown(void) {
    g_g__up = 0;
    fm_free(g_fm);
    g_fm = NULL;
}

int pico8_g__ready(void) {
    return g_g__up;
}

const pico8_struct_class_t *pico8_g__struct_class(void) {
    return &g_g__struct_class;
}

pico8_g__t *pico8_g__of(pico8_struct_header_t *e) {
    const pico8_struct_class_t *cls = e != NULL ? e->cls : NULL;

    while (cls != NULL && cls != &g_g__struct_class) {
        cls = cls->parent != NULL ? cls->parent() : NULL;
    }
    return cls != NULL ? (pico8_g__t *)e : NULL;
}

pico8_emitter_t *pico8_g__emitter_of(void *self) {
    return ((pico8_g__t *)self)->emitter;
}

pico8_probe_t *pico8_g__probe_of(void *self) {
    return ((pico8_g__t *)self)->probe;
}

pico8_g__t *pico8_g__new(const pico8_g__keys_t *keys) {
    return pico8_g__struct_new(keys);
}

const pico8_struct_class_t *pico8_fm_placeholder_struct_class(void) {
    return &g_fm_placeholder_struct_class;
}

pico8_struct_header_t *pico8_fm(void) {
    return g_fm;
}

pico8_hy_t *pico8_fm_hy(void) {
    if (g_fm == NULL || g_fm == &g_fm_placeholder.header) {
        return NULL;
    }
    assert(g_fm->cls == pico8_hy_struct_class() && "fm: only the provisional or a hy is ever stored");
    return (pico8_hy_t *)g_fm;
}

const pico8_fm_placeholder_t *pico8_fm_placeholder(void) {
    return g_fm == &g_fm_placeholder.header ? &g_fm_placeholder : NULL;
}

void pico8_probe_set_fm(pico8_struct_header_t *instance) {
    if (g_fm == instance) {
        return;
    }
    fm_free(g_fm);
    g_fm = instance;
}

const pico8_struct_class_t *pico8_bv_et_struct_class(void) {
    return &g_bv_et_struct_class;
}
