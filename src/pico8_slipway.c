#include "pico8_slipway.h"

#include <stddef.h>
#include <stdlib.h>

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

/* Phase 3, Chunk 14 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): jc's own class declaration plus its "core lifecycle/dispatch"
 * methods (pq, hq, ey, ce, eb, ev). See pico8_slipway.h's own top comment
 * and each function's own comment there for the full Lua-source trace;
 * this file carries the port itself. pico8_jc_truthy/pico8_jc_field_*
 * are this module's own private copies of the same field-access/truthy
 * patterns already independently duplicated by pico8_probe.c/pico8_
 * planet.c/pico8_planet_trade.c/etc (documented, accepted duplication,
 * not a new pattern). */
static pico8_class_t *g_jc_class = NULL;

static int pico8_jc_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static pico8_value_t pico8_jc_field_value(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v)) {
        return v;
    }
    return pico8_value_ref(NULL);
}

static int pico8_jc_field_truthy(const pico8_instance_t *inst, const char *name) {
    return pico8_jc_truthy(pico8_jc_field_value(inst, name));
}

static pico8_instance_t *pico8_jc_field_instance(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v = pico8_jc_field_value(inst, name);
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_instance_t *)v.as.ref;
    }
    return NULL;
}

static pico8_vec_t pico8_jc_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t pico8_jc_field_vec(const pico8_instance_t *inst, const char *name) {
    return pico8_jc_value_vec(pico8_jc_field_value(inst, name));
}

void pico8_jc_pq(pico8_instance_t *self) {
    pico8_list_t *ts;
    pico8_ob_table_t *hg;

    if (self == NULL) {
        return;
    }

    pico8_rn(pico8_value_fixed(pico8_fixed_from_int(50)));

    ts = malloc(sizeof *ts);
    pico8_list_init(ts);
    pico8_ob_table_set_name(self->fields, "ts", pico8_value_ref(ts));

    hg = pico8_ob_table_new();
    pico8_ob_table_set_name(self->fields, "hg", pico8_value_ref(hg));
}

void pico8_jc_hq(pico8_instance_t *self, pico8_value_t mp) {
    pico8_instance_t *ok;
    pico8_instance_t *hovered;
    pico8_value_t re_val;
    pico8_instance_t *qu;
    pico8_instance_t *re;
    pico8_fixed_t od;
    pico8_value_t qu_mt;
    pico8_value_t mt_val;
    pico8_value_t et;

    if (self == NULL) {
        return;
    }

    ok = pico8_ok();
    hovered = pico8_jc_field_instance(ok, "o_");

    if (hovered != NULL && pico8_jc_field_truthy(hovered, "gv")) {
        re_val = pico8_value_ref(hovered);
    } else {
        pico8_instance_t *placeholder = malloc(sizeof *placeholder);
        pico8_vec_t *p_heap = malloc(sizeof *p_heap);
        placeholder->cls = NULL;
        placeholder->fields = pico8_ob_table_new();
        pico8_ob_table_set_name(placeholder->fields, "ra", mp);
        *p_heap = pico8_vec_div(pico8_jc_value_vec(mp), pico8_fixed_from_int(128));
        pico8_ob_table_set_name(placeholder->fields, "p", pico8_value_ref(p_heap));
        re_val = pico8_value_ref(placeholder);
    }
    pico8_ob_table_set_name(self->fields, "re", re_val);

    qu = pico8_jc_field_instance(self, "qu");
    re = (re_val.kind == PICO8_VALUE_REF) ? (pico8_instance_t *)re_val.as.ref : NULL;
    od = pico8_dj(qu, re);
    pico8_ob_table_set_name(self->fields, "od", pico8_value_fixed(od));

    qu_mt = pico8_jc_field_value(qu, "mt");
    mt_val = pico8_jc_truthy(qu_mt) ? qu_mt : pico8_jc_field_value(re, "mt");
    pico8_ob_table_set_name(self->fields, "mt", mt_val);

    et = pico8_jc_ce(self);
    pico8_ob_table_set_name(self->fields, "et", et);
    if (pico8_jc_truthy(et) && et.kind == PICO8_VALUE_REF && et.as.ref != NULL) {
        pico8_instance_t *et_inst = (pico8_instance_t *)et.as.ref;
        pico8_ob_table_set_name(et_inst->fields, "fo", pico8_value_ref(self));
    }

    pico8_ob_table_set_name(self->fields, "fa",
                             pico8_jc_truthy(mt_val) ? pico8_value_fixed(pico8_fixed_from_int(2))
                                                      : pico8_value_fixed(pico8_fixed_from_int(4)));
}

