#include "pico8_slipway.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_economy_formulas.h"
#include "pico8_entity.h"
#include "pico8_geom.h"
#include "pico8_input.h"
#include "pico8_ke.h"
#include "pico8_planet.h"
#include "pico8_probe.h"
#include "pico8_slipway_display.h"
#include "pico8_tech_gate.h"
#include "pico8_ui.h"
#include "pico8_value.h"
#include "pico8_vec.h"
#include "pico8_wormhole.h"

static int g_jc_up = 0;

static const pico8_struct_class_t g_jc_too_far_struct_class = {0};

/* The `{qs="-too far-"}` `jc:ce` returns beyond range: never allocated,
 * never freed; `qs` is fixed, only `fo` is written, and the next `hq`
 * overwrites it. */
static pico8_jc_too_far_t g_jc_too_far = {{PICO8_TYPE_TAG_STRUCT, &g_jc_too_far_struct_class}, "-too far-", NULL};

static const pico8_vec_t *jc_mouse_point_get_ra(const void *self) {
    return ((const pico8_jc_mouse_point_t *)self)->ra;
}

static const pico8_vec_t *jc_mouse_point_get_p(const void *self) {
    return ((const pico8_jc_mouse_point_t *)self)->p;
}

static const pico8_struct_class_t g_jc_mouse_point_struct_class = {
    .get_ra = jc_mouse_point_get_ra,
    .get_p = jc_mouse_point_get_p,
};

/* Lua truthiness: only `false` and nil are falsy -- a fixed 0 is truthy. */
static int pico8_jc_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static pico8_struct_header_t *pico8_jc_value_instance(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_struct_header_t *)v.as.ref;
    }
    return NULL;
}

static const pico8_jc_mouse_point_t *jc_end_point(const pico8_struct_header_t *end) {
    assert(end->cls == &g_jc_mouse_point_struct_class &&
           "jc: an end of a slipway is a planet or the mouse point jc:hq hangs on re -- stop");
    return (const pico8_jc_mouse_point_t *)end;
}

