#include "pico8_economy.h"

#include <stdlib.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_economy_formulas.h"
#include "pico8_economy_score.h"
#include "pico8_economy_tiers.h"
#include "pico8_entity.h"
#include "pico8_msgbox.h"
#include "pico8_stash.h"
#include "pico8_tech_gate.h"

/* See pico8_economy.h's own top comment for the full trace, including the
 * FM RECONCILIATION decision (Chunk 22's eu() builds the real `fm=hy()`
 * instance, not this module). */

static pico8_class_t *g_hy_class = NULL;

static pico8_fixed_t field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

/* function hy:pq()qj(self,ob([[
 * nz=100,ge=0,
 * jk=0,ew=1,
 * invented=o(),
 * lo=1,yr=3401,
 * ]]))end
 * (pico8/lua.lua:1198-1203.) `qj(self,ob([[...]]))` merges the freshly
 * parsed spec INTO self (the spec's own values win on a shared key,
 * pico8_qj's own established merge direction). `invented=o()` parses, via
 * ob()'s generic "identifier(...)" branch, as a nested EMPTY
 * pico8_ob_table_t (pi has no 'o' entry registered, so oo(NULL,...) falls
 * back to the freshly-parsed empty table itself, pico8_collect.h's own
 * pico8_oo comment) -- but every REAL consumer of self.invented in this
 * port's own scope (`hy:kt`'s add(), `oj()`'s gu(fm.invented,px),
 * `ky()`'s #fm.invented) treats it as a Lua LIST, never a keyed table.
 * Immediately replaced here with a fresh, empty pico8_list_t (REF-owned
 * by this instance) -- the same "spec-produced placeholder value swapped
 * for the actually-needed live representation right after construction"
 * decision already established for vector fields (Chunk 1) and `lq`'s own
 * `fs` field (Chunk 3), not a guess: every real call site is list-shaped.
 * The spec's own nested empty table (now unreachable) is freed explicitly
 * first, since pico8_ob_table_free never follows a nested REF (pico8_ob.h).
 * A NULL `self` is a safe no-op. */
static void pico8_hy_pq(pico8_instance_t *self) {
    pico8_ob_table_t *spec;
    pico8_value_t old_invented;
    pico8_list_t *invented;

    if (self == NULL) {
        return;
    }

    spec = pico8_ob(
        "nz=100,ge=0,"
        "jk=0,ew=1,"
        "invented=o(),"
        "lo=1,yr=3401,",
        NULL);
    pico8_qj(self->fields, spec);
    pico8_ob_table_free(spec);

    if (pico8_ob_table_get_name(self->fields, "invented", &old_invented) && old_invented.kind == PICO8_VALUE_REF &&
        old_invented.as.ref != NULL) {
        pico8_ob_table_free((pico8_ob_table_t *)old_invented.as.ref);
    }
    invented = malloc(sizeof *invented);
    pico8_list_init(invented);
    pico8_ob_table_set_name(self->fields, "invented", pico8_value_ref(invented));
}

void pico8_economy_init(void) {
    pico8_economy_shutdown();
    g_hy_class = pico8_class_declare(NULL, NULL);
    pico8_class_set_pq(g_hy_class, pico8_hy_pq);
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

pico8_instance_t *pico8_hy_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_hy_class, overrides);
}

/* function hy:nx(hx)local kx=ov("time compression",12,15)
 * self.lo+=hx
 * self:lj()if self.lo>kx then
 * self.lo-=kx
 * self:hk()end
 * end
 * (pico8/lua.lua:1204-1209.) See pico8_economy.h's own top comment on this
 * declaration for the full trace. */
void pico8_hy_nx(pico8_instance_t *self, pico8_fixed_t hx) {
    pico8_value_t kxv;
    pico8_fixed_t kx;
    pico8_fixed_t lo;

    if (self == NULL) {
        return;
    }

    kxv = pico8_ov(pico8_value_string("time compression"), pico8_value_fixed(pico8_fixed_from_int(12)),
                   pico8_value_fixed(pico8_fixed_from_int(15)));
    kx = (kxv.kind == PICO8_VALUE_FIXED) ? kxv.as.fixed : pico8_fixed_from_int(12);

    lo = pico8_fixed_add(field_fixed(self, "lo"), hx);
    pico8_ob_table_set_name(self->fields, "lo", pico8_value_fixed(lo));

    pico8_hy_lj(self);

    if (lo > kx) {
        pico8_ob_table_set_name(self->fields, "lo", pico8_value_fixed(pico8_fixed_sub(lo, kx)));
        pico8_hy_hk(self);
    }
}

