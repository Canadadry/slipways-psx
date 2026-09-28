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
#include "pico8_ob.h"
#include "pico8_planet.h"
#include "pico8_probe.h"
#include "pico8_slipway_display.h"
#include "pico8_tech_gate.h"
#include "pico8_ui.h"
#include "pico8_value.h"
#include "pico8_vec.h"
#include "pico8_wormhole.h"
#include "pico8_ref_audit.h"

static int g_jc_up = 0;

typedef struct {
    pico8_struct_header_t header;
    const char *qs;
    pico8_jc_t *fo;
} pico8_jc_too_far_t;

static int pico8_jc_too_far_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_jc_too_far_t *too_far = self;

    if (strcmp(name, "qs") == 0) {
        if (too_far->qs == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(too_far->qs);
        return 1;
    }
    if (strcmp(name, "fo") == 0) {
        if (too_far->fo == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(too_far->fo);
        return 1;
    }
    return 0;
}

static void jc_too_far_take_jc(pico8_value_t value, pico8_jc_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)value.as.ref)->cls == pico8_jc_struct_class() &&
           "jc_too_far.fo: a write neither nil nor a jc does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

static int pico8_jc_too_far_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_jc_too_far_t *too_far = self;

    if (strcmp(name, "fo") == 0) {
        jc_too_far_take_jc(value, &too_far->fo);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_jc_too_far_struct_class = {
    {NULL},
    pico8_jc_too_far_field,
    pico8_jc_too_far_set_field,
};

/* The `{qs="-too far-"}` `jc:ce` returns beyond range: never allocated,
 * never freed; `qs` is fixed, only `fo` is written, and the next `hq`
 * overwrites it. */
static pico8_jc_too_far_t g_jc_too_far = {{PICO8_TYPE_TAG_STRUCT, &g_jc_too_far_struct_class}, "-too far-", NULL};

/* The `{ra,p}` `jc:hq` hangs on `re` over nothing: a fresh one per call,
 * never freed, nor its `p`; `ra` is the caller's `mp` itself, not a copy. */
typedef struct {
    pico8_struct_header_t header;
    pico8_vec_t *ra;
    pico8_vec_t *p;
} pico8_jc_mouse_point_t;

static int pico8_jc_mouse_point_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_jc_mouse_point_t *point = self;

    if (strcmp(name, "ra") == 0) {
        if (point->ra == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(point->ra);
        return 1;
    }
    if (strcmp(name, "p") == 0) {
        if (point->p == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(point->p);
        return 1;
    }
    return 0;
}

static pico8_vec_t *pico8_jc_mouse_point_take_ra(pico8_value_t mp) {
    if (mp.kind == PICO8_VALUE_REF && mp.as.ref == NULL) {
        return NULL;
    }
    assert(mp.kind == PICO8_VALUE_REF && pico8_type_tag_of(mp.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "jc_mouse_point.ra: an mp neither nil nor a vector ref does not fit the member's type "
           "-- stop, do not widen the member");
    return mp.as.ref;
}

static const pico8_struct_class_t g_jc_mouse_point_struct_class = {
    {NULL},
    pico8_jc_mouse_point_field,
    NULL,
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

/* Readers of another entity, of either form: an absent field, a wrong
 * kind, or a NULL entity all read as the zero/false/NULL default. */
static pico8_value_t pico8_jc_entity_value(const void *e, const char *name) {
    pico8_value_t v;
    if (e != NULL && pico8_entity_get_field(e, name, &v)) {
        return v;
    }
    return pico8_value_ref(NULL);
}

static pico8_struct_header_t *pico8_jc_value_instance(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_struct_header_t *)v.as.ref;
    }
    return NULL;
}

static pico8_struct_header_t *pico8_jc_entity_instance(const void *e, const char *name) {
    return pico8_jc_value_instance(pico8_jc_entity_value(e, name));
}

static pico8_vec_t pico8_jc_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t pico8_jc_entity_vec(const void *e, const char *name) {
    return pico8_jc_value_vec(pico8_jc_entity_value(e, name));
}

static void jc_take_fixed(pico8_value_t value, pico8_fixed_t *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_FIXED &&
           "jc.mt: an mt read off qu or re neither nil nor a fixed does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.fixed;
    *has = 1;
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
    pico8_list_t *ts;

    if (self == NULL) {
        return;
    }

    pico8_rn(pico8_value_fixed(pico8_fixed_from_int(50)));

    ts = malloc(sizeof *ts);
    pico8_list_init(ts);
    self->ts = ts;

    self->hg_own.count = 0;
    self->hg = &self->hg_own;
}

void pico8_jc_hq(pico8_jc_t *self, pico8_value_t mp) {
    pico8_ui_t *ok;
    pico8_struct_header_t *hovered;
    pico8_value_t gv_val;
    pico8_value_t qu_mt;
    pico8_value_t mt_val;

    if (self == NULL) {
        return;
    }

    ok = pico8_ok();
    hovered = pico8_jc_entity_instance(ok, "o_");

    if (hovered != NULL && pico8_entity_get_field(hovered, "gv", &gv_val) && pico8_jc_truthy(gv_val)) {
        self->re = hovered;
    } else {
        pico8_jc_mouse_point_t *point = malloc(sizeof *point);
        pico8_vec_t *p_heap = malloc(sizeof *p_heap);
        point->header.tag = PICO8_TYPE_TAG_STRUCT;
        point->header.cls = &g_jc_mouse_point_struct_class;
        point->ra = pico8_jc_mouse_point_take_ra(mp);
        *p_heap = pico8_vec_div(pico8_jc_value_vec(mp), pico8_fixed_from_int(128));
        point->p = p_heap;
        self->re = (pico8_struct_header_t *)point;
    }

    self->od = pico8_dj(self->qu, self->re);
    self->has_od = 1;

    qu_mt = pico8_jc_entity_value(self->qu, "mt");
    mt_val = pico8_jc_truthy(qu_mt) ? qu_mt : pico8_jc_entity_value(self->re, "mt");
    jc_take_fixed(mt_val, &self->mt, &self->has_mt);

    self->et = pico8_jc_value_instance(pico8_jc_ce(self));
    if (self->et != NULL) {
        assert(pico8_type_tag_of(self->et) == PICO8_TYPE_TAG_STRUCT &&
               "jc:ce must yield an entity to hang `fo` on: a planet, a jc struct or the static `{qs}` struct");
        pico8_entity_set_field(self->et, "fo", pico8_value_ref(self));
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
        pico8_value_t discard;
        int qu_registered;
        int re_registered;
        qu_registered = pico8_mv(qu, "kf", 1, pico8_value_ref(self), pico8_value_ref(NULL), &discard);
        re_registered = pico8_mv(re, "kf", 1, pico8_value_ref(self), pico8_value_ref(NULL), &discard);
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
static int pico8_jc_ce_np_predicate(pico8_value_t e, void *ctx_ptr) {
    const pico8_jc_ce_np_ctx_t *ctx = (const pico8_jc_ce_np_ctx_t *)ctx_ptr;
    pico8_struct_header_t *n;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    n = (pico8_struct_header_t *)e.as.ref;
    return pico8_geom_be(ctx->sp, ctx->dp, pico8_jc_entity_vec(n, "p"), PICO8_FIXED_LIT(0x000000d1, 0.0032));
}

typedef struct {
    pico8_vec_t sp;
    pico8_vec_t dp;
    pico8_value_t self_mt;
} pico8_jc_ce_jc_ctx_t;

/* `self` is already registered in `cg("jc")` when this runs; `cl()` of a
 * segment against itself is always false, so no self-exclusion is needed. */
static int pico8_jc_ce_jc_predicate(pico8_value_t e, void *ctx_ptr) {
    const pico8_jc_ce_jc_ctx_t *ctx = (const pico8_jc_ce_jc_ctx_t *)ctx_ptr;
    const void *sw;
    pico8_value_t sw_mt;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    sw = e.as.ref;
    sw_mt = pico8_jc_entity_value(sw, "mt");
    if (!pico8_value_equal(sw_mt, ctx->self_mt)) {
        return 0;
    }
    {
        pico8_struct_header_t *sw_qu = pico8_jc_entity_instance(sw, "qu");
        pico8_struct_header_t *sw_re = pico8_jc_entity_instance(sw, "re");
        return pico8_geom_cl(ctx->sp, ctx->dp, pico8_jc_entity_vec(sw_qu, "p"), pico8_jc_entity_vec(sw_re, "p"));
    }
}

pico8_value_t pico8_jc_ce(pico8_jc_t *self) {
    pico8_struct_header_t *qu;
    pico8_struct_header_t *re;
    pico8_vec_t sp;
    pico8_vec_t dp;
    pico8_value_t ov_result;
    pico8_fixed_t threshold;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    qu = self->qu;
    re = self->re;
    sp = pico8_jc_entity_vec(qu, "p");
    dp = pico8_jc_entity_vec(re, "p");

    ov_result = pico8_ov(pico8_value_string("space folding"), pico8_value_fixed(PICO8_FIXED_LIT(0x00008000, 0.5)),
                          pico8_value_fixed(PICO8_FIXED_LIT(0x0001147a, 1.08)));
    threshold = (ov_result.kind == PICO8_VALUE_FIXED) ? ov_result.as.fixed : pico8_fixed_from_int(0);

    if (pico8_vec_len_sq(pico8_vec_sub(sp, dp)) > threshold) {
        return pico8_value_ref(&g_jc_too_far);
    }

    {
        pico8_value_t oe_val = pico8_jc_entity_value(qu, "oe");
        if (oe_val.kind == PICO8_VALUE_REF && oe_val.as.ref != NULL) {
            pico8_list_iter_t it = pico8_list_all(PICO8_REF_AS_LIST_CONST(oe_val.as.ref));
            pico8_value_t w_val;
            while (pico8_list_iter_next(&it, &w_val)) {
                pico8_struct_header_t *w;
                pico8_value_t eb_result;
                if (w_val.kind != PICO8_VALUE_REF || w_val.as.ref == NULL) {
                    continue;
                }
                assert(pico8_type_tag_of(w_val.as.ref) == PICO8_TYPE_TAG_STRUCT &&
                       ((const pico8_struct_header_t *)w_val.as.ref)->cls == pico8_jc_struct_class() &&
                       "qu.oe must hold jc structs: only jo:kf adds to it, and only slipways");
                w = (pico8_struct_header_t *)w_val.as.ref;
                if (pico8_mv(w, "eb", 1, pico8_value_ref(qu), pico8_value_ref(NULL), &eb_result) &&
                    pico8_value_equal(eb_result, pico8_value_ref(re))) {
                    return w_val;
                }
            }
        }
    }

    {
        pico8_jc_ce_np_ctx_t np_ctx;
        pico8_jc_ce_jc_ctx_t jc_ctx;
        pico8_list_t part1;
        pico8_list_t part2;
        pico8_list_t ds;
        pico8_value_t result = pico8_value_ref(NULL);

        np_ctx.sp = sp;
        np_ctx.dp = dp;
        jc_ctx.sp = sp;
        jc_ctx.dp = dp;
        jc_ctx.self_mt = self->has_mt ? pico8_value_fixed(self->mt) : pico8_value_ref(NULL);

        part1 = pico8_jp(pico8_cg("np"), pico8_jc_ce_np_predicate, &np_ctx);
        part2 = pico8_jp(pico8_cg("jc"), pico8_jc_ce_jc_predicate, &jc_ctx);
        ds = pico8_la(&part1, &part2);
        pico8_list_free(&part1);
        pico8_list_free(&part2);

        if (ds.count > 0) {
            result = ds.items[0];
        }
        pico8_list_free(&ds);
        return result;
    }
}

pico8_value_t pico8_jc_eb(const pico8_jc_t *self, const pico8_struct_header_t *jo_instance) {
    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    /* `A and B or C`: a falsy `self.re` falls through to `self.qu`, so this
     * is not a plain ternary. */
    if (jo_instance == self->qu && self->re != NULL) {
        return pico8_value_ref(self->re);
    }
    return pico8_value_ref(self->qu);
}

void pico8_jc_ev(pico8_jc_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->qu != self->re) {
        pico8_dl(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_struct_header_t *)self);
    }
}

static pico8_value_t pico8_jc_hq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg1;
    pico8_jc_hq((pico8_jc_t *)self, arg0);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_jc_ey_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_struct_header_t *re = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        re = (pico8_struct_header_t *)arg0.as.ref;
    }
    pico8_jc_ey((pico8_jc_t *)self, re);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_jc_eb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const pico8_struct_header_t *jo_instance = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        jo_instance = (const pico8_struct_header_t *)arg0.as.ref;
    }
    return pico8_jc_eb((const pico8_jc_t *)self, jo_instance);
}

