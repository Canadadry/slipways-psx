#include "pico8_economy.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_economy_formulas.h"
#include "pico8_economy_score.h"
#include "pico8_economy_tiers.h"
#include "pico8_entity.h"
#include "pico8_msgbox.h"
#include "pico8_stash.h"
#include "pico8_tech_gate.h"
#include "pico8_ref_audit.h"
#include "pico8_type_tag.h"

static pico8_class_t *g_hy_class = NULL;

static pico8_value_t *hy_member(pico8_hy_t *self, const char *name) {
    if (strcmp(name, "nz") == 0) {
        return &self->nz;
    }
    if (strcmp(name, "ge") == 0) {
        return &self->ge;
    }
    if (strcmp(name, "jk") == 0) {
        return &self->jk;
    }
    if (strcmp(name, "ew") == 0) {
        return &self->ew;
    }
    if (strcmp(name, "invented") == 0) {
        return &self->invented;
    }
    if (strcmp(name, "lo") == 0) {
        return &self->lo;
    }
    if (strcmp(name, "yr") == 0) {
        return &self->yr;
    }
    if (strcmp(name, "po") == 0) {
        return &self->po;
    }
    if (strcmp(name, "fu") == 0) {
        return &self->fu;
    }
    if (strcmp(name, "happiness") == 0) {
        return &self->happiness;
    }
    if (strcmp(name, "hl") == 0) {
        return &self->hl;
    }
    return NULL;
}

static int pico8_hy_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_value_t *member = hy_member((pico8_hy_t *)self, name);

    if (member == NULL || (member->kind == PICO8_VALUE_REF && member->as.ref == NULL)) {
        return 0;
    }
    *out_value = *member;
    return 1;
}

static const pico8_struct_class_t g_hy_struct_class = {
    {NULL},
    pico8_hy_field,
    NULL,
};

static pico8_fixed_t member_fixed(pico8_value_t v) {
    return v.kind == PICO8_VALUE_FIXED ? v.as.fixed : pico8_fixed_from_int(0);
}

static void *pico8_hy_struct_new(const pico8_ob_table_t *overrides) {
    pico8_hy_t *self = malloc(sizeof *self);
    pico8_list_t *invented;

    self->header.tag = PICO8_TYPE_TAG_STRUCT;
    self->header.cls = &g_hy_struct_class;
    self->nz = pico8_value_ref(NULL);
    self->ge = pico8_value_ref(NULL);
    self->jk = pico8_value_ref(NULL);
    self->ew = pico8_value_ref(NULL);
    self->invented = pico8_value_ref(NULL);
    self->lo = pico8_value_ref(NULL);
    self->yr = pico8_value_ref(NULL);
    self->po = pico8_value_ref(NULL);
    self->fu = pico8_value_ref(NULL);
    self->happiness = pico8_value_ref(NULL);
    self->hl = pico8_value_ref(NULL);
    assert((overrides == NULL || overrides->count == 0) &&
           "hy: every measured construction passes no key; a key here would be a field under no rule of "
           "plan-3 section 5 -- stop, do not add a member");

    self->nz = pico8_value_fixed(pico8_fixed_from_int(100));
    self->ge = pico8_value_fixed(pico8_fixed_from_int(0));
    self->jk = pico8_value_fixed(pico8_fixed_from_int(0));
    self->ew = pico8_value_fixed(pico8_fixed_from_int(1));
    /* TROU-7 */
    invented = malloc(sizeof *invented);
    pico8_list_init(invented);
    self->invented = pico8_value_ref(invented);
    self->lo = pico8_value_fixed(pico8_fixed_from_int(1));
    self->yr = pico8_value_fixed(pico8_fixed_from_int(3401));

#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("hy");
#endif
    return self;
}

static void assert_hy(const pico8_hy_t *self) {
    assert((self == NULL || (pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT &&
                             self->header.cls == &g_hy_struct_class)) &&
           "hy: every measured call passes the hy of fm; anything else would be read through the wrong layout");
}

void pico8_economy_init(void) {
    pico8_economy_shutdown();
    g_hy_class = pico8_class_declare(NULL, NULL);
    PICO8_CLASS_COVERAGE_NAME(g_hy_class, "hy");
    pico8_class_set_struct_new(g_hy_class, pico8_hy_struct_new);
}

void pico8_economy_shutdown(void) {
    if (g_hy_class != NULL) {
        pico8_class_free(g_hy_class);
        g_hy_class = NULL;
    }
}

pico8_class_t *pico8_hy_class(void) {
    return g_hy_class;
}

const pico8_struct_class_t *pico8_hy_struct_class(void) {
    return &g_hy_struct_class;
}

pico8_hy_t *pico8_hy_new(const pico8_ob_table_t *overrides) {
    return pico8_hy_struct_new(overrides);
}

