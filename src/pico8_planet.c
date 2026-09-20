#include "pico8_planet.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_entity_geom.h"
#include "pico8_ke.h"
#include "pico8_pi.h"
#include "pico8_research.h"
#include "pico8_research_display.h"

static pico8_class_t *g_jo_class = NULL;

static int pico8_jo_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static void pico8_jo_pq(pico8_instance_t *self);

static pico8_value_t pico8_jo_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_gk((const pico8_instance_t *)self, arg0, arg1);
}

static pico8_value_t pico8_jo_gg_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_jo_gg((const pico8_instance_t *)self);
}

static pico8_value_t pico8_jo_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_jo_hb((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_jo_init(void) {
    pico8_jo_shutdown();
    pico8_pi_init();
    g_jo_class = pico8_class_declare(pico8_ke_class(),
                                      "\n"
                                      "nu=o(\"jo\",\"np\",\"prober\"),\n"
                                      "me=\"unknown\",\n"
                                      "hf=1,hs=1,\n"
                                      "fa=6,\n"
                                      "kq=b(-7,-7,7,7),\n"
                                      "ma=0,\n"
                                      "ka=o(2,3,9,12),\n"
                                      "dz=o(\n"
                                      "o(\"unhappy\",c=8),\n"
                                      "o(\"content\",c=3),\n"
                                      "o(\"prosperous\",c=9),\n"
                                      "o(\"rich\",c=12),\n"
                                      "),\n"
                                      "cq=o(12,36,24,25),\n"
                                      "kw=o(mu=0),\n"
                                      "gq=v(0,11),ca=9,\n"
                                      "em=o(f,3,0,-1),\n");
    pico8_class_set_pq(g_jo_class, pico8_jo_pq);
    pico8_ob_table_set_name(g_jo_class->defaults, "hd", pico8_value_function(pico8_jo_hd_dispatch));
    pico8_ob_table_set_name(g_jo_class->defaults, "gg", pico8_value_function(pico8_jo_gg_dispatch));
    pico8_ob_table_set_name(g_jo_class->defaults, "hb", pico8_value_function(pico8_jo_hb_dispatch));
    pico8_class_register_named("jo", g_jo_class);
}

void pico8_jo_shutdown(void) {
    if (g_jo_class != NULL) {
        pico8_class_free(g_jo_class);
        g_jo_class = NULL;
    }
}

pico8_class_t *pico8_jo_class(void) {
    return g_jo_class;
}

pico8_instance_t *pico8_jo_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_jo_class, overrides);
}

static void pico8_jo_pq(pico8_instance_t *self) {
    pico8_value_t hp_val;
    pico8_list_t *oe;

    if (self == NULL) {
        return;
    }

    oe = malloc(sizeof *oe);
    pico8_list_init(oe);
    pico8_ob_table_set_name(self->fields, "oe", pico8_value_ref(oe));

    if (pico8_instance_get_field(self, "hp", &hp_val) && pico8_jo_truthy(hp_val)) {
        const pico8_ob_table_t *gb = pico8_gb();
        pico8_value_t target;
        int found = 0;

        if (hp_val.kind == PICO8_VALUE_FIXED) {
            found = pico8_ob_table_get_index(gb, (int)pico8_fixed_to_double(hp_val.as.fixed), &target);
        } else if (hp_val.kind == PICO8_VALUE_STRING) {
            found = pico8_ob_table_get_name(gb, hp_val.as.string, &target);
        }
        if (found && target.kind == PICO8_VALUE_REF && target.as.ref != NULL) {
            pico8_jo_lz(self, (pico8_instance_t *)target.as.ref);
        }
    }
}

static int pico8_jo_lz_mw_step(pico8_value_t e, void *ctx, pico8_value_t *out_value) {
    (void)ctx;
    if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
        *out_value = pico8_value_ref(pico8_mw((const pico8_ob_table_t *)e.as.ref));
        return 1;
    }
    return 0;
}

