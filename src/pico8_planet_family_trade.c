#include "pico8_planet_family_trade.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_class.h"
#include "pico8_collect.h"
#include "pico8_entity.h"
#include "pico8_list.h"
#include "pico8_ob.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_planet_trade.h"
#include "pico8_trade.h"
#include "pico8_ref_audit.h"

/* See pico8_planet_family_trade.h's own top comment for the full Lua-
 * source trace of every class/method this file ports. Ordering
 * requirement: pico8_planet_family_init() (pico8_planet_family.c) MUST
 * run before pico8_planet_family_trade_init() below -- `nr` extends `qh`
 * and `mx` extends `hj`, both built by that other module. Local helpers
 * (`pico8_pft_truthy`, `positional_values`) are one more module-private
 * copy of the exact same patterns already independently duplicated
 * across `pico8_planet.c`/`pico8_planet_display.c`/`pico8_planet_
 * trade.c`/`pico8_planet_family.c`/etc. -- documented duplication, not a
 * new pattern (grep run before writing any of this file, per this
 * chunk's own required per-lot check). */

static pico8_class_t *g_nr_class = NULL;
static pico8_class_t *g_fr_class = NULL;
static pico8_class_t *g_mx_class = NULL;

static int pico8_pft_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Same "positional entries of a pico8_ob_table_t -> pico8_list_t" bridge
 * already independently duplicated by pico8_jo_positional_values
 * (pico8_planet_display.c) / the identically-named private helper in
 * pico8_planet_trade.c / pico8_research_display_positional_values / etc.
 * -- used here for mx:kf's own `gu(self.ht,i.qe)`. */
static pico8_list_t positional_values(const pico8_ob_table_t *table) {
    pico8_list_t list;
    int i;

    pico8_list_init(&list);
    if (table == NULL) {
        return list;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
            pico8_list_add(&list, table->entries[i].value);
        }
    }
    return list;
}

/* function nr:gv(s)
 * return s.qu~=self and #self.oe<3
 * end
 * (pico8/lua.lua:1109-1111.) See pico8_planet_family_trade.h's own
 * declaration comment for the full trace. */
pico8_value_t pico8_nr_gv(const pico8_instance_t *self, const pico8_instance_t *s) {
    pico8_value_t qu_val = pico8_value_ref(NULL);
    int qu_not_self;
    pico8_value_t oe_val;
    int oe_count = 0;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    if (s != NULL) {
        pico8_instance_get_field(s, "qu", &qu_val);
    }
    qu_not_self = !(qu_val.kind == PICO8_VALUE_REF && qu_val.as.ref == (const void *)self);

    if (pico8_instance_get_field(self, "oe", &oe_val) && oe_val.kind == PICO8_VALUE_REF && oe_val.as.ref != NULL) {
        /* REVERSE-CAST AUDIT (pico8_ref_audit.h): SAFE BY CONSTRUCTION.
         * Producer: pico8_jo_pq() (src/pico8_planet.c), which mallocs and
         * pico8_list_init()s `oe` and is the ONLY writer of "oe" in the
         * port; no class spec string declares an `oe` default. */
        oe_count = PICO8_REF_AS_LIST_CONST(oe_val.as.ref)->count;
    }

    return pico8_value_bool(qu_not_self && oe_count < 3);
}

static pico8_value_t pico8_nr_gv_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const pico8_instance_t *s = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        s = (const pico8_instance_t *)arg0.as.ref;
    }
    return pico8_nr_gv((const pico8_instance_t *)self, s);
}

/* function nr:ldown()
 * return self:gv({})and jc({qu=self})end
 * (pico8/lua.lua:1112-1113.) See pico8_planet_family_trade.h's own
 * declaration comment for the full trace. */
pico8_value_t pico8_nr_ldown(pico8_instance_t *self) {
    pico8_ob_table_t *empty_fields;
    pico8_instance_t empty;
    pico8_value_t gv_result;
    pico8_class_t *jc_class;
    pico8_ob_table_t *overrides;
    pico8_instance_t *made;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    empty_fields = pico8_ob_table_new();
    empty.tag = PICO8_TYPE_TAG_INSTANCE;
    empty.fields = empty_fields;
    empty.cls = NULL;
    gv_result = pico8_nr_gv(self, &empty);
    pico8_ob_table_free(empty_fields);

    if (!pico8_pft_truthy(gv_result)) {
        return gv_result;
    }

    jc_class = pico8_class_lookup_named("jc");
    if (jc_class == NULL) {
        return pico8_value_ref(NULL);
    }
    overrides = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides, "qu", pico8_value_ref(self));
    made = pico8_class_instantiate(jc_class, overrides);
    pico8_ob_table_free(overrides);
    return pico8_value_ref(made);
}