static pico8_value_t pico8_jc_ev_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_jc_ev((pico8_jc_t *)self);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_jc_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_jc_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jc_t *)self);
    return pico8_value_ref(NULL);
}

static int jc_ref_field(const void *ref, pico8_value_t *out_value) {
    if (ref == NULL) {
        return 0;
    }
    *out_value = pico8_value_ref((void *)ref);
    return 1;
}

static int jc_fixed_field(pico8_fixed_t x, int has, pico8_value_t *out_value) {
    if (!has) {
        return 0;
    }
    *out_value = pico8_value_fixed(x);
    return 1;
}

static int pico8_jc_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_jc_t *jc = self;

    if (strcmp(name, "nu") == 0) {
        return jc_ref_field(jc->nu, out_value);
    }
    if (strcmp(name, "fa") == 0) {
        return jc_fixed_field(jc->fa, 1, out_value);
    }
    if (strcmp(name, "fp") == 0) {
        *out_value = pico8_value_bool(jc->fp);
        return 1;
    }
    if (strcmp(name, "ik") == 0) {
        return jc_fixed_field(jc->ik, 1, out_value);
    }
    if (strcmp(name, "dw") == 0) {
        return jc_fixed_field(jc->dw, 1, out_value);
    }
    if (strcmp(name, "k_") == 0) {
        return jc_ref_field(jc->k_, out_value);
    }
    if (strcmp(name, "nh") == 0) {
        return jc_fixed_field(jc->nh, 1, out_value);
    }
    if (strcmp(name, "qu") == 0) {
        return jc_ref_field(jc->qu, out_value);
    }
    if (strcmp(name, "ts") == 0) {
        return jc_ref_field(jc->ts, out_value);
    }
    if (strcmp(name, "hg") == 0) {
        return jc_ref_field(jc->hg, out_value);
    }
    if (strcmp(name, "re") == 0) {
        return jc_ref_field(jc->re, out_value);
    }
    if (strcmp(name, "od") == 0) {
        return jc_fixed_field(jc->od, jc->has_od, out_value);
    }
    if (strcmp(name, "mt") == 0) {
        return jc_fixed_field(jc->mt, jc->has_mt, out_value);
    }
    if (strcmp(name, "et") == 0) {
        return jc_ref_field(jc->et, out_value);
    }
    if (strcmp(name, "fo") == 0) {
        return jc_ref_field(jc->fo, out_value);
    }
    if (strcmp(name, "p_") == 0) {
        if (!jc->has_p_) {
            return 0;
        }
        *out_value = pico8_value_bool(jc->p_);
        return 1;
    }
    return pico8_ke_field(self, name, out_value);
}

