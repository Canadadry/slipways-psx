#include "pico8_economy_formulas.h"

#include <math.h>

#include "pico8_entity.h"
#include "pico8_game_data.h"
#include "pico8_stash.h"
#include "pico8_vec.h"

static int economy_truthy(pico8_value_t v) {
    return !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

static pico8_fixed_t fixed_max(pico8_fixed_t a, pico8_fixed_t b) {
    return (a > b) ? a : b;
}

static int field_truthy(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst == NULL || !pico8_instance_get_field(inst, name, &v)) {
        return 0;
    }
    return economy_truthy(v);
}

static pico8_vec_t field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static const pico8_ob_table_t *field_ref_table(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (const pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

static const pico8_list_t *field_list(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (const pico8_list_t *)v.as.ref;
    }
    return NULL;
}

static void set_by_value(pico8_ob_table_t *table, pico8_value_t key, pico8_value_t value) {
    if (table == NULL) {
        return;
    }
    if (key.kind == PICO8_VALUE_FIXED) {
        pico8_ob_table_set_index(table, (int)pico8_fixed_to_double(key.as.fixed), value);
    } else if (key.kind == PICO8_VALUE_STRING && key.as.string != NULL) {
        pico8_ob_table_set_name(table, key.as.string, value);
    }
}

static int get_by_value(const pico8_ob_table_t *table, pico8_value_t key, pico8_value_t *out) {
    if (table == NULL) {
        return 0;
    }
    if (key.kind == PICO8_VALUE_FIXED) {
        return pico8_ob_table_get_index(table, (int)pico8_fixed_to_double(key.as.fixed), out);
    } else if (key.kind == PICO8_VALUE_STRING && key.as.string != NULL) {
        return pico8_ob_table_get_name(table, key.as.string, out);
    }
    return 0;
}

typedef void (*cg_visit_fn)(pico8_instance_t *e, void *ctx);

static void for_each_cg(const char *tag, cg_visit_fn fn, void *ctx) {
    const pico8_list_t *list = pico8_cg(tag);
    pico8_list_iter_t it;
    pico8_value_t e;
    if (list == NULL) {
        return;
    }
    it = pico8_list_all(list);
    while (pico8_list_iter_next(&it, &e)) {
        if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
            fn((pico8_instance_t *)e.as.ref, ctx);
        }
    }
}

/* function cm(jm)local p={}
 * for _,qe in pairs(r)do
 * p[qe]=jm
 * end
 * return p
 * end
 * (pico8/lua.lua:1239-1244.) See pico8_economy_formulas.h's own top
 * comment on this declaration for the full trace. */
pico8_ob_table_t *pico8_cm(pico8_fixed_t jm) {
    const pico8_ob_table_t *r = pico8_r();
    pico8_ob_table_t *p = pico8_ob_table_new();
    int i;
    if (r != NULL) {
        for (i = 0; i < r->count; i++) {
            set_by_value(p, r->entries[i].value, pico8_value_fixed(jm));
        }
    }
    return p;
}

static void bx_visit(pico8_instance_t *pt, void *ctx) {
    pico8_ob_table_t *p = (pico8_ob_table_t *)ctx;
    const pico8_ob_table_t *kw;
    pico8_value_t qe_val;
    pico8_value_t mu_val;
    pico8_value_t current;
    pico8_fixed_t mu = pico8_fixed_from_int(0);

    if (!field_truthy(pt, "jy")) {
        return;
    }
    kw = field_ref_table(pt, "kw");
    if (kw == NULL || !pico8_ob_table_get_name(kw, "qe", &qe_val)) {
        return;
    }
    if (pico8_ob_table_get_name(kw, "mu", &mu_val) && mu_val.kind == PICO8_VALUE_FIXED) {
        mu = mu_val.as.fixed;
    }
    if (!get_by_value(p, qe_val, &current) || current.kind != PICO8_VALUE_FIXED) {
        current = pico8_value_fixed(pico8_fixed_from_int(0));
    }
    set_by_value(p, qe_val, pico8_value_fixed(pico8_fixed_add(current.as.fixed, mu)));
}