static pico8_value_t pico8_nr_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_nr_ldown((pico8_instance_t *)self);
}

/* function nr:hb()self.s=self.hz[#self.oe+1]
 * end
 * (pico8/lua.lua:1114-1115.) See pico8_planet_family_trade.h's own
 * declaration comment for the full trace. */
void pico8_nr_hb(pico8_instance_t *self) {
    pico8_value_t oe_val;
    int oe_count = 0;
    pico8_value_t hz_val;
    pico8_fixed_t s_val = pico8_fixed_from_int(0);

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "oe", &oe_val) && oe_val.kind == PICO8_VALUE_REF && oe_val.as.ref != NULL) {
        /* REVERSE-CAST AUDIT: SAFE BY CONSTRUCTION -- "oe", producer
         * pico8_jo_pq() (src/pico8_planet.c), sole writer. */
        oe_count = PICO8_REF_AS_LIST_CONST(oe_val.as.ref)->count;
    }

    if (pico8_instance_get_field(self, "hz", &hz_val) && hz_val.kind == PICO8_VALUE_REF && hz_val.as.ref != NULL) {
        pico8_value_t entry;
        if (pico8_ob_table_get_index((const pico8_ob_table_t *)hz_val.as.ref, oe_count + 1, &entry) &&
            entry.kind == PICO8_VALUE_FIXED) {
            s_val = entry.as.fixed;
        }
    }

    pico8_ob_table_set_name(self->fields, "s", pico8_value_fixed(s_val));
}

static pico8_value_t pico8_nr_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_nr_hb((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* function nr:cn(lw)local t=lw.t
 * for w in all(self.oe)do
 * if lw.mu>0 and not gu(t.rl,w)then
 * local gf=mw(lw)gf.t=t:kg(self,w)w:eb(self):cn(gf)lw.mu=gf.mu
 * end
 * end
 * end
 * (pico8/lua.lua:1116-1122.) See pico8_planet_family_trade.h's own
 * declaration comment for the full trace. */
void pico8_nr_cn(pico8_instance_t *self, pico8_jo_trade_proposal_t *lw) {
    pico8_instance_t *t;
    pico8_value_t oe_val;
    const pico8_list_t *oe = NULL;
    pico8_list_iter_t it;
    pico8_value_t w_val;

    if (self == NULL || lw == NULL) {
        return;
    }
    t = lw->t;

    if (pico8_instance_get_field(self, "oe", &oe_val) && oe_val.kind == PICO8_VALUE_REF && oe_val.as.ref != NULL) {
        /* REVERSE-CAST AUDIT: SAFE BY CONSTRUCTION -- "oe", producer
         * pico8_jo_pq() (src/pico8_planet.c), sole writer. */
        oe = PICO8_REF_AS_LIST_CONST(oe_val.as.ref);
    }

    it = pico8_list_all(oe);
    while (pico8_list_iter_next(&it, &w_val)) {
        pico8_instance_t *w;
        pico8_value_t t_rl_val;
        const pico8_list_t *t_rl = NULL;

        if (w_val.kind != PICO8_VALUE_REF || w_val.as.ref == NULL) {
            continue;
        }
        w = (pico8_instance_t *)w_val.as.ref;

        if (lw->mu <= pico8_fixed_from_int(0)) {
            continue;
        }

        if (t != NULL && pico8_instance_get_field(t, "rl", &t_rl_val) && t_rl_val.kind == PICO8_VALUE_REF &&
            t_rl_val.as.ref != NULL) {
            /* REVERSE-CAST AUDIT (pico8_ref_audit.h): SAFE BY
             * CONSTRUCTION. `t` is a `trade` instance; "rl" is written
             * only by the two `overrides` tables handed to
             * pico8_trade_new() -- pico8_trade_kg()
             * (src/pico8_trade.c:118) and pico8_jo_ib()
             * (src/pico8_planet_trade.c:354) -- both heap lists, and the
             * `trade` class is declared with no spec string at all. */
            t_rl = PICO8_REF_AS_LIST_CONST(t_rl_val.as.ref);
        }
        if (pico8_gu(t_rl, pico8_value_ref(w))) {
            continue;
        }

        {
            pico8_jo_trade_proposal_t gf = *lw;
            pico8_instance_t *extended = pico8_trade_kg(t, pico8_value_ref(self), pico8_value_ref(w));
            pico8_value_t eb_result;

            gf.t = extended;
            if (pico8_mv(w, "eb", 1, pico8_value_ref(self), pico8_value_ref(NULL), &eb_result) &&
                eb_result.kind == PICO8_VALUE_REF && eb_result.as.ref != NULL) {
                pico8_value_t discard;
                pico8_mv((pico8_instance_t *)eb_result.as.ref, "cn", 1, pico8_value_ref(&gf), pico8_value_ref(NULL),
                         &discard);
            }
            lw->mu = gf.mu;
        }
    }
}

static pico8_value_t pico8_nr_cn_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_jo_trade_proposal_t *lw = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        lw = (pico8_jo_trade_proposal_t *)arg0.as.ref;
    }
    pico8_nr_cn((pico8_instance_t *)self, lw);
    return pico8_value_ref(NULL);
}