pico8_vec_t pico8_jc_end_p(const pico8_struct_header_t *end) {
    const pico8_vec_t *p = NULL;

    if (end != NULL) {
        p = pico8_jo_is_family(end) ? ((const pico8_jo_t *)end)->p : jc_end_point(end)->p;
    }
    return p != NULL ? *p : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

pico8_vec_t pico8_jc_end_ra(const pico8_struct_header_t *end) {
    const pico8_vec_t *ra = NULL;

    if (end != NULL) {
        ra = pico8_jo_is_family(end) ? ((const pico8_jo_t *)end)->ra : jc_end_point(end)->ra;
    }
    return ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static int jc_mt_of(const pico8_struct_header_t *e, pico8_fixed_t *out) {
    return e != NULL && e->cls->get_mt != NULL && e->cls->get_mt(e, out);
}

void pico8_jc_hg_add(pico8_jc_hg_t *hg, pico8_value_t id) {
    assert(id.kind == PICO8_VALUE_STRING && id.as.string != NULL &&
           "jc.hg: an id that is not a string does not fit the member's type -- stop, do not widen the member");
    if (pico8_jc_hg_has(hg, id)) {
        return;
    }
    assert(hg->count < PICO8_JC_HG_CAP && "jc.hg: more planets than bw(90) makes -- stop, do not widen the member");
    hg->ids[hg->count] = id.as.string;
    hg->count++;
}

int pico8_jc_hg_has(const pico8_jc_hg_t *hg, pico8_value_t id) {
    int i;

    if (hg == NULL || id.kind != PICO8_VALUE_STRING || id.as.string == NULL) {
        return 0;
    }
    for (i = 0; i < hg->count; i++) {
        if (strcmp(hg->ids[i], id.as.string) == 0) {
            return 1;
        }
    }
    return 0;
}

void pico8_jc_pq(pico8_jc_t *self) {
    pico8_ptrs_t *ts;

    if (self == NULL) {
        return;
    }

    pico8_rn(pico8_value_fixed(pico8_fixed_from_int(50)));

    ts = malloc(sizeof *ts);
    pico8_ptrs_init(ts);
    self->ts = ts;

    self->hg_own.count = 0;
    self->hg = &self->hg_own;
}

void pico8_jc_hq(pico8_jc_t *self, pico8_vec_t *mp) {
    pico8_ui_t *ok;
    pico8_struct_header_t *hovered;

    if (self == NULL) {
        return;
    }

    ok = pico8_ok();
    hovered = ok != NULL ? ok->o_ : NULL;

    if (hovered != NULL && hovered->cls->gv != NULL) {
        self->re = hovered;
    } else {
        pico8_jc_mouse_point_t *point = malloc(sizeof *point);
        pico8_vec_t *p_heap = malloc(sizeof *p_heap);
        point->header.tag = PICO8_TYPE_TAG_STRUCT;
        point->header.cls = &g_jc_mouse_point_struct_class;
        point->ra = mp;
        *p_heap = pico8_vec_div(mp != NULL ? *mp : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0)),
                                pico8_fixed_from_int(128));
        point->p = p_heap;
        self->re = (pico8_struct_header_t *)point;
    }

    self->probe->od = pico8_dj(self->qu, self->re);
    self->probe->has_od = 1;

    /* A fixed is always true in Lua: `qu.mt or re.mt` takes the first present. */
    self->mt = pico8_fixed_from_int(0);
    self->has_mt = jc_mt_of(self->qu, &self->mt) || jc_mt_of(self->re, &self->mt);

    self->probe->et = pico8_jc_value_instance(pico8_jc_ce(self));
    self->probe->has_et = self->probe->et != NULL;
    if (self->probe->et != NULL) {
        assert(pico8_type_tag_of(self->probe->et) == PICO8_TYPE_TAG_STRUCT &&
               "jc:ce must yield an entity to hang `fo` on: a planet, a jc struct or the static `{qs}` struct");
        pico8_probe_et_hang_fo(self->probe->et, &self->ke.header);
    }

    self->fa = self->has_mt ? pico8_fixed_from_int(2) : pico8_fixed_from_int(4);
}

