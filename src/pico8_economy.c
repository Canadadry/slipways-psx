#include "pico8_economy.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_economy_formulas.h"
#include "pico8_economy_score.h"
#include "pico8_economy_tiers.h"
#include "pico8_entity.h"
#include "pico8_game_data.h"
#include "pico8_map_tables.h"
#include "pico8_msgbox.h"
#include "pico8_tech_gate.h"
#include "pico8_type_tag.h"

static int g_hy_up = 0;

static int pico8_hy_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_hy_t *hy = self;

    if (strcmp(name, "nz") == 0) {
        *out_value = pico8_value_fixed(hy->nz);
        return 1;
    }
    if (strcmp(name, "ge") == 0) {
        *out_value = pico8_value_fixed(hy->ge);
        return 1;
    }
    if (strcmp(name, "jk") == 0) {
        *out_value = pico8_value_fixed(hy->jk);
        return 1;
    }
    if (strcmp(name, "ew") == 0) {
        *out_value = pico8_value_fixed(hy->ew);
        return 1;
    }
    if (strcmp(name, "invented") == 0) {
        if (hy->invented == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(hy->invented);
        return 1;
    }
    if (strcmp(name, "lo") == 0) {
        *out_value = pico8_value_fixed(hy->lo);
        return 1;
    }
    if (strcmp(name, "yr") == 0) {
        *out_value = pico8_value_fixed(hy->yr);
        return 1;
    }
    if (strcmp(name, "po") == 0) {
        if (hy->po == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(hy->po);
        return 1;
    }
    if (strcmp(name, "fu") == 0) {
        if (hy->fu == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(hy->fu);
        return 1;
    }
    if (strcmp(name, "happiness") == 0) {
        if (!hy->has_happiness) {
            return 0;
        }
        *out_value = pico8_value_fixed(hy->happiness);
        return 1;
    }
    if (strcmp(name, "hl") == 0) {
        pico8_value_t v = pico8_hy_hl(hy);
        if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
            return 0;
        }
        *out_value = v;
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_hy_struct_class = {
    {NULL},
    pico8_hy_field,
    NULL,
};

static void *pico8_hy_struct_new(const pico8_ob_table_t *overrides) {
    pico8_hy_t *self = malloc(sizeof *self);
    pico8_list_t *invented;

    self->header.tag = PICO8_TYPE_TAG_STRUCT;
    self->header.cls = &g_hy_struct_class;
    self->nz = pico8_fixed_from_int(0);
    self->ge = pico8_fixed_from_int(0);
    self->jk = pico8_fixed_from_int(0);
    self->ew = pico8_fixed_from_int(0);
    self->invented = NULL;
    self->lo = pico8_fixed_from_int(0);
    self->yr = pico8_fixed_from_int(0);
    self->po = NULL;
    self->fu = NULL;
    self->po_own.count = 0;
    self->fu_own.count = 0;
    self->happiness = pico8_fixed_from_int(0);
    self->has_happiness = 0;
    self->hl = NULL;
    self->has_hl = 0;
    assert((overrides == NULL || overrides->count == 0) &&
           "hy: every construction passes no key; a key has no member -- stop, do not add one");

    self->nz = pico8_fixed_from_int(100);
    self->ge = pico8_fixed_from_int(0);
    self->jk = pico8_fixed_from_int(0);
    self->ew = pico8_fixed_from_int(1);
    /* TROU-7 */
    invented = malloc(sizeof *invented);
    pico8_list_init(invented);
    self->invented = invented;
    self->lo = pico8_fixed_from_int(1);
    self->yr = pico8_fixed_from_int(3401);

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
    g_hy_up = 1;
}

void pico8_economy_shutdown(void) {
    g_hy_up = 0;
}

int pico8_hy_ready(void) {
    return g_hy_up;
}

const pico8_struct_class_t *pico8_hy_struct_class(void) {
    return &g_hy_struct_class;
}

pico8_value_t pico8_hy_hl(const pico8_hy_t *self) {
    if (self == NULL || !self->has_hl) {
        return pico8_value_ref(NULL);
    }
    if (self->hl == NULL) {
        return pico8_value_bool(0);
    }
    return pico8_value_string(self->hl);
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

    lo = pico8_fixed_add(self->lo, hx);
    self->lo = lo;

    pico8_hy_lj(self);

    if (lo > kx) {
        self->lo = pico8_fixed_sub(lo, kx);
        pico8_hy_hk(self);
    }
}

void pico8_hy_hk(pico8_hy_t *self) {
    pico8_fixed_t nz;
    pico8_fixed_t ge;
    pico8_fixed_t jk;
    pico8_fixed_t new_yr;
    pico8_fixed_t po_rk = pico8_fixed_from_int(0);
    const pico8_list_t *hk_list;
    pico8_list_iter_t it;
    pico8_value_t e;

    assert_hy(self);
    if (self == NULL) {
        return;
    }

    nz = self->nz;
    ge = self->ge;
    jk = self->jk;

    if (self->po != NULL) {
        int k;
        if (pico8_r_lookup("k", &k)) {
            pico8_qe_map_get(self->po, pico8_fixed_from_int(k), &po_rk);
        }
    }

    self->nz = pico8_fixed_add(nz, ge);
    self->jk = pico8_fixed_add(pico8_fixed_add(jk, po_rk), pico8_fixed_from_int(1));
    new_yr = pico8_fixed_add(self->yr, pico8_fixed_from_int(1));
    self->yr = new_yr;

    pico8_hy_lj(self);

    hk_list = pico8_cv("hk");
    if (hk_list != NULL) {
        it = pico8_list_all(hk_list);
        while (pico8_list_iter_next(&it, &e)) {
            if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
                pico8_value_t discard;
                pico8_mv((pico8_struct_header_t *)e.as.ref, "hk", 1, pico8_value_fixed(new_yr), pico8_value_ref(NULL),
                         &discard);
            }
        }
    }
}

int pico8_hy_lu(pico8_hy_t *self, pico8_fixed_t jt) {
    pico8_fixed_t nz;
    pico8_ob_table_t *overrides;

    assert_hy(self);
    nz = self != NULL ? self->nz : pico8_fixed_from_int(0);

    if (nz < jt) {
        overrides = pico8_ob_table_new();
        pico8_ob_table_set_index(overrides, 1, pico8_value_string("-no money-"));
        pico8_ob_table_set_index(overrides, 2, pico8_value_fixed(pico8_fixed_from_int(8)));
        pico8_iv_new(overrides);
        pico8_ob_table_free(overrides);
        return 0;
    }

    if (self != NULL) {
        self->nz = pico8_fixed_sub(nz, jt);
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

    assert_hy(self);
    if (self == NULL) {
        return;
    }

    /* Terminates because `bb()` reports only tier INCREASES and a tier is
     * capped at 4; a `jw != mi` test there would spin forever. */
    do {
        pico8_bd();
    } while (pico8_bb());

    pico8_bx(&self->po_own);
    pico8_ck(&self->fu_own);
    self->po = &self->po_own;
    self->fu = &self->fu_own;

    jo_list = pico8_cg("jo");
    if (jo_list != NULL) {
        it = pico8_list_all(jo_list);
        while (pico8_list_iter_next(&it, &e)) {
            if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
                pico8_struct_header_t *p = (pico8_struct_header_t *)e.as.ref;
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
    self->ge = ge;

    happiness = pico8_bf();
    self->happiness = happiness;
    self->has_happiness = 1;

    yr = self->yr;
    nz = self->nz;
    if (yr >= pico8_fixed_from_int(3426)) {
        self->hl = "final score";
    } else if (ge <= pico8_fixed_from_int(0) && nz < pico8_fixed_from_int(3)) {
        self->hl = "bankrupt";
    } else {
        self->hl = NULL;
    }
    self->has_hl = 1;

    {
        const pico8_list_t *hb_list = pico8_cv("hb");
        if (hb_list != NULL) {
            pico8_list_iter_t it2;
            pico8_value_t e2;
            it2 = pico8_list_all(hb_list);
            while (pico8_list_iter_next(&it2, &e2)) {
                if (e2.kind == PICO8_VALUE_REF && e2.as.ref != NULL) {
                    pico8_value_t discard;
                    pico8_mv((pico8_struct_header_t *)e2.as.ref, "hb", 0, pico8_value_ref(NULL), pico8_value_ref(NULL),
                             &discard);
                }
            }
        }
    }
}

void pico8_hy_kt(pico8_hy_t *self, const pico8_nb_entry_t *t) {
    pico8_fixed_t jk;
    pico8_fixed_t ew;
    int count = 0;

    assert_hy(self);
    if (self == NULL) {
        return;
    }

    jk = self->jk;
    self->jk = pico8_fixed_sub(jk, pico8_gi_nb(t));

    if (self->invented != NULL) {
        if (t != NULL && t->px != NULL) {
            pico8_list_add(self->invented, pico8_value_string(t->px));
        }
        count = self->invented->count;
    }

    ew = self->ew;
    if (ew < pico8_fixed_from_int(4)) {
        int threshold;
        if (pico8_int_seq_at(pico8_ek_table, PICO8_EK_COUNT, pico8_fixed_flr_int(ew), &threshold) &&
            pico8_fixed_from_int(count) >= pico8_fixed_from_int(threshold)) {
            self->ew = pico8_fixed_add(ew, pico8_fixed_from_int(1));
        }
    }

    pico8_hy_lj(self);
}