/* function mx:kf(sw)local cp=sw:eb(self)for i in all(cp.jy)do
 * if i.gc==0
 * and gu(self.ht,i.qe)then
 * self.kw.qe,self.ht=
 * i.qe,{}
 * end
 * end
 * jo.kf(self,sw)end
 * (pico8/lua.lua:1049-1056.) See pico8_planet_family_trade.h's own
 * declaration comment for the full trace. */
void pico8_mx_kf(pico8_instance_t *self, pico8_instance_t *sw) {
    pico8_value_t eb_result;
    pico8_instance_t *cp = NULL;

    if (self != NULL && sw != NULL &&
        pico8_mv(sw, "eb", 1, pico8_value_ref(self), pico8_value_ref(NULL), &eb_result) &&
        eb_result.kind == PICO8_VALUE_REF && eb_result.as.ref != NULL) {
        cp = (pico8_instance_t *)eb_result.as.ref;
    }

    if (cp != NULL) {
        pico8_value_t jy_val;
        const pico8_list_t *jy = NULL;
        pico8_list_iter_t it;
        pico8_value_t i_val;

        if (pico8_instance_get_field(cp, "jy", &jy_val) && jy_val.kind == PICO8_VALUE_REF && jy_val.as.ref != NULL) {
            /* REVERSE-CAST AUDIT: SAFE BY CONSTRUCTION -- "jy",
             * producers pico8_jo_lz() (src/pico8_planet.c) and
             * pico8_jo_cn() (src/pico8_planet_trade.c). */
            jy = PICO8_REF_AS_LIST_CONST(jy_val.as.ref);
        }

        it = pico8_list_all(jy);
        while (pico8_list_iter_next(&it, &i_val)) {
            const pico8_ob_table_t *i_table;
            pico8_value_t gc_val;
            pico8_fixed_t gc = pico8_fixed_from_int(0);
            pico8_value_t qe_val;

            if (i_val.kind != PICO8_VALUE_REF || i_val.as.ref == NULL) {
                continue;
            }
            i_table = (const pico8_ob_table_t *)i_val.as.ref;

            if (pico8_ob_table_get_name(i_table, "gc", &gc_val) && gc_val.kind == PICO8_VALUE_FIXED) {
                gc = gc_val.as.fixed;
            }
            if (gc != pico8_fixed_from_int(0)) {
                continue;
            }

            if (pico8_ob_table_get_name(i_table, "qe", &qe_val)) {
                pico8_value_t ht_val;
                const pico8_ob_table_t *ht_table = NULL;

                if (pico8_instance_get_field(self, "ht", &ht_val) && ht_val.kind == PICO8_VALUE_REF &&
                    ht_val.as.ref != NULL) {
                    ht_table = (const pico8_ob_table_t *)ht_val.as.ref;
                }
                if (ht_table != NULL) {
                    pico8_list_t ht_list = positional_values(ht_table);
                    int accepted = pico8_gu(&ht_list, qe_val);
                    pico8_list_free(&ht_list);
                    if (accepted) {
                        pico8_value_t kw_val;
                        if (pico8_instance_get_field(self, "kw", &kw_val) && kw_val.kind == PICO8_VALUE_REF &&
                            kw_val.as.ref != NULL) {
                            pico8_ob_table_set_name((pico8_ob_table_t *)kw_val.as.ref, "qe", qe_val);
                        }
                        pico8_ob_table_set_name(self->fields, "ht", pico8_value_ref(pico8_ob_table_new()));
                    }
                }
            }
        }
    }

    pico8_jo_kf(self, sw);
}