void pico8_jo_lz(pico8_instance_t *self, pico8_instance_t *hp_instance) {
    pico8_value_t i_val;
    pico8_value_t o_val;
    pico8_list_t jy;
    pico8_list_t *jy_heap;
    pico8_ob_table_t *kw;

    if (self == NULL) {
        return;
    }

    pico8_ob_table_set_name(self->fields, "hn", pico8_value_ref(hp_instance));
    pico8_ob_table_set_name(self->fields, "ep", pico8_value_ref(NULL));

    pico8_list_init(&jy);
    if (hp_instance != NULL && pico8_instance_get_field(hp_instance, "i", &i_val) &&
        i_val.kind == PICO8_VALUE_REF && i_val.as.ref != NULL) {
        jy = pico8_nv((const pico8_list_t *)i_val.as.ref, pico8_jo_lz_mw_step, NULL);
    }
    jy_heap = malloc(sizeof *jy_heap);
    *jy_heap = jy;
    pico8_ob_table_set_name(self->fields, "jy", pico8_value_ref(jy_heap));

    kw = pico8_ob_table_new();
    if (hp_instance != NULL && pico8_instance_get_field(hp_instance, "o", &o_val)) {
        pico8_ob_table_set_name(kw, "qe", o_val);
    }
    pico8_ob_table_set_name(kw, "mu", pico8_value_fixed(pico8_fixed_from_int(0)));
    pico8_ob_table_set_name(self->fields, "kw", pico8_value_ref(kw));
}

pico8_value_t pico8_jo_gg(const pico8_instance_t *self) {
    pico8_value_t hf_val;
    pico8_value_t me_val;

    if (!pico8_instance_get_field(self, "hf", &hf_val)) {
        return pico8_value_ref(NULL);
    }
    if (!pico8_jo_truthy(hf_val)) {
        return hf_val;
    }
    if (pico8_instance_get_field(self, "me", &me_val) && me_val.kind == PICO8_VALUE_STRING) {
        return pico8_value_bool(strcmp(me_val.as.string, "ly") == 0);
    }
    return pico8_value_bool(0);
}

void pico8_jo_kd(pico8_instance_t *self) {
    if (self == NULL) {
        return;
    }
    pico8_ob_table_set_name(self->fields, "ma", pico8_value_fixed(pico8_fixed_from_double(1.25)));
}

void pico8_jo_ll(pico8_instance_t *self, pico8_fixed_t t) {
    if (self == NULL) {
        return;
    }
    pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(t > pico8_fixed_from_int(60)));
}

void pico8_jo_hb(pico8_instance_t *self) {
    pico8_value_t jy_val;
    pico8_value_t kw_val;
    pico8_value_t by_val;
    const pico8_list_t *jy_list = NULL;
    const pico8_list_t *by_list = NULL;
    pico8_value_t qe = pico8_value_ref(NULL);
    pico8_fixed_t mu = pico8_fixed_from_int(0);
    pico8_instance_t *lq_result;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "jy", &jy_val) && jy_val.kind == PICO8_VALUE_REF && jy_val.as.ref != NULL) {
        jy_list = (const pico8_list_t *)jy_val.as.ref;
    }
    if (pico8_instance_get_field(self, "kw", &kw_val) && kw_val.kind == PICO8_VALUE_REF && kw_val.as.ref != NULL) {
        const pico8_ob_table_t *kw = (const pico8_ob_table_t *)kw_val.as.ref;
        pico8_value_t mu_val;
        if (!pico8_ob_table_get_name(kw, "qe", &qe)) {
            qe = pico8_value_ref(NULL);
        }
        if (pico8_ob_table_get_name(kw, "mu", &mu_val) && mu_val.kind == PICO8_VALUE_FIXED) {
            mu = mu_val.as.fixed;
        }
    }
    if (pico8_instance_get_field(self, "by", &by_val) && by_val.kind == PICO8_VALUE_REF && by_val.as.ref != NULL) {
        by_list = (const pico8_list_t *)by_val.as.ref;
    }

    lq_result = pico8_jo_ex(jy_list, qe, mu, pico8_fixed_from_int(by_list != NULL ? by_list->count : 0),
                             pico8_fixed_from_int(0));
    pico8_ob_table_set_name(self->fields, "lq", pico8_value_ref(lq_result));
}