void pico8_jc_ey(pico8_instance_t *self, pico8_instance_t *re) {
    int should_fail;

    if (self == NULL) {
        return;
    }

    should_fail = (re == NULL) || !pico8_jc_truthy(pico8_d_(self));
    if (should_fail) {
        /* stat(19)==50: pico8_stat's own dead-branch fallback (see this
         * file's own header comment on pico8_jc_ey) -- always 0 for index
         * 19, so this is dead-but-compiled, same treatment as the branch
         * itself. pico8_stat, like pico8_btn, requires a non-NULL `input`
         * (the CALLER guards, matching pico8_qb_ph's/pico8_nf_ph's own
         * established "if (input==NULL) return/skip" precedent, pico8_
         * scroll.c/pico8_mouse_reader.c) -- pico8_dispatch_input() is
         * still NULL before dg()/_update60() (Chunk 22) ever sets a real
         * input, so this guard is genuinely load-bearing today, not
         * speculative. */
        const pico8_input_t *input = pico8_dispatch_input();
        if (input != NULL && pico8_stat(input, 19) == 50) {
            pico8_rn(pico8_value_fixed(pico8_fixed_from_int(-1)));
        }
        pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(1));
        return;
    }

    pico8_ob_table_set_name(self->fields, "nh", pico8_value_fixed(pico8_fixed_from_int(1)));
    pico8_ob_table_set_name(self->fields, "re", pico8_value_ref(re));

    {
        pico8_instance_t *qu = pico8_jc_field_instance(self, "qu");
        pico8_value_t discard;
        pico8_mv(qu, "kf", 1, pico8_value_ref(self), pico8_value_ref(NULL), &discard);
        pico8_mv(re, "kf", 1, pico8_value_ref(self), pico8_value_ref(NULL), &discard);
        /* fm:nx(1) -- a direct call, NOT pico8_mv(): hy (Chunk 12) is
         * never subclassed anywhere in the real cart, so there is no
         * runtime-polymorphic receiver to resolve here, unlike qu:kf(self)/
         * re:kf(self) above (a real jo/mx-family ambiguity). Matches
         * pico8_economy.h's own explicit, documented convention for every
         * hy method ("none ... are ever called via mv()/dc() ... ported as
         * direct, known C functions") -- see pico8_probe.c's own pico8_d_
         * for the CORRECTION note on the identical fm:lu/fm:nx pattern this
         * chunk's own reading of pico8_economy.h surfaced and fixed there
         * too (Chunk 10's own pico8_d_ had stale pico8_mv() routing left
         * over from before hy existed). */
        pico8_hy_nx(pico8_fm(), pico8_fixed_from_int(1));
    }
}

typedef struct {
    pico8_vec_t sp;
    pico8_vec_t dp;
} pico8_jc_ce_np_ctx_t;

static int pico8_jc_ce_np_predicate(pico8_value_t e, void *ctx_ptr) {
    const pico8_jc_ce_np_ctx_t *ctx = (const pico8_jc_ce_np_ctx_t *)ctx_ptr;
    pico8_instance_t *n;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    n = (pico8_instance_t *)e.as.ref;
    return pico8_geom_be(ctx->sp, ctx->dp, pico8_jc_field_vec(n, "p"), pico8_fixed_from_double(0.0032));
}

typedef struct {
    pico8_vec_t sp;
    pico8_vec_t dp;
    pico8_value_t self_mt;
} pico8_jc_ce_jc_ctx_t;

static int pico8_jc_ce_jc_predicate(pico8_value_t e, void *ctx_ptr) {
    const pico8_jc_ce_jc_ctx_t *ctx = (const pico8_jc_ce_jc_ctx_t *)ctx_ptr;
    pico8_instance_t *sw;
    pico8_value_t sw_mt;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    sw = (pico8_instance_t *)e.as.ref;
    sw_mt = pico8_jc_field_value(sw, "mt");
    if (!pico8_value_equal(sw_mt, ctx->self_mt)) {
        return 0;
    }
    {
        pico8_instance_t *sw_qu = pico8_jc_field_instance(sw, "qu");
        pico8_instance_t *sw_re = pico8_jc_field_instance(sw, "re");
        return pico8_geom_cl(ctx->sp, ctx->dp, pico8_jc_field_vec(sw_qu, "p"), pico8_jc_field_vec(sw_re, "p"));
    }
}