/* function bx()local p=cm(0)for pt in all(cg.jo)do
 * if pt.jy then
 * p[pt.kw.qe]+=pt.kw.mu
 * end
 * end
 * return p
 * end
 * (pico8/lua.lua:1245-1251.) See pico8_economy_formulas.h's own top
 * comment on this declaration for the full trace. */
pico8_ob_table_t *pico8_bx(void) {
    pico8_ob_table_t *p = pico8_cm(pico8_fixed_from_int(0));
    for_each_cg("jo", bx_visit, p);
    return p;
}

static void ck_visit(pico8_instance_t *l, void *ctx) {
    pico8_ob_table_t *d = (pico8_ob_table_t *)ctx;
    const pico8_list_t *jy = field_list(l, "jy");
    const pico8_ob_table_t *last;
    pico8_value_t last_val;
    pico8_value_t qe_val;
    pico8_value_t current;

    if (jy == NULL || jy->count == 0) {
        return;
    }
    last_val = jy->items[jy->count - 1];
    if (last_val.kind != PICO8_VALUE_REF || last_val.as.ref == NULL) {
        return;
    }
    last = (const pico8_ob_table_t *)last_val.as.ref;
    if (!pico8_ob_table_get_name(last, "qe", &qe_val)) {
        return;
    }
    if (!get_by_value(d, qe_val, &current) || current.kind != PICO8_VALUE_FIXED) {
        current = pico8_value_fixed(pico8_fixed_from_int(1));
    }
    set_by_value(d, qe_val, pico8_value_fixed(pico8_fixed_sub(current.as.fixed, pico8_fixed_from_double(0.15))));
}

/* function ck()local d=cm(1)for l in all(cg.qh)do
 * d[l.jy[#l.jy].qe]-=0.15
 * end
 * return d
 * end
 * (pico8/lua.lua:1252-1256.) See pico8_economy_formulas.h's own top
 * comment on this declaration for the full trace. */
pico8_ob_table_t *pico8_ck(void) {
    pico8_ob_table_t *d = pico8_cm(pico8_fixed_from_int(1));
    for_each_cg("qh", ck_visit, d);
    return d;
}

/* function dj(pb,to)local ec=
 * (#(to.p-pb.p)/bq)^0.75
 * return flr(bn*bk()*max(0.5,ec))end
 * (pico8/lua.lua:1259-1261.) See pico8_economy_formulas.h's own top
 * comment on this declaration for the full trace, including the `^0.75`
 * fractional-power numeric-fidelity note. */
pico8_fixed_t pico8_dj(const pico8_instance_t *pb, const pico8_instance_t *to) {
    pico8_vec_t diff = pico8_vec_sub(field_vec(to, "p"), field_vec(pb, "p"));
    pico8_fixed_t len_sq = pico8_vec_len_sq(diff);
    double ec_d = pow(pico8_fixed_to_double(len_sq) / pico8_fixed_to_double(pico8_bq()), 0.75);
    pico8_fixed_t ec = pico8_fixed_from_double(ec_d);
    pico8_fixed_t clamped = fixed_max(pico8_fixed_from_double(0.5), ec);
    return pico8_fixed_flr(pico8_fixed_mul(pico8_fixed_mul(pico8_bn(), pico8_bk()), clamped));
}

/* function bk()
 * return max(1,sqrt(#(cg.jc or {}))*0.27-0.1
 * )end
 * (pico8/lua.lua:1262-1264.) See pico8_economy_formulas.h's own top
 * comment on this declaration for the full trace. */
pico8_fixed_t pico8_bk(void) {
    const pico8_list_t *jc = pico8_cg("jc");
    int count = (jc != NULL) ? jc->count : 0;
    pico8_fixed_t sq = pico8_fixed_sqrt(pico8_fixed_from_int(count));
    pico8_fixed_t val = pico8_fixed_sub(pico8_fixed_mul(sq, pico8_fixed_from_double(0.27)), pico8_fixed_from_double(0.1));
    return fixed_max(pico8_fixed_from_int(1), val);
}
