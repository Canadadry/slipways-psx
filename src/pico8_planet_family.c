#include "pico8_planet_family.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_class.h"
#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_ob.h"
#include "pico8_particle.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_planet_display.h"
#include "pico8_probe.h"
#include "pico8_ref_audit.h"

static pico8_class_t *g_qh_class = NULL;
static pico8_class_t *g_nl_class = NULL;
static pico8_class_t *g_os_class = NULL;
static pico8_class_t *g_hj_class = NULL;
static pico8_class_t *g_pm_class = NULL;
static pico8_class_t *g_ga_class = NULL;

static int pico8_pf_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static pico8_fixed_t pico8_pf_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static pico8_vec_t pico8_pf_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t pico8_pf_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v)) {
        return pico8_pf_value_vec(v);
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_value_t pico8_qh_ly_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        t = arg0.as.fixed;
    }
    pico8_na((pico8_instance_t *)self, t);
    return pico8_value_ref(NULL);
}

void pico8_nl_ly(pico8_instance_t *self, pico8_fixed_t t) {
    pico8_value_t mi_val;
    int has_mi;
    pico8_fixed_t mi;

    if (self == NULL) {
        return;
    }
    pico8_na(self, t);

    has_mi = pico8_instance_get_field(self, "mi", &mi_val) && mi_val.kind == PICO8_VALUE_FIXED;
    assert(has_mi && "nl:ly reads self.mi unguarded, unlike hj:kj");
    mi = has_mi ? mi_val.as.fixed : pico8_fixed_from_int(0);
    if (mi >= pico8_fixed_from_int(2)) {
        pico8_fixed_t qw = pico8_fixed_add(
            pico8_fixed_from_int(8),
            pico8_fixed_div(pico8_fixed_mod(t, pico8_fixed_from_int(42)), pico8_fixed_from_int(6)));
        pico8_ob_table_set_name(self->fields, "qw", pico8_value_fixed(qw));
    }
}

static pico8_value_t pico8_nl_ly_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        t = arg0.as.fixed;
    }
    pico8_nl_ly((pico8_instance_t *)self, t);
    return pico8_value_ref(NULL);
}

void pico8_hj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p) {
    pico8_value_t mi_val;

    if (self == NULL) {
        return;
    }
    if (pico8_instance_get_field(self, "mi", &mi_val) && pico8_pf_truthy(mi_val) &&
        mi_val.kind == PICO8_VALUE_FIXED && mi_val.as.fixed >= pico8_fixed_from_int(2)) {
        pico8_ei(gfx, mem, self);
    }
    pico8_jo_kj(gfx, mem, self, p);
}

static pico8_value_t pico8_hj_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_hj_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self, p);
    return pico8_value_ref(NULL);
}

void pico8_pm_pq(pico8_instance_t *self) {
    pico8_vec_t ra;
    pico8_vec_t ng;
    pico8_vec_t spawn_ra;
    pico8_vec_t *spawn_ra_heap;
    pico8_ob_table_t *defaults;
    pico8_ob_table_t *overrides;
    pico8_instance_t *spawned;

    if (self == NULL) {
        return;
    }

    ra = pico8_pf_field_vec(self, "ra");
    ng = pico8_pf_field_vec(self, "ng");
    spawn_ra = pico8_vec_add(ra, ng);
    spawn_ra_heap = malloc(sizeof *spawn_ra_heap);
    *spawn_ra_heap = spawn_ra;

    defaults = pico8_ob_table_new();
    pico8_ob_table_set_name(defaults, "ra", pico8_value_ref(spawn_ra_heap));

    overrides = pico8_ob("\n"
                          "qo=0.97,ro=-0.1255,\n"
                          "qg=0.1255,q_=0.97,\n"
                          "k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1),\n"
                          "iu=-0.009,py=9,ms=1.5,\n",
                          defaults);
    pico8_ob_table_free(defaults);

    spawned = pico8_ga_new(overrides);
    (void)spawned;
    pico8_ob_table_free(overrides);
}

void pico8_ga_pq(pico8_instance_t *self) {
    pico8_instance_t *mo;

    if (self == NULL) {
        return;
    }
    pico8_f_(self);
    pico8_ez(self, pico8_fixed_from_int(140));

    mo = pico8_mo();
    if (mo != NULL) {
        pico8_entity_set_field(mo, "ma", pico8_value_fixed(pico8_fixed_from_int(3)));
    }
}