static pico8_value_t pico8_mx_kf_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *sw = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        sw = (pico8_instance_t *)arg0.as.ref;
    }
    pico8_mx_kf((pico8_instance_t *)self, sw);
    return pico8_value_ref(NULL);
}

void pico8_planet_family_trade_init(void) {
    pico8_planet_family_trade_shutdown();
    pico8_pi_init();

    /* nr=qh:jr([[...]])function nr:gv(s)...end function nr:ldown()...end
     * function nr:hb()...end function nr:cn(lw)...end
     * (pico8/lua.lua:1101-1122). */
    g_nr_class = pico8_class_declare(pico8_qh_class(),
                                      "\n"
                                      "nu=o(\"nr\",\"np\",\"prober\"),\n"
                                      "od=10,fp=1,\n"
                                      "kq=b(-4,-4,5,5),\n"
                                      "s=170,\n"
                                      "hz=o(170,170,170,168),\n"
                                      "dw=0.01,\n"
                                      "rr=\"slipgates\",\n");
    pico8_ob_table_set_name(g_nr_class->defaults, "gv", pico8_value_function(pico8_nr_gv_dispatch));
    pico8_ob_table_set_name(g_nr_class->defaults, "ldown", pico8_value_function(pico8_nr_ldown_dispatch));
    pico8_ob_table_set_name(g_nr_class->defaults, "hb", pico8_value_function(pico8_nr_hb_dispatch));
    pico8_ob_table_set_name(g_nr_class->defaults, "cn", pico8_value_function(pico8_nr_cn_dispatch));

    /* fr=nr:jr([[...]]) -- no new bodies, no field reuse
     * (pico8/lua.lua:1123-1127). */
    g_fr_class = pico8_class_declare(g_nr_class,
                                      "\n"
                                      "mt=1,\n"
                                      "rr=\"infraspace\",\n"
                                      "od=25,\n"
                                      "s=78,hz=o(78,78,78,76),\n");

    /* mx=hj:jr([[...]])function mx:kf(sw)...end
     * (pico8/lua.lua:1036-1056). */
    g_mx_class = pico8_class_declare(pico8_hj_class(),
                                      "\n"
                                      "nu=o(\"jo\",\"np\",\"prober\"),\n"
                                      "rr=\"void synthesis\",\n"
                                      "hp=5,s=66,px=o(\"synthesizer\",c=13),\n"
                                      "od=40,\n"
                                      "ht=o(28,20,21,22),\n"
                                      "py=0.12,ki=9,\n"
                                      "pn=5,pk=1,\n"
                                      "fc=0.3,\n"
                                      "k_=o(1,1,5,5,3,3,3,5,5,1),\n"
                                      "du=5,\n"
                                      "qo=0.98,ro=-0.07,\n"
                                      "qg=0.07,q_=0.98,\n");
    pico8_ob_table_set_name(g_mx_class->defaults, "kf", pico8_value_function(pico8_mx_kf_dispatch));
}

void pico8_planet_family_trade_shutdown(void) {
    if (g_mx_class != NULL) {
        pico8_class_free(g_mx_class);
        g_mx_class = NULL;
    }
    if (g_fr_class != NULL) {
        pico8_class_free(g_fr_class);
        g_fr_class = NULL;
    }
    if (g_nr_class != NULL) {
        pico8_class_free(g_nr_class);
        g_nr_class = NULL;
    }
}

pico8_class_t *pico8_nr_class(void) {
    return g_nr_class;
}

pico8_instance_t *pico8_nr_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_nr_class, overrides);
}

pico8_class_t *pico8_fr_class(void) {
    return g_fr_class;
}

pico8_instance_t *pico8_fr_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_fr_class, overrides);
}

pico8_class_t *pico8_mx_class(void) {
    return g_mx_class;
}

pico8_instance_t *pico8_mx_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_mx_class, overrides);
}