/* function hy:hk()
 * self.nz+=self.ge
 * self.jk+=self.po[r.k]+1
 * self.yr+=1
 * self:lj()for e in all(cv.hk)do
 * e:hk(self.yr)end
 * end
 * (pico8/lua.lua:1210-1216.) See pico8_economy.h's own top comment on this
 * declaration for the full trace. */
void pico8_hy_hk(pico8_instance_t *self) {
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

    if (self == NULL) {
        return;
    }

    nz = field_fixed(self, "nz");
    ge = field_fixed(self, "ge");
    jk = field_fixed(self, "jk");

    if (pico8_instance_get_field(self, "po", &po_val) && po_val.kind == PICO8_VALUE_REF && po_val.as.ref != NULL &&
        r != NULL) {
        pico8_value_t k_val;
        pico8_value_t rk_val;
        if (pico8_ob_table_get_name(r, "k", &k_val)) {
            int found = 0;
            if (k_val.kind == PICO8_VALUE_FIXED) {
                found = pico8_ob_table_get_index((const pico8_ob_table_t *)po_val.as.ref,
                                                  (int)pico8_fixed_to_double(k_val.as.fixed), &rk_val);
            } else if (k_val.kind == PICO8_VALUE_STRING && k_val.as.string != NULL) {
                found = pico8_ob_table_get_name((const pico8_ob_table_t *)po_val.as.ref, k_val.as.string, &rk_val);
            }
            if (found && rk_val.kind == PICO8_VALUE_FIXED) {
                po_rk = rk_val.as.fixed;
            }
        }
    }

    pico8_ob_table_set_name(self->fields, "nz", pico8_value_fixed(pico8_fixed_add(nz, ge)));
    pico8_ob_table_set_name(
        self->fields, "jk", pico8_value_fixed(pico8_fixed_add(pico8_fixed_add(jk, po_rk), pico8_fixed_from_int(1))));
    new_yr = pico8_fixed_add(field_fixed(self, "yr"), pico8_fixed_from_int(1));
    pico8_ob_table_set_name(self->fields, "yr", pico8_value_fixed(new_yr));

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

/* function hy:lu(jt)if self.nz<jt then
 * iv({"-no money-",8})
 * return
 * end
 * self.nz-=jt
 * if jt~=0 then
 * iv({ne(-jt,10,"$")})end
 * return true
 * end
 * (pico8/lua.lua:1217-1225.) See pico8_economy.h's own top comment on this
 * declaration for the full trace. */
int pico8_hy_lu(pico8_instance_t *self, pico8_fixed_t jt) {
    pico8_fixed_t nz = field_fixed(self, "nz");
    pico8_ob_table_t *overrides;

    if (nz < jt) {
        overrides = pico8_ob_table_new();
        pico8_ob_table_set_index(overrides, 1, pico8_value_string("-no money-"));
        pico8_ob_table_set_index(overrides, 2, pico8_value_fixed(pico8_fixed_from_int(8)));
        pico8_iv_new(overrides);
        pico8_ob_table_free(overrides);
        return 0;
    }

    if (self != NULL) {
        pico8_ob_table_set_name(self->fields, "nz", pico8_value_fixed(pico8_fixed_sub(nz, jt)));
    }

    if (jt != pico8_fixed_from_int(0)) {
        int color;
        char *text = pico8_ne(pico8_fixed_neg(jt), 10, "$", &color);
        overrides = pico8_ob_table_new();
        pico8_ob_table_set_index(overrides, 1, pico8_value_string(text));
        pico8_ob_table_set_index(overrides, 2, pico8_value_fixed(pico8_fixed_from_int(color)));
        pico8_iv_new(overrides);
        pico8_ob_table_free(overrides);
        free(text);
    }

    return 1;
}

/* function hy:lj()repeat
 * bd()until not bb()self.po,self.fu=
 * bx(),ck()local dd=0
 * for p in all(cg.jo)do
 * p.kr=flr(db(p)-
 * da(p))
 * dd+=p.kr
 * end
 * self.ge=flr(dd*ov("skill implants",1,1.15))- cz()self.happiness=
 * bf()self.hl=
 * self.yr>=3426 and "final score"or self.ge<=0 and self.nz<3 and "bankrupt"for e in all(cv.hb)do
 * e:hb()end
 * end
 * (pico8/lua.lua:1226-1238.) See pico8_economy.h's own top comment on this
 * declaration for the full trace. */
void pico8_hy_lj(pico8_instance_t *self) {
    pico8_value_t old_po_val;
    pico8_value_t old_fu_val;
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

    if (self == NULL) {
        return;
    }

    do {
        pico8_bd();
    } while (pico8_bb());

    if (pico8_ob_table_get_name(self->fields, "po", &old_po_val) && old_po_val.kind == PICO8_VALUE_REF &&
        old_po_val.as.ref != NULL) {
        pico8_ob_table_free((pico8_ob_table_t *)old_po_val.as.ref);
    }
    if (pico8_ob_table_get_name(self->fields, "fu", &old_fu_val) && old_fu_val.kind == PICO8_VALUE_REF &&
        old_fu_val.as.ref != NULL) {
        pico8_ob_table_free((pico8_ob_table_t *)old_fu_val.as.ref);
    }
    new_po = pico8_bx();
    new_fu = pico8_ck();
    pico8_ob_table_set_name(self->fields, "po", pico8_value_ref(new_po));
    pico8_ob_table_set_name(self->fields, "fu", pico8_value_ref(new_fu));

    jo_list = pico8_cg("jo");
    if (jo_list != NULL) {
        it = pico8_list_all(jo_list);
        while (pico8_list_iter_next(&it, &e)) {
            if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
                pico8_instance_t *p = (pico8_instance_t *)e.as.ref;
                pico8_fixed_t kr = pico8_fixed_flr(pico8_fixed_sub(pico8_db(p), pico8_da(p)));
                pico8_ob_table_set_name(p->fields, "kr", pico8_value_fixed(kr));
                dd = pico8_fixed_add(dd, kr);
            }
        }
    }

    skill = pico8_ov(pico8_value_string("skill implants"), pico8_value_fixed(pico8_fixed_from_int(1)),
                      pico8_value_fixed(pico8_fixed_from_double(1.15)));
    skill_mult = (skill.kind == PICO8_VALUE_FIXED) ? skill.as.fixed : pico8_fixed_from_int(1);
    ge = pico8_fixed_sub(pico8_fixed_flr(pico8_fixed_mul(dd, skill_mult)), pico8_cz());
    pico8_ob_table_set_name(self->fields, "ge", pico8_value_fixed(ge));

    happiness = pico8_bf();
    pico8_ob_table_set_name(self->fields, "happiness", pico8_value_fixed(happiness));

    yr = field_fixed(self, "yr");
    nz = field_fixed(self, "nz");
    if (yr >= pico8_fixed_from_int(3426)) {
        hl = pico8_value_string("final score");
    } else if (ge <= pico8_fixed_from_int(0) && nz < pico8_fixed_from_int(3)) {
        hl = pico8_value_string("bankrupt");
    } else {
        hl = pico8_value_bool(0);
    }
    pico8_ob_table_set_name(self->fields, "hl", hl);

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

/* function hy:kt(t)
 * self.jk-=gi(t)
 * add(self.invented,t.px)
 * if self.ew<4
 * and #self.invented>=ek[self.ew]then
 * self.ew+=1
 * end
 * self:lj()
 * end
 * (pico8/lua.lua:1446-1454.) See pico8_economy.h's own top comment on this
 * declaration for the full trace. */
void pico8_hy_kt(pico8_instance_t *self, const pico8_ob_table_t *t) {
    pico8_value_t px_val;
    pico8_fixed_t jk;
    pico8_fixed_t ew;
    pico8_value_t invented_val;
    int count = 0;

    if (self == NULL) {
        return;
    }

    jk = field_fixed(self, "jk");
    pico8_ob_table_set_name(self->fields, "jk", pico8_value_fixed(pico8_fixed_sub(jk, pico8_gi(t))));

    if (pico8_instance_get_field(self, "invented", &invented_val) && invented_val.kind == PICO8_VALUE_REF &&
        invented_val.as.ref != NULL) {
        pico8_list_t *invented = (pico8_list_t *)invented_val.as.ref;
        if (t != NULL && pico8_ob_table_get_name(t, "px", &px_val)) {
            pico8_list_add(invented, px_val);
        }
        count = invented->count;
    }

    ew = field_fixed(self, "ew");
    if (ew < pico8_fixed_from_int(4)) {
        const pico8_ob_table_t *ek = pico8_ek();
        pico8_value_t threshold_val;
        if (ek != NULL && pico8_ob_table_get_index(ek, (int)pico8_fixed_to_double(ew), &threshold_val) &&
            threshold_val.kind == PICO8_VALUE_FIXED && pico8_fixed_from_int(count) >= threshold_val.as.fixed) {
            pico8_ob_table_set_name(self->fields, "ew", pico8_value_fixed(pico8_fixed_add(ew, pico8_fixed_from_int(1))));
        }
    }

    pico8_hy_lj(self);
}