pico8_value_t pico8_ga_mm(void *self_ptr, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *self = (pico8_instance_t *)self_ptr;
    pico8_fixed_t qd;
    pico8_fixed_t ms;
    pico8_fixed_t iu;
    pico8_fixed_t py;
    pico8_vec_t p;
    pico8_vec_t v;
    pico8_vec_t a;
    pico8_ob_table_t *particle;
    pico8_vec_t *p_heap;
    pico8_vec_t *v_heap;
    pico8_vec_t *a_heap;

    (void)argc;
    (void)arg0;
    (void)arg1;

    qd = pico8_fixed_add(pico8_fixed_rnd(PICO8_FIXED_LIT(0x00018000, 1.5)), PICO8_FIXED_LIT(0x00008000, 0.5));
    if (pico8_fixed_rnd_default() < PICO8_FIXED_LIT(0x00009999, 0.6)) {
        qd = pico8_fixed_mul(pico8_fixed_flr(pico8_fixed_div(qd, PICO8_FIXED_LIT(0x00008000, 0.5))),
                              PICO8_FIXED_LIT(0x00008000, 0.5));
    }
    p = pico8_vec_qm(qd, pico8_fixed_rnd_default());

    ms = pico8_pf_field_fixed(self, "ms");
    iu = pico8_pf_field_fixed(self, "iu");
    py = pico8_pf_field_fixed(self, "py");

    v = pico8_vec_mul(p, ms);
    a = pico8_vec_mul(p, iu);

    particle = pico8_ob_table_new();
    p_heap = malloc(sizeof *p_heap);
    *p_heap = p;
    v_heap = malloc(sizeof *v_heap);
    *v_heap = v;
    a_heap = malloc(sizeof *a_heap);
    *a_heap = a;
    pico8_ob_table_set_name(particle, "p", pico8_value_ref(p_heap));
    pico8_ob_table_set_name(particle, "v", pico8_value_ref(v_heap));
    pico8_ob_table_set_name(particle, "a", pico8_value_ref(a_heap));
    pico8_ob_table_set_name(particle, "l", pico8_value_fixed(pico8_fixed_add(pico8_fixed_rnd(py), py)));

    return pico8_value_ref(particle);
}

void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p) {
    pico8_value_t qy_val;
    const pico8_list_t *qy = NULL;
    pico8_ob_table_t *last_pt = NULL;
    int i;
    int px;
    int py;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "qy", &qy_val) && qy_val.kind == PICO8_VALUE_REF && qy_val.as.ref != NULL) {
        qy = PICO8_REF_AS_LIST_CONST(qy_val.as.ref);
    }

    px = pico8_fixed_flr_int(p.x);
    py = pico8_fixed_flr_int(p.y);

    for (i = 1; i <= 3; i++) {
        pico8_ob_table_t *pt = NULL;

        if (qy != NULL && i <= qy->count) {
            pico8_value_t entry = qy->items[i - 1];
            if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
                pt = (pico8_ob_table_t *)entry.as.ref;
            }
        }

        if (pt != NULL) {
            pico8_value_t p_field;
            pico8_vec_t pt_p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
            pico8_value_t l_field;
            pico8_fixed_t l = pico8_fixed_from_int(0);
            int idx;
            int color = 0;
            pico8_value_t k_val;
            pico8_value_t k_entry;

            if (pico8_ob_table_get_name(pt, "p", &p_field) && p_field.kind == PICO8_VALUE_REF &&
                p_field.as.ref != NULL) {
                pt_p = *(const pico8_vec_t *)p_field.as.ref;
            }
            if (pico8_ob_table_get_name(pt, "l", &l_field) && l_field.kind == PICO8_VALUE_FIXED) {
                l = l_field.as.fixed;
            }
            idx = pico8_fixed_flr_int(pico8_fixed_flr(pico8_fixed_div(l, pico8_fixed_from_int(2))));
            if (g_ga_class != NULL && pico8_ob_table_get_name(g_ga_class->defaults, "k_", &k_val) &&
                k_val.kind == PICO8_VALUE_REF && k_val.as.ref != NULL &&
                pico8_ob_table_get_index((const pico8_ob_table_t *)k_val.as.ref, idx, &k_entry) &&
                k_entry.kind == PICO8_VALUE_FIXED) {
                color = pico8_fixed_flr_int(k_entry.as.fixed);
            }
            pico8_circ(gfx, mem, px, py, pico8_fixed_flr_int(pico8_vec_qx(pt_p)), color);
        }

        /* Lua's `pt` outlives the loop, so `p_` follows the THIRD slot
         * alone: the burst dies when fewer than 3 particles remain, not
         * when none do. */
        last_pt = pt;
    }

    pico8_ei(gfx, mem, self);
    pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(last_pt == NULL));
}