static int jc_is_endpoint_class(const pico8_struct_class_t *cls) {
    return cls->methods[PICO8_METHOD_LY] != NULL || cls == pico8_jo_struct_class();
}

static int jc_is_fo_holder_class(const pico8_struct_class_t *cls) {
    return cls->methods[PICO8_METHOD_HQ] == pico8_bv_dispatch || cls == pico8_jc_struct_class() ||
           cls == pico8_mz_struct_class();
}

static void jc_take_holder(pico8_value_t value, pico8_struct_header_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
           jc_is_fo_holder_class(((const pico8_struct_header_t *)value.as.ref)->cls) &&
           "jc.fo: a write neither nil nor what hangs fo on a slipway (an object whose hq runs bv: g_, fv, qh, nl, os, "
           "hj, pm, nr, fr, mx; an mz; another jc) does not fit the member's type -- stop, do not widen the member");
    *out = value.as.ref;
}

static void jc_take_bool(pico8_value_t value, int *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_BOOL &&
           "jc.p_: a write neither nil nor a boolean does not fit the member's type -- stop, do not widen the member");
    *out = value.as.boolean != 0;
    *has = 1;
}

/* `fo` (bv, of a probe, a planet or an mz, and jc:hq of another jc) and
 * `p_` (d_): the only names a generic writer ever writes on a `jc`. */