pico8_value_t pico8_jc_ce(pico8_instance_t *self) {
    pico8_instance_t *qu;
    pico8_instance_t *re;
    pico8_vec_t sp;
    pico8_vec_t dp;
    pico8_value_t ov_result;
    pico8_fixed_t threshold;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    qu = pico8_jc_field_instance(self, "qu");
    re = pico8_jc_field_instance(self, "re");
    sp = pico8_jc_field_vec(qu, "p");
    dp = pico8_jc_field_vec(re, "p");

    ov_result = pico8_ov(pico8_value_string("space folding"), pico8_value_fixed(pico8_fixed_from_double(0.5)),
                          pico8_value_fixed(pico8_fixed_from_double(1.08)));
    threshold = (ov_result.kind == PICO8_VALUE_FIXED) ? ov_result.as.fixed : pico8_fixed_from_int(0);

    if (pico8_vec_len_sq(pico8_vec_sub(sp, dp)) > threshold) {
        pico8_instance_t *placeholder = malloc(sizeof *placeholder);
        placeholder->cls = NULL;
        placeholder->fields = pico8_ob_table_new();
        pico8_ob_table_set_name(placeholder->fields, "qs", pico8_value_string("-too far-"));
        return pico8_value_ref(placeholder);
    }

    {
        pico8_value_t oe_val = pico8_jc_field_value(qu, "oe");
        if (oe_val.kind == PICO8_VALUE_REF && oe_val.as.ref != NULL) {
            pico8_list_iter_t it = pico8_list_all((const pico8_list_t *)oe_val.as.ref);
            pico8_value_t w_val;
            while (pico8_list_iter_next(&it, &w_val)) {
                pico8_instance_t *w;
                pico8_value_t eb_result;
                if (w_val.kind != PICO8_VALUE_REF || w_val.as.ref == NULL) {
                    continue;
                }
                w = (pico8_instance_t *)w_val.as.ref;
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
        jc_ctx.self_mt = pico8_jc_field_value(self, "mt");

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

pico8_value_t pico8_jc_eb(const pico8_instance_t *self, const pico8_instance_t *jo_instance) {
    pico8_instance_t *qu;
    pico8_value_t re_val;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    qu = pico8_jc_field_instance(self, "qu");
    re_val = pico8_jc_field_value(self, "re");

    if (jo_instance == qu && pico8_jc_truthy(re_val)) {
        return re_val;
    }
    return pico8_value_ref(qu);
}

void pico8_jc_ev(pico8_instance_t *self) {
    pico8_instance_t *qu;
    pico8_instance_t *re;

    if (self == NULL) {
        return;
    }

    qu = pico8_jc_field_instance(self, "qu");
    re = pico8_jc_field_instance(self, "re");
    if (qu != re) {
        pico8_dl(pico8_dispatch_gfx(), pico8_dispatch_mem(), self);
    }
}

static pico8_value_t pico8_jc_hq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg1;
    pico8_jc_hq((pico8_instance_t *)self, arg0);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_jc_ey_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *re = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        re = (pico8_instance_t *)arg0.as.ref;
    }
    pico8_jc_ey((pico8_instance_t *)self, re);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_jc_eb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const pico8_instance_t *jo_instance = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        jo_instance = (const pico8_instance_t *)arg0.as.ref;
    }
    return pico8_jc_eb((const pico8_instance_t *)self, jo_instance);
}

static pico8_value_t pico8_jc_ev_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_jc_ev((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_jc_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_jc_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_jc_init(void) {
    pico8_jc_shutdown();
    g_jc_class = pico8_class_declare(pico8_ke_class(), "\n"
                                                        "nu=o(\"jc\"),\n"
                                                        "me=\"kz\",\n"
                                                        "fa=2,\n"
                                                        "fp=f,ik=48,\n"
                                                        "dw=0.004,\n"
                                                        "k_=o(0,0,7,7,10,10,9,9,9,9,9,9,9,9,4,4,4,2,2,1),\n"
                                                        "nh=20,\n");
    pico8_class_set_pq(g_jc_class, pico8_jc_pq);
    pico8_ob_table_set_name(g_jc_class->defaults, "hq", pico8_value_function(pico8_jc_hq_dispatch));
    pico8_ob_table_set_name(g_jc_class->defaults, "ey", pico8_value_function(pico8_jc_ey_dispatch));
    pico8_ob_table_set_name(g_jc_class->defaults, "eb", pico8_value_function(pico8_jc_eb_dispatch));
    pico8_ob_table_set_name(g_jc_class->defaults, "ev", pico8_value_function(pico8_jc_ev_dispatch));
    pico8_ob_table_set_name(g_jc_class->defaults, "kj", pico8_value_function(pico8_jc_kj_dispatch));
    pico8_class_register_named("jc", g_jc_class);
}

void pico8_jc_shutdown(void) {
    if (g_jc_class != NULL) {
        pico8_class_free(g_jc_class);
        g_jc_class = NULL;
    }
}

pico8_class_t *pico8_jc_class(void) {
    return g_jc_class;
}

pico8_instance_t *pico8_jc_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_jc_class, overrides);
}