void pico8_jc_ey(pico8_jc_t *self, pico8_struct_header_t *re) {
    int should_fail;

    if (self == NULL) {
        return;
    }

    /* `d_()` charges the player and advances the clock, so it must stay
     * behind the `re == NULL` short-circuit. */
    should_fail = (re == NULL) || !pico8_jc_truthy(pico8_d_((pico8_struct_header_t *)self));
    if (should_fail) {
        /* pico8_stat has no case for 19, so it reads 0 and this sfx never
         * fires; ported anyway. It dereferences `input` without checking it,
         * so the NULL guard below is load-bearing. */
        const pico8_input_t *input = pico8_dispatch_input();
        if (input != NULL && pico8_stat(input, 19) == 50) {
            pico8_rn(pico8_value_fixed(pico8_fixed_from_int(-1)));
        }
        self->p_ = 1;
        self->has_p_ = 1;
        return;
    }

    self->nh = pico8_fixed_from_int(1);
    self->re = re;

    {
        pico8_struct_header_t *qu = self->qu;
        int qu_registered = qu != NULL && qu->cls->kf != NULL;
        int re_registered;
        if (qu_registered) {
            qu->cls->kf(qu, &self->ke.header);
        }
        re_registered = re != NULL && re->cls->kf != NULL;
        if (re_registered) {
            re->cls->kf(re, &self->ke.header);
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
static int pico8_jc_ce_np_predicate(void *e, void *ctx_ptr) {
    const pico8_jc_ce_np_ctx_t *ctx = (const pico8_jc_ce_np_ctx_t *)ctx_ptr;
    pico8_struct_header_t *n;

    if (e == NULL) {
        return 0;
    }
    n = e;
    return pico8_geom_be(ctx->sp, ctx->dp, pico8_jc_end_p(n), PICO8_FIXED_LIT(0x000000d1, 0.0032));
}

typedef struct {
    pico8_vec_t sp;
    pico8_vec_t dp;
    int self_has_mt;
    pico8_fixed_t self_mt;
} pico8_jc_ce_jc_ctx_t;

/* `self` is already registered in `cg("jc")` when this runs; `cl()` of a
 * segment against itself is always false, so no self-exclusion is needed. */
static int pico8_jc_ce_jc_predicate(void *e, void *ctx_ptr) {
    const pico8_jc_ce_jc_ctx_t *ctx = (const pico8_jc_ce_jc_ctx_t *)ctx_ptr;
    const pico8_jc_t *sw;

    if (e == NULL) {
        return 0;
    }
    assert(((const pico8_struct_header_t *)e)->cls == pico8_jc_struct_class() && "cg.jc holds slipways only");
    sw = e;
    if (sw->has_mt != ctx->self_has_mt || (sw->has_mt && sw->mt != ctx->self_mt)) {
        return 0;
    }
    return pico8_geom_cl(ctx->sp, ctx->dp, pico8_jc_end_p(sw->qu), pico8_jc_end_p(sw->re));
}

pico8_value_t pico8_jc_ce(pico8_jc_t *self) {
    pico8_struct_header_t *qu;
    pico8_struct_header_t *re;
    pico8_vec_t sp;
    pico8_vec_t dp;
    pico8_fixed_t threshold;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    qu = self->qu;
    re = self->re;
    sp = pico8_jc_end_p(qu);
    dp = pico8_jc_end_p(re);

    threshold = pico8_ov("space folding", PICO8_FIXED_LIT(0x00008000, 0.5), PICO8_FIXED_LIT(0x0001147a, 1.08));

    if (pico8_vec_len_sq(pico8_vec_sub(sp, dp)) > threshold) {
        return pico8_value_ref(&g_jc_too_far);
    }

    {
        const pico8_ptrs_t *oe = NULL;

        if (qu != NULL) {
            assert(pico8_jo_is_family(qu) && "jc.qu is a planet: the constructor asserts it");
            oe = ((const pico8_jo_t *)qu)->oe;
        }
        if (oe != NULL) {
            pico8_ptrs_iter_t it = pico8_ptrs_all(oe);
            void *w_val;
            while (pico8_ptrs_iter_next(&it, &w_val)) {
                pico8_struct_header_t *w;
                if (w_val == NULL) {
                    continue;
                }
                assert(pico8_type_tag_of(w_val) == PICO8_TYPE_TAG_STRUCT &&
                       ((const pico8_struct_header_t *)w_val)->cls == pico8_jc_struct_class() &&
                       "qu.oe must hold jc structs: only jo:kf adds to it, and only slipways");
                w = w_val;
                if (pico8_jc_eb((const pico8_jc_t *)w, qu) == re) {
                    return pico8_value_ref(w);
                }
            }
        }
    }

    {
        pico8_jc_ce_np_ctx_t np_ctx;
        pico8_jc_ce_jc_ctx_t jc_ctx;
        pico8_ptrs_t part1;
        pico8_ptrs_t part2;
        pico8_ptrs_t ds;
        pico8_value_t result = pico8_value_ref(NULL);

        np_ctx.sp = sp;
        np_ctx.dp = dp;
        jc_ctx.sp = sp;
        jc_ctx.dp = dp;
        jc_ctx.self_has_mt = self->has_mt;
        jc_ctx.self_mt = self->mt;

        part1 = pico8_jp_ptrs(pico8_cg("np"), pico8_jc_ce_np_predicate, &np_ctx);
        part2 = pico8_jp_ptrs(pico8_cg("jc"), pico8_jc_ce_jc_predicate, &jc_ctx);
        ds = pico8_la_ptrs(&part1, &part2);
        pico8_ptrs_free(&part1);
        pico8_ptrs_free(&part2);

        if (ds.count > 0) {
            result = pico8_value_ref(ds.items[0]);
        }
        pico8_ptrs_free(&ds);
        return result;
    }
}

pico8_struct_header_t *pico8_jc_eb(const pico8_jc_t *self, const pico8_struct_header_t *jo_instance) {
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
        pico8_dl(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_struct_header_t *)self);
    }
}

static void pico8_jc_hq_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    (void)ra;
    pico8_jc_hq((pico8_jc_t *)self, qa);
}