void pico8_hy_nx(pico8_hy_t *self, pico8_fixed_t hx) {
    pico8_value_t kxv;
    pico8_fixed_t kx;
    pico8_fixed_t lo;

    assert_hy(self);
    if (self == NULL) {
        return;
    }

    kxv = pico8_ov(pico8_value_string("time compression"), pico8_value_fixed(pico8_fixed_from_int(12)),
                   pico8_value_fixed(pico8_fixed_from_int(15)));
    kx = (kxv.kind == PICO8_VALUE_FIXED) ? kxv.as.fixed : pico8_fixed_from_int(12);

    lo = pico8_fixed_add(member_fixed(self->lo), hx);
    self->lo = pico8_value_fixed(lo);

    pico8_hy_lj(self);

    if (lo > kx) {
        self->lo = pico8_value_fixed(pico8_fixed_sub(lo, kx));
        pico8_hy_hk(self);
    }
}

void pico8_hy_hk(pico8_hy_t *self) {
    pico8_fixed_t nz;
    pico8_fixed_t ge;
    pico8_fixed_t jk;
    pico8_fixed_t new_yr;
    pico8_value_t po_val;
    pico8_fixed_t po_rk = pico8_fixed_from_int(0);
    const pico8_ob_table_t *r = pico8_r();
    const pico8_list_t *hk_list;
    pico8_list_iter_t it;
    pico8_value_t e;

    assert_hy(self);
    if (self == NULL) {
        return;
    }

    nz = member_fixed(self->nz);
    ge = member_fixed(self->ge);
    jk = member_fixed(self->jk);

    po_val = self->po;
    if (po_val.kind == PICO8_VALUE_REF && po_val.as.ref != NULL && r != NULL) {
        pico8_value_t k_val;
        pico8_value_t rk_val;
        if (pico8_ob_table_get_name(r, "k", &k_val)) {
            int found = 0;
            if (k_val.kind == PICO8_VALUE_FIXED) {
                found = pico8_ob_table_get_index((const pico8_ob_table_t *)po_val.as.ref,
                                                  pico8_fixed_flr_int(k_val.as.fixed), &rk_val);
            } else if (k_val.kind == PICO8_VALUE_STRING && k_val.as.string != NULL) {
                found = pico8_ob_table_get_name((const pico8_ob_table_t *)po_val.as.ref, k_val.as.string, &rk_val);
            }
            if (found && rk_val.kind == PICO8_VALUE_FIXED) {
                po_rk = rk_val.as.fixed;
            }
        }
    }

    self->nz = pico8_value_fixed(pico8_fixed_add(nz, ge));
    self->jk = pico8_value_fixed(pico8_fixed_add(pico8_fixed_add(jk, po_rk), pico8_fixed_from_int(1)));
    new_yr = pico8_fixed_add(member_fixed(self->yr), pico8_fixed_from_int(1));
    self->yr = pico8_value_fixed(new_yr);

    pico8_hy_lj(self);

    hk_list = pico8_cv("hk");
    if (hk_list != NULL) {
        it = pico8_list_all(hk_list);
        while (pico8_list_iter_next(&it, &e)) {
            if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
                pico8_value_t discard;
                pico8_mv((pico8_instance_t *)e.as.ref, "hk", 1, pico8_value_fixed(new_yr), pico8_value_ref(NULL),
                         &discard);
            }
        }
    }
}

int pico8_hy_lu(pico8_hy_t *self, pico8_fixed_t jt) {
    pico8_fixed_t nz;
    pico8_ob_table_t *overrides;

    assert_hy(self);
    nz = self != NULL ? member_fixed(self->nz) : pico8_fixed_from_int(0);

    if (nz < jt) {
        overrides = pico8_ob_table_new();
        pico8_ob_table_set_index(overrides, 1, pico8_value_string("-no money-"));
        pico8_ob_table_set_index(overrides, 2, pico8_value_fixed(pico8_fixed_from_int(8)));
        pico8_iv_new(overrides);
        pico8_ob_table_free(overrides);
        return 0;
    }

    if (self != NULL) {
        self->nz = pico8_value_fixed(pico8_fixed_sub(nz, jt));
    }

    if (jt != pico8_fixed_from_int(0)) {
        int color;
        char *text = pico8_ne(pico8_fixed_neg(jt), 10, "$", &color);
        overrides = pico8_ob_table_new();
        pico8_ob_table_set_index(overrides, 1, pico8_value_string(text));
        pico8_ob_table_set_index(overrides, 2, pico8_value_fixed(pico8_fixed_from_int(color)));
        pico8_iv_new(overrides);
        pico8_ob_table_free(overrides);
        /* `text` is now owned by the new `iv` instance, which reads it every
         * frame it stays on screen: not freed here. */
    }

    return 1;
}