static pico8_value_t pico8_ga_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_ga_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self, p);
    return pico8_value_ref(NULL);
}

void pico8_planet_family_init(void) {
    assert(pico8_jo_class() != NULL && "pico8_jo_init() must run first");
    pico8_planet_family_shutdown();
    pico8_pi_init();

    g_qh_class = pico8_class_declare(pico8_jo_class(),
                                      "\n"
                                      "nu=o(\"qh\",\"jo\",\"np\",\"prober\"),\n"
                                      "hp=1,px=o(\"laboratory\",c=13),\n"
                                      "me=\"kz\",dq=\"ly\",\n"
                                      "hf=f,qh=1,\n"
                                      "od=15,fp=2,\n"
                                      "dw=0.025,\n"
                                      "mi=1,\n"
                                      "s=174,\n"
                                      "ba=o(28,18,20,21,22,48),\n");
    PICO8_CLASS_COVERAGE_NAME(g_qh_class, "qh");
    pico8_ob_table_set_name(g_qh_class->defaults, "hq", pico8_value_function(pico8_bv_dispatch));
    pico8_ob_table_set_name(g_qh_class->defaults, "ly", pico8_value_function(pico8_qh_ly_dispatch));
    pico8_ob_table_set_name(g_qh_class->defaults, "ev", pico8_value_function(pico8_dl_dispatch));
    pico8_ob_table_set_name(g_qh_class->defaults, "ey", pico8_value_function(pico8_d__dispatch));

    g_nl_class = pico8_class_declare(g_qh_class,
                                      "\n"
                                      "nu=o(\"jo\",\"np\",\"prober\"),\n"
                                      "rr=\"trade league\",qh=f,nl=1,\n"
                                      "hp=6,s=68,px=o(\"trading hub\",c=13),\n"
                                      "od=20,qw=15,\n"
                                      "ba=o(28,48,18,20,21,22),\n");
    PICO8_CLASS_COVERAGE_NAME(g_nl_class, "nl");
    pico8_ob_table_set_name(g_nl_class->defaults, "ly", pico8_value_function(pico8_nl_ly_dispatch));

    g_os_class = pico8_class_declare(g_qh_class,
                                      "\n"
                                      "nu=o(\"jo\",\"np\",\"prober\"),\n"
                                      "hp=4,px=o(\"processor\",c=13),\n"
                                      "qh=f,s=100,\n"
                                      "od=15,\n"
                                      "hs=0.5,\n");
    PICO8_CLASS_COVERAGE_NAME(g_os_class, "os");

    g_hj_class = pico8_class_declare(g_qh_class,
                                      "\n"
                                      "nu=o(\"hj\",\"jo\",\"np\",\"prober\"),\n"
                                      "rr=\"ascension\",\n"
                                      "hp=2,qh=f,\n"
                                      "px=o(\"ascension nr\",c=13),\n"
                                      "od=60,\n"
                                      "s=166,\n"
                                      "py=0.12,ki=9,\n"
                                      "pn=5,pk=1,\n"
                                      "fc=0.5,\n"
                                      "k_=o(1,13,12,12,12,13,13,5,1,1,1),\n"
                                      "du=7.5,\n"
                                      "qo=0.98,ro=-0.07,\n"
                                      "qg=0.07,q_=0.98,\n");
    PICO8_CLASS_COVERAGE_NAME(g_hj_class, "hj");
    pico8_class_set_pq(g_hj_class, pico8_f_);
    pico8_ob_table_set_name(g_hj_class->defaults, "mm", pico8_value_function(pico8_g__mm));
    pico8_ob_table_set_name(g_hj_class->defaults, "kj", pico8_value_function(pico8_hj_kj_dispatch));

    g_pm_class = pico8_class_declare(g_hj_class,
                                      "\n"
                                      "fz=\"pm\",\n"
                                      "nu=o(\"jo\",\"np\",\"prober\"),\n"
                                      "hp=3,px=o(\"protostar\",c=13),\n"
                                      "me=\"ly\",s=98,\n"
                                      "py=0.06,ki=9,\n"
                                      "pn=8,pk=f,\n"
                                      "ng=v(1,0),\n"
                                      "du=2,\n"
                                      "fc=1,\n"
                                      "k_=o(1,2,4,9,10,7),\n"
                                      "qo=1.02,ro=-0.0525,\n"
                                      "qg=0.0525,q_=1.02,\n");
    PICO8_CLASS_COVERAGE_NAME(g_pm_class, "pm");
    pico8_class_set_pq(g_pm_class, pico8_pm_pq);
    pico8_class_register_named("pm", g_pm_class);

    /* `nu=o()` is empty and wins outright: chain fallback stops at the
     * first class that defines a field, so no `ga` instance is ever
     * registered under jo/np/prober in `cg`. This is not a merge. */
    g_ga_class = pico8_class_declare(pico8_jo_class(),
                                      "\n"
                                      "fz=\"ga\",\n"
                                      "nu=o(),\n"
                                      "hf=f,\n"
                                      "by=o(),\n"
                                      "kq=b(0,0,-1,-1),\n"
                                      "fa=3,\n"
                                      "fc=0,\n"
                                      "qo=0.9,ro=0,\n"
                                      "qg=0,q_=0.9,\n"
                                      "k_=o(1,2,4,9,9,10,10,7,7,7,7,7),\n"
                                      "iu=-0.06,py=6,ms=3,\n");
    PICO8_CLASS_COVERAGE_NAME(g_ga_class, "ga");
    pico8_class_set_pq(g_ga_class, pico8_ga_pq);

    /* `by=o()` parses to an ob_table, but every reader of `by` casts it
     * to a list; without this swap the tag check in PICO8_REF_AS_LIST
     * aborts on the first read. The raw table ob() built is freed here. */
    {
        pico8_ob_table_t *raw_by = NULL;
        pico8_value_t by_val;
        pico8_list_t *heap_by;

        if (pico8_ob_table_get_name(g_ga_class->defaults, "by", &by_val) && by_val.kind == PICO8_VALUE_REF) {
            raw_by = (pico8_ob_table_t *)by_val.as.ref;
        }
        heap_by = malloc(sizeof *heap_by);
        pico8_list_init(heap_by);
        pico8_ob_table_set_name(g_ga_class->defaults, "by", pico8_value_ref(heap_by));
        pico8_ob_table_free(raw_by);
    }

    pico8_ob_table_set_name(g_ga_class->defaults, "mm", pico8_value_function(pico8_ga_mm));
    pico8_ob_table_set_name(g_ga_class->defaults, "kj", pico8_value_function(pico8_ga_kj_dispatch));
    pico8_class_register_named("ga", g_ga_class);
}