static void pico8_jc_ey_dispatch(void *self, pico8_struct_header_t *kv) {
    pico8_jc_ey((pico8_jc_t *)self, kv);
}

static void pico8_jc_ev_dispatch(void *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_jc_ev((pico8_jc_t *)self);
}

static void pico8_jc_kj_dispatch(void *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_jc_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jc_t *)self);
}

static int jc_is_endpoint_class(const pico8_struct_class_t *cls) {
    return cls->states[PICO8_STATE_LY] != NULL || cls == pico8_jo_struct_class();
}

static pico8_probe_t *jc_probe_of(void *self) {
    return ((pico8_jc_t *)self)->probe;
}

static const pico8_str_seq_t *jc_get_nu(const void *self) {
    return ((const pico8_jc_t *)self)->nu;
}

static int jc_get_fa(const void *self, pico8_fixed_t *out) {
    *out = ((const pico8_jc_t *)self)->fa;
    return 1;
}

static int jc_get_mt(const void *self, pico8_fixed_t *out) {
    const pico8_jc_t *jc = self;

    if (!jc->has_mt) {
        return 0;
    }
    *out = jc->mt;
    return 1;
}

static int jc_get_p_(const void *self) {
    const pico8_jc_t *jc = self;

    return jc->has_p_ && jc->p_;
}

static const pico8_struct_class_t g_jc_struct_class = {
    .kj = pico8_jc_kj_dispatch,
    .ev = pico8_jc_ev_dispatch,
    .hq = pico8_jc_hq_dispatch,
    .ey = pico8_jc_ey_dispatch,
    .probe_of = jc_probe_of,
    .get_nu = jc_get_nu,
    .get_fa = jc_get_fa,
    .get_mt = jc_get_mt,
    .get_p_ = jc_get_p_,
};

static void *pico8_jc_struct_new(const pico8_jc_keys_t *keys) {
    pico8_jc_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->ke, &g_jc_struct_class);
    self->ke.me = pico8_jc_start.me;
    self->nu = pico8_jc_start.nu;
    self->fa = pico8_jc_start.fa;
    self->probe = &self->probe_own;
    self->probe->dq = NULL;
    self->probe->fp = pico8_jc_start.fp;
    self->probe->fp_is_bool = 1;
    self->probe->ik = pico8_jc_start.ik;
    self->probe->has_ik = 1;
    self->probe->dw = pico8_jc_start.dw;
    self->k_ = pico8_jc_start.k_;
    self->nh = pico8_jc_start.nh;
    self->qu = NULL;
    self->ts = NULL;
    self->hg = NULL;
    self->re = NULL;
    self->probe->od = 0;
    self->probe->has_od = 0;
    self->mt = 0;
    self->has_mt = 0;
    self->probe->et = NULL;
    self->probe->has_et = 0;
    self->fo = NULL;
    self->p_ = 0;
    self->has_p_ = 0;
    if (keys != NULL && keys->qu != NULL) {
        assert(pico8_type_tag_of(keys->qu) == PICO8_TYPE_TAG_STRUCT && jc_is_endpoint_class(keys->qu->cls) &&
               "jc.qu: a key neither nil nor a planet a slipway starts from (jo, qh, nl, os, hj, pm, nr, fr, mx) "
               "does not fit the member's type -- stop, do not widen the member");
        self->qu = keys->qu;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("jc");
#endif
    pico8_jc_pq(self);
    pico8_ke_struct_pq(&self->ke);
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

const pico8_struct_class_t *pico8_jc_struct_class(void) {
    return &g_jc_struct_class;
}

pico8_jc_t *pico8_jc_new(const pico8_jc_keys_t *keys) {
    return pico8_jc_struct_new(keys);
}

const pico8_struct_class_t *pico8_jc_too_far_struct_class(void) {
    return &g_jc_too_far_struct_class;
}

const pico8_struct_class_t *pico8_jc_mouse_point_struct_class(void) {
    return &g_jc_mouse_point_struct_class;
}