void pico8_hy_lj(pico8_hy_t *self) {
    pico8_ob_table_t *new_po;
    pico8_ob_table_t *new_fu;
    pico8_fixed_t dd = pico8_fixed_from_int(0);
    pico8_fixed_t ge;
    pico8_fixed_t happiness;
    pico8_value_t skill;
    pico8_fixed_t skill_mult;
    const pico8_list_t *jo_list;
    pico8_list_iter_t it;
    pico8_value_t e;
    pico8_fixed_t yr;
    pico8_fixed_t nz;
    pico8_value_t hl;

    assert_hy(self);
    if (self == NULL) {
        return;
    }

    /* Terminates because `bb()` reports only tier INCREASES and a tier is
     * capped at 4; a `jw != mi` test there would spin forever. */
    do {
        pico8_bd();
    } while (pico8_bb());

    if (self->po.kind == PICO8_VALUE_REF && self->po.as.ref != NULL) {
        pico8_ob_table_free((pico8_ob_table_t *)self->po.as.ref);
    }
    if (self->fu.kind == PICO8_VALUE_REF && self->fu.as.ref != NULL) {
        pico8_ob_table_free((pico8_ob_table_t *)self->fu.as.ref);
    }
    new_po = pico8_bx();
    new_fu = pico8_ck();
    self->po = pico8_value_ref(new_po);
    self->fu = pico8_value_ref(new_fu);

    jo_list = pico8_cg("jo");
    if (jo_list != NULL) {
        it = pico8_list_all(jo_list);
        while (pico8_list_iter_next(&it, &e)) {
            if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
                pico8_instance_t *p = (pico8_instance_t *)e.as.ref;
                pico8_fixed_t kr = pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)));
                pico8_entity_set_field(p, "kr", pico8_value_fixed(kr));
                dd = pico8_fixed_add(dd, kr);
            }
        }
    }

    skill = pico8_ov(pico8_value_string("skill implants"), pico8_value_fixed(pico8_fixed_from_int(1)),
                      pico8_value_fixed(PICO8_FIXED_LIT(0x00012666, 1.15)));
    skill_mult = (skill.kind == PICO8_VALUE_FIXED) ? skill.as.fixed : pico8_fixed_from_int(1);
    ge = pico8_fixed_sub(pico8_fixed_flr(pico8_fixed_mul(dd, skill_mult)), pico8_cz());
    self->ge = pico8_value_fixed(ge);

    happiness = pico8_bf();
    self->happiness = pico8_value_fixed(happiness);

    yr = member_fixed(self->yr);
    nz = member_fixed(self->nz);
    if (yr >= pico8_fixed_from_int(3426)) {
        hl = pico8_value_string("final score");
    } else if (ge <= pico8_fixed_from_int(0) && nz < pico8_fixed_from_int(3)) {
        hl = pico8_value_string("bankrupt");
    } else {
        hl = pico8_value_bool(0);
    }
    self->hl = hl;

    {
        const pico8_list_t *hb_list = pico8_cv("hb");
        if (hb_list != NULL) {
            pico8_list_iter_t it2;
            pico8_value_t e2;
            it2 = pico8_list_all(hb_list);
            while (pico8_list_iter_next(&it2, &e2)) {
                if (e2.kind == PICO8_VALUE_REF && e2.as.ref != NULL) {
                    pico8_value_t discard;
                    pico8_mv((pico8_instance_t *)e2.as.ref, "hb", 0, pico8_value_ref(NULL), pico8_value_ref(NULL),
                             &discard);
                }
            }
        }
    }
}

void pico8_hy_kt(pico8_hy_t *self, const pico8_ob_table_t *t) {
    pico8_value_t px_val;
    pico8_fixed_t jk;
    pico8_fixed_t ew;
    pico8_value_t invented_val;
    int count = 0;

    assert_hy(self);
    if (self == NULL) {
        return;
    }

    jk = member_fixed(self->jk);
    self->jk = pico8_value_fixed(pico8_fixed_sub(jk, pico8_gi(t)));

    invented_val = self->invented;
    if (invented_val.kind == PICO8_VALUE_REF && invented_val.as.ref != NULL) {
        pico8_list_t *invented = PICO8_REF_AS_LIST(invented_val.as.ref);
        if (t != NULL && pico8_ob_table_get_name(t, "px", &px_val)) {
            pico8_list_add(invented, px_val);
        }
        count = invented->count;
    }

    ew = member_fixed(self->ew);
    if (ew < pico8_fixed_from_int(4)) {
        const pico8_ob_table_t *ek = pico8_ek();
        pico8_value_t threshold_val;
        if (ek != NULL && pico8_ob_table_get_index(ek, pico8_fixed_flr_int(ew), &threshold_val) &&
            threshold_val.kind == PICO8_VALUE_FIXED && pico8_fixed_from_int(count) >= threshold_val.as.fixed) {
            self->ew = pico8_value_fixed(pico8_fixed_add(ew, pico8_fixed_from_int(1)));
        }
    }

    pico8_hy_lj(self);
}
