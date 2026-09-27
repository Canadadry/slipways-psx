#include "pico8_slipway.h"

#include <assert.h>
#include <stddef.h>
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
#include "pico8_probe.h"
#include "pico8_slipway_display.h"
#include "pico8_tech_gate.h"
#include "pico8_ui.h"
#include "pico8_value.h"
#include "pico8_vec.h"
#include "pico8_ref_audit.h"

static pico8_ob_table_t *g_jc_spec = NULL;
static pico8_ob_table_t *g_jc_nu = NULL;
static pico8_ob_table_t *g_jc_k_ = NULL;

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
           "jc_too_far.fo: a write neither nil nor a jc is off its census row (ref:pico8_jc_t) -- a field under no "
           "rule of plan-3 section 5: stop, do not widen the member");
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
           "jc_mouse_point.ra: an mp neither nil nor a vector ref is off its census row (ref:autre) -- a field under "
           "no rule of plan-3 section 5: stop, do not widen the member");
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

void pico8_jc_pq(pico8_jc_t *self) {
    pico8_list_t *ts;
    pico8_ob_table_t *hg;

    if (self == NULL) {
        return;
    }

    pico8_rn(pico8_value_fixed(pico8_fixed_from_int(50)));

    ts = malloc(sizeof *ts);
    pico8_list_init(ts);
    self->ts = pico8_value_ref(ts);

    hg = pico8_ob_table_new();
    self->hg = pico8_value_ref(hg);
}

void pico8_jc_hq(pico8_jc_t *self, pico8_value_t mp) {
    pico8_ui_t *ok;
    pico8_struct_header_t *hovered;
    pico8_value_t gv_val;
    pico8_value_t re_val;
    pico8_struct_header_t *qu;
    pico8_struct_header_t *re;
    pico8_fixed_t od;
    pico8_value_t qu_mt;
    pico8_value_t mt_val;
    pico8_value_t et;

    if (self == NULL) {
        return;
    }

    ok = pico8_ok();
    hovered = pico8_jc_entity_instance(ok, "o_");

    if (hovered != NULL && pico8_entity_get_field(hovered, "gv", &gv_val) && pico8_jc_truthy(gv_val)) {
        re_val = pico8_value_ref(hovered);
    } else {
        pico8_jc_mouse_point_t *point = malloc(sizeof *point);
        pico8_vec_t *p_heap = malloc(sizeof *p_heap);
        point->header.tag = PICO8_TYPE_TAG_STRUCT;
        point->header.cls = &g_jc_mouse_point_struct_class;
        point->ra = pico8_jc_mouse_point_take_ra(mp);
        *p_heap = pico8_vec_div(pico8_jc_value_vec(mp), pico8_fixed_from_int(128));
        point->p = p_heap;
        PICO8_MEMBER_CENSUS_SEE(point, pico8_jc_mouse_point_t);
        re_val = pico8_value_ref(point);
    }
    self->re = re_val;

    qu = pico8_jc_value_instance(self->qu);
    re = (re_val.kind == PICO8_VALUE_REF) ? (pico8_struct_header_t *)re_val.as.ref : NULL;
    od = pico8_dj(qu, re);
    self->od = pico8_value_fixed(od);

    qu_mt = pico8_jc_entity_value(qu, "mt");
    mt_val = pico8_jc_truthy(qu_mt) ? qu_mt : pico8_jc_entity_value(re, "mt");
    self->mt = mt_val;

    et = pico8_jc_ce(self);
    self->et = et;
    if (pico8_jc_truthy(et) && et.kind == PICO8_VALUE_REF && et.as.ref != NULL) {
        assert(pico8_type_tag_of(et.as.ref) == PICO8_TYPE_TAG_STRUCT &&
               "jc:ce must yield an entity to hang `fo` on: a planet, a jc struct or the static `{qs}` struct");
        pico8_entity_set_field(et.as.ref, "fo", pico8_value_ref(self));
    }

    self->fa = pico8_jc_truthy(mt_val) ? pico8_value_fixed(pico8_fixed_from_int(2))
                                        : pico8_value_fixed(pico8_fixed_from_int(4));
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
        self->p_ = pico8_value_bool(1);
        return;
    }

    self->nh = pico8_value_fixed(pico8_fixed_from_int(1));
    self->re = pico8_value_ref(re);

    {
        pico8_struct_header_t *qu = pico8_jc_value_instance(self->qu);
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

    qu = pico8_jc_value_instance(self->qu);
    re = pico8_jc_value_instance(self->re);
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
        jc_ctx.self_mt = self->mt;

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
    pico8_struct_header_t *qu;
    pico8_value_t re_val;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    qu = pico8_jc_value_instance(self->qu);
    re_val = self->re;

    /* `A and B or C`: a falsy `self.re` falls through to `self.qu`, so this
     * is not a plain ternary. */
    if (jo_instance == qu && pico8_jc_truthy(re_val)) {
        return re_val;
    }
    return pico8_value_ref(qu);
}