static int pico8_jc_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_jc_t *jc = self;

    if (strcmp(name, "fo") == 0) {
        jc_take_holder(value, &jc->fo);
        return 1;
    }
    if (strcmp(name, "p_") == 0) {
        jc_take_bool(value, &jc->p_, &jc->has_p_);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_jc_struct_class = {
    {
        [PICO8_METHOD_HQ] = pico8_jc_hq_dispatch,
        [PICO8_METHOD_EY] = pico8_jc_ey_dispatch,
        [PICO8_METHOD_EB] = pico8_jc_eb_dispatch,
        [PICO8_METHOD_EV] = pico8_jc_ev_dispatch,
        [PICO8_METHOD_KJ] = pico8_jc_kj_dispatch,
    },
    pico8_jc_field,
    pico8_jc_set_field,
};

static void *pico8_jc_struct_new(const pico8_jc_keys_t *keys) {
    pico8_jc_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->ke, &g_jc_struct_class);
    self->ke.me = pico8_jc_start.me;
    self->nu = pico8_jc_start.nu;
    self->fa = pico8_jc_start.fa;
    self->fp = pico8_jc_start.fp;
    self->ik = pico8_jc_start.ik;
    self->dw = pico8_jc_start.dw;
    self->k_ = pico8_jc_start.k_;
    self->nh = pico8_jc_start.nh;
    self->qu = NULL;
    self->ts = NULL;
    self->hg = NULL;
    self->re = NULL;
    self->od = 0;
    self->has_od = 0;
    self->mt = 0;
    self->has_mt = 0;
    self->et = NULL;
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