void pico8_planet_family_shutdown(void) {
    if (g_qh_class != NULL) {
        pico8_class_free(g_qh_class);
        g_qh_class = NULL;
    }
    if (g_nl_class != NULL) {
        pico8_class_free(g_nl_class);
        g_nl_class = NULL;
    }
    if (g_os_class != NULL) {
        pico8_class_free(g_os_class);
        g_os_class = NULL;
    }
    if (g_hj_class != NULL) {
        pico8_class_free(g_hj_class);
        g_hj_class = NULL;
    }
    if (g_pm_class != NULL) {
        pico8_class_free(g_pm_class);
        g_pm_class = NULL;
    }
    if (g_ga_class != NULL) {
        pico8_class_free(g_ga_class);
        g_ga_class = NULL;
    }
}

pico8_class_t *pico8_qh_class(void) {
    return g_qh_class;
}

pico8_instance_t *pico8_qh_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_qh_class, overrides);
}

pico8_class_t *pico8_nl_class(void) {
    return g_nl_class;
}

pico8_instance_t *pico8_nl_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_nl_class, overrides);
}

pico8_class_t *pico8_os_class(void) {
    return g_os_class;
}

pico8_instance_t *pico8_os_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_os_class, overrides);
}

pico8_class_t *pico8_hj_class(void) {
    return g_hj_class;
}

pico8_instance_t *pico8_hj_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_hj_class, overrides);
}

pico8_class_t *pico8_pm_class(void) {
    return g_pm_class;
}

pico8_instance_t *pico8_pm_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_pm_class, overrides);
}

pico8_class_t *pico8_ga_class(void) {
    return g_ga_class;
}

pico8_instance_t *pico8_ga_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_ga_class, overrides);
}