void pico8_jc_ev(pico8_jc_t *self) {
    pico8_struct_header_t *qu;
    pico8_struct_header_t *re;

    if (self == NULL) {
        return;
    }

    qu = pico8_jc_value_instance(self->qu);
    re = pico8_jc_value_instance(self->re);
    if (qu != re) {
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

static int pico8_jc_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_jc_t *jc = self;
    static const struct {
        const char *name;
        size_t offset;
    } members[] = {
        {"nu", offsetof(pico8_jc_t, nu)}, {"fa", offsetof(pico8_jc_t, fa)}, {"fp", offsetof(pico8_jc_t, fp)},
        {"ik", offsetof(pico8_jc_t, ik)}, {"dw", offsetof(pico8_jc_t, dw)}, {"k_", offsetof(pico8_jc_t, k_)},
        {"nh", offsetof(pico8_jc_t, nh)}, {"qu", offsetof(pico8_jc_t, qu)}, {"ts", offsetof(pico8_jc_t, ts)},
        {"hg", offsetof(pico8_jc_t, hg)}, {"re", offsetof(pico8_jc_t, re)}, {"od", offsetof(pico8_jc_t, od)},
        {"mt", offsetof(pico8_jc_t, mt)}, {"et", offsetof(pico8_jc_t, et)}, {"fo", offsetof(pico8_jc_t, fo)},
        {"p_", offsetof(pico8_jc_t, p_)},
    };
    size_t i;

    for (i = 0; i < sizeof members / sizeof members[0]; i++) {
        if (strcmp(name, members[i].name) == 0) {
            pico8_value_t value = *(const pico8_value_t *)((const char *)jc + members[i].offset);
            if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
                return 0;
            }
            *out_value = value;
            return 1;
        }
    }
    return pico8_ke_field(self, name, out_value);
}

/* `fo` (bv, and jc:hq of another jc) and `p_` (d_): the only names a
 * generic writer ever writes on a `jc`. */
static int pico8_jc_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_jc_t *jc = self;

    if (strcmp(name, "fo") == 0) {
        jc->fo = value;
    } else if (strcmp(name, "p_") == 0) {
        jc->p_ = value;
    } else {
        return 0;
    }
    return 1;
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

static void *pico8_jc_struct_new(const pico8_ob_table_t *overrides) {
    pico8_jc_t *self = malloc(sizeof *self);
    int i;

    pico8_ke_struct_init(&self->ke, &g_jc_struct_class);
    self->ke.me = "kz";
    self->nu = pico8_value_ref(g_jc_nu);
    self->fa = pico8_value_fixed(pico8_fixed_from_int(2));
    self->fp = pico8_value_bool(0);
    self->ik = pico8_value_fixed(pico8_fixed_from_int(48));
    self->dw = pico8_value_fixed(PICO8_FIXED_LIT(0x00000106, 0.004));
    self->k_ = pico8_value_ref(g_jc_k_);
    self->nh = pico8_value_fixed(pico8_fixed_from_int(20));
    self->qu = pico8_value_ref(NULL);
    self->ts = pico8_value_ref(NULL);
    self->hg = pico8_value_ref(NULL);
    self->re = pico8_value_ref(NULL);
    self->od = pico8_value_ref(NULL);
    self->mt = pico8_value_ref(NULL);
    self->et = pico8_value_ref(NULL);
    self->fo = pico8_value_ref(NULL);
    self->p_ = pico8_value_ref(NULL);
    for (i = 0; overrides != NULL && i < overrides->count; i++) {
        pico8_ob_key_t key = overrides->entries[i].key;

        assert(key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "qu") == 0 &&
               "jc: the only measured construction key is qu (jo:ldown, nr:ldown); another would be a field "
               "under no rule of plan-3 section 5 -- stop, do not add a member");
        self->qu = overrides->entries[i].value;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("jc");
#endif
    pico8_jc_pq(self);
    pico8_ke_struct_pq(&self->ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_jc_t);
    return self;
}

static pico8_ob_table_t *pico8_jc_spec_table(const pico8_ob_table_t *spec, const char *name) {
    pico8_value_t v = pico8_value_ref(NULL);
    int found = pico8_ob_table_get_name(spec, name, &v);

    assert(found && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
           pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
           "jc's nu and k_ spec entries must parse to tables (the spec's trailing comma is load-bearing)");
    (void)found;
    return (pico8_ob_table_t *)v.as.ref;
}

void pico8_jc_init(void) {
    pico8_ob_table_t *spec;

    pico8_jc_shutdown();
    g_jc_spec = pico8_ob("\n"
                         "nu=o(\"jc\"),\n"
                         "me=\"kz\",\n"
                         "fa=2,\n"
                         "fp=f,ik=48,\n"
                         "dw=0.004,\n"
                         "k_=o(0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1),\n"
                         "nh=20,\n",
                         NULL);

    spec = pico8_ob("\nnu=o(\"jc\"),\nk_=o(0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1),\n", NULL);
    g_jc_nu = pico8_jc_spec_table(spec, "nu");
    g_jc_k_ = pico8_jc_spec_table(spec, "k_");
    pico8_ob_table_free(spec);
    PICO8_MEMBER_CENSUS_SEE(&g_jc_too_far, pico8_jc_too_far_t);
}

void pico8_jc_shutdown(void) {
    pico8_ob_table_free(g_jc_spec);
    g_jc_spec = NULL;
    pico8_ob_table_free(g_jc_nu);
    g_jc_nu = NULL;
    pico8_ob_table_free(g_jc_k_);
    g_jc_k_ = NULL;
}

int pico8_jc_ready(void) {
    return g_jc_spec != NULL;
}

int pico8_jc_default(const char *name, pico8_value_t *out_value) {
    return g_jc_spec != NULL &&
           (pico8_ob_table_get_name(g_jc_spec, name, out_value) || pico8_ke_default(name, out_value));
}

const pico8_struct_class_t *pico8_jc_struct_class(void) {
    return &g_jc_struct_class;
}

pico8_jc_t *pico8_jc_new(const pico8_ob_table_t *overrides) {
    return pico8_jc_struct_new(overrides);
}

const pico8_struct_class_t *pico8_jc_too_far_struct_class(void) {
    return &g_jc_too_far_struct_class;
}

const pico8_struct_class_t *pico8_jc_mouse_point_struct_class(void) {
    return &g_jc_mouse_point_struct_class;
}
